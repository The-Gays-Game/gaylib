#pragma STDC FENV_ACCESS ON

#include <cmath>
#include<cstdint>
#include <format>
#include<iostream>
#include<limits>
#include<iomanip>
#include<cfenv>
#include "arithmetic.h"
#include<chrono>
#include<random>
 import fixed;
uint32_t side;
std::mt19937 rg;
/*void test0(size_t a,size_t b) {
  ufx<uint32_t,16> c;
  c.repr=rg();
  for (size_t i=0;i<a;++i) {
    c=c.pow(b);
  }
  side=c.repr;
}
void test1(size_t a,size_t b) {
  ufx<uint32_t,16> c;
  c.repr=rg();
  for (size_t i=0;i<a;++i) {
    c=intPow(c,b,ufx<uint32_t,16>(1));
  }
  side=c.repr;
}
 void time() {
  const size_t n=10000,e=1024,all=10000;
  uint64_t t=0;
  for (int i=0;i<all;++i) {
    auto begin=std::chrono::steady_clock::now();
    test0(n,rg()%e);
    t+=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()- begin).count();
  }
  std::cout<<t/all<<std::endl;
  t=0;
  for (int i=0;i<all;++i) {
    auto begin=std::chrono::steady_clock::now();
    test1(n,rg()%e);
    t+=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()- begin).count();
  }
  std::cout<<t/all<<std::endl;
  std::cout<<side;
 }*/
int main()
{
  aint_dt<int8_t> b(int16_t(-129));
  std::cout<<(b-=uint8_t(255)).merge()<<std::endl;
  using A=ufx<uint16_t,0,std::round_toward_infinity>;
  A a(6);
  //std::cout<<(uint64_t(a.sqrt().repr))<<std::endl;
  std::cout<<uRoot2(uint16_t{7},std::round_to_nearest);
}
