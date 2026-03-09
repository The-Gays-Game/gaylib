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
	// uint16_t a = uRoot2(uint16_t(65024), std::round_toward_zero);
	// std::cout<<a;
	std::fesetround(FE_TOWARDZERO);
	for (unsigned int i = 1; i <= NL<uint16_t>::max(); ++i) {
		uint16_t y = uRoot2(uint16_t(i), std::round_toward_zero);
		uint16_t t = std::lrint(std::sqrt(i));
		if (y!=t) {
			std::cout<<i<<' '<<y<<' '<<t<<std::endl;
			break;
		}

	}
}
