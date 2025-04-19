module;
#include<concepts>
#include<cstdint>
#include<climits>
#include<bit>
#include<iostream>
export module fixed;
using namespace std;

constexpr struct//https://en.wikipedia.org/wiki/Single-precision_floating-point_format#IEEE_754_standard:_binary32
{
    uint8_t exponentBits,fractionBits,exponentBias;
}F32{8,23,127};
constexpr struct//https://en.wikipedia.org/wiki/Double-precision_floating-point_format#IEEE_754_double-precision_binary_floating-point_format:_binary64
{
    uint8_t exponentBits,fractionBits;
    uint16_t exponentBias;
    constexpr uint16_t maskExponent(uint16_t exponent)const noexcept
    {
        uint16_t exponentBitsMask=(1<<exponentBits)-1;
        return exponent&exponentBitsMask;
    }
}F64{11,52,1023};

template<unsigned_integral B>constexpr
uint32_t toF32U(B v,uint8_t precision)noexcept {
    constexpr uint8_t boneSize=sizeof(B)*CHAR_BIT;
    uint8_t leading0=countl_zero(v);
    v<<=leading0;//drop first bit
    v<<=1;//when leading0+1==boneSize, undefined.
    uint32_t fraction;
    if constexpr(boneSize>=F32.fractionBits) //keep only first fractionBits bits.
        fraction=v>>(boneSize-F32.fractionBits);
    else //lshift leading bit to fractionBit
        fraction=U(v)<<(F32.fractionBits-boneSize);
    uint8_t exponent=boneSize-leading0-precision-1;
    return uint8_t(exponent+F32.exponentBias)<<F32.fractionBits|fraction;
}

template<unsigned_integral B>constexpr
uint64_t toF64U(B v, uint8_t precision)noexcept {
    constexpr uint8_t boneSize=sizeof(B)*CHAR_BIT;
    uint8_t leading0=countl_zero(v);
    v<<=leading0;//drop first bit
    v<<=1;//when leading0+1==boneSize, undefined.
    uint64_t fraction;
    if constexpr(boneSize>=F64.fractionBits) //keep only first fractionBits bits.
        fraction=v>>(boneSize-F64.fractionBits);
    else //lshift leading bit to fractionBit
        fraction=U(v)<<(F64.fractionBits-boneSize);
    uint8_t exponent=boneSize-leading0-precision-1;
    return uint64_t(F64.maskExponent(exponent + F64.exponentBias) )<<F64.fractionBits|fraction;
}

export{
# define testSize sizeof(Bone)*CHAR_BIT>=Precision
    template<signed_integral Bone,uint8_t Precision> requires(testSize)struct fx;

        template<unsigned_integral Bone,uint8_t Precision> requires(testSize)
    struct ufx
    {
        ufx(const Bone& v):v(v){}

            //conversion from float point can cause lost of all precision or defined behavior depending on exponent, so we make this explicit.
        constexpr
        explicit ufx(float v) noexcept{
            auto a=bit_cast<uint32_t>(v);
            uint8_t exponent=uint8_t(a>>F32.fractionBits)-F32.exponentBias;//ignore sign bit for now
            constexpr uint32_t fractionBitsMask=(1<<F32.fractionBits)-1;
            uint32_t normalized=(a&fractionBitsMask)|(fractionBitsMask+1);
            if (uint8_t currentPrecision=F32.fractionBits-exponent; currentPrecision>Precision)
                this->v=normalized>>currentPrecision-Precision;
            else if (currentPrecision<Precision)
            {
                this->v=normalized;//in case sizeof(Bone)>sizeof(float), try to retain more bits as possible.
                this->v<<=Precision-currentPrecision;
            }else
                this->v=normalized;
        }

        constexpr
        explicit ufx(double v) noexcept{
            auto a=bit_cast<uint64_t>(v);
            uint16_t exponent=F64.maskExponent(a>>F64.fractionBits)-F64.exponentBias;//ignore sign bit for now
            constexpr uint64_t fractionBitsMask=(uint64_t{1}<<F64.fractionBits)-1;
            uint64_t normalized=(a&fractionBitsMask)|(fractionBitsMask+1);
            if (uint16_t currentPrecision=F64.fractionBits-exponent; currentPrecision>Precision)
                this->v=normalized>>currentPrecision-Precision;
            else if (currentPrecision<Precision)
            {
                this->v=normalized;
                this->v<<=Precision-currentPrecision;
            }else
                this->v=normalized;
        }

        constexpr
            ufx(fx<make_signed_t<Bone>,Precision>a)noexcept:v(a.v){}

    //only allow conversion with 1 template param changed. when both params are changed say ufx<B1,P1>x and ufx<B2,P2>y when sizeof(B1)>sizeof(B2) and P1>P2, then x.v=y.v<<(P1-P2) can have different value then x.v=B1(y.v)<<(P1-P2). Thus let user decide which one they want.
        template<unsigned_integral B1>constexpr
            ufx(ufx<B1,Precision>o)noexcept:v(o.v){}

            template<uint8_t P1>constexpr
            ufx(ufx<Bone,P1>o)noexcept:v(o.v)
        {
            if constexpr(Precision>P1)
                v<<=Precision-P1;
            else
                v>>=P1-Precision;
        }

//conversion to float point is always possible at the cost of some precision(not all) and fast, so it's implicit.
        constexpr operator float ()const {
            return bit_cast<float>(toF32U(v,Precision));
        }
        constexpr operator double ()const {
            return bit_cast<double>(toF64U(v,Precision));
        }

        Bone v;
    };

    template<signed_integral Bone,uint8_t Precision> requires(testSize)
    struct fx
    {
    private:
        using U=make_unsigned_t<Bone>;
    public:
        fx(const Bone& v):v(v){}
        constexpr
        fx(ufx<U,Precision>a)noexcept:v(a.v){}
        template<floating_point F>explicit constexpr
        fx(F v):v(ufx<U,Precision>(v).v) {
            Bone sign=v<0;
            this->v=(this->v^-sign)+sign;//negate if <0.
        }

        template<signed_integral B1>constexpr
        fx(fx<B1,Precision>o)noexcept:v(o.v){}
        template<uint8_t P1>constexpr
        fx(fx<Bone,P1>o)noexcept:v(ufx<U,Precision>(ufx<U,P1>(o)).v){}
#define quickAbs(a,sign) (a-sign^-sign)//abs when we already know the sign.
        constexpr operator float ()const {
            uint32_t sign=v<0;
            uint32_t r=toF32U<U>(quickAbs(v,sign),Precision);
            return bit_cast<float>(sign << F32.exponentBits + F32.fractionBits |r);
        }
        constexpr operator double ()const {
            uint64_t sign=v<0;
            uint64_t r=toF64U<U>(quickAbs(v,sign),Precision);
            return bit_cast<double>(sign << F64.exponentBits + F64.fractionBits |r);
        }
        Bone v;
    };

}