#include<cstdint>
#include <format>
#include<iostream>
#include<limits>
#include<iomanip>
#define debug_arithmetic
#include "arithmetic.h"
import fixed;
template <class Y,class T>
static void printPair(T a)
{
    auto b=Y(a,true),c=Y(-a,true);
    std::cout<<std::setprecision(9)<<float(b)<<" "<<float(c)<<std::endl;
}
template <class Y,class T>
static void printPair1(T a)
{
    auto b=Y(a),c=Y(-a);
    std::cout<<std::format("{:b}",b.repr)<<" "<<std::format("{:b}",c.repr)<<std::endl;
}
void test0()
{
    std::cout<<"A  ";
    printPair<fx<int8_t,4>>(0b10010);

    std::cout<<"B  ";
    printPair<fx<int32_t,11,std::round_toward_neg_infinity>>(1);

    std::cout<<"C  ";
    printPair<fx<int32_t,1,std::round_toward_neg_infinity>>(0b1111111111111111111111111);

    std::cout<<"D  ";
    printPair<fx<int32_t,1,std::round_toward_zero>>((1<<26));
    const auto t=1<<26;
}
void test2()
{
    std::cout<<"A  ";
    printPair1<fx<int8_t,4>>(1.125f);
    printPair1<ufx<uint8_t,4>>(1.125f);

    std::cout<<"B  ";
    printPair1<fx<int32_t,11,std::round_toward_neg_infinity>>(0.00048828125f);

    std::cout<<"C  ";
    printPair1<fx<int32_t,1,std::round_toward_neg_infinity>>(33554431);

    std::cout<<"D  ";//33554432.f
    printPair1<fx<int32_t,1,std::round_toward_infinity>>(float(1<<26));
}
void test3() {
    fx<int16_t,4> a(0b10010,true);
    std::cout<<"A "<<float(a)<<std::endl;
    fx<int16_t,2>b(a);
    std::cout<<"B "<<float(b)<<std::endl;
}

int main()
{
    int16_t a=3000,b=INT16_MIN;
    auto x=wideMul(a,b);
    int32_t c0=x.merge();
    int32_t c1=int32_t(a)*int32_t(b);
    std::cout<<c0<<" "<<c1<<std::endl;

    constexpr uint32_t d=403890;
    constexpr uint16_t e=0x8100,f=0;
    constexpr auto t=narrow2Div<uint16_t>(aint_dw<uint16_t>(d),e);
    uint16_t g1=d/uint32_t{e},r1=d%e;
    auto [g0,r0]=t;
    std::cout<<g0<<" "<<g1<<std::endl;
    std::cout<<r0<<" "<<r1<<std::endl;
}
