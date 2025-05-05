module;
#include<concepts>
#include<cstdint>
#include<climits>
#include<limits>
#include<bit>
#include<utility>
#include<cstdlib>
#include<algorithm>
#include <cmath>
export module fixed;
using namespace std;

template<integral B>
constexpr
B preRoundTo(B v,uint8_t digit,float_round_style s)//make digit the LSB, starting from 0. doesn't remove rounded digits.
noexcept
{
    B a=1<<digit;
    switch (s)
    {
    case round_to_nearest:
        if (is_unsigned_v<B>||v>=0)
        {
            B q=v>>digit,r=v&a-1;
            if (r==(a>>=1)&&(q&1)==0)[[unlikely]]
                return v;
            return v+a;
        }else
        {
            B q=v/a,r=v%a;
            if (-r==(a>>=1)&&(q&1)==0)[[unlikely]]
                return v;
            return v-a;
        }
    case round_toward_infinity:
        if (is_unsigned_v<B>||v>0)
            return v-1+a;//v+((1<<digit)-1)
        return v;
    case round_toward_neg_infinity:
        if (is_signed_v<B>&&v<0)
            return v+1-a;//v-((1<<digit)-1)
        return v;
    default:
        return v;
    }
}
#define condNeg(v,a) ((v^-a)+a)
template<float_round_style S,signed_integral B>
constexpr
B divr(const B a,const B b)
noexcept
{
    B q=a/b;
    if constexpr(S==round_toward_infinity)
    {
        B r=a%b;
        return q+(r!=0&&(a^b)>=0);
    }else if constexpr(S==round_toward_neg_infinity)
    {
        B r=a%b;
        return q-(r!=0&&(a^b)<0);
    }else if constexpr(S==round_to_nearest)//tie to even
    {
        B r=a%b;
        B special=q&(b&1^1);//round up tie when odd quotient even divisor. round down tie when even quotient and divisor
        return q+condNeg(B(abs(r))>B(abs(b))/2-special,q<0);
    }else
    {
        return q;
    }
}
template<float_round_style S,unsigned_integral B>
constexpr
B divr(const B a,const B b)
noexcept
{
    B q=a/b;
    if constexpr(S==round_toward_infinity)
    {
        B r=a%b;
        return q+(r!=0);
    }else if constexpr(S==round_to_nearest)//tie to even
    {
        B r=a%b;
        B special=q&(b&1^1);
        return q+(r>b/2-special);
    }else
    {
        return q;
    }
}
template <integral B>
constexpr
float toF32(B v,uint8_t radix,float_round_style S)
    noexcept
{
    using nl=numeric_limits<float>;
    using Equiv=uint32_t;

    if constexpr(sizeof(B)>3)
    {
        make_unsigned_t<B> av=is_signed_v<B>?abs(v):v;
        uint8_t sd=numeric_limits<decltype(av)>::digits-countl_zero(av);

        if (constexpr uint8_t expBias=nl::max_exponent-1;int8_t(radix-sd+1)>=int8_t(expBias))//subnormal. compilers should be able to remove this branch unless using 128bit int and radix>126
        {//value is always exact for 128bit int and float there can be at most 2 significant digits.
            Equiv denorm=av;
            denorm<<=uint8_t(nl::digits-1)-(radix-sd-(expBias-1));
            if constexpr(is_signed_v<B>)
                denorm|=v&~numeric_limits<B>::max();
            return bit_cast<float>(denorm);
        }

        if (int8_t more=sd-nl::digits;more>0)
        {
            v=preRoundTo(v,more,S);
            v/=B{1}<<more;
            radix-=more;
        }

        auto f=bit_cast<Equiv>(float(v));
        int16_t exponent=uint8_t(f>>nl::digits-1);
        exponent=clamp<int16_t>(exponent-int8_t(radix),0,UINT8_MAX);
        f&=~(UINT8_MAX<<nl::digits-1);
        f|=uint32_t(exponent)<<nl::digits-1;
        return bit_cast<float>(f);
    }else
    {//float can always cover entire range.
        auto f=bit_cast<Equiv>(float(v));
        f-=radix*(v!=0)<<nl::digits-1;
        return bit_cast<float>(f);
    }

}

template <integral B>
constexpr
double toF64(B v,uint8_t radix,float_round_style S)
    noexcept
{
    using nl=numeric_limits<double>;
    using Equiv=uint64_t;

    if constexpr(sizeof(B)>6)
    {
        uint8_t sd=numeric_limits<make_unsigned_t<B>>::digits-countl_zero(is_signed_v<B>?B(abs(v)):v);

        if (int8_t more=sd-nl::digits;more>0)
        {
            v=preRoundTo(v,more,S);
            v/=B{1}<<more;
            radix-=more;
        }

        auto f=bit_cast<Equiv>(double(v));
        constexpr uint16_t maxExpU=nl::max_exponent-nl::min_exponent+2;
        int16_t exponent=uint16_t(f>>nl::digits-1)&maxExpU;
        exponent=clamp<int16_t>(exponent-int8_t(radix),0,maxExpU);
        f&=~(Equiv(maxExpU)<<nl::digits-1);
        f|=Equiv(exponent)<<nl::digits-1;
        return bit_cast<double>(f);
    }else
    {
        auto f=bit_cast<Equiv>(double(v));
        f-=Equiv(radix*(v!=0))<<nl::digits-1;
        return bit_cast<double>(f);
    }
}
#define bitSize(T) uint8_t(sizeof(T)*CHAR_BIT)
template <class T, uint8_t R> concept testSize = bitSize(T) >= R;
/*
 *Design choices:
 *  what operators are explicit:
 *      cause 100% loss of information.
 *      can cause undefined behaviors.
 *      doesn't preserve original arithmatic meanings.
 *  what operations aren't supported?
 *      have ambiguous meanings.
 *      uses explicit operators.
 *      completely replaced by stl or builtin features.
 */
