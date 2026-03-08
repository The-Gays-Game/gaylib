module;
#include "defs.h"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>

export module fpn:core;
import :ari;
// using namespace fpn::ari;
template <std::floating_point F>
using Tbits = std::conditional_t<sizeof(F) == 4, uint32_t,
#if defined(__SIZEOF_INT128__)
                                 std::conditional_t<sizeof(F) == 8, uint64_t, unsigned __int128>
#else
                                 uint64_t
#endif
                                 >;
template <std::floating_point F, std::integral B>
struct canFastCvt {             // we try to convert directly on all x86 because compiler gives a bunch of instructions(on top of cvtss2si...) and branches.
  static constexpr bool value = // arm and risc-v both have hardware support for unsigned or fixed point conversion.
#ifdef ARCH_x86
      ((NL<B>::digits > NL<F>::digits && sizeof(F) > sizeof(float)) || (sizeof(B) > sizeof(uintptr_t) && sizeof(F) > sizeof(uintptr_t))) && NL<F>::is_iec559 && (std::endian::native == std::endian::big || std::endian::native == std::endian::little)
#else
      false
#endif
      ;
};

export namespace fpn::core {
template <std::floating_point F, std::integral B>
CMATH_CE23 F toF(B v, uint8_t radix, std::float_round_style S)
noexcept(noexcept(std::ldexp(v, int{}))) {
  using nl = NL<F>;
  if constexpr (NL<B>::digits > nl::digits) {
    uint8_t sd;
    if constexpr (std::is_unsigned_v<B>)
      sd = NL<B>::digits - std::countl_zero(v);
    else {
      auto av = condNeg<std::make_unsigned_t<B>>(v, v < 0);
      sd = NL<decltype(av)>::digits - std::countl_zero(av);
    }

    bool subnorm = nl::has_denorm == std::denorm_present && int8_t(sd - radix) <= nl::min_exponent - 1;
    if (int8_t more = sd - nl::digits; S != std::round_indeterminate && !subnorm && __builtin_expect_with_probability(more > int8_t{0}, true, (NL<B>::digits - nl::digits) / static_cast<float>(NL<B>::digits))) {
      // with radix<=128, sd<=128, then sd<=2 needs no rounding. we provide rounding because int to float point conversion rounding type is unspecified.
      v = rnd(v, more, S); // rnd makes sure v only has `nl::digits` digits and no trailing 0.
      radix -= more;

      constexpr uint8_t explicitDigs = nl::digits - 1;
      using Tb = Tbits<F>;
      F cvt = canFastCvt<F, B>::value ? std::bit_cast<F>(Tb(Tb(nl::max_exponent - 1 + explicitDigs) << explicitDigs | v & ~(Tb{1} << explicitDigs))) /*no denorm or 0 here*/ : v;

      return std::ldexp(cvt, -int8_t(radix));
    }
  }
  return std::ldexp(v, -int16_t(radix)); // if F is bigger than B, then B isn't largest, so efficient conversion by compiler is possible.
}
template <std::integral B, std::floating_point F>
CMATH_CE23 B fromF(F v, uint8_t radix)
noexcept(noexcept(std::ldexp(v, radix))) {
  if (canFastCvt<F, B>::value) {
    int exp;
    v = std::frexp(v, &exp);
    exp += radix;
    constexpr int minFrExp(NL<F>::min_exponent - NL<F>::digits + 2);
    if (__builtin_expect_with_probability(exp <= 0, true, (NL<B>::digits / 2.f - minFrExp + 1) / (NL<F>::max_exponent - minFrExp + 1)))
      return 0;

    using Tb = Tbits<F>;
    Tb cvt = Tb{1} << NL<Tb>::digits - 1 | std::bit_cast<Tb>(v) << NL<Tb>::digits - NL<F>::digits;
    cvt >>= NL<Tb>::digits - exp;
    return cvt;
  }
  return std::ldexp(v, radix);
}

template <std::integral Bone>
constexpr Bone fromB(Bone a, uint8_t radix) {
  assert(radix <= NL<std::make_unsigned_t<Bone>>::digits);
  if (std::is_unsigned_v<Bone>)
    return radix < NL<Bone>::digits ? a << radix : 0;
  return a << radix;
}
#define use_lrint NL<Bone>::digits <= NL<long>::digits
#define use_llrint NL<Bone>::digits <= NL<long long>::digits
template <std::integral Bone> requires(use_llrint)
Bone br(std::floating_point auto a, uint8_t radix)
noexcept(((use_lrint && noexcept(std::lrint(a))) || (use_llrint && noexcept(std::llrint(a)))) && noexcept(std::ldexp(a, int{})))
 {
  a = std::ldexp(a, radix);
  return use_lrint ? std::lrint(a) : std::llrint(a);
}
#undef use_lrint
#undef use_llrint

template <std::integral Bone>
constexpr Bone chngRdx(Bone a, uint8_t rFrom, uint8_t rTo, std::float_round_style style) {
  if (rTo > rFrom)
    return a << rTo - rFrom;
  if (rTo < rFrom)
    return rnd(a, rFrom - rTo, style);
  return a;
}

template <std::signed_integral Bone>
CMATH_CE23 Bone div(Bone dividend, Bone divisor, uint8_t radix, std::float_round_style style) {
  assert(radix <= NL<Bone>::digits);
#ifdef checkArgs
  if (divisor == 0)
    throw std::domain_error("zero divisor.");
#endif
  return lsDivRnd(dividend, divisor, radix, style);
}
template <std::unsigned_integral Bone>
constexpr Bone div(Bone dividend, Bone divisor, uint8_t radix, std::float_round_style style) {
  assert(radix <= NL<Bone>::digits);
#ifdef checkArgs
  if (divisor == 0)
    throw std::domain_error("zero divisor.");
#endif
  return lsDivRnd(dividend, divisor, radix, style);
}

template <std::integral Bone>
constexpr Bone mul(Bone a, Bone b, uint8_t radix, std::float_round_style style)
#ifdef NDEBUG
    noexcept(std::is_unsigned_v<Bone>)
#endif
{
  if (radix == 0)
    return (std::is_unsigned_v<Bone> ? std::common_type_t<Bone, unsigned int>(a) : a) * b;
  assert(radix <= NL<std::make_unsigned_t<Bone>>::digits);
  if constexpr (requires { typename rankOf<Bone>::two; }) {
    using Tt = rankOf<Bone>::two;
    return rnd<Tt>(Tt(a) * b, radix, style);
  } else
    return wideMul(a, b).narrowRnd(radix, style);
}
template <std::unsigned_integral Bone>
constexpr Bone sqrt(Bone base, uint8_t radix, std::float_round_style style)
noexcept {
  if (radix == 0)
    return uRoot2(base, style);
  assert(radix <= NL<Bone>::digits);
  if constexpr (requires { typename rankOf<Bone>::two; }) {
    using Tt = rankOf<Bone>::two;
    return uRoot2<Tt>(Tt(base) << radix, style);
  } else {
    if (base == 0)
      return 0;
    using Th = rankOf<Bone>::half;
    uint8_t shift = std::countl_zero(base) + NL<Bone>::digits;
    shift -= (shift ^ radix) & 1;
    auto a = wideLS(base, shift);
    assert(a.l >> NL<Bone>::digits - 1 <= 1);
    const Bone s0 = Th(uRoot2(a.h, std::round_toward_zero));
    shift -= radix;
    const Bone r0 = a.h - s0 * s0;
    Bone b = r0 << NL<Th>::digits - 1 | a.l >> NL<Th>::digits + 1;
    Bone q = b / s0, u = b % s0;
    assOrAss(q <= Bone(NL<Th>::max()) + 1);
    if (__builtin_expect_with_probability(q > NL<Th>::max(), true, 1. / (Bone{1} << NL<Th>::digits))) {
      --q;
      u += s0;
    }
    Bone s1 = aint_dt<Th>(s0, q).merge();
    aint_dt<Bone> r1;
    if (style != std::round_indeterminate) {
      using Ts = std::make_signed_t<Bone>;
      r1 = wideLS(u, 1 + NL<Th>::digits) - q * q;
      if (__builtin_expect_with_probability(Ts(r1.h) < 0, true, 0.2469)) {
        r1 += s1--;
        r1 += s1;
      }
    }
    s1 >>= shift / 2;
    if (style == std::round_toward_infinity)
      s1 += r1.h != 0 || r1.l != 0;
    else if (style == std::round_to_nearest) { // there doesn't seem to be a way to determine rounding without recomputing r after denormalizing.
      r1 = wideLS(base, radix) - s1;
      a = wideMul(s1, s1);
      s1 += r1.h > a.h || r1.l > a.l;
    }
    return s1;
  }
}
/*template<std::unsigned_integral Bone>
constexpr
Bone rSqrt(Bone a,uint8_t radix,std::float_round_style style)
noexcept {
    assOrAss(radix < NL<Bone>::digits);
    assert(a>0);
    if (style==std::round_indeterminate) {
      uint8_t b=NL<Bone>::digits-std::countl_zero(a);
      b+=std::max(b-radix,0)&1;
      if (b<NL<Bone>::digits) {
        radix+=b;
        Bone c=sqrt(div(Bone{1},a,radix,std::round_to_nearest),radix,std::round_to_nearest);
        return c>>b/2;
      }
      if constexpr(requires{typename rankOf<Bone>::two;}) {
        using Tt=rankOf<Bone>::two;;
        radix+=b;
        Tt c=sqrt(div<Tt>(1,a,radix,std::round_to_nearest),radix,std::round_to_nearest);
        return c>>b/2;
      }
    }
    uint16_t b=radix*3;
    if (b<=)
  }*/
} // namespace fpn::core