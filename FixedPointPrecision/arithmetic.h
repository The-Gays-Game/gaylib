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

template<std::signed_integral Ts>
#ifdef INT_ABS_CE
#define S_DIVR_CE
constexpr
#endif
Ts divr(const Ts a,const Ts b,const std::float_round_style s)
{
    Ts q=a/b;
    switch (s) {
    case std::round_toward_infinity: {
            Ts r=a%b;
            return q+(r!=0&&(a^b)>0);//a^b==0 implies r==0
    }
    case std::round_toward_neg_infinity: {
            Ts r=a%b;
            return q-(r!=0&&(a^b)<0);
    }
    case std::round_to_nearest: {
            Ts r=a%b;
            Ts special=q&(b&1^1);//round up tie when odd quotient even divisor. round down tie when even quotient and divisor
            return q+condNeg(Ts(std::abs(r))>Ts(std::abs(b/2))-special,q<0);//B(abs(b))/2-special>=0;r and b/2 will never overflow after abs.
    }
    default:
        return q;
    }
}
template<std::unsigned_integral Tu>
constexpr
Tu divr(const Tu a,const Tu b,const std::float_round_style s)
{
    Tu q=a/b;
    switch (s) {
    case std::round_toward_infinity: {
            Tu r=a%b;
            return q+(r!=0);
    }
    case std::round_to_nearest: {//tie to even
            Tu r=a%b;
            Tu special=q&(b&1^1);
            return q+(r>b/2-special);
    }
    default:
        return q;
    }
}

//auto [q,r]=longMul(a,b);
//result=q<<width | r
//basically, q is euclidean quotient of result%max, r is euclidean reminder.
template<std::integral T>
constexpr
std::tuple<T,std::make_unsigned_t<T>> longMul(const T a,const T b)
noexcept
{
    using U=std::make_unsigned_t<T>;
    constexpr T halfWidth=std::numeric_limits<U>::digits/2;
    constexpr T halfWidthMask=(T{1}<<halfWidth)-1;

    T aL=a&halfWidthMask,aH=a>>halfWidth;
    T bL=b&halfWidthMask,bH=b>>halfWidth;

    T d=aH*bL+((aL*bL)>>halfWidth);
    T c1=d&halfWidthMask;
    T c2=d>>halfWidth;
    c1+=aL*bH;

    T eH=aH*bH+c2+(c1>>halfWidth);
    U eL=U(a)*U(b);
    return {eH,eL};
}

template<std::unsigned_integral Tu>
constexpr
std::tuple<Tu,Tu>  knuth(const std::tuple<Tu,Tu> dividend,const Tu divisor)
{

}