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
  test0();
}
