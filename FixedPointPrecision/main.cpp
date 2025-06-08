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
int main()
{
    ufx<unsigned __int128, 128>a(0b1,true);
    double b=a;
    std::cout<<b<<" "<<std::isnormal(std::ldexp(1,-128))<<std::endl;
}
