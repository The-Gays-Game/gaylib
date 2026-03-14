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
long double sqrtBranchProb(uint8_t D)//D is half total width.
noexcept {
	using ld=long double;

	const ld e0=std::ldexpl(1,2*D+1);
	const double e1=std::ldexp(1,D);
	const ld a=std::floorl(std::sqrtl(e0-e1));
	ld b=a*(a+1)*(2*a+1)/6;
	ld c=(e1*2-a)*(e0-e1);
	ld d=std::ldexpl(1,3*D+2)-e1;
	const ld rawProb=(b+c)/d;

	const ld seriesMean=(1-std::ldexpl(0.5,-D))*2/(D+1);//more right shifts == less likely to overestimate. shift count is half total width.
	return rawProb*seriesMean;
}
int main() {
  using u128=unsigned __int128;
  using i128=unsigned __int128;
	// uint16_t a = uRoot2(uint16_t(65024), std::round_toward_zero);
	// std::cout<<a;
	std::cout<<sqrt(1u,0,std::round_to_nearest);
}
