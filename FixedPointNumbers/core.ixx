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
template<std::unsigned_integral T0,class T1>requires std::unsigned_integral<T1>||std::same_as<T1,aint_dt<T0>>
constexpr T0 rndRecSqrt(T0 s, T0 r0,T0 d,const T1 &r1,std::float_round_style style){
	switch (style) {
	case std::round_toward_infinity:
		if constexpr(std::unsigned_integral<T1>)
			return s+(r0!=0||r1!=0);
		else
			return s+(r0!=0||r1.h!=0||r1.l!=0);
	case std::round_to_nearest:
		if constexpr(std::unsigned_integral<T1>)
			return s+(s+(r0<d)<=r1);
		else
			return s+(r1.h!=0||s+(r0<d)<=r1.l);
	default:
		return s;
	}
}
constexpr float karatsubaUnderestimateProb[]={0.117326,0.0621874,0.0320361,0.0162645,0.00819528};
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
constexpr Bone sqrt(Bone base, uint8_t exp, std::float_round_style style)
noexcept {
  if (exp == 0) {
	  auto[s,r]=sqrtRem(base);
  	return rndRecSqrt<decltype(s)>(s,0,2,r,style);
  }
  assert(exp <= NL<Bone>::digits);
  if constexpr (requires { typename rankOf<Bone>::two; }) {
  	auto [s,r]=sqrtRem(typename rankOf<Bone>::two(base)<<exp);
  	return rndRecSqrt<Bone>(s,0,1,r,style);
  } else {
    if (base == 0)
      return 0;
    using Th = rankOf<Bone>::half;
    uint8_t shift = std::countl_zero(base) + NL<Bone>::digits;
    shift -= (shift ^ exp) & 1;
    auto a = wideLS(base, shift);
    assert(a.l >> NL<Bone>::digits - 1 <= 1);
  	shift -= exp;
  	const auto [s0,r0]=sqrtRem(a.h);
  	const Bone q=(r0 << NL<Th>::digits - 1 | a.l >> NL<Th>::digits + 1)/s0;
  	Bone s1=aint_dt<Th>(s0,std::min<Bone>(q,NL<Th>::max())).merge()>>shift/2;
  	if (style==std::round_indeterminate)
  		return s1;
  	aint_dt<Bone> r1=wideLS(base,exp)-wideMul(s1,s1);
	if (__builtin_expect_with_probability(std::make_signed_t<Bone>(r1.h)<0,true,karatsubaUnderestimateProb[std::bit_width(sizeof(Bone))-1])) {
		r1-=s1--;
		r1-=s1;
	}
  	return rndRecSqrt(s1,Bone{0},Bone{1},r1,style);
  }
}
template <std::unsigned_integral Bone>
constexpr Bone recSqrt(Bone a, uint8_t exp, std::float_round_style style) {
	constexpr uint8_t D = NL<Bone>::digits;
	assOrAss(exp < D);
	assert(a > 0);
	if (uint16_t b = 2 * exp + 2; b < D && a == Bone{1} << b) // tie to even.
		return 0;
	uint16_t e3 = exp * 3;
	const Bone d = a / 4 + (a % 4 != 0);
	if (e3 <= D) {
		Bone q, r0;
		if constexpr (slowDiv<Bone>::v) {
			uint8_t shift = std::countl_zero(a);
			a <<= shift;
			std::tie(q, r0) = nDivNormRem(wideLS(Bone{1}, e3 + shift), a);
			r0 >>= shift;
		} else {
			using Tt = rankOf<Bone>::two;
			std::tie(q, r0) = nDivRem<Tt>(Tt{1} << e3, a);
		}
		auto [s, r1] = sqrtRem(q);
		return rndRecSqrt<Bone>(s, r0, d, r1, style);
	}
	if constexpr (!slowDiv<Bone>::v) {
		using Tt = rankOf<Bone>::two;
		Tt dividend = Tt{1} << e3 - D;
		auto [q1, r0] = nDivRem(dividend, a);
		dividend = Tt(r0) << D;
		Tt q = Tt(q1) << D;
		std::tie(q1, r0) = nDivRem(dividend, a);
		q |= q1;

		auto [s, r1] = sqrtRem(q);
		return rndRecSqrt(s, r0, d, r1, style);
	}
	uint8_t shift = std::countl_zero(a);
	a <<= shift;
	aint_dt<Bone> c = wideLS(Bone{1}, e3 - D + shift), q;
	std::tie(q.h, c.h) = nDivNormRem(c, a);
	c.l = 0;
	Bone r0;
	std::tie(q.l, r0) = nDivNormRem(c, a);
	r0 >>= shift;

	if (q.h == 0) {
		auto [s, r1] = sqrtRem(q.l);
		return rndRecSqrt<Bone>(s, r0, d, r1, style);
	}
	shift = std::countl_zero(q.h) & ~1;
	auto [s0, r2] = sqrtRem(q.h << shift | q.l >> D - shift);

	Bone e = (r2 << D / 2 - 1 | q.l >> D / 2 + 1) / s0;

	using Th = rankOf<Bone>::half;
	aint_dt<Bone> r1;
	Bone s;
	if (__builtin_expect_with_probability(e > NL<Th>::max(),true,1./(Bone{1}<<D/2))) {
		s = aint_dt<Th>(s0, NL<Th>::max()).merge() >> shift / 2;
		r1 = q - wideMul(s, s);
	} else {
		s = aint_dt<Th>(s0, e).merge() >> shift / 2;
		r1 = q - wideMul(s, s);
		if (__builtin_expect_with_probability(std::make_signed_t<Bone>(r1.h)<0,true,karatsubaUnderestimateProb[std::bit_width(sizeof(Bone))-1])) {
			r1 += s--;
			r1 += s;
		}
	}
	return rndRecSqrt(s, r0, d, r1, style);
}
} // namespace fpn::core