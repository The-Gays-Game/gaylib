#pragma STDC FENV_ACCESS ON
#include "cmake-build-debug-gcc/_deps/catch2-src/src/catch2/generators/catch_generators_range.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include<cfenv>
import fixed;
TEST_CASE("toF") {
  auto mode=GENERATE(Catch::Generators::RangeGenerator<size_t>(0,std::size(styleMacroMap)));
    std::fesetround(styleMacroMap[mode]);
    SECTION("general") {


  }
}