module;
#include "defs.h"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <tuple>
#include<array>
export module fpn:ari;
// namespace fpn::ari {
template <std::integral T>
#if defined(__GNUG__) || defined(__clang__)
    [[gnu::hot]]
#endif
constexpr T condNeg (const T v, const bool doNeg)
noexcept {
  return doNeg ? -v : v; // return (v^-T{a})+a;
}
dbgHelperExport {
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
#if defined(__SIZEOF_INT128__) && !defined(dbgNo128)
    using two = unsigned __int128;
#endif
  };

  template <>
  struct rankOf<int64_t> {
    using half = int32_t;
#if defined(__SIZEOF_INT128__) && !defined(dbgNo128)
    using two = __int128;
#endif
  };

#if defined(__SIZEOF_INT128__)
  template <>
  struct rankOf<unsigned __int128> {
    using half = uint64_t;
  };

  template <>
  struct rankOf<__int128> {
    using half = int64_t;
  };
#endif

  template <class T>
  using NL = std::numeric_limits<T>;
}

/*template <std::integral T> using wide=std::conditional_t<std::unsigned_integral<T>,
#ifdef __SIZEOF_INT128__
  unsigned __int128,__int128
#else
uint64_t,int64_t
#endif
>;
}*/
template <std::integral Ta>
struct aint_dt {
  using Tu = std::make_unsigned_t<Ta>;
  Ta h;
  Tu l;

  aint_dt() = default;

  constexpr aint_dt(const Ta h, const Tu l)
  noexcept
  : h(h), l(l) {
  }

