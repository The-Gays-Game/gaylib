#pragma once
#include<version>
#if defined(__GLIBCXX__) || (defined(__cpp_lib_constexpr_cmath)&&__cpp_lib_constexpr_cmath>=202202L)
  #define CMATH_CE23 constexpr
#else
  #define CMATH_CE23
#endif

#if defined(__GLIBCXX__) || (defined(__cpp_lib_constexpr_cmath)&&__cpp_lib_constexpr_cmath>=202306L)
  #define CMATH_CE26 constexpr
#else
  #define CMATH_CE26
#endif

#define HAS_CONTENTS(...) 0 __VA_OPT__(+ 1)

#ifndef __has_builtin
  #define __has_builtin(x) 0
#endif

#ifndef __has_constexpr_builtin
  #define __has_constexpr_builtin(x) 0
#endif



#if !__has_builtin(__builtin_assume)
  #ifdef NDEBUG
    #define assOrAss(x)
  #else
    #define assOrAss assert
  #endif
  #define __builtin_assume(a)
#else
  #ifdef NDEBUG
    #define assOrAss __builtin_assume
  #else
    #define assOrAss assert
  #endif
#endif


#if !__has_builtin(__builtin_expect_with_probability)
  #define __builtin_expect_with_probability(cond,val,prob) cond
#endif

#ifdef checkArgs
  #include<stdexcept>
#endif

#ifdef NDEBUG
  #define dbgHelperExport
#else
  #define dbgHelperExport export
#endif



