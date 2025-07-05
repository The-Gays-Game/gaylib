#pragma once

#include<cstdint>
#include<limits>
#include<tuple>
#include<bit>
#include<cstdlib>
#include<algorithm>
#include<version>
#ifdef checkArgs
#include<stdexcept>
#endif
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 202302L) || __cplusplus >= 202302L)
    #define CPP23
#endif

#ifndef __has_builtin
    #define __has_builtin(x) 0
#endif

#ifndef __has_constexpr_builtin
    #define __has_constexpr_builtin(x) 0
#endif

#if defined(CPP23)||defined(__GLIBCXX__)
#define FP_MANIP_CE
#define INT_ABS_CE
#endif

template <std::integral T>
constexpr
T condNeg
#if defined(__GNUG__)||defined(__clang__)
[[gnu::hot]]
#endif
(const T v, const bool doNeg)
  noexcept {
  //return (v^-T{a})+a;
  return doNeg ? -v : v;
}

template <std::integral>
struct rankOf {
};

template <>
struct rankOf<uint8_t> {
  using two = uint16_t;
};

template <>
struct rankOf<int8_t> {
  using two = int16_t;
};

template <>
struct rankOf<uint16_t> {
  using half = uint8_t;
  using two = uint32_t;
};

template <>
struct rankOf<int16_t> {
  using half = int8_t;
  using two = int32_t;
};

template <>
struct rankOf<uint32_t> {
  using half = uint16_t;
  using two = uint64_t;
};

template <>
struct rankOf<int32_t> {
  using half = int16_t;
  using two = int64_t;
};

template <>
struct rankOf<uint64_t> {
  using half = uint32_t;
#ifdef __SIZEOF_INT128__
  using two = unsigned __int128;
#endif
};

template <>
struct rankOf<int64_t> {
  using half = int32_t;
#ifdef __SIZEOF_INT128__
  using two = __int128;
#endif
};

#ifdef __SIZEOF_INT128__
template <>
struct rankOf<unsigned __int128> {
  using half = uint64_t;
};

template <>
struct rankOf<__int128> {
  using half = int64_t;
};
#endif

template <class T> using NL = std::numeric_limits<T>;

template <std::integral Ta>
struct aint_dt {
  using Tu = std::make_unsigned_t<Ta>;
  Ta h;
  Tu l;

  aint_dt() = default;

  constexpr
  aint_dt(const Ta h, const Tu l)
    noexcept: h(h), l(l) {
  }

  template <std::integral T> requires (std::is_signed_v<T> == std::is_signed_v<Ta> && (NL<Ta>::digits + NL<Tu>::digits >= NL<T>::digits))
  constexpr
  explicit aint_dt(T v)
    noexcept: l(v) {
    if (std::is_signed_v<T>)
      h = v >> std::min(NL<T>::digits, NL<Tu>::digits);
    else if (NL<Tu>::digits < NL<T>::digits)
      h = v >> NL<Tu>::digits;
    else
      h = 0;
  }

  constexpr
  auto merge
#if defined(__GNUG__)||defined(__clang__)
  [[gnu::artificial]]
#endif
  () const
    noexcept requires requires { typename rankOf<Ta>::two; } {
    constexpr uint8_t width = NL<Tu>::digits;
    return typename rankOf<Ta>::two(typename rankOf<Ta>::two(h) << width | l);
  }

