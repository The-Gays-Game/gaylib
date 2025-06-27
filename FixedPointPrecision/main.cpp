#pragma STDC FENV_ACCESS ON

#include <cmath>
#include<cstdint>
#include <format>
#include<iostream>
#include<limits>
#include<iomanip>
#include<cfenv>
#include "arithmetic.h"
import fixed;
void test0() {
  std::fesetround(FE_TOWARDZERO);
  int32_t a=1072625963;
  fx<int32_t,0>b(a);
  float c=std::ldexp(a,0);
  std::cout<<std::setprecision(9)<<float(b)<<" "<<c;
}
int main()
{
  auto a=fx<int16_t,0>::raw(12256);
  fx<int16_t,2>b(a);

  auto c=fx<int16_t,0>(b);
  std::cout<<std::format("{:16b}",uint16_t(a.repr))<<std::endl;
  std::cout<<std::format("{:16b}",uint16_t(b.repr))<<std::endl;
  std::cout<<std::format("{:16b}",uint16_t(c.repr))<<std::endl;
  std::cout<<float(a)<<std::endl;
  std::cout<<float(b)<<std::endl;
  std::cout<<float(c)<<std::endl;
}
