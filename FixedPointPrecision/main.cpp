#include<cstdint>
#include <format>
#include<iostream>
#include<limits>
#include<iomanip>
//#define debug_arithmetic
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
}

int main()
{
    fx<int16_t,8,std::round_to_nearest>a(32),b(-0.5);
    std::cout<<double(a/b)<<" "<<(double(a)/double(b))<<std::endl;
}
