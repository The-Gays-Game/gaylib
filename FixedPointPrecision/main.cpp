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
  // ufx<uint16_t,8> a(1.5f);
  //  auto b=a.pow(3);
  //  std::cout<<float(b)<<std::endl;
  //
  // ufx<unsigned __int128,128> c(0.763f);
  // auto d=c.pow(1);
  // std::cout<<double(d)<<std::endl;

   ufx<uint16_t,16>e=ufx<uint16_t,16>::raw(31259);
   auto f=e.pow(4);
   std::cout<<f.repr<<std::endl;

   unsigned __int128 a=e.repr;
   std::cout<<uint16_t(intPow<unsigned __int128>(a,uint8_t(4),1)>>48);
}
