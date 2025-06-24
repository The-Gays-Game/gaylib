#pragma STDC FENV_ACCESS ON
#include "arithmetic.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include<catch2/generators/catch_generators_range.hpp>
#include<cfenv>
#include<cmath>
import fixed;
TEMPLATE_TEST_CASE("general","",int16_t,uint16_t) {
  auto mode=GENERATE(range(size_t{0},std::size(styleEnumMap)));
    std::fesetround(styleMacroMap[mode]);
  using Tt=rankOf<TestType>::two;
    SECTION("general") {
      for (Tt i=NL<TestType>::min();i<=NL<TestType>::max();++i){
        const TestType repr=i;
        for (uint8_t radix=0;radix<=NL<TestType>::digits;++radix) {
          CAPTURE(repr,radix,styleMacroMap[mode]);
          float t=std::ldexpf(repr,-radix);
          float y=e_toF<float>(repr,radix,styleEnumMap[mode]);
          REQUIRE(t==y);
        }
      }
  }
}