export
{
    //Radix is how many bits the decimal point is from the decimal point of integer (right of LSB).
    template <unsigned_integral Bone, uint8_t Radix,float_round_style Style=round_to_nearest> requires(testSize<Bone, Radix>&&Radix>0&&Style>round_indeterminate)//radix==0 is equivalent to int.
    struct ufx
    {
        Bone repr;

        //v's arithmatic meaning changes when Radix!=0
        constexpr
        explicit ufx(Bone v,bool raw=false)
            noexcept: repr(v)
        {
            if (!raw)
            {
                repr<<=1;
                repr<<=Radix-1;
            }
        }

        //conversion from float point is narrowing even causing undefined behaviors depending on exponent.
        template<floating_point F>
        constexpr
        explicit ufx(F v)
            noexcept:repr(ldexp(v,Radix))
        {

        }

        //when both params are changed say ufx<B1,P1>x and ufx<B2,P2>y when sizeof(B1)>sizeof(B2) and P1>P2, then x.repr=y.repr<<(P1-P2) can have different value then x.repr=B1(y.repr)<<(P1-P2). This is ambiguous.

        template <unsigned_integral B1>
        constexpr
        explicit ufx(ufx<B1, Radix> o)
            noexcept: repr(o.repr)
        {
        }

        template <uint8_t P1>
        constexpr
        explicit ufx(ufx<Bone, P1> o)
            noexcept: repr(o.repr)
        {
            if constexpr (Radix > P1)
                repr <<= Radix - P1;
            else
                repr >>= P1 - Radix;
        }

        strong_ordering operator<=>(const ufx&) const = default;

        constexpr
        explicit operator Bone()const
        noexcept
        {
            return (repr>>1)>>(Radix-1);
        }

        //conversion to float point is always defined and never lose all precision.
        constexpr
        operator float() const
            noexcept
        {
            return toF32(repr,Radix,Style);
        }

        constexpr
        operator double() const
            noexcept
        {
            return toF64(repr,Radix,Style);
        }
    };

    template <signed_integral Bone, uint8_t Radix,float_round_style Style=round_to_nearest> requires (testSize<Bone, Radix+1>&&Radix>0&&Style>round_indeterminate)//reserve 1 bit for sign
    class fx
    {
        using U = make_unsigned_t<Bone>;

    public:
        Bone repr;//c++20 defined bit shift on signed integers, right shift additionally comes with sign extending.

        constexpr
        explicit fx(Bone v, bool raw=false)
        noexcept:repr(v)
        {
            if (!raw)
                repr<<=Radix;
        }

        template <floating_point F>
        constexpr
        explicit fx(F v)
            noexcept: repr(ldexp(v,Radix))
        {
        }

        template <signed_integral B1>
        constexpr
        explicit fx(fx<B1, Radix> o)
            noexcept: repr(o.repr)
        {
        }

        template <uint8_t P1>
        constexpr
        explicit fx(fx<Bone, P1> o)
            noexcept: repr(ufx<U, Radix,Style>(bit_cast<ufx<U, P1,Style>>(o)).repr)
        {
        }

        strong_ordering operator <=>(const fx&) const = default;

        /*intcmp functions in <utility> doesn't offer threeway. default threeway can't compare signed and unsigned.
         *fx::U is already defined, for ufx we need to redefine make_signed_t<Bone>. we'd also need a forward declaration.
         *comparing between signed and unsigned of same size is always meaningful arithmetically.
         */
        constexpr
        strong_ordering operator<=>(ufx<U, Radix,Style> o) const
            noexcept
        {
            //compiler will optimize if branches out when told to.
            if (cmp_less(repr, o.repr))
                return strong_ordering::less;
            if (cmp_equal(repr, o.repr))
                return strong_ordering::equivalent;
            return strong_ordering::greater;
        }

        constexpr
        explicit operator Bone()const
        noexcept
        {
            return repr>>Radix;
        }

#define quickAbs(a,sign) (a-sign^-sign)//abs when we already know the sign.

        constexpr
        operator float() const
            noexcept
        {
            return toF32(repr,Radix,Style);
        }

        constexpr
        operator double() const
            noexcept
        {
            return toF64(repr,Radix,Style);
        }
    };
}
