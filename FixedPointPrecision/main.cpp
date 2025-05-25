#include<cstdint>
#include <format>
#include<iostream>
#include<limits>
#include<iomanip>
#include "arithmetic.h"
import fixed;
void test4()
{
    int16_t a=3000,b=INT16_MIN;
    auto x=wideMul(a,b);
    int32_t c0=x.merge();
    int32_t c1=int32_t(a)*int32_t(b);
    std::cout<<c0<<" "<<c1<<std::endl;

    constexpr uint32_t d=0x9ffff;
    constexpr uint16_t e=0x8001,f=0;
    constexpr auto t=uNarrow211Div<uint16_t>(aint_dw<uint16_t>(d),e);
    uint16_t g1=d/uint32_t{e},r1=d%e;
    auto [g0,r0]=t;
    std::cout<<g0<<" "<<g1<<std::endl;
    std::cout<<r0<<" "<<r1<<std::endl;

    fx<int16_t,8,std::round_to_nearest>i(32),j(-0.5);
    std::cout<<double(i/j)<<" "<<(double(i)/double(j))<<std::endl;
}

int main()
{
    ufx<uint16_t,8,std::round_to_nearest> a(4.8),b(39);
    double c0=a*b;
    double c1=double(a)*double(b);
    std::cout<<std::setprecision(10)<<c0<<" "<<c1<<std::endl;
    std::cout<<double(a)<<" "<<double(b)<<std::endl;


    // aint_dw<uint8_t> a(1,2);
    // a<<=0;
    // std::cout<<int(a.h)<<" "<<int(a.l)<<std::endl;
}
