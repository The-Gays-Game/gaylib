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
void test1() {
  using A=fx<int16_t,10,std::round_to_nearest>;
  std::fesetround(FE_TONEAREST);
  auto a=A::raw(30656),b=A::raw(1443);
  float af=a,bf=b;
  std::cout<<std::setprecision(9)<<float(af)<<" "<<float(bf)<<std::endl;
  auto c=a/b;
  auto cf=af/bf;
  std::cout<<std::setprecision(9)<<float(c)<<" "<<std::setprecision(9)<<std::ldexp(double(af)/double(bf),10)<<std::endl;

  auto d=A::br(cf*(1<<10));
  std::cout<<std::setprecision(9)<<float(A(cf))<<" "<<std::lrint(cf*(1<<10))<<std::endl;
  std::cout<<std::format("{:16b} {:16b}",c.repr,d.repr)<<std::endl;
  std::cout<<c.repr<<" "<<d.repr<<std::endl;
}
int main()
{
  __int128 a=-1;
  std::cout<<std::unsigned_integral<unsigned __int128><<std::endl;
}
