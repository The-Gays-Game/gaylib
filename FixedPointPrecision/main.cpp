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
    int16_t c=lsDivRnd(a,b,0,std::round_to_nearest);

    std::cout<<int16_t(c)<<" "<<std::endl;
}
int main()
{
    //test0();
    fx<int16_t,15,std::round_to_nearest> a(0.4),b(-0.7),c(0.1);
    double c0=(a-c)/b;
    double c1=(double(a)-double(c))/double(b);
    std::cout<<std::setprecision(10)<<c0<<" "<<c1<<std::endl;
    std::cout<<double(a)<<" "<<double(b)<<std::endl;


    // aint_dw<uint8_t> a(1,2);
    // a<<=0;
    // std::cout<<int(a.h)<<" "<<int(a.l)<<std::endl;
}
