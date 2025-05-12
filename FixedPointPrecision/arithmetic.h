#pragma once

#include<cstdlib>
#include <semaphore>

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 202302L) || __cplusplus >= 202302L)
#define CPP23
#endif

#if defined(CPP23)||defined(__GNUG__)
#define FP_MANIP_CE
#endif

#if defined(CPP23)||defined(__GNUG__)
#define INT_ABS_CE
#endif

#define condNeg(v,a) ((v^-a)+a)

template<std::signed_integral B>
#ifdef INT_ABS_CE
#define S_DIVR_CE
constexpr
#endif
B divr(const B a,const B b,const std::float_round_style S)
noexcept
{
    B q=a/b;
    switch (S) {
    case std::round_toward_infinity: {
            B r=a%b;
            return q+(r!=0&&(a^b)>0);//a^b==0 implies r==0
    }
    case std::round_toward_neg_infinity: {
            B r=a%b;
            return q-(r!=0&&(a^b)<0);
    }
    case std::round_to_nearest: {
            B r=a%b;
            B special=q&(b&1^1);//round up tie when odd quotient even divisor. round down tie when even quotient and divisor
            return q+condNeg(B(std::abs(r))>B(std::abs(b/2))-special,q<0);//B(abs(b))/2-special>=0;r and b/2 will never overflow after abs.
    }
    default:
        return q;
    }
}
template<std::unsigned_integral B>
constexpr
B divr(const B a,const B b,const std::float_round_style S)
noexcept
{
    B q=a/b;
    switch (S) {
    case std::round_toward_infinity: {
            B r=a%b;
            return q+(r!=0);
    }
    case std::round_to_nearest: {//tie to even
            B r=a%b;
            B special=q&(b&1^1);
            return q+(r>b/2-special);
    }
    default:
        return q;
    }
}

template<std::unsigned_integral B>
constexpr
std::tuple<B,B> longMul(const B a,const B b)
noexcept {
    constexpr uint8_t halfWidth=std::numeric_limits<B>::digits/2;
    constexpr B halfWidthMask=(B{1}<<halfWidth)-1;

    B aL=a&halfWidthMask,aH=a>>halfWidth;
    B bL=b&halfWidthMask,bH=b>>halfWidth;

    B d=aH*bL+((aL*bL)>>halfWidth);
    B c1=d&halfWidthMask;
    B c2=d>>halfWidth;
    c1+=aL*bH;

    B eH=aH*bH+c2+(c1>>halfWidth);
    B eL=a*b;
    return {eH,eL};
}

template<std::signed_integral B>
constexpr
std::tuple<B,std::make_unsigned_t<B>> longMul(const B a,const B b)
noexcept
{
    using U=std::make_unsigned_t<B>;
    constexpr B halfWidth=std::numeric_limits<U>::digits/2;
    constexpr uint8_t halfWidthDivisor=B{1}<<halfWidth;
    constexpr B halfWidthMask=halfWidthDivisor-1;
    //1100000010000000=-16256；10000000=-128，01111111=127
    //aH*bH=11001000=-56
    //aH*bL=-8*15=-120=10001000
    //
    B aL=a&halfWidthMask,aH=a>>halfWidth;
    B bL=b&halfWidthMask,bH=b>>halfWidth;

    B d=aH*bL+((aL*bL)>>halfWidth);
    B c1=d&halfWidthMask;
    B c2=d>>halfWidth;
    c1+=aL*bH;

    B eH=aH*bH+c2+(c1>>halfWidth);
    U eL=U(a)*U(b);
    return {eH,eL};
}
//TODO: combine signed and unsigned bc they are the same.