module;
#include<concepts>
#include<cstdint>
#include<limits>
#include<bit>
#include<utility>
#include<algorithm>
#include "arithmetic.h"
#include <cmath>
export module fixed;
using namespace std;

template<integral B>
static constexpr
aint_dw<B> extend(const B v)
noexcept
{
    return aint_dw<B>(v>>numeric_limits<B>::digits,v);
}
template<floating_point F,integral B>
static
#ifdef S_DIVR_CE
#define TOF_CE
constexpr
#endif
F toF(B v,uint8_t radix,float_round_style S)
noexcept(noexcept(ldexp(v,int{})))
{
    using nl=numeric_limits<F>;
    if (numeric_limits<B>::digits>nl::digits)
    {
        uint8_t sd;
        if constexpr(is_unsigned_v<B>) {
            sd=numeric_limits<B>::digits-countl_zero(v);
        }else {
            make_unsigned_t<B> av=abs(v);
            sd=numeric_limits<decltype(av)>::digits-countl_zero(av);
        }

        bool subnorm=nl::has_denorm==denorm_present&&int8_t(radix-sd)>=-(nl::min_exponent-1);
        if (int8_t more=sd-nl::digits;S!=round_indeterminate&&!subnorm&&more>0) {//with radix<=128, sd<=128, then sd<=2 needs no rounding.
            v=extend(v).narrowRSr(more,S);
            radix-=more;
            return ldexp(v,-int8_t(radix));
        }
    }
    return ldexp(v,-int16_t(radix));
}

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
template<class T>concept wider=requires{typename rankOf<T>::two;};
export
{
    template <class T, uint8_t R> concept testSize = numeric_limits<T>::digits>=R;
    //Radix is how many bits the decimal point is from the decimal point of integer (right of LSB).
    template <unsigned_integral Bone, uint8_t Radix,float_round_style Style=round_toward_zero> requires testSize<Bone, Radix>//radix==0 is equivalent to int.
    struct ufx
    {
        Bone repr;

        //v's arithmatic meaning changes when Radix!=0
        constexpr
        explicit ufx(Bone v,bool raw=false)
            noexcept: repr(v)
        {
            if (!raw)
                if (Radix<numeric_limits<Bone>::digits)
                    repr<<=Radix;
                else
                    repr=0;
        }

        //conversion from float point is narrowing even causing undefined behaviors depending on exponent.
        template<floating_point F>
#ifdef FP_MANIP_CE
        constexpr
#endif
        explicit ufx(F v)
            noexcept(noexcept(ldexp(v,int{}))):repr(ldexp(v,Radix))
        {}

        //when both params are changed say ufx<B1,P1>x and ufx<B2,P2>y when sizeof(B1)>sizeof(B2) and P1>P2, then x.repr=y.repr<<(P1-P2) can have different value then x.repr=B1(y.repr)<<(P1-P2). This is ambiguous.

        template <unsigned_integral B1>
        constexpr
        explicit ufx(ufx<B1, Radix,Style> o)
            noexcept: repr(o.repr)
        {
        }

        template <uint8_t P1>
        constexpr
        explicit ufx(ufx<Bone, P1,Style> o)
        noexcept: repr(o.repr)
        {
            if  (Radix > P1)
                repr <<= Radix - P1;
            else if (Radix<P1)
                repr =aint_dw<Bone>(0,repr).narrowRSr(P1-Radix,Style);
        }

        strong_ordering operator<=>(const ufx&) const = default;

        constexpr
        explicit operator Bone()const
        noexcept
        {
            return aint_dw<Bone>(0,repr).narrowRSr(Radix,round_toward_zero);;
        }

        //conversion to float point is always defined and never lose all precision.
        template<floating_point F>
#ifdef TOF_CE
        constexpr
#endif
        operator F()const
        noexcept(noexcept(toF<F>(repr,Radix,Style))) {
            return toF<F>(repr,Radix,Style);
        }

        constexpr
        ufx operator/(ufx divisor)const
        {
            if (Radix==0)
            {
                return ufx(divr(repr,divisor.repr,Style),true);
            }
            else if constexpr(requires{typename rankOf<Bone>::two;})
            {
                typename rankOf<Bone>::two dividend=repr;
                return ufx(divr<typename rankOf<Bone>::two>(dividend<<Radix,divisor.repr,Style),true);
            }else
            {
                uint8_t shift=countl_zero(divisor.repr);
                divisor.repr<<=shift;
                aint_dw<Bone> dividend=wideLS(repr,shift+Radix);
                return ufx(divr(dividend,divisor.repr,Style),true);//rounding behavior depends on q, r, divisor. q doesn't change. r scales with divisor, so when odd q then inequality doesn't change. when even divisor, scaling by even number is still even.
            }
        }

        constexpr
        ufx operator*(ufx o)const
        noexcept
        {
            if (Radix==0)
            {
                return repr*o.repr;
            }
            else if (requires{typename rankOf<Bone>::two;})
            {
                auto a=typename rankOf<Bone>::two(repr)*o.repr;
                return ufx(aint_dw<typename rankOf<Bone>::two>(0,a).narrowRSr(Radix,Style),true);
            }else
            {
                aint_dw<Bone> a=wideMul(repr,o.repr);
                return ufx(a.narrowRSr(Radix,Style),true);
            }
        }
    };

    template <signed_integral Bone, uint8_t Radix,float_round_style Style=round_toward_zero> requires testSize<Bone, Radix>
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
#ifdef FP_MANIP_CE
        constexpr
#endif
        explicit fx(F v)
            noexcept(noexcept(ldexp(v,int{}))): repr(ldexp(v,Radix))
        {}

        template <signed_integral B1>
        constexpr
        explicit fx(fx<B1, Radix,Style> o)
            noexcept: repr(o.repr)
        {}

        template <uint8_t P1>
        constexpr
        explicit fx(fx<Bone, P1,Style> o)
        noexcept: repr(o.repr)
        {
            if (Radix > P1)
                repr <<= Radix - P1;
            else if (Radix<P1)
                repr =extend(repr).narrowRSr(P1-Radix,Style);
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
            return extend(repr).narrowRSr(Radix,round_toward_zero);
        }

        template<floating_point F>
#ifdef TOF_CE
        constexpr
#endif
        operator F()const
        noexcept(noexcept(toF<F>(repr,Radix,Style))) {
            return toF<F>(repr,Radix,Style);
        }

        constexpr
        fx operator/(fx divisor)const
        {
            if (Radix==0){
                return fx(divr(repr,divisor.repr,Style),true);
            }
            else if constexpr (requires{typename rankOf<Bone>::two;})
            {
                typename rankOf<Bone>::two dividend=repr;
                dividend<<=Radix;
                return fx(divr<decltype(dividend)>(dividend,divisor.repr,Style),true);
            }else
            {
                return fx(lsDivR(repr,divisor.repr,Radix,Style),true);
        }
    }
    };
    }