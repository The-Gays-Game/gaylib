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
  //  auto b=a.pow(0);
  //  std::cout<<float(b)<<std::endl;
  //
  //  ufx<unsigned __int128,64> c(1.5f);
  //  auto d=c.pow(3);
  //  std::cout<<double(d)<<std::endl;

   //weird at 158,159,160.
   auto e=ufx<uint8_t,8,std::round_toward_zero>::raw(0xff);
   std::cout<<std::setprecision(9)<<double(e)<<std::endl;
   // auto f0=e.pow(158);
   // auto f1=f0*e;
   // auto f2=f1*e;
   // std::cout<<double(f0)<<' '<<double(f1)<<' '<<double(f2)<<std::endl;
   // std::cout<<double(e.pow(159))<<' '<<double(e.pow(160));
   for (uint16_t i=128;i<256;++i) {
     auto f=e.pow(i);
     std::cout<<i<<' '<<std::setprecision(9)<<double(f)<<std::endl;
   }


}
