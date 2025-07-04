module;
#include "arithmetic.h"
#include<cstdint>
#include<limits>
#include<bit>
#include<utility>
#include<algorithm>
#include <cmath>
export module fixed;
#ifdef t_fixed_cxx
#undef t_fixed_cxx
#define cexport export
#else
#define cexport
#endif
cexport template <std::floating_point F,std::integral B>
#ifdef FP_MANIP_CE
#define TOF_CE
constexpr
#endif
F toF(B v, uint8_t radix, std::float_round_style S)
  noexcept(noexcept(std::ldexp(v, int{}))) {
  using nl = NL<F>;
  constexpr uint8_t d = NL<B>::digits;
  if constexpr (d > nl::digits) {
    uint8_t sd;
    if constexpr (std::is_unsigned_v<B>)
      sd = d - std::countl_zero(v);
    else {
      auto av = condNeg<std::make_unsigned_t<B> >(v, v < 0);
      sd = NL<decltype(av)>::digits - std::countl_zero(av);
    }

    bool subnorm = nl::has_denorm == std::denorm_present && int8_t(sd - radix) <= nl::min_exponent - 1;
    if (int8_t more = sd - nl::digits; S != std::round_indeterminate && !subnorm &&
#if defined(__GNUG__)||__has_builtin(__builtin_expect_with_probability)
                                       __builtin_expect_with_probability(more > 0, true, (d - nl::digits) / static_cast<double>(d))
#else
        more > 0
#endif
    ) {
      //with radix<=128, sd<=128, then sd<=2 needs no rounding.
      v = rnd(v, more, S);
      radix -= more;
      return std::ldexp(v, -int8_t(radix));
    }
  }
  return std::ldexp(v, -int16_t(radix));
}

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

#define fxDecl  template <std::signed_integral Bone, uint8_t Radix, std::float_round_style Style = std::round_toward_zero> requires testSize<Bone, Radix> class fx
  fxDecl;

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
      noexcept(noexcept(std::ldexp(v, int{}))): repr(std::ldexp(v, Radix)) {
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
      if (Radix > P1)
        repr <<= Radix - P1;
      else if (Radix < P1)
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
        repr = divRnd<typename rankOf<Bone>::two>(dividend << Radix, divisor.repr, Style);
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
      return std::tuple<ufx,Ts>{raw(q),Ts::raw(r)};
    }
  };

  fxDecl {
#undef fxDecl
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
      noexcept(noexcept(std::ldexp(v, int{}))): repr(std::ldexp(v, Radix)) {
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
     *fx::U is already defined, for ufx we need to redefine make_signed_t<Bone>. we'd also need a forward declaration.
     *comparing between signed and unsigned of same size is always meaningful arithmetically.
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
    std::tuple<fx,fx> remQuo(fx divisor)const {
      auto [q,r]=sRemQuo(repr,divisor.repr);
      return {raw(q),raw(r)};
    }
  };
}
