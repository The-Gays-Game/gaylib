#pragma STDC FENV_ACCESS ON
#include "arithmetic.h"

#include <catch2/catch_get_random_seed.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <cfenv>
#include <cmath>
#include <random>
#include<utility>
#include<bit>
import fixed;
namespace {
constexpr int styleMacroMap[4]{FE_TOWARDZERO, FE_TONEAREST, FE_UPWARD, FE_DOWNWARD};
constexpr std::float_round_style styleEnumMap[4]{
    std::round_toward_zero, std::round_to_nearest, std::round_toward_infinity, std::round_toward_neg_infinity};
constexpr auto styleEnumSeq = std::integer_sequence<int8_t, std::round_toward_zero, std::round_to_nearest, std::round_toward_infinity, std::round_toward_neg_infinity>{};
static_assert(std::size(styleEnumMap) == std::size(styleMacroMap));
std::minstd_rand rg32;

template <class, uint8_t, std::float_round_style>
struct intToFpn {
};

template <test_Tsint T0, uint8_t V0, std::float_round_style V1>
struct intToFpn<T0, V0, V1> {
  using type = fx<T0, V0, V1>;
};

template <test_Tuint T0, uint8_t V0, std::float_round_style V1>
struct intToFpn<T0, V0, V1> {
  using type = ufx<T0, V0, V1>;
};
} // namespace
template <std::integral T, T... Ints>
using IntSeq = std::integer_sequence<T, Ints...>;
TEMPLATE_TEST_CASE("no float round 16", "", int16_t, uint16_t) {
  using Tt = rankOf<TestType>::two;
  SECTION("toF") {
    auto mode = GENERATE(range(size_t{0}, std::size(styleEnumMap)));
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
  constexpr auto radixes = std::make_integer_sequence<uint8_t, NL<TestType>::digits>{};
  SECTION("ctor<f>") {
    for (Tt i = NL<TestType>::min(); i <= NL<TestType>::max(); ++i) {
      using resT = std::array<TestType, NL<TestType>::digits>;
      const auto ys = [=]<uint8_t ... radix>(IntSeq<uint8_t, radix...>)-> resT {
        return {(typename intToFpn<TestType, radix, std::round_toward_zero>::type(i).repr)...};
      }(radixes);
      const auto ts = [=]<uint8_t ... radix>(IntSeq<uint8_t, radix...>)-> resT {
        return {(TestType(std::ldexpf(i, radix)))...};
      }(radixes);
      for (size_t j = 0; j < std::size(ys); ++j) {
        CAPTURE(j, i);
        REQUIRE(ts[j]==ys[j]);
      }
    }
  }
  SECTION("ctor change radix") {
    using Tu = std::make_unsigned_t<TestType>;
    [=]<uint8_t ... r0>(IntSeq<uint8_t, r0...>) {
      ([]<uint8_t ... r1>(IntSeq<uint8_t, r1...>, auto radix0) {
        ([radix0]<int8_t ... ss>(IntSeq<int8_t, ss...>, auto radix1) {
          ([radix0,radix1](auto s) {

            constexpr auto se = static_cast<std::float_round_style>(s());
            using A = intToFpn<TestType, radix0, se>::type;
            using B = intToFpn<TestType, radix1, se>::type;
            auto a = A::raw(rg32());
            auto b = B(a);
            auto c = ufx<Tu, radix0, std::round_indeterminate>::raw(a.repr);
            auto d = ufx<Tu, radix1, std::round_indeterminate>(c);
            CAPTURE(a.repr, b.repr, radix0, radix1, se);
            if ((std::is_unsigned_v<TestType> || c.repr >> NL<Tu>::digits - 1 == d.repr >> NL<Tu>::digits - 1) && std::popcount(Tu(a.repr)) == std::popcount(d.repr)) {
              REQUIRE(A(b).repr==a.repr);
              REQUIRE(static_cast<float>(a)==static_cast<float>(b));
            } else {
              if (radix1 > radix0) {
                REQUIRE(b.repr==TestType(a.repr<<(radix1-radix0)));
              } else if (radix1 < radix0) {
                REQUIRE(b.repr==rnd(a.repr,radix0-radix1,se));
              }
            }

          }(std::integral_constant<int8_t, ss>{}), ...);
        }(styleEnumSeq, std::integral_constant<uint8_t, r1>{}), ...);
      }(radixes, std::integral_constant<uint8_t, r0>{}), ...);
    }(radixes);
  }
}

/*
[=]<uint8_t r0,uint8_t r1,int8_t s>(){
  using A=intToFpn<TestType,r0,static_cast<std::float_round_style>(s)>;
  using B=intToFpn<TestType,r1,static_cast<std::float_round_style>(s)>;
  auto a=A::raw(rg32());
  auto b=B(a);
  if (std::popcount(a.repr)==std::popcount(b.repr)) {
    REQUIRE(static_cast<float>(a)==static_cast<float>(b));
    REQUIRE(A(b).repr==a.repr);
  }else {
    if (r1>r0) {
      REQUIRE(b.repr==a.repr<<(r1-r0));
    }else if (r1<r0) {
      REQUIRE(b.repr==rnd(a.repr,r0-r1,s));
    }
  }
}*/
// TEMPLATE_PRODUCT_TEST_CASE_SIG("fpn round 16","",(((class T,std::float_round_style S),T,S),(int16_t,uint16_t),(std::round_toward_zero, std::round_to_nearest, std::round_toward_infinity, std::round_toward_neg_infinity))) {
//
// }
TEMPLATE_TEST_CASE("float round 32", "", int32_t, uint32_t) {
  rg32.seed(Catch::getSeed());
  SECTION("toF") {
    auto mode = GENERATE(range(size_t{0}, std::size(styleEnumMap)));
    std::fesetround(styleMacroMap[mode]);
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