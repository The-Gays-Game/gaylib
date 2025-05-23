#pragma once

#include<cstdlib>

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 202302L) || __cplusplus >= 202302L)
    #define CPP23
#endif

#ifndef __has_builtin
    #define __has_builtin(x) 0
#endif

#ifndef __has_constexpr_builtin
    #define __has_constexpr_builtin(x) 0
#endif

#if defined(CPP23)||defined(__GNUG__)
    #define FP_MANIP_CE
#endif

#if defined(CPP23)||defined(__GNUG__)
    #define INT_ABS_CE
#endif

template <std::integral T>
constexpr
T condNeg(const T v,const bool a)
noexcept
{
    return (v^-T{a})+a;
}
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
            return q+condNeg<Ts>(std::abs(r)>std::abs(b/2)-special,q<0);//B(abs(b))/2-special>=0;r and b/2 will never overflow after abs.
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

template<std::integral> struct rankOf{};

template<>struct rankOf<uint8_t>{using two=uint16_t;};
template<>struct rankOf<int8_t>{using two=int16_t;};

template<>struct rankOf<uint16_t>{using half=uint8_t;using two=uint32_t;};
template<>struct rankOf<int16_t>{using half=int8_t;using two=int32_t;};

template<>struct rankOf<uint32_t>{using half=uint16_t;using two=uint64_t;};
template<>struct rankOf<int32_t>{using half=int16_t;using two=int64_t;};

template<>struct rankOf<uint64_t>{using half=uint32_t;
#if defined(__GNUG__)||defined(__clang__)
    using two=__uint128_t;
#endif
};
template<>struct rankOf<int64_t>{using half=int32_t;
#if defined(__GNUG__)||defined(__clang__)
    using two=__int128_t;
#endif
};

#if defined(__GNUG__)||defined(__clang__)
template<>struct rankOf<__uint128_t>{using half=uint64_t;};
template<>struct rankOf<__int128_t>{using half=int64_t;};
#endif

template<std::integral Ta>
struct aint_dw{
    using Tu=std::make_unsigned_t<Ta>;
    Ta h;
    Tu l;
    aint_dw()=default;
    constexpr
    aint_dw(const Ta h,const Tu l)
    noexcept:h(h),l(l){}
    template<std::integral V>requires std::is_same_v<typename rankOf<Ta>::two,V>
    constexpr
    explicit aint_dw(V v)
    noexcept:h(v>>std::numeric_limits<Tu>::digits),l(v)
    {
    }
    constexpr
    typename rankOf<Ta>::two merge()const
    noexcept
    {
        constexpr uint8_t width=std::numeric_limits<Tu>::digits;
        return typename rankOf<Ta>::two(h)<<width|l;
    }
};

template<std::integral T>
constexpr
aint_dw<T> wideMul(const T a,const T b)
noexcept
{
    using Tu=typename aint_dw<T>::Tu;
    using Th=typename rankOf<Tu>::half;
    constexpr T halfWidth=std::numeric_limits<Th>::digits;

    T aL=Th(a),aH=a>>halfWidth;
    T bL=Th(b),bH=b>>halfWidth;

    T d=aH*bL+((aL*bL)>>halfWidth);
    T c1=Th(d);
    T c2=d>>halfWidth;
    c1+=aL*bH;

    T eH=aH*bH+c2+(c1>>halfWidth);
    Tu eL=Tu(a)*b;
    return {eH,eL};
}

template<std::integral T>
constexpr
aint_dw<T> wideLS(const T a,const uint8_t/*assume by>0*/ by)
noexcept
{
#ifdef debug_arithmetic
    if (by==0)
        throw std::domain_error("can't shift by 0");
#elif __has_builtin(__builtin_assume)
    __builtin_assume(by>0);
#endif
    using Tu=typename aint_dw<T>::Tu;
    T h=a>>std::numeric_limits<Tu>::digits-by;
    Tu l=Tu(a)<<by-1;
    l<<=1;
    return {h,l};
}
template<std::unsigned_integral T>
constexpr
std::tuple<T,T>  narrow2Div(const aint_dw<T>/*.h >0*/ dividend,const T/*assume normalized*/ divisor)
{
#ifdef debug_arithmetic
    if (std::countl_zero(divisor))
        throw std::domain_error("unnormalized divisor");
    if (!dividend.h)
        throw std::domain_error("dividend.h==0");
#elif __has_builtin(__builtin_assume)
    __builtin_assume(std::countl_zero(divisor)==0&&dividend.h>0);
#endif
    using Th=typename rankOf<T>::half;
    constexpr uint8_t halfWidth=std::numeric_limits<Th>::digits;

    const aint_dw<Th> divisorSplit(divisor),dividendLSplit(dividend.l);
    aint_dw<Th> q;

    T qhat=dividend.h/divisorSplit.h,rhat=dividend.h%divisorSplit.h;
    T c1=qhat*divisorSplit.l,c2=rhat<<halfWidth|dividendLSplit.h;
    if (c1>c2)
        qhat-=c1-c2>divisor?2:1;
    q.h=qhat;

    T r=(dividend.h<<halfWidth|dividendLSplit.h)-q.h*divisor;

    qhat=r/divisorSplit.h,rhat=r%divisorSplit.h;
    c1=qhat*divisorSplit.l,c2=rhat<<halfWidth|dividendLSplit.l;
    if (c1>c2)
        qhat-=c1-c2>divisor?2:1;
    q.l=qhat;

    r=(r<<halfWidth|dividendLSplit.l)-q.l*divisor;

    return {q.merge(),r};
}
/*
static int div_round(int a, int b)
{
    auto [q,r]=std::div(a,b);
    //when divisor id even, quotient is odd, round half upward.
    auto special=q&(b&1^1);//(b&1)==0&&(q&1)==1
    if (a>0&&b>0)//6/4=1%2,10/4=2%2
    {
        return q+(r>b/2-special);//q+(-r<-b/2+special);
    }else if (a<0&&b<0)//-6/-4=1%-2,-10/-4=2%-2
    {
        return q+(-r>-b/2-special);//q+(r<b/2+special);
    }else if (a>0&&b<0)//-6/4=-1%-2,-10/4=-2%-2
    {
        return q-(-r>b/2-special);//q-(r<-b/2+special);
    }else//6/-4=-1%2,10/-4=-2%2
    {
        return q-(r>-b/2-special);//q-(-r<b/2+special);
    }
}
*/