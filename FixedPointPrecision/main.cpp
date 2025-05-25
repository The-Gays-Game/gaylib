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
    int16_t a=-32768;
    aint_dw<int8_t>b(a);
    int8_t d=8;
    int8_t c0=b.narrowRS(d);
    int8_t c1=a>>d;
    std::cout<<int(c0)<<" "<<int(c1)<<std::endl;
}
