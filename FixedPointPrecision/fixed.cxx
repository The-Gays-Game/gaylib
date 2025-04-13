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

//template<class T> concept IEEE754Size=;

template<class T>requires same_as<T,uint64_t>||same_as<T,uint32_t>
struct IEEE754 {
    using underlying=T;
    uint8_t exponentBits,fractionBits;
    uint16_t exponentBias;
    uint16_t constexpr truncateExponent(uint16_t exponent)const noexcept {
        return exponent&(UINT16_MAX>>(16-exponentBits));
    }
};
constexpr IEEE754<uint32_t> F32{8,23,127};
constexpr IEEE754<uint64_t>F64{11,52,1023};;

template<auto const fmt,unsigned_integral B,class U=typename decltype(fmt)::underlying>constexpr
U toFloatFmtU(B v,uint8_t precision)noexcept {
    constexpr uint8_t boneSize=sizeof(B)*CHAR_BIT;
    uint8_t leading0=countl_zero(v);
    v<<=leading0;//drop first bit
    v<<=1;//when leading0+1==boneSize, undefined.
    U fraction;
    if constexpr(boneSize>=fmt.fractionBits) //keep only first fractionBits bits.
        fraction=v>>(boneSize-fmt.fractionBits);
    else //lshift leading bit to fractionBit
        fraction=U(v)<<(fmt.fractionBits-boneSize);
    uint8_t exponent=boneSize-leading0-precision-1;
    return fmt.truncateExponent(exponent+fmt.exponentBias)<<fmt.fractionBits|fraction;
}
template<auto const fmt,signed_integral B,class U = typename decltype(fmt)::underlying>constexpr
U toFloatFmt(B v,uint8_t precision)noexcept {
    uint8_t sign=v<0;
    U r=toFloatFmtU<fmt,make_unsigned_t<B>>(abs(v),precision);
    return sign << fmt.exponentBits + fmt.fractionBits |r;
}
export{
    template<signed_integral Bone,uint8_t Precision> requires(sizeof(Bone)*CHAR_BIT>=Precision)
    class fx
    {
        public:
        fx(const Bone& v):v(v){}
        fx(const fx&o):v(o.v){}
        fx operator+(const fx& o)const noexcept
        {
            return fx(v+o.v);
        }
        constexpr explicit operator float ()const {
            return bit_cast<float>(toFloatFmt<F32>(v,Precision));
        }
        constexpr explicit operator double ()const {
            return bit_cast<double>(toFloatFmt<F64>(v,Precision));
        }
    private:
Bone v;
    };
}