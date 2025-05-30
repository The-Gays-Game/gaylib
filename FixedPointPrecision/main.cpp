#include<cstdint>
#include <format>
#include<iostream>
#include<limits>
#include<iomanip>
#include "arithmetic.h"
import fixed;
void test0()
{
    int16_t a=-4,b=-8;
    int16_t c=divr(a,b,0,std::round_to_nearest);

    std::cout<<int16_t(c)<<" "<<std::endl;
}
int main()
{
    //test0();
    ufx<uint16_t,16,std::round_to_nearest> a(0xffff,true),b(0.78);
    double c0=a*b;
    double c1=double(a)*double(b);
    std::cout<<std::setprecision(10)<<c0<<" "<<c1<<std::endl;
    std::cout<<double(a)<<" "<<double(b)<<std::endl;


    // aint_dw<uint8_t> a(1,2);
    // a<<=0;
    // std::cout<<int(a.h)<<" "<<int(a.l)<<std::endl;
}
