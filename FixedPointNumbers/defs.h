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

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#define ARCH_x86 64
#elif defined(__i386) || defined(__i386__) || defined(__IA32__) || defined(_M_I86) || defined(_M_IX86) || defined(__X86__) || defined(_X86_)
#define ARCH_x86 32
#elif defined(__arm__) || defined(_M_ARM) || defined(_ARM)
#define ARCH_ARM 32
#elif defined(__ARM64_ARCH_8_32__)||defined(__aarch64__) || defined(_M_ARM64)
#define ARCH_ARM 64
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



