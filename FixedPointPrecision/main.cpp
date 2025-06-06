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
    fx<int16_t,4,std::round_to_nearest> a(-1,true),b(0.5);
    a/=b;
    fx<int16_t,3,std::round_to_nearest> c(a);
    std::cout<<((double)a);
}
