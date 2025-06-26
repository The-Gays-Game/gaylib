#pragma STDC FENV_ACCESS ON
#include "arithmetic.h"
#include "catch2/catch_get_random_seed.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <cfenv>
#include <cmath>
#include <random>
import fixed;
namespace {
constexpr int styleMacroMap[4]{FE_TOWARDZERO, FE_TONEAREST, FE_UPWARD, FE_DOWNWARD};
constexpr std::float_round_style styleEnumMap[4]{
    std::round_toward_zero, std::round_to_nearest, std::round_toward_infinity, std::round_toward_neg_infinity};
static_assert(std::size(styleEnumMap) == std::size(styleMacroMap));
std::minstd_rand rg32;
} // namespace

TEMPLATE_TEST_CASE("no round", "", int16_t, uint16_t) {
  auto mode = GENERATE(range(size_t{0}, std::size(styleEnumMap)));
  using Tt = rankOf<TestType>::two;
  SECTION("toF") {
    for (Tt i = NL<TestType>::min(); i <= NL<TestType>::max(); ++i) {
      const TestType repr = i;
      for (int8_t radix = 0; radix <= NL<TestType>::digits; ++radix) {
        CAPTURE(repr, radix, styleEnumMap[mode]);
        float t = std::ldexpf(repr, -radix);
        float y = toF<float>(repr, radix, styleEnumMap[mode]);
        REQUIRE(t == y);
      }
    }
  }
}

TEMPLATE_TEST_CASE("round", "", int32_t, uint32_t) {
  rg32.seed(Catch::getSeed());
  auto mode = GENERATE(range(size_t{0}, std::size(styleEnumMap)));
  std::fesetround(styleMacroMap[mode]);
  SECTION("toF") {
    for (int8_t radix = 0; radix <= NL<TestType>::digits; ++radix) {
      for (uint32_t i = 0; i < uint32_t{1 << 15}; ++i) {
        TestType repr = rg32();
        CAPTURE(repr, radix, styleEnumMap[mode]);
        float t = std::ldexp(repr, -radix);
        float y = toF<float>(repr, radix, styleEnumMap[mode]);
        REQUIRE(t == y);
      }
    }
  }
}
TEMPLATE_TEST_CASE("large", "", __int128, unsigned __int128) {
  rg32.seed(Catch::getSeed());
  auto mode = GENERATE(range(size_t{0}, std::size(styleEnumMap)));
  std::fesetround(styleMacroMap[mode]);
  SECTION("toF") {
    for (int16_t radix = 0; radix <= NL<TestType>::digits; ++radix) {
      for (TestType i = 0; i < 1 << 8; ++i) {
        /*
         *we can't test values near NL<TestType>::max() because we can only control rounding for conversion between floating point types, conversion from int types to floating
         *point types is unspecified, so we first convert to double, hoping it can get an exact representation, then convert to float in a controlled manner, but int128 and friends
         *can't be represented in double, let alone 80bit long double(which doesn't exist on MSVC). therefore we only test small values. This also fits our need, we just need to makre
         *sure subnormal part is working in toF.
         */
        CAPTURE(i, radix, styleEnumMap[mode]);
        float t = std::ldexp(i, -radix);
        float y = toF<float>(i, radix, styleEnumMap[mode]);
        REQUIRE(t == y);
        if constexpr (std::is_signed_v<TestType>) {
          CAPTURE(-i, radix, styleEnumMap[mode]);
          t = std::ldexp(-i, -radix);
          y = toF<float>(-i, radix, styleEnumMap[mode]);
          REQUIRE(t == y);
        }
      }
    }
  }
}