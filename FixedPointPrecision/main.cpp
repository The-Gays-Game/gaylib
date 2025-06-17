#include <cmath>
#include<cstdint>
#include <format>
#include<iostream>
#include<limits>
#include<iomanip>
#include "arithmetic.h"
import fixed;
void test0()
{
    int16_t a=5,b=-8;
    int16_t c=lsDivRnd(a,b,0,std::round_to_nearest);

    std::cout<<int16_t(c)<<" "<<std::endl;
}
void test1() {
    fx<__int128, 127,std::round_to_nearest>a(0.000001f);
    float b=a;
    std::cout<<testSize<__int128,127><<std::endl;
    std::cout<<b<<" "<<std::isnormal(std::ldexp(1,-128))<<std::endl;
    int32_t e=std::numeric_limits<int32_t>::max();
    e<<=31;
    using C=std::common_type_t<int64_t,uint16_t>;
    std::cout<<divRnd(e,e,std::round_to_nearest)<<std::endl;
}
int main()
{
    int8_t a=-1;
    int16_t b=a;
    uint16_t c=a;
    std::cout<<int16_t(a)<<" "<<b<<" "<<uint16_t(a)<<std::endl;
}
