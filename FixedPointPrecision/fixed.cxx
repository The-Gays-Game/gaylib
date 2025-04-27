module;
#include<concepts>
#include<cstdint>
#include<climits>
#include<bit>
#include<utility>
export module fixed;
using namespace std;

constexpr struct //https://en.wikipedia.org/wiki/Single-precision_floating-point_format#IEEE_754_standard:_binary32
{
    uint8_t exponentBits, fractionBits, exponentBias;
} F32{8, 23, 127};

constexpr struct
    //https://en.wikipedia.org/wiki/Double-precision_floating-point_format#IEEE_754_double-precision_binary_floating-point_format:_binary64
{
    uint8_t exponentBits, fractionBits;
    uint16_t exponentBias;

    constexpr
    uint16_t maskExponent(uint16_t exponent) const
        noexcept
    {
        uint16_t exponentBitsMask = (1 << exponentBits) - 1;
        return exponent & exponentBitsMask;
    }
} F64{11, 52, 1023};

template <unsigned_integral B>
constexpr
uint32_t toF32U(B v, uint8_t radix)
    noexcept
{
    constexpr uint8_t boneSize = sizeof(B) * CHAR_BIT;
    uint8_t leading0 = countl_zero(v);
    v <<= leading0; //drop first bit
    v <<= 1; //when leading0+1==boneSize, undefined.
    uint32_t fraction;
    if constexpr (boneSize >= F32.fractionBits) //keep only first fractionBits bits.
        fraction = v >> (boneSize - F32.fractionBits);
    else //lshift leading bit to fractionBit
        fraction = U(v) << (F32.fractionBits - boneSize);
    uint8_t exponent = boneSize - leading0 - radix - 1;
    return uint8_t(exponent + F32.exponentBias) << F32.fractionBits | fraction;
}

template <unsigned_integral B>
constexpr
uint64_t toF64U(B v, uint8_t radix)
    noexcept
{
    constexpr uint8_t boneSize = sizeof(B) * CHAR_BIT;
    uint8_t leading0 = countl_zero(v);
    v <<= leading0; //drop first bit
    v <<= 1; //when leading0+1==boneSize, undefined.
    uint64_t fraction;
    if constexpr (boneSize >= F64.fractionBits) //keep only first fractionBits bits.
        fraction = v >> (boneSize - F64.fractionBits);
    else //lshift leading bit to fractionBit
        fraction = U(v) << (F64.fractionBits - boneSize);
    uint8_t exponent = boneSize - leading0 - radix - 1;
    return uint64_t(F64.maskExponent(exponent + F64.exponentBias)) << F64.fractionBits | fraction;
}

template <class T, uint8_t R> concept testSize = sizeof(T) * CHAR_BIT >= R;
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
    template <unsigned_integral Bone, uint8_t Radix> requires(testSize<Bone, Radix>&&Radix>0)//radix==0 is equivalent to int.
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
        constexpr
        explicit ufx(float v)
            noexcept
        {
            auto a = bit_cast<uint32_t>(v);
            uint8_t exponent = uint8_t(a >> F32.fractionBits) - F32.exponentBias; //ignore sign bit for now
            constexpr uint32_t fractionBitsMask = (1 << F32.fractionBits) - 1;
            uint32_t normalized = (a & fractionBitsMask) | (fractionBitsMask + 1);
            if (uint8_t currentRadix = F32.fractionBits - exponent; currentRadix > Radix)
                repr = normalized >> currentRadix - Radix;
            else if (currentRadix < Radix)
            {
                repr = normalized; //in case sizeof(Bone)>sizeof(float), try to retain more bits as possible.
                repr <<= Radix - currentRadix;
            }
            else
                repr = normalized;
        }

        constexpr
        explicit ufx(double v)
            noexcept
        {
            auto a = bit_cast<uint64_t>(v);
            uint16_t exponent = F64.maskExponent(a >> F64.fractionBits) - F64.exponentBias; //ignore sign bit for now
            constexpr uint64_t fractionBitsMask = (uint64_t{1} << F64.fractionBits) - 1;
            uint64_t normalized = (a & fractionBitsMask) | (fractionBitsMask + 1);
            if (uint16_t currentRadix = F64.fractionBits - exponent; currentRadix > Radix)
                repr = normalized >> currentRadix - Radix;
            else if (currentRadix < Radix)
            {
                repr = normalized;
                repr <<= Radix - currentRadix;
            }
            else
                repr = normalized;
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
            return bit_cast<float>(toF32U(repr, Radix));
        }

        constexpr
        operator double() const
            noexcept
        {
            return bit_cast<double>(toF64U(repr, Radix));
        }
    };

    template <signed_integral Bone, uint8_t Radix> requires (testSize<Bone, Radix+1>&&Radix>0)//reserve 1 bit for sign
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
            noexcept: repr(ufx<U, Radix>(v).repr)
        {
            Bone sign = v < 0;
            repr = (repr ^ -sign) + sign; //negate if <0.
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
            noexcept: repr(ufx<U, Radix>(bit_cast<ufx<U, P1>>(o)).repr)
        {
        }

        strong_ordering operator <=>(const fx&) const = default;

        /*intcmp functions in <utility> doesn't offer threeway. default threeway can't compare signed and unsigned.
         *fx::U is already defined, for ufx we need to redefine make_signed_t<Bone>. we'd also need a forward declaration.
         *comparing between signed and unsigned of same size is always meaningful arithmetically.
         */
        constexpr
        strong_ordering operator<=>(ufx<U, Radix> o) const
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
            uint32_t sign = repr < 0;
            uint32_t r = toF32U<U>(quickAbs(repr, sign), Radix);
            return bit_cast<float>(sign << F32.exponentBits + F32.fractionBits | r);
        }

        constexpr
        operator double() const
            noexcept
        {
            uint64_t sign = repr < 0;
            uint64_t r = toF64U<U>(quickAbs(repr, sign), Radix);
            return bit_cast<double>(sign << F64.exponentBits + F64.fractionBits | r);
        }
    };
}
