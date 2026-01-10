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
uint32_t branches[4]={0};
std::mt19937 rg;

template <std::unsigned_integral T>
constexpr
std::tuple<T, T> count
#if defined(__GNUG__)||defined(__clang__)
[[gnu::hot]]
#endif
(const aint_dt<T> &dividend, const T/*assume normalized*/ divisor) {
#if __has_builtin(__builtin_assume)
  __builtin_assume(__builtin_clzg(divisor)==0&&dividend.h<divisor);
#endif
  //using Th = rankOf<T>::half;
  constexpr uint8_t halfWidth = NL<T>::digits/2;

  const aint_dt<T> divisorSplit(divisor>>halfWidth,divisor& (1 << halfWidth) - 1), dividendLSplit(dividend.l>>halfWidth,dividend.l& (1 << halfWidth) - 1);
  aint_dt<T> q;

  T qhat = dividend.h / divisorSplit.h, rhat = dividend.h % divisorSplit.h;
  T c1 = qhat * divisorSplit.l, c2 = rhat << halfWidth | dividendLSplit.h;
  if (c1 > c2) {
    --qhat;
    ++branches[0];
    if (c1 - c2 > divisor) {
      --qhat;
      ++branches[1];
    }
  }
  q.h = qhat;

  T r = (dividend.h << halfWidth | dividendLSplit.h) - q.h * divisor;

  qhat = r / divisorSplit.h, rhat = r % divisorSplit.h;
  c1 = qhat * divisorSplit.l, c2 = rhat << halfWidth | dividendLSplit.l;
  if (c1 > c2) {
    ++branches[2];
    --qhat;
    if (c1 - c2 > divisor) {
      --qhat;
      ++branches[3];
    }
  }
  q.l = qhat;

  r = (r << halfWidth | dividendLSplit.l) - q.l * divisor;

  return {q.merge(), r};
}

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
  uint32_t total=0;
  for (uint16_t divisor=1<<7;divisor<=NL<uint8_t>::max();++divisor) {
    for (uint32_t dividend=0;dividend>>8<divisor;++dividend) {
      ++total;
      aint_dt<uint8_t> a(static_cast<uint16_t>(dividend));
      count(a, static_cast<uint8_t>(divisor));
    }
  }
  std::cout<<branches[0]<<' '<<branches[1]<<' '<<branches[2]<<' '<<branches[3]<<' '<<total<<std::endl;
  std::cout<<branches[0]/ static_cast<double>(total)<<' '
  <<branches[1]/ static_cast<double>(total)<<' '
  <<branches[2]/ static_cast<double>(total)<<' '
  <<branches[3]/ static_cast<double>(total)<<' ';
}
