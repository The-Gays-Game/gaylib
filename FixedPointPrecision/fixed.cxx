module;
#include "arithmetic.h"
#include<cstdint>
#include<limits>
#include<bit>
#include<utility>
#include<algorithm>
#include <cmath>
#include<stdexcept>
export module fixed;
#ifdef t_fixed_cxx
#undef t_fixed_cxx
#define cexport export
#else
#define cexport
#endif
template <std::floating_point F> using Tbits=std::conditional_t<sizeof(F) == 4, uint32_t,
#ifdef __SIZEOF_INT128__
                              std::conditional_t<sizeof(F) == 8, uint64_t, unsigned __int128>
#else
                              uint64_t
#endif
                              >;
template <std::floating_point F,std::integral B>
struct canFastCvt {// we try to convert directly on all x86 because compiler gives a bunch of instructions(on top of cvtss2si...) and branches.
  static constexpr bool value=//arm and risc-v both have hardware support for unsigned or fixed point conversion.
#if defined(__x86_64__) || defined(_M_X64)||defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
    sizeof(F) >= sizeof(uintptr_t) && sizeof(B) >= sizeof(uintptr_t) && std::unsigned_integral<B> && NL<F>::is_iec559 && (std::endian::native==std::endian::big||std::endian::native==std::endian::little)
#else
      false
#endif
  ;
};
cexport template <std::floating_point F, std::integral B>
#ifdef FP_MANIP_CE
#define TOF_CE
constexpr
#endif
    F
    toF(B v, uint8_t radix, std::float_round_style S) noexcept(noexcept(std::ldexp(v, int{}))) {
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
    if (int8_t more = sd - nl::digits; S != std::round_indeterminate && !subnorm &&
#if __has_builtin(__builtin_expect_with_probability)
                                       __builtin_expect_with_probability(more > int8_t{0}, true, (NL<B>::digits - nl::digits) / static_cast<double>(NL<B>::digits))
#else
                                       more > int8_t{0}
#endif
    ) {
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
cexport template <std::integral B, std::floating_point F>
#ifdef FP_MANIP_CE
#define FROMF_CE
constexpr
#endif
    B
    fromF(F v, uint8_t radix) noexcept(noexcept(std::ldexp(v, radix))) {
  if (canFastCvt<F, B>::value) {
    int exp;
    v = std::frexp(v, &exp);
    exp += radix;
    if (exp <= 0)
      return 0;

    using Tb = Tbits<F>;
    Tb cvt = Tb{1} << NL<Tb>::digits - 1 | std::bit_cast<Tb>(v) << NL<Tb>::digits - NL<F>::digits;
    cvt >>= NL<Tb>::digits - exp;
    return cvt;
  }
  return std::ldexp(v, radix);
}

template <std::integral T,uint8_t R>
struct batchFxMul {
  using Tm=decltype(widest<T>());
  static constexpr uint8_t batchSize=sizeof(Tm)/sizeof(T);
  Tm v;
  const std::float_round_style s;
  uint8_t c=1;

  constexpr
  batchFxMul& operator*=(T o)
  noexcept(std::is_unsigned_v<T>) {
    if constexpr(batchSize==1) {
      if (c!=0)
        v=wideMul(v,o).narrowRnd(R,s);
      else {
        v=o;
        c=1;
      }
    }else {
      if (c>=batchSize) {
        v=rnd(v,(batchSize-1)*R,s);
        c=1;
      }
      v*=o;
      ++c;
    }
    return *this;
  }
  constexpr
  batchFxMul& operator*=(batchFxMul o)
  noexcept(std::is_unsigned_v<T>) {
    if (batchSize==1) {
#if __has_builtin(__builtin_assume)
      __builtin_assume(c<=batchSize);
      __builtin_assume(o.c<=batchSize);
#endif
      if (c+o.c>batchSize) {
        v=wideMul(v,o.v).narrowRnd(R,s);
        return *this;
      }
    }else {
      if (c>=batchSize) {
        v=rnd(v,(batchSize-1)*R,s);
        c=1;
      }
      if (o.c+c>=batchSize) {
        o.v=rnd(o.v,(o.c-1)*R,s);
        o.c=1;
      }
    }
    v*=o.v;
    c+=o.c;
    return *this;
  }
  template <class X>
  constexpr
  operator X()const
  noexcept {
    return X::raw(rnd(v,(c-1)*R,s));
  }
};
/*
 *Design choices:
 *  what operators are explicit:
 *      cause 100% loss of information.
 *      can cause undefined behaviors.
 *      doesn't preserve original arithmetic meanings.
 *  what operations aren't supported?
 *      have ambiguous meanings.
 *      uses explicit operators.
 *      completely replaced by stl or builtin features.
 */
export
{
  template <class T, uint8_t R> concept testSize = NL<T>::digits >= R;

  template <std::signed_integral Bone, uint8_t Radix, std::float_round_style> requires testSize<Bone, Radix> class fx;

  //Radix is how many bits the decimal point is from the decimal point of integer (right of LSB).
  template <std::unsigned_integral Bone, uint8_t Radix, std::float_round_style Style = std::round_toward_zero> requires testSize<Bone, Radix> //radix==0 is equivalent to int.
  struct ufx {
    Bone repr;

    constexpr
    static ufx raw(Bone v)
      noexcept {
      ufx a;
      a.repr = v;
      return a;
    }

    ufx() = default;

    //can overflow to 0 when Radix==digits.
    constexpr
    explicit ufx(Bone v)
      noexcept: repr(Radix < NL<Bone>::digits ? v << Radix : 0) {
    }

    //conversion from float point is narrowing even causing undefined behaviors depending on exponent.
#ifdef FP_MANIP_CE
        constexpr
#endif
    explicit ufx(std::floating_point auto v)
      noexcept(noexcept(fromF<Bone>(v,Radix))): repr(fromF<Bone>(v,Radix)) {
    }

#define use_lrint NL<Bone>::digits<=NL<long>::digits
#define use_llrint NL<Bone>::digits<=NL<long long>::digits
    static ufx br(std::floating_point auto v)
    noexcept(((use_lrint&&noexcept(std::lrint(v)))||(use_llrint&&noexcept(std::llrint(v))))&&noexcept(std::ldexp(v, int{}))) requires(use_llrint){
      v=std::ldexp(v, Radix);
      Bone a=use_lrint?std::lrint(v):std::llrint(v);
      return raw(a);
    }

    //when both params are changed say ufx<B1,P1>x and ufx<B2,P2>y when sizeof(B1)>sizeof(B2) and P1>P2, then x.repr=y.repr<<(P1-P2) can have different value then x.repr=B1(y.repr)<<(P1-P2). This is ambiguous.

    template <std::unsigned_integral B1>
    constexpr
    explicit ufx(ufx<B1, Radix, Style> o)
      noexcept: repr(o.repr) {
    }

    template <uint8_t P1>
    constexpr
    explicit ufx(ufx<Bone, P1, Style> o)
      noexcept: repr(o.repr) {
      if constexpr(Radix > P1)
        repr <<= Radix - P1;
      else if constexpr(Radix < P1)
        repr = rnd(repr, P1 - Radix, Style);
    }

    auto operator<=>(const ufx &) const = default;

    constexpr
    explicit operator Bone() const
      noexcept {
      return rnd(repr, Radix, std::round_toward_zero);
    }

    //conversion to float point is always defined and never lose all precision.
    template <std::floating_point F>
#ifdef TOF_CE
        constexpr
#endif
    operator F() const
      noexcept(noexcept(toF<F>(repr, Radix, Style))) {
      return toF<F>(repr, Radix, Style);
    }
    constexpr
    ufx& operator++()
    noexcept {
      ++repr;
      return *this;
    }

    constexpr
    ufx operator++(int)const
    noexcept {
      return raw(repr++);
    }

    //same with fmod.
    constexpr
    ufx& operator %=(ufx divisor) {
      repr%=divisor.repr;//floor(floor(a*r/b)/r)=floor(a*r/b/r)=floor(a/b) same with trunc
      return *this;
    }
    constexpr
    auto operator %(ufx divisor)const {
      return ufx(*this)%=divisor;
    }
    constexpr
    ufx &operator/=(ufx divisor) {
      if (Radix == 0)
        repr = divRnd(repr, divisor.repr, Style);
      else if constexpr (requires { typename rankOf<Bone>::two; }) {
        typename rankOf<Bone>::two dividend = repr;
        repr = divRnd<typename rankOf<Bone>::two>(decltype(dividend)(dividend << Radix), divisor.repr, Style);
      } else {
        uint8_t shift = std::countl_zero(divisor.repr);
        divisor.repr <<= shift;
        aint_dt<Bone> dividend = wideLS(repr, shift + Radix);
        repr = divRnd(dividend, divisor.repr, Style);
        //rounding behavior depends on q, r, divisor. q doesn't change. r scales with divisor, so when odd q then inequality doesn't change. when even divisor, scaling by even number is still even.
      }
      return *this;
    }

    constexpr
    auto operator /(ufx divisor) const {
      return ufx(*this) /= divisor;
    }

    constexpr
    ufx &operator*=(ufx o)
      noexcept {
      using Tmul=std::common_type_t<uint32_t,Bone>;
      if (Radix == 0)
        repr *= Tmul(o.repr);
      else if constexpr (requires { typename rankOf<Bone>::two; }) {
        typename rankOf<Bone>::two a = Tmul(typename rankOf<Bone>::two(repr)) * o.repr;
        repr = aint_dt<Bone>(a).narrowRnd(Radix, Style);
      } else {
        aint_dt<Bone> a = wideMul(repr, o.repr);
        repr = a.narrowRnd(Radix, Style);
      }
      return *this;
    }

    constexpr
    auto operator*(ufx o) const
      noexcept {
      return ufx(*this) *= o;
    }

    constexpr
    ufx &operator+=(ufx o)
      noexcept {
      repr += o.repr;
      return *this;
    }

    constexpr
    auto operator+(ufx o) const
      noexcept {
      return ufx(*this) += o;
    }

    constexpr
    ufx &operator-=(ufx o)
      noexcept {
      repr -= o.repr;
      return *this;
    }

    constexpr
    auto operator-(ufx o) const
      noexcept {
      return ufx(*this) -= o;
    }

    constexpr
    auto remQuo(ufx divisor)const
    requires(Radix<NL<Bone>::digits)//at Radix==digits there's a loss of precision when converting to fx
    {
      using Ts=fx<std::make_signed_t<Bone>,Radix,Style>;
      auto [q,r]=sRemQuo(repr,divisor.repr);
      return std::tuple<Bone,Ts>{q,Ts::raw(r)};
    }

    constexpr
    ufx pow(std::conditional_t<Radix%NL<Bone>::digits==0,Bone,std::make_signed_t<Bone>> e)const
#ifndef checkArgs
    noexcept
#endif
    {
      if (Radix==NL<Bone>::digits) {
#ifdef checkArgs
        if (e==0)
          throw std::overflow_error("x^0==1 outside of range.");
#elif __has_builtin(__builtin_assume)
        __builtin_assume(e>0);
#endif
        return intPow(repr,e,batchFxMul<Bone,Radix>{1,Style,0});
      } else{
        bool negE=e<0;
#ifdef checkArgs
        if (repr==0&&negE)
          throw std::domain_error("0^(a<0) is undefined.");
#endif
        Bone absE=condNeg<Bone>(e,negE);
        ufx a;
        if constexpr (requires{typename rankOf<typename rankOf<Bone>::two>::two;})
          a=intPow(repr,absE,batchFxMul<Bone,Radix>{ufx(1).repr,Style});
        else
          a=intPow(*this,absE,ufx(1));
        return negE?ufx(1)/a:a;
      }
    }
  };

  template <std::signed_integral Bone, uint8_t Radix, std::float_round_style Style = std::round_toward_zero> requires testSize<Bone, Radix>
  class fx {
    using U = std::make_unsigned_t<Bone>;

  public:
    Bone repr; //c++20 defined bit shift on signed integers, right shift additionally comes with sign extending.
    fx() = default;

    constexpr
    static fx raw(Bone v)
      noexcept {
      fx a;
      a.repr = v;
      return a;
    }

    constexpr
    explicit fx(Bone v)
      noexcept: repr(v << Radix) {
    }

#ifdef FP_MANIP_CE
        constexpr
#endif
    explicit fx(std::floating_point auto v)
      noexcept(noexcept(fromF<Bone>(v,Radix))): repr(fromF<Bone>(v,Radix)) {
    }
    static fx br(std::floating_point auto v)
    noexcept(((use_lrint&&noexcept(std::lrint(v)))||(use_llrint&&noexcept(std::llrint(v))))&&noexcept(std::ldexp(v, int{}))) requires(use_llrint){
      v=std::ldexp(v, Radix);
      Bone a=use_lrint?std::lrint(v):std::llrint(v);
      return raw(a);
    }
#undef use_lrint
#undef use_llrint
    template <std::signed_integral B1>
    constexpr
    explicit fx(fx<B1, Radix, Style> o)
      noexcept: repr(o.repr) {
    }

    template <uint8_t P1>
    constexpr
    explicit fx(fx<Bone, P1, Style> o)
      noexcept: repr(o.repr) {
      if (Radix > P1)
        repr <<= Radix - P1;
      else if (Radix < P1)
        repr = rnd(repr, P1 - Radix, Style);
    }

    auto operator <=>(const fx &) const = default;

    /*intcmp functions in <utility> doesn't offer threeway. default threeway can't compare signed and unsigned.
     *comparing between signed and unsigned of same size is always meaningful arithmetically. can't put in ufx because
     *ufx can have more precision than fx.
     */
    constexpr
    std::strong_ordering operator<=>(ufx<U, Radix, Style> o) const
      noexcept {
      if (cmp_less(repr, o.repr))
        return std::strong_ordering::less;
      if (cmp_equal(repr, o.repr))
        return std::strong_ordering::equivalent;
      return std::strong_ordering::greater;
    }

    constexpr
    explicit operator Bone() const
      noexcept {
      return rnd(repr, Radix, std::round_toward_zero);
    }

    template <std::floating_point F>
#ifdef TOF_CE
        constexpr
#endif
    operator F() const
      noexcept(noexcept(toF<F>(repr, Radix, Style))) {
      return toF<F>(repr, Radix, Style);
    }

  constexpr
    fx&operator %=(fx divisor) {
      repr%=divisor.repr;
      return *this;
    }
    constexpr
    auto operator%(fx divisor)const {
      return fx(*this)%=divisor;
    }

#ifdef S_DIVR_CE
        constexpr
#endif
    fx &operator/=(fx divisor) {
      if (Radix == 0)
        repr = divRnd(repr, divisor.repr, Style);
      else if constexpr (requires { typename rankOf<Bone>::two; }) {
        typename rankOf<Bone>::two dividend = repr;
        repr = divRnd<decltype(dividend)>(dividend << Radix, divisor.repr, Style);
      } else
        repr = lsDivRnd(repr, divisor.repr, Radix, Style);
      return *this;
    }
#ifdef S_DIVR_CE
        constexpr
#endif
    auto operator/(fx divisor) const {
      return fx(*this) /= divisor;
    }

    constexpr
    fx &operator*=(fx o) {
      if (Radix == 0)
        repr *= o.repr;
      else if constexpr (requires { typename rankOf<Bone>::two; }) {
        typename rankOf<Bone>::two a = typename rankOf<Bone>::two(repr) * o.repr;
        repr = aint_dt<Bone>(a).narrowRnd(Radix, Style);
      } else {
        aint_dt<Bone> a = wideMul(repr, o.repr);
        repr = a.narrowRnd(Radix, Style);
      }
      return *this;
    }

    constexpr
    auto operator*(fx o) const {
      return fx(*this) *= o;
    }

    constexpr
    fx &operator+=(fx o) {
      repr += o.repr;
      return *this;
    }

    constexpr
    auto operator+(fx o) const {
      return fx(*this) += o;
    }

    constexpr
    fx &operator-=(fx o) {
      repr -= o.repr;
      return *this;
    }

    constexpr
    auto operator-(fx o) const {
      return fx(*this) -= o;
    }

    constexpr
    fx operator-()const {
      return raw(-repr);
    }

    constexpr
    fx abs()const {
      if constexpr(requires{std::abs(repr);})
        return raw(std::abs(repr));
      else
        return raw(condNeg(repr,repr<0));
    }

    constexpr
    std::tuple<Bone,fx> remQuo(fx divisor)const {
      auto [q,r]=sRemQuo(repr,divisor.repr);
      return {q,raw(r)};
    }
  };
}