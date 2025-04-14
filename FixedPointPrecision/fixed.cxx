module;
#include<concepts>
#include<cstdint>
#include<climits>
#include<bit>
#include<tuple>
#include<type_traits>
#include<iostream>
export module fixed;
using namespace std;

constexpr struct
{
    uint8_t exponentBits,fractionBits,exponentBias;
}F32{8,23,127};
constexpr struct
{
    uint8_t exponentBits,fractionBits;
    uint16_t exponentBias;
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
    constexpr uint16_t exponentBitsMask=(1<<F64.exponentBits)-1;
    return uint64_t(exponent + F64.exponentBias &exponentBitsMask)<<F64.fractionBits|fraction;
}


template<signed_integral B>constexpr
uint32_t toF32(B v,uint8_t precision)noexcept {
    uint8_t sign=v<0;
    uint32_t r=toF32U<make_unsigned_t<B>>(abs(v),precision);
    return sign << F32.exponentBits + F32.fractionBits |r;
}

template<signed_integral B>constexpr
uint64_t toF64(B v,uint8_t precision)noexcept {
    uint64_t sign=v<0;
    uint64_t r=toF64U<make_unsigned_t<B>>(abs(v),precision);
    return sign << F64.exponentBits + F64.fractionBits |r;
}

export{
        template<unsigned_integral Bone,uint8_t Precision> requires(sizeof(Bone)
    *CHAR_BIT>=Precision)
    class ufx
    {
    public:
        ufx(const Bone& v):v(v){}
        explicit constexpr
        ufx(float v) noexcept{
            auto a=bit_cast<uint32_t>(v);
            uint8_t exponent=uint8_t(a>>F32.fractionBits)-F32.exponentBias;//ignore sign bit for now
            constexpr uint32_t fractionBitsMask=(1<<F32.fractionBits)-1;
            uint32_t normalized=(a&fractionBitsMask)|(fractionBitsMask+1);
            uint8_t currentPrecision=F32.fractionBits-exponent;
            if (currentPrecision>Precision)
            {
                this->v=normalized>>currentPrecision-Precision;
            }else if (currentPrecision<Precision)
            {
                this->v=normalized;
                this->v<<=Precision-currentPrecision;
            }else
            {
                this->v=normalized;
            }
            Bone sign=v<0;
            this->v=(this->v^-sign)+sign;//negate if <0 to be consistent with unsigned int behavior.
        }
        explicit constexpr
        ufx(double v) noexcept{
            auto a=bit_cast<uint64_t>(v);
            constexpr uint16_t exponentBitsMask=(1<<F64.exponentBits)-1;
            uint16_t exponent=(exponentBitsMask&(a>>F64.fractionBits))-F64.exponentBias;//ignore sign bit for now
            constexpr uint64_t fractionBitsMask=(uint64_t(1)<<F64.fractionBits)-1;
            uint64_t normalized=(a&fractionBitsMask)|(fractionBitsMask+1);
            uint16_t currentPrecision=F64.fractionBits-exponent;
            if (currentPrecision>Precision)
            {
                this->v=normalized>>currentPrecision-Precision;
            }else if (currentPrecision<Precision)
            {
                this->v=normalized;
                this->v<<=Precision-currentPrecision;
            }else
            {
                this->v=normalized;
            }
            Bone sign=v<0;
            this->v=(this->v^-sign)+sign;//negate if <0 to be consistent with unsigned int behavior.
        }
        template<unsigned_integral B1,uint8_t P1> constexpr explicit
        ufx(ufx<B1,P1> o)noexcept
        {
            auto a=common_type_t<Bone,B1>(o.v);
            if constexpr(Precision>P1)
                v=a<<Precision-P1;
            else if constexpr(Precision<P1)
                v=a>>P1-Precision;
            else
                v=a;
        }

        constexpr explicit operator float ()const {
            return bit_cast<float>(toF32U(v,Precision));
        }
        constexpr explicit operator double ()const {
            return bit_cast<double>(toF64U(v,Precision));
        }
        Bone v;
    private:

    };
    template<signed_integral Bone,uint8_t Precision> requires(sizeof(Bone)*CHAR_BIT>=Precision)
    class fx
    {
        public:
        fx(const Bone& v):v(v){}
        explicit constexpr fx(float v):v(ufx<make_unsigned_t<Bone>,Precision>
        (v).v) {

        }
        constexpr explicit operator float ()const {
            return bit_cast<float>(toF32(v,Precision));
        }
        constexpr explicit operator double ()const {
            return bit_cast<double>(toF64(v,Precision));
        }
    private:
Bone v;
    };

}