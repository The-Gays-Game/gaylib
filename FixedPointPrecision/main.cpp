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
  constexpr uint8_t explicitDigs=NL<float>::digits-1;
  uint32_t a=0b0'111111111001110111111111;
  float cvt = std::bit_cast<float>(uint32_t(NL<float>::max_exponent - 1+explicitDigs) << explicitDigs | uint32_t(a) & ~(1 << explicitDigs));
  std::cout<<a<<" "<<std::setprecision(10)<<cvt<<std::endl;
}
void test2() {
  double a=0.1;
  uint64_t b=fromF<uint64_t>(a,0);
  std::cout<<b<<" "<<std::setprecision(10)<<a;
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
  test0();
  test2();
}
