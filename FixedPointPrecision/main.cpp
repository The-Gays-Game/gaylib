#include <cmath>
#include<cstdint>
#include <format>
#include<iostream>
#include<limits>
#include<iomanip>
#include "arithmetic.h"
import fixed;
void test0() {
  int32_t a=1830390437;
  fx<int32_t,0>b(a);
  std::cout<<float(b);
}
int main()
{
  int16_t a=-32768;
  uint8_t b=8;
  std::cout<<rnd(a,b,std::round_toward_zero)<<std::endl;
  std::cout<<rnd(a,b,std::round_to_nearest)<<std::endl;
  std::cout<<rnd(a,b,std::round_toward_neg_infinity)<<std::endl;
  std::cout<<rnd(a,b,std::round_toward_infinity)<<std::endl;
}
