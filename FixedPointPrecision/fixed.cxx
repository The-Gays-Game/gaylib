module;
#include "arithmetic.h"
#include<cstdint>
#include<limits>
#include<bit>
#include<utility>
#include<algorithm>
#include <cmath>
export module fixed;

template <std::floating_point F,test_Tint B>
#ifdef FP_MANIP_CE
#define TOF_CE
constexpr
#endif
F toF(B v, uint8_t radix, std::float_round_style S)
    noexcept(noexcept(std::ldexp(v, int{})))
{
    using nl= std::numeric_limits<F>;
    if (std::numeric_limits<B>::digits > nl::digits)
    {
        uint8_t sd;
        if constexpr (std::is_unsigned_v<B>)
            sd = std::numeric_limits<B>::digits - std::countl_zero(v);
        else
        {
            auto av=condNeg<std::make_unsigned_t<B>>(v,v<0);
            sd = std::numeric_limits<decltype(av)>::digits - std::countl_zero(av);
        }

        bool subnorm = nl::has_denorm == std::denorm_present && int8_t(sd-radix) <= nl::min_exponent - 1;
        if (int8_t more = sd - nl::digits; S != std::round_indeterminate && !subnorm && more > 0)
        {
            //with radix<=128, sd<=128, then sd<=2 needs no rounding.
            v = aint_dt<B>(v).narrowArsRnd(more, S);
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
 *      doesn't preserve original arithmatic meanings.
 *  what operations aren't supported?
 *      have ambiguous meanings.
 *      uses explicit operators.
 *      completely replaced by stl or builtin features.
 */
export
{
    template <class T, uint8_t R> concept testSize = std::numeric_limits<T>::digits >= R;

    //Radix is how many bits the decimal point is from the decimal point of integer (right of LSB).
    template <test_Tuint Bone, uint8_t Radix, std::float_round_style Style = std::round_toward_zero> requires testSize<Bone, Radix> //radix==0 is equivalent to int.
    struct ufx
    {
        Bone repr;

        //v's arithmatic meaning changes when Radix!=0
        constexpr
        explicit ufx(Bone v, bool raw = false)
            noexcept: repr(v)
        {
            if (!raw)
                if (Radix < std::numeric_limits<Bone>::digits)
                    repr <<= Radix;
                else
                    repr = 0;
        }

        //conversion from float point is narrowing even causing undefined behaviors depending on exponent.
#ifdef FP_MANIP_CE
        constexpr
#endif
        explicit ufx(std::floating_point auto v)
            noexcept(noexcept(std::ldexp(v, int{}))): repr(std::ldexp(v, Radix))
        {
        }

        //when both params are changed say ufx<B1,P1>x and ufx<B2,P2>y when sizeof(B1)>sizeof(B2) and P1>P2, then x.repr=y.repr<<(P1-P2) can have different value then x.repr=B1(y.repr)<<(P1-P2). This is ambiguous.

        template <test_Tuint B1>
        constexpr
        explicit ufx(ufx<B1, Radix, Style> o)
            noexcept: repr(o.repr)
        {
        }

        template <uint8_t P1>
        constexpr
        explicit ufx(ufx<Bone, P1, Style> o)
            noexcept: repr(o.repr)
        {
            if (Radix > P1)
                repr <<= Radix - P1;
            else if (Radix < P1)
                repr = aint_dt<Bone>(0, repr).narrowArsRnd(P1 - Radix, Style);
        }

        auto operator<=>(const ufx&) const = default;

        constexpr
        explicit operator Bone() const
            noexcept
        {
            return aint_dt<Bone>(0, repr).narrowArsRnd(Radix, std::round_toward_zero);;
        }

        //conversion to float point is always defined and never lose all precision.
        template <std::floating_point F>
#ifdef TOF_CE
        constexpr
#endif
        operator F() const
            noexcept(noexcept(toF<F>(repr, Radix, Style)))
        {
            return toF<F>(repr, Radix, Style);
        }

        constexpr
        ufx& operator/=(ufx divisor)
        {
            if (Radix == 0)
                repr = divRnd(repr, divisor.repr, Style);
            else if constexpr (requires { typename rankOf<Bone>::two; })
            {
                typename rankOf<Bone>::two dividend = repr;
                repr = divRnd<typename rankOf<Bone>::two>(dividend << Radix, divisor.repr, Style);
            }
            else
            {
                uint8_t shift = std::countl_zero(divisor.repr);
                divisor.repr <<= shift;
                aint_dt<Bone> dividend = wideLS(repr, shift + Radix);
                repr = divRnd(dividend, divisor.repr, Style);
                //rounding behavior depends on q, r, divisor. q doesn't change. r scales with divisor, so when odd q then inequality doesn't change. when even divisor, scaling by even number is still even.
            }
            return *this;
        }

        constexpr
        auto operator /(ufx divisor) const
        {
            return ufx(*this) /= divisor;
        }

        constexpr
        ufx& operator*=(ufx o)
            noexcept
        {
            if (Radix == 0)
                repr *= o.repr;
            else if constexpr(requires { typename rankOf<Bone>::two; })
            {
                auto a = typename rankOf<Bone>::two(repr) * o.repr;
                repr = aint_dt<typename rankOf<Bone>::two>(0, a).narrowArsRnd(Radix, Style);
            }
            else
            {
                aint_dt<Bone> a = wideMul(repr, o.repr);
                repr = a.narrowArsRnd(Radix, Style);
            }
            return *this;
        }

        constexpr
        auto operator*(ufx o) const
            noexcept
        {
            return ufx(*this) *= o;
        }

        constexpr
        ufx& operator+=(ufx o)
            noexcept
        {
            repr += o.repr;
            return *this;
        }

        constexpr
        auto operator+(ufx o) const
            noexcept
        {
            return ufx(*this) += o;
        }

        constexpr
        ufx& operator-=(ufx o)
            noexcept
        {
            repr -= o.repr;
            return *this;
        }

        constexpr
        auto operator-(ufx o) const
            noexcept
        {
            return ufx(*this) -= o;
        }
    };

    template <test_Tsint Bone, uint8_t Radix, std::float_round_style Style =std:: round_toward_zero> requires testSize<Bone, Radix>
    class fx
    {
        using U = std::make_unsigned_t<Bone>;

    public:
        Bone repr; //c++20 defined bit shift on signed integers, right shift additionally comes with sign extending.

        constexpr
        explicit fx(Bone v, bool raw = false)
            noexcept: repr(v)
        {
            if (!raw)
                repr <<= Radix;
        }

#ifdef FP_MANIP_CE
        constexpr
#endif
        explicit fx(std::floating_point auto v)
            noexcept(noexcept(std::ldexp(v, int{}))): repr(std::ldexp(v, Radix))
        {
        }

        template <test_Tsint B1>
        constexpr
        explicit fx(fx<B1, Radix, Style> o)
            noexcept: repr(o.repr)
        {
        }

        template <uint8_t P1>
        constexpr
        explicit fx(fx<Bone, P1, Style> o)
            noexcept: repr(o.repr)
        {
            if (Radix > P1)
                repr <<= Radix - P1;
            else if (Radix < P1)
                repr = aint_dt(repr).narrowArsRnd(P1 - Radix, Style);
        }

        auto operator <=>(const fx&) const = default;

        /*intcmp functions in <utility> doesn't offer threeway. default threeway can't compare signed and unsigned.
         *fx::U is already defined, for ufx we need to redefine make_signed_t<Bone>. we'd also need a forward declaration.
         *comparing between signed and unsigned of same size is always meaningful arithmetically.
         */
        constexpr
        std::strong_ordering operator<=>(ufx<U, Radix, Style> o) const
            noexcept
        {
            if (cmp_less(repr, o.repr))
                return std::strong_ordering::less;
            if (cmp_equal(repr, o.repr))
                return std::strong_ordering::equivalent;
            return std::strong_ordering::greater;
        }

        constexpr
        explicit operator Bone() const
            noexcept
        {
            return aint_dt(repr).narrowArsRnd(Radix, std::round_toward_zero);
        }

        template <std::floating_point F>
#ifdef TOF_CE
        constexpr
#endif
        operator F() const
            noexcept(noexcept(toF<F>(repr, Radix, Style)))
        {
            return toF<F>(repr, Radix, Style);
        }
#ifdef S_DIVR_CE
        constexpr
#endif
        fx& operator/=(fx divisor)
        {
            if (Radix == 0)
                repr = divRnd(repr, divisor.repr, Style);
            else if constexpr (requires { typename rankOf<Bone>::two; })
            {
                typename rankOf<Bone>::two dividend = repr;
                repr = divRnd<decltype(dividend)>(dividend << Radix, divisor.repr, Style);
            }
            else
                repr = lsDivRnd(repr, divisor.repr, Radix, Style);
            return *this;
        }
#ifdef S_DIVR_CE
        constexpr
#endif
        auto operator/(fx dividend) const
        {
            return fx(*this) /= dividend;
        }

        constexpr
        fx& operator*=(fx o)
        {
            if (Radix == 0)
                repr *= o.repr;
            else if constexpr (requires { typename rankOf<Bone>::two; })
            {
                typename rankOf<Bone>::two a = typename rankOf<Bone>::two(repr) * o.repr;
                repr = aint_dt(a).narrowArsRnd(Radix, Style);
            }
            else
            {
                aint_dt<Bone> a = wideMul(repr, o.repr);
                repr = a.narrowArsRnd(Radix, Style);
            }
            return *this;
        }

        constexpr
        auto operator*(fx o) const
        {
            return fx(*this) *= o;
        }

        constexpr
        fx& operator+=(fx o)
        {
            repr += o.repr;
            return *this;
        }

        constexpr
        auto operator+(fx o) const
        {
            return fx(*this) += o;
        }

        constexpr
        fx& operator-=(fx o)
        {
            repr -= o.repr;
            return *this;
        }

        constexpr
        auto operator-(fx o) const
        {
            return fx(*this) -= o;
        }
    };
}
