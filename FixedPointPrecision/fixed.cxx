module;
#include<concepts>
#include<cstdint>
#include<climits>
#include<bit>
#include<iostream>
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

    constexpr uint16_t maskExponent(uint16_t exponent) const noexcept
    {
        uint16_t exponentBitsMask = (1 << exponentBits) - 1;
        return exponent & exponentBitsMask;
    }
} F64{11, 52, 1023};

template <unsigned_integral B>
constexpr
uint32_t toF32U(B v, uint8_t precision) noexcept
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
    uint8_t exponent = boneSize - leading0 - precision - 1;
    return uint8_t(exponent + F32.exponentBias) << F32.fractionBits | fraction;
}

template <unsigned_integral B>
constexpr
uint64_t toF64U(B v, uint8_t precision) noexcept
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
    uint8_t exponent = boneSize - leading0 - precision - 1;
    return uint64_t(F64.maskExponent(exponent + F64.exponentBias)) << F64.fractionBits | fraction;
}

template <class T, uint8_t P> concept testSize = sizeof(T) * CHAR_BIT >= P;

export
{
    template <signed_integral Bone, uint8_t Precision> requires(testSize<Bone, Precision>)
    class fx;

    template <unsigned_integral Bone, uint8_t Precision> requires(testSize<Bone, Precision>)
    struct ufx
    {
        ufx(const Bone& v): repr(v)
        {
        }

        //conversion from float point can cause lost of all precision or defined behavior depending on exponent, so we make this explicit.
        constexpr
        explicit ufx(float v)
            noexcept
        {
            auto a = bit_cast<uint32_t>(v);
            uint8_t exponent = uint8_t(a >> F32.fractionBits) - F32.exponentBias; //ignore sign bit for now
            constexpr uint32_t fractionBitsMask = (1 << F32.fractionBits) - 1;
            uint32_t normalized = (a & fractionBitsMask) | (fractionBitsMask + 1);
            if (uint8_t currentPrecision = F32.fractionBits - exponent; currentPrecision > Precision)
                repr = normalized >> currentPrecision - Precision;
            else if (currentPrecision < Precision)
            {
                repr = normalized; //in case sizeof(Bone)>sizeof(float), try to retain more bits as possible.
                repr <<= Precision - currentPrecision;
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
            if (uint16_t currentPrecision = F64.fractionBits - exponent; currentPrecision > Precision)
                repr = normalized >> currentPrecision - Precision;
            else if (currentPrecision < Precision)
            {
                repr = normalized;
                repr <<= Precision - currentPrecision;
            }
            else
                repr = normalized;
        }

        //only allow conversion with 1 template param changed. when both params are changed say ufx<B1,P1>x and ufx<B2,P2>y when sizeof(B1)>sizeof(B2) and P1>P2, then x.repr=y.repr<<(P1-P2) can have different value then x.repr=B1(y.repr)<<(P1-P2). Thus let user decide which one they want.
        template <unsigned_integral B1>
        constexpr
        explicit ufx(ufx<B1, Precision> o)
            noexcept: repr(o.repr)
        {
        }

        template <uint8_t P1>
        constexpr
        explicit ufx(ufx<Bone, P1> o)
            noexcept: repr(o.repr)
        {
            if constexpr (Precision > P1)
                repr <<= Precision - P1;
            else
                repr >>= P1 - Precision;
        }

        strong_ordering operator<=>(const ufx&) const = default;

        template <unsigned_integral B1>
        constexpr
        strong_ordering operator<=>(ufx<B1, Precision> o) const
            noexcept
        {
            return repr <=> o.repr;
        }

        template <unsigned_integral B1>
        constexpr
        bool operator ==(ufx<B1, Precision> o) const
            noexcept
        {
            return repr == o.repr;
        }

        constexpr
        strong_ordering operator<=>(fx<make_signed_t<Bone>, Precision> o) const noexcept
        {
            if (cmp_less(repr, o.repr))
                return strong_ordering::less;
            else if (cmp_equal(repr, o.repr))
                return strong_ordering::equivalent;
            else
                return strong_ordering::greater;
        }

        //conversion to float point is always possible at the cost of some precision(not all) and fast, so it's implicit.
        constexpr
        operator float() const
            noexcept
        {
            return bit_cast<float>(toF32U(repr, Precision));
        }

        constexpr
        operator double() const
            noexcept
        {
            return bit_cast<double>(toF64U(repr, Precision));
        }

        Bone repr;
    };

    template <signed_integral Bone, uint8_t Precision> requires (testSize<Bone, Precision>)
    class fx
    {
        using U = make_unsigned_t<Bone>;

    public:
        fx(const Bone& v): repr(v)
        {
        }

        template <floating_point F>
        constexpr
        explicit fx(F v): repr(ufx<U, Precision>(v).repr)
        {
            Bone sign = v < 0;
            repr = (repr ^ -sign) + sign; //negate if <0.
        }

        strong_ordering operator <=>(const fx&) const = default;

        template <signed_integral B1>
        constexpr
        strong_ordering operator <=>(fx<B1, Precision> o) const
            noexcept
        {
            return repr <=> o.repr;
        }

        template <signed_integral B1>
        constexpr
        bool operator ==(fx<B1, Precision>& o) const
            noexcept
        {
            return repr == o.repr;
        }

        template <signed_integral B1>
        constexpr
        explicit fx(fx<B1, Precision> o)
            noexcept: repr(o.repr)
        {
        }

        template <uint8_t P1>
        constexpr
        explicit fx(fx<Bone, P1> o)
            noexcept: repr(ufx<U, Precision>(bit_cast<ufx<U, P1>>(o)).repr)
        {
        }

#define quickAbs(a,sign) (a-sign^-sign)//abs when we already know the sign.

        constexpr
        operator float() const
            noexcept
        {
            uint32_t sign = repr < 0;
            uint32_t r = toF32U<U>(quickAbs(repr, sign), Precision);
            return bit_cast<float>(sign << F32.exponentBits + F32.fractionBits | r);
        }

        constexpr
        operator double() const
            noexcept
        {
            uint64_t sign = repr < 0;
            uint64_t r = toF64U<U>(quickAbs(repr, sign), Precision);
            return bit_cast<double>(sign << F64.exponentBits + F64.fractionBits | r);
        }

        Bone repr;
    };
}