  constexpr
  aint_dt &operator +=(const Tu b) //this function assumes Tu,Ta are the only things we know.
    noexcept(std::is_unsigned_v<Ta>) {
    Tu co;
#ifdef __clang__
    if constexpr (std::is_same_v<Tu, unsigned char>)
      l = __builtin_addcb(l, b, 0, &co);
    else if constexpr (std::is_same_v<Tu, unsigned short>)
      l = __builtin_addcs(l, b, 0, &co);
    else if constexpr (std::is_same_v<Tu, unsigned>)
      l = __builtin_addc(l, b, 0, &co);
    else if constexpr (std::is_same_v<Tu, unsigned long>)
      l = __builtin_addcl(l, b, 0, &co);
    else if constexpr (std::is_same_v<Tu, unsigned long long>)
      l = __builtin_addcll(l, b, 0, &co);
    else {
#elif defined(__GNUG__)
        if constexpr(std::is_same_v<Tu,unsigned int>)
            l=__builtin_addc(l,b,0,&co);
        else if constexpr(std::is_same_v<Tu,unsigned long int>)
            l=__builtin_addcl(l,b,0,&co);
        else if constexpr(std::is_same_v<Tu,unsigned long long int>)
            l=__builtin_addcll(l,b,0,&co);
        else{
#endif
      Tu s = l + b;
      co = (l & b | (l | b) & ~s) >> NL<Tu>::digits - 1;
      l = s;
    }
    h += co;
    return *this;
  }

  constexpr
  auto operator +(const Tu b) const
    noexcept(noexcept(aint_dt() += b)) {
    return aint_dt(*this) += b;
  }

  constexpr
  aint_dt &operator >>=(uint8_t by) {
    if (constexpr uint8_t ud = NL<Tu>::digits; by < ud) {
      l >>= by;
      const uint8_t a = ud - by;
      l |= h << (a - 1) << 1;
      h >>= by;
    } else {
      by -= ud;
      l = h >> by;
      if constexpr(std::is_signed_v<Ta>)
        h >>= NL<Ta>::digits;
      else
        h = 0;
    }
    return *this;
  }

  constexpr
  auto operator >>(const uint8_t by) const {
    return aint_dt(*this) >>= by;
  }

  constexpr
  Ta narrowRnd(const uint8_t to, const std::float_round_style s) const {
#ifdef checkArgs
    if (to == 0)
      throw std::domain_error("to can't be 0.");
#endif
    const Ta eucQ = (*this >> to).l;
    const Tu mod = l & (NL<Tu>::max() >> NL<Tu>::digits - to);
    switch (s) {
    case std::round_toward_infinity:
      return eucQ + (mod != 0);
    case std::round_to_nearest: {
      Ta special = eucQ & 1;
      Tu halfDivisor = Tu{1} << to - 1;
      return eucQ + (mod > halfDivisor - special);
    }
    case std::round_toward_zero:
      if (std::is_signed_v<Ta>)
        return eucQ + (mod != 0 && h < 0);
    default:
      return eucQ;
    }
  }
};

template <std::integral T>
constexpr
T rnd(const T v, const uint8_t to, const std::float_round_style s) {
#if __has_builtin(__builtin_expect_with_probability)
  if (__builtin_expect_with_probability(to == 0, true, 1. / NL<T>::digits))
#else
  if (to==0)
#endif
    return v;
  using Tu = std::make_unsigned_t<T>;
  const T eucQ = v >> to - 1 >> 1;
  const Tu mod = v & (NL<Tu>::max() >> NL<Tu>::digits - to);
  switch (s) {
  case std::round_toward_infinity:
    return eucQ + (mod != 0);
  case std::round_to_nearest: {
    T special = eucQ & 1;
    Tu halfDivisor = Tu{1} << to - 1;
    return eucQ + (mod > halfDivisor - special);
  }
  case std::round_toward_zero:
    if (std::is_signed_v<T>)
      return eucQ + (mod != 0 && v < 0);
  default:
    return eucQ;
  }
}

template <std::integral T>
constexpr
aint_dt<T> wideMul(const T a, const T b)
  noexcept(std::is_unsigned_v<T>) {
  using Tu = aint_dt<T>::Tu;
  using Th = rankOf<Tu>::half;
  using Tm = std::common_type_t<Tu, unsigned int>;
  constexpr T halfWidth = NL<Th>::digits;

  const T aL = Th(a), aH = a >> halfWidth;
  const T bL = Th(b), bH = b >> halfWidth;

  T d = aH * bL + (Tm(aL) * bL >> halfWidth);
  T c1 = Th(d);
  T c2 = d >> halfWidth;
  c1 += aL * bH;

  T eH = aH * bH + c2 + (c1 >> halfWidth);
  Tu eL = Tm(a) * b; //unfortunately if we have uint16_t*uint16_t can overflow int32
  return {eH, eL};
}

template <std::integral T>
constexpr
aint_dt<T> wideLS(const T a, const uint8_t/*assume by>0*/ by) {
  using Tu = aint_dt<T>::Tu;
#ifdef checkArgs
  if (by == 0)
    throw std::domain_error("can't shift by 0");
#elif __has_builtin(__builtin_assume)
    __builtin_assume(by>0);
#endif
#if __has_builtin(__builtin_expect_with_probability)
  constexpr double prob = []()consteval {
    constexpr uint16_t d = NL<T>::digits;
    uint16_t can = 0;
    for (uint8_t i = 1; i <= d; ++i)
      can += i + 1 + d - NL<Tu>::digits;
    return static_cast<double>(can) / (d * (d + 1));
  }();
  if (__builtin_expect_with_probability(by >= NL<Tu>::digits, true, prob))
#else
    if (by >= NL<Tu>::digits)
#endif

  {
    T h = a << by - NL<Tu>::digits;
    return {h, 0};
  } else {
    T h = a >> NL<Tu>::digits - by;
    Tu l = Tu(a) << by;
    return {h, l};
  }
}

template <std::unsigned_integral T>
constexpr
std::tuple<T, T> uNarrow211Div
#if defined(__GNUG__)||defined(__clang__)
[[gnu::hot]]
#endif
(const aint_dt<T> &dividend, const T/*assume normalized*/ divisor) {
#ifdef checkArgs
  if (std::countl_zero(divisor))
    throw std::domain_error("unnormalized divisor");
  if (dividend.h >= divisor)
    throw std::overflow_error("q can't fit in 1 part");
#elif __has_builtin(__builtin_assume)
    __builtin_assume(__builtin_clzg(divisor)==0&&dividend.h<divisor);
#endif
  using Th = rankOf<T>::half;
  constexpr uint8_t halfWidth = NL<Th>::digits;

  const aint_dt<Th> divisorSplit(divisor), dividendLSplit(dividend.l);
  aint_dt<Th> q;

  T qhat = dividend.h / divisorSplit.h, rhat = dividend.h % divisorSplit.h;
  T c1 = qhat * divisorSplit.l, c2 = rhat << halfWidth | dividendLSplit.h;
  if (c1 > c2) {
    --qhat;
    qhat -= c1 - c2 > divisor;
  }
  q.h = qhat;

  T r = (dividend.h << halfWidth | dividendLSplit.h) - q.h * divisor;

  qhat = r / divisorSplit.h, rhat = r % divisorSplit.h;
  c1 = qhat * divisorSplit.l, c2 = rhat << halfWidth | dividendLSplit.l;
  if (c1 > c2) {
    --qhat;
    qhat -= c1 - c2 > divisor;
  }
  q.l = qhat;

  r = (r << halfWidth | dividendLSplit.l) - q.l * divisor;

  return {q.merge(), r};
}

template <std::unsigned_integral Tdivisor, class Tdividend> requires std::same_as<Tdividend, Tdivisor> || std::same_as<Tdividend, aint_dt<Tdivisor> >
constexpr
Tdivisor divRnd(const Tdividend &dividend, const Tdivisor divisor, const std::float_round_style s) {
  Tdivisor q, r;
  if constexpr (std::is_same_v<Tdividend, Tdivisor>)
    q = dividend / divisor, r = dividend % divisor;
  else {
    auto [a,b] = uNarrow211Div(dividend, divisor);
    q = a, r = b;
  }
  switch (s) {
  case std::round_toward_infinity:
    return q + (r != 0);
  case std::round_to_nearest: {
    //tie to even
    Tdivisor special = q & (divisor & 1 ^ 1);
    return q + (r > divisor / 2 - special);
  }
  default:
    return q;
  }
}

template <std::integral Ta>
#ifdef INT_ABS_CE
constexpr
#endif
std::tuple<Ta, std::make_signed_t<Ta> > sRemQuo(const Ta dividend, const Ta divisor) {
  using Ts = std::make_signed_t<Ta>;
  Ta q = dividend / divisor, r = dividend % divisor;
  Ta special = q & (divisor & 1 ^ 1);
  Ta absR, absHalfDivisor;
  if constexpr(std::is_signed_v<Ta>) {
    if constexpr (requires { std::abs(r); }) {
      absR = std::abs(r), absHalfDivisor = std::abs(divisor / 2);
    } else {
      absR = condNeg(r, dividend < 0);
      absHalfDivisor = condNeg(divisor / 2, divisor < 0);
    }
  } else {
    absR = r, absHalfDivisor = divisor / 2;
  }
  if (absR > absHalfDivisor - special) {
    if (std::is_signed_v<Ta>) {
      bool qNeg=(divisor^dividend)<0;
      q+=condNeg<Ta>(1,qNeg);
      r-=condNeg<std::make_unsigned_t<Ta>>(divisor,qNeg);
    }else {
#ifdef checkArgs
      if (absR>=divisor&&absR-divisor>NL<Ts>::max())
        throw std::overflow_error("modified rem too big.");
      if (absR<divisor&&divisor-absR>Ta(-Ta(NL<Ts>::min())))
        throw std::underflow_error("modified rem too small.");;
#endif
      ++q;
      r-=divisor;
    }
  }
  return {q, r};
}

template <std::signed_integral Ts>
#ifdef INT_ABS_CE
#define S_DIVR_CE
constexpr
#endif
Ts divRnd(const Ts dividend, const Ts divisor, const std::float_round_style s) {
  Ts q = dividend / divisor, r = dividend % divisor;
  bool qNeg = (dividend ^ divisor) < 0;
  switch (s) {
  case std::round_toward_infinity:
    return q + (r != 0 && !qNeg); //r==0 when dividend^divisor==0.
  case std::round_toward_neg_infinity:
    return q - (r != 0 && qNeg);
  case std::round_to_nearest: {
    Ts special = q & (divisor & 1 ^ 1); // round up tie when odd quotient even divisor. round down tie when even quotient and divisor

    if constexpr (requires{std::abs(r);}) {
      return q + condNeg<Ts>(std::abs(r) > std::abs(divisor / 2) - special, qNeg);
    } else {
      Ts a = condNeg(r, dividend < 0), b = condNeg<Ts>(divisor / 2, divisor < 0);//sign of remainder only depends on dividend.
      return q + condNeg<Ts>(a > b - special, qNeg);
    }
    //B(abs(b))/2-special>=0;r and b/2 will never overflow after abs. 5%-8==5, 5/-8==0, but we need -1, so must use dividend^divisor.
  }
  default:
    return q;
  }
}

template <std::signed_integral Ts>
constexpr
Ts lsDivRnd(const Ts dividend, const Ts divisor, const uint8_t scale, const std::float_round_style s) {
  using Tu = typename aint_dt<Ts>::Tu;
  Tu absDivisor = condNeg(Tu(divisor), divisor < 0);
  uint8_t shift = std::countl_zero(absDivisor);
  absDivisor <<= shift;
  aint_dt<Tu> absDividend = wideLS(condNeg(Tu(dividend), dividend < 0), scale + shift);

  auto [absQ,absR] = uNarrow211Div(absDividend, absDivisor);

  bool qNeg = (dividend ^ divisor) < 0;
  switch (s) {
  case std::round_toward_infinity:
    absQ += absR != 0 && !qNeg;
    break;
  case std::round_toward_neg_infinity:
    absQ += absR != 0 && qNeg; //a^b==0 implies r==0
    break;
  case std::round_to_nearest: {
    Tu special = absQ & (divisor & 1 ^ 1);
    absQ += absR > absDivisor / 2 - special;
    break;
  }
  default: ;
  }
  return condNeg(absQ, qNeg);
}

template <std::unsigned_integral T>
static
constexpr
std::tuple<aint_dt<T>, T> u212Div(const aint_dt<T> &dividend, const T/*should be normalized for uNarrow211Div*/ divisor) {
#ifdef checkArgs
  if (divisor == 0)
    throw std::domain_error("0 divisor");
#endif
  aint_dt<T> q;
  q.h = dividend.h / divisor;
  T r0 = dividend.h % divisor;

  auto [a,r1] = uNarrow211Div(aint_dt<T>(r0, dividend.l), divisor);
  q.l = a;
  return {q, r1};
}
