#pragma STDC FENV_ACCESS ON

#include <cmath>
#include<cstdint>
#include <format>
#include<iostream>
#include<limits>
#include<iomanip>
#include<cfenv>
#include<chrono>
#include<random>
#include<version>
#include "defs.h"
 import fpn;
using namespace fpn::core;
#define STRINGIZE_(x) #x
std::ostream&
operator<<( std::ostream& dest, __int128 value )
{
  std::ostream::sentry s( dest );
  if ( s ) {
    unsigned __int128 tmp = value < 0 ? -value : value;
    char buffer[ 128 ];
    char* d = std::end( buffer );
    do
    {
      -- d;
      *d = "0123456789"[ tmp % 10 ];
      tmp /= 10;
    } while ( tmp != 0 );
    if ( value < 0 ) {
      -- d;
      *d = '-';
    }
    int len = std::end( buffer ) - d;
    if ( dest.rdbuf()->sputn( d, len ) != len ) {
      dest.setstate( std::ios_base::badbit );
    }
  }
  return dest;
}
std::ostream&
operator<<( std::ostream& dest, unsigned __int128 value )
{
  std::ostream::sentry s( dest );
  if ( s ) {
    unsigned __int128 tmp = value;
    char buffer[ 128 ];
    char* d = std::end( buffer );
    do
    {
      -- d;
      *d = "0123456789"[ tmp % 10 ];
      tmp /= 10;
    } while ( tmp != 0 );
    int len = std::end( buffer ) - d;
    if ( dest.rdbuf()->sputn( d, len ) != len ) {
      dest.setstate( std::ios_base::badbit );
    }
  }
  return dest;
}
int main() {
  using u128=unsigned __int128;
  using i128=unsigned __int128;
  // uint64_t base=std::numeric_limits<uint64_t>::max();
  // uint8_t radix=63;
  // uint64_t a=sqrt(base,radix,std::round_toward_zero);
  // uint64_t b=uRoot2<u128>(u128(static_cast<u128>(base) <<radix),std::round_toward_zero);
  // std::cout<<a<<' '<<b<<std::endl;
  // i128 c=(i128(base)<<radix)-i128(a)*a,d=(i128(base)<<radix)-i128(b)*b;
  // std::cout<<c<<' '<<d<<std::endl;
  // std::cout<<(i128(base)<<radix)<<' '<<i128(a)*a<<' '<<i128(b)*b<<std::endl;
  float a=1;
  a=std::numeric_limits<float>::max();
  int e;
  a=std::frexpf(a,&e);
  std::cout<<a<<' '<<e;
}