  template <std::integral T>
  requires(std::is_signed_v<T> == std::is_signed_v<Ta> && (NL<Ta>::digits + NL<Tu>::digits >= NL<T>::digits))
#if defined(__GNUG__) || defined(__clang__)
  [[using gnu:artificial, hot]]
#endif
  constexpr explicit aint_dt(T v)
  noexcept
      : l(v) {
    if (std::is_signed_v<T>)
      h = v >> std::min(NL<T>::digits, NL<Tu>::digits);
    else if (NL<Tu>::digits < NL<T>::digits)
      h = v >> NL<Tu>::digits;
    else
      h = 0;
  }


#if defined(__GNUG__) || defined(__clang__)
      [[gnu::artificial, gnu::hot]]
#endif
    constexpr auto merge() const
      noexcept
      requires requires { typename rankOf<Ta>::two; }
  {
    constexpr uint8_t width = NL<Tu>::digits;
    return typename rankOf<Ta>::two(typename rankOf<Ta>::two(h) << width | l);
  }
  constexpr aint_dt &operator-=(const Tu b) // this function assumes Tu,Ta are the only things we know.
      noexcept(std::is_unsigned_v<Ta>) {
    Tu co;
#ifdef __clang__
    if constexpr (std::is_same_v<Tu, unsigned char>)
      l = __builtin_subcb(l, b, 0, &co);
    else if constexpr (std::is_same_v<Tu, unsigned short>)
      l = __builtin_subcs(l, b, 0, &co);
    else if constexpr (std::is_same_v<Tu, unsigned>)
      l = __builtin_subc(l, b, 0, &co);
    else if constexpr (std::is_same_v<Tu, unsigned long>)
      l = __builtin_subcl(l, b, 0, &co);
  #ifndef dbgNo128
    else if constexpr (std::is_same_v<Tu, unsigned long long>)
      l = __builtin_subcll(l, b, 0, &co);
  #endif
    else
#elif defined(__GNUG__)
    if constexpr (std::is_same_v<Tu, unsigned int>)
      l = __builtin_subc(l, b, 0, &co);
    else if constexpr (std::is_same_v<Tu, unsigned long int>)
      l = __builtin_subcl(l, b, 0, &co);
  #ifndef dbgNo128
    else if constexpr (std::is_same_v<Tu, unsigned long long int>)
      l = __builtin_subcll(l, b, 0, &co);
  #endif
    else
#endif
    {
      Tu d = l - b;
      co = (~l & b | ~(l ^ b) & d) >> NL<Tu>::digits - 1;
      l = d;
    }
    h -= co;
    return *this;
  }
	constexpr aint_dt&operator-=(const aint_dt&b)
	noexcept(std::is_unsigned_v<Ta>) {
	  *this-=b.l;
  		h-=b.h;
  		return *this;
  }
  constexpr aint_dt &operator+=(const Tu b) // this function assumes Tu,Ta are the only things we know.
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
    else
#elif defined(__GNUG__)
    if constexpr (std::is_same_v<Tu, unsigned int>)
      l = __builtin_addc(l, b, 0, &co);
    else if constexpr (std::is_same_v<Tu, unsigned long int>)
      l = __builtin_addcl(l, b, 0, &co);
    else if constexpr (std::is_same_v<Tu, unsigned long long int>)
      l = __builtin_addcll(l, b, 0, &co);
    else
#endif
    {
      Tu s = l + b;
      co = (l & b | (l | b) & ~s) >> NL<Tu>::digits - 1;
      l = s;
    }
    h += co;
    return *this;
  }

  constexpr auto operator+(const Tu b) const
      noexcept(noexcept(aint_dt() += b)) {
    return aint_dt(*this) += b;
  }
  constexpr aint_dt operator-(const Tu b) const
      noexcept(noexcept(aint_dt() -= b)) {
    return aint_dt(*this) -= b;
  }
	constexpr aint_dt operator-(const aint_dt&b) const
	 noexcept(noexcept(aint_dt() -= b)) {
  	return aint_dt(*this) -= b;
  }
  constexpr aint_dt &operator>>=(uint8_t by) {
    if (constexpr uint8_t ud = NL<Tu>::digits; by < ud) {
      l >>= by;
      l |= h << (ud - by - 1) << 1;
      h >>= by;
    } else {
      by -= ud;
      assert(by < ud);
      l = h >> by;
      if constexpr (std::is_signed_v<Ta>) // silence a warning about potential shift undefined behavior from clang.
        h >>= NL<Ta>::digits;
      else
        h = 0;
    }
    return *this;
  }

  constexpr auto operator>>(const uint8_t by) const {
    return aint_dt(*this) >>= by;
  }

  constexpr Ta narrowRnd(const uint8_t to, const std::float_round_style s) const {
    assert(to != 0);
    const Ta eucQ = (*this >> to).l;
    const Tu mod = l & NL<Tu>::max() >> NL<Tu>::digits - to;
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
constexpr T rnd(const T v, const uint8_t to, const std::float_round_style s) {
  if (__builtin_expect_with_probability(to == 0, true, 1.f / NL<T>::digits))
    return v;
  using Tu = std::make_unsigned_t<T>;
  assert(to <= NL<Tu>::digits);
  const T eucQ = v >> to - 1 >> 1;
  const Tu mod = v & NL<Tu>::max() >> NL<Tu>::digits - to;
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
constexpr aint_dt<T> wideMul(const T a, const T b) noexcept(std::is_unsigned_v<T>) {
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
  Tu eL = Tm(a) * b; // unfortunately if we have uint16_t*uint16_t can overflow int32
  return {eH, eL};
}

template <std::integral T>
constexpr aint_dt<T> wideLS(const T a, const uint8_t /*assume by>0*/ by) {
  using Tu = aint_dt<T>::Tu;
  assOrAss(by > 0);
  if (__builtin_expect_with_probability(by >= NL<Tu>::digits, true, NL<Tu>::digits / (NL<Tu>::digits * 2.f - 1))) {
    T h = a << by - NL<Tu>::digits;
    return {h, 0};
  }
  T h = a >> NL<Tu>::digits - by;
  Tu l = Tu(a) << by;
  return {h, l};
}
template <std::integral T>
struct slowDiv {
	static constexpr bool v=sizeof(T) >
#if ARCH_x86 == 64 || __riscv_xlen == 128
																							 8
#else
																							 4
#endif
;
};
template <std::unsigned_integral T>
constexpr std::tuple<T, T> nDivNormRem(const aint_dt<T> &dividend, const T /*assume normalized*/ divisor) {
  assOrAss(std::countl_zero(divisor) == 0);
  assOrAss(dividend.h < divisor);
  using Th = rankOf<T>::half;
  constexpr uint8_t halfWidth = NL<Th>::digits;
  constexpr double b1Prob = 0.326549, b2Prob = 0.00977582; // measured using all valid combination from T=uint8_t.

  const aint_dt<Th> divisorSplit(divisor), dividendLSplit(dividend.l);
  aint_dt<Th> q;

  T qhat = dividend.h / divisorSplit.h, rhat = dividend.h % divisorSplit.h;
  T c1 = qhat * divisorSplit.l, c2 = rhat << halfWidth | dividendLSplit.h;

  if (__builtin_expect_with_probability(c1 > c2, true, b1Prob)) {
    --qhat;
    qhat -= __builtin_expect_with_probability(c1 - c2 > divisor, true, b2Prob);
  }
  q.h = qhat;

  T r = (dividend.h << halfWidth | dividendLSplit.h) - q.h * divisor;

  qhat = r / divisorSplit.h, rhat = r % divisorSplit.h;
  c1 = qhat * divisorSplit.l, c2 = rhat << halfWidth | dividendLSplit.l;
  if (__builtin_expect_with_probability(c1 > c2, true, b1Prob)) {
    --qhat;
    qhat -= __builtin_expect_with_probability(c1 - c2 > divisor, true, b2Prob);
  }
  q.l = qhat;

  r = (r << halfWidth | dividendLSplit.l) - q.l * divisor;

  return {q.merge(), r};
}
#define asmDiv(op, h, l) asm(op " %[divisor]" : "=a"(q), "=d"(r) : [divisor] "rm"(divisor), "a"(l), "d"(h) : "cc")
template<std::unsigned_integral Tu>
constexpr auto nDivRem(Tu dividend, typename rankOf<Tu>::half divisor) {
	using Th=rankOf<Tu>::half;
	Th q,r;
#ifdef ARCH_x86
	if constexpr (sizeof(Tu) == 2) {
		asm("divb %[divisor]" :
			"+a"(dividend) :
			 [divisor] "qm"(divisor) :
			 "cc");
		q = dividend, r = dividend >> 8;
	} else if constexpr (const aint_dt<Th> b(dividend); sizeof(Tu) == 4)
		asmDiv("divw", b.h, b.l);
	else if constexpr (sizeof(Tu) == 8)
		asmDiv("divl", b.h, b.l);
	else
		asmDiv("divq", b.h, b.l);
#else
	q = dividend / divisor, r = dividend % divisor;
#endif
	return std::tuple<Th,Th>{q,r};
}
template <std::unsigned_integral Tu>
constexpr Tu lsDivRnd(const Tu dividend, Tu divisor, const uint8_t by, const std::float_round_style style) {
  Tu q, r;
  if (by == 0) {
    q = dividend / divisor, r = dividend % divisor;
    if (style == std::round_toward_infinity)
      return q + (r != 0);
  } else if constexpr (!requires { typename rankOf<Tu>::two; } || slowDiv<Tu>::v) {
    uint8_t shift = std::countl_zero(divisor);
    divisor <<= shift;
    aint_dt<Tu> c = wideLS(dividend, shift + by);
    if (style == std::round_toward_infinity)
      c += divisor - 1;
    std::tie(q,r)= nDivNormRem(c, divisor);// rounding behavior depends on q, r, divisor. q doesn't change. r scales with divisor, so when odd q then inequality doesn't change. when even divisor, scaling by even number is still even.
  } else {
    using Tt = rankOf<Tu>::two;
    Tt a = Tt(dividend) << by;
    if (style == std::round_toward_infinity)
      a += divisor - 1;
  	std::tie(q,r)=nDivRem(a,divisor);
  }
  if (style == std::round_to_nearest) {
    Tu special = q & (divisor & 1 ^ 1);
    q += r > divisor / 2 - special;
  }
  return q;
}

template <std::signed_integral Ts>
CMATH_CE23 Ts lsDivRnd(const Ts dividend, const Ts divisor, const uint8_t by, const std::float_round_style s) {
  Ts q, r;
  if (by == 0) {
    q = dividend / divisor, r = dividend % divisor;
  } else if constexpr (slowDiv<Ts>::v) {
    using Tu = aint_dt<Ts>::Tu;
    Tu absDivisor = condNeg(Tu(divisor), divisor < 0);
    uint8_t shift = std::countl_zero(absDivisor);
    absDivisor <<= shift;
    aint_dt<Tu> absDividend = wideLS(condNeg(Tu(dividend), dividend < 0), by + shift);

    auto [absQ, absR] = nDivNormRem(absDividend, absDivisor);

    bool qNeg = (dividend ^ divisor) < 0;
    switch (s) {
    case std::round_toward_infinity:
      absQ += absR != 0 && !qNeg;
      break;
    case std::round_toward_neg_infinity:
      absQ += absR != 0 && qNeg; // a^b==0 implies r==0
      break;
    case std::round_to_nearest: {
      Tu special = absQ & (divisor & 1 ^ 1);
      absQ += absR > absDivisor / 2 - special;
      break;
    }
    default:;
    }
    return condNeg(absQ, qNeg);
  } else {
#ifdef ARCH_x86
    if constexpr (sizeof(Ts) == 1) {
      int16_t a = dividend << by;
      asm("idivb %[divisor]" : "+a"(a) :
          [divisor] "qm"(divisor) : "cc");
      q = a, r = a >> 8;
    } else if constexpr (const aint_dt<Ts> a = wideLS(dividend, by); sizeof(Ts) == 2)
      asmDiv("idivw", a.h, a.l);
    else if constexpr (sizeof(Ts) == 4)
      asmDiv("idivl", a.h, a.l);
    else
      asmDiv("idivq", a.h, a.l);
#else
    Tt a = Tt(dividend) << by;
    q = a / divisor, r = a % divisor;
#endif
  }
  bool qNeg = (dividend ^ divisor) < 0;
  switch (s) {
  case std::round_toward_infinity:
    return q + (r != 0 && !qNeg); // r==0 when dividend^divisor==0.
  case std::round_toward_neg_infinity:
    return q - (r != 0 && qNeg);
  case std::round_to_nearest: {
    Ts special = q & (divisor & 1 ^ 1); // round up tie when odd quotient even divisor. round down tie when even quotient and divisor
    if constexpr (requires { std::abs(r); }) {
      return q + condNeg<Ts>(std::abs(r) > std::abs(divisor / 2) - special, qNeg);
    } else {
      __builtin_assume((dividend ^ r) >= 0); // sign of remainder only depends on dividend.
      Ts a = condNeg(r, dividend < 0), b = condNeg<Ts>(divisor / 2, divisor < 0);
      return q + condNeg<Ts>(a > b - special, qNeg);
    }
    // B(abs(b))/2-special>=0;r and b/2 will never overflow after abs. 5%-8==5, 5/-8==0, but we need -1, so must use dividend^divisor.
  }
  default:
    return q;
  }
}

/*template <std::regular Tb, std::unsigned_integral Te>
  requires requires(Tb b) {
    b *= b;
    Tb{1};
  }
constexpr
Tb APowU(const Tb &base, const Te exp, bool support0) {
  Tb r = support0 ? Tb{1} : base;
  for (int8_t i = NL<Te>::digits - 1 - std::countl_zero(exp) - !support0; i > -1; --i) {
    r *= r;
    if ((exp >> i & 1) == 1)
      r *= base;
  }
  return r;
}
template<std::unsigned_integral Tu>
struct boundedMul {
  Tu v;
  constexpr
  boundedMul& operator *=(const boundedMul&o)
  noexcept {
    if (!(bool(*this)&&bool(o)))
      v*=o.v;
    return *this;
  }
  constexpr
  operator bool()const
  noexcept {
    return v>=Tu{1}<<NL<Tu>::digits/2;
  }
};
template<std::unsigned_integral Tu>
constexpr
Tu uRootN(const Tu base, const uint8_t degree, const std::float_round_style S) {
  assOrAss(base>0);
  if (degree<=NL<Tu>::digits) {
    Tu guess=Tu{1} << divRnd<uint8_t, uint8_t>(NL<Tu>::digits - std::countl_zero(base), degree, std::round_toward_infinity);
  for (Tu a=base/APowU(base,degree-1);a<guess;a=base/APowU(base,degree-1)) {
    a=((degree-1)*guess+a)/degree;
  }
  }
}*/
template<uint8_t>
constexpr std::tuple<uint8_t,uint8_t>sqrtRem(const uint8_t base)
noexcept {
	constexpr auto c=[] {
		std::array<uint8_t,128> a;
		uint8_t b=0;
		for (size_t i=0;i<256;++i) {
			b+=(b+1)*(b+1)<=i;
			if (i%2==0)
				a[i/2]=b;
			else
				a[i/2]|=b<<4;
		}
		return a;
	}();
	uint8_t root=c[base];
	if (base%2==0)
		root&=0xf;
	else
		root>>=4;
	return {root,base-root*root};
}
template <std::unsigned_integral Tu>
constexpr auto sqrtRem(const Tu base)
noexcept {
	  /*
   * Suppose sqrt(a)=b computes square root of a float point; `a` and `b` both have `c` bit prec. However, sqrt has an unknown
   * rounding mode, but we know |b-true answer|<=1 ulp. Then, to have `b` be correctly rounded with `c` prec, we need some extra
   * prec. For round_inf and round_neg_inf, we need 1 bit more. For round_nearest, we need 3 bits more. This can be proven.
   */
	using Th=rankOf<Tu>::half;
	using R=std::tuple<Th,Tu>;
	if (base<2)
		return R{base,0};
	#ifdef ARCH_x86
	Th guess=NL<Th>::max();
	if (Tu a=Tu(guess)*guess;base>=a)
		return R{guess,base-a};
	else if (base==a-1)
		return R{guess-1,2*(guess-1)};
	guess=std::min<Tu>(guess,Tu{1} << (NL<Tu>::digits - std::countl_zero(base) + 1) / 2);
	for (Th a=std::get<0>(nDivRem(base,guess));a<guess;a=std::get<0>(nDivRem(base,guess)))
		guess=(Tu(guess)+a)/2;
	#else
	Tu guess = Tu{1} << (NL<Tu>::digits - std::countl_zero(base) + 1) / 2;
	for (Tu a = base / guess; a < guess; a = base / guess)
		guess = (guess + a) / 2;
	#endif
	return R{guess,base-Tu(guess)*guess};
}
dbgHelperExport
template <std::unsigned_integral Tu>
constexpr Tu uRoot2(const Tu base, const std::float_round_style S)
noexcept {
	Tu root,rem;
	std::tie(root,rem)=sqrtRem(base);
  switch (S) {
  case std::round_toward_infinity: {
  	root+=rem!=0;
  } break;
  case std::round_to_nearest: {
  	root+=rem>root;
  }
  default:;
  }
  return root;
}

template <std::signed_integral Bone>
CMATH_CE23 std::tuple<Bone, Bone>remQuoS(const Bone dividend, const Bone divisor) {
  Bone q = dividend / divisor, r = dividend % divisor;
  Bone special = q & (divisor & 1 ^ 1);
  Bone absR, absHalfDivisor;
  if constexpr (requires { std::abs(r); })
    absR = std::abs(r), absHalfDivisor = std::abs(divisor / 2);
  else {
    absR = condNeg(r, dividend < 0);
    absHalfDivisor = condNeg(divisor / 2, divisor < 0);
  }
  if (absR > absHalfDivisor - special) {
    bool qNeg = (divisor ^ dividend) < 0;
    q += condNeg<Bone>(1, qNeg);
    r -= condNeg<Bone>(divisor, qNeg);
  }
  return {q, r};
}

template <std::unsigned_integral Bone>
constexpr std::tuple<Bone, std::make_signed_t<Bone>, bool> remQuoS(const Bone dividend, const Bone divisor) {
  Bone q = dividend / divisor, r = dividend % divisor;
  Bone special = q & (divisor & 1 ^ 1);
  bool of;
  if (r > divisor / 2 - special) {
    ++q;
    r -= divisor; // this won't underflow.
    of = false;
  } else
    of = r > NL<std::make_signed_t<Bone>>::max();
  return {q, r, of};
}
//}
