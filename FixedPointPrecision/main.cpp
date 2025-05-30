#include<cstdint>
#include <format>
#include<iostream>
#include<limits>
#include<iomanip>
#include "arithmetic.h"
import fixed;
void test0()
{
    aint_dw<int8_t> a(int16_t{28});
    uint8_t b=3;

    std::cout<<int16_t(a.narrowRSr(b,std::round_to_nearest))<<" "<<std::endl;
}
int main()
{
    test0();
    // ufx<uint16_t,16,std::round_toward_infinity> a(0.25),b(0.75);
    // double c0=a/b;
    // double c1=double(a)/double(b);
    // std::cout<<std::setprecision(10)<<c0<<" "<<c1<<std::endl;
    // std::cout<<double(a)<<" "<<double(b)<<std::endl;


    // aint_dw<uint8_t> a(1,2);
    // a<<=0;
    // std::cout<<int(a.h)<<" "<<int(a.l)<<std::endl;
}
