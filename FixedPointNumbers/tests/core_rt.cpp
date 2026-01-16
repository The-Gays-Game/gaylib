#pragma STDC FENV_ACCESS ON
#include <bit>
#include <catch2/catch_get_random_seed.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <cfenv>
#include <cmath>
#include <cstdint>
#include <random>
#include <ranges>
#include <utility>
import fpn;
import helpers;
using namespace fpn::core;
using namespace fpn;
TEST_CASE("fast path") {
  SECTION("toF") {
    for (const auto _ : std::ranges::views::iota(0, 1 << 18)) {
      for (int8_t radix : std::ranges::views::iota(int8_t{0}, int8_t{64})) {
        uint64_t repr = uint64_t(rg32()) << 32 | rg32();
        double t = std::ldexp(repr, -radix);
        double y = toF<double>(repr, radix, std::round_indeterminate);
        REQUIRE(t == y);
      }
    }
  }
  SECTION("fromF") {
    for (const auto _ : std::ranges::views::iota(0, 1 << 18)) {
      for (int8_t radix : std::ranges::views::iota(int8_t{0}, int8_t{64})) {
        double a = std::bit_cast<double>(static_cast<uint64_t>(rg32() >> 1) << 32 | rg32());
        // CAPTURE(uint16_t(radix),a);
        const double fpnMax = toF<double>(NL<uint64_t>::max(), radix, std::round_indeterminate);
        if (a > fpnMax || a < 0 || std::isnan(a)) {
          continue;
        }
        uint64_t t = std::ldexp(a, radix);
        uint64_t y = fromF<uint64_t>(a, radix);

        REQUIRE(t == y);
      }
    }
  }
  SECTION("sqrt") {
    uint8_t si = GENERATE(range(size_t{0}, std::size(styleEnumMap)));
    std::fesetround(styleMacroMap[si]);
    uint8_t base=GENERATE(range(0,NL<uint8_t >::max()+1));
    //uint8_t radix = GENERATE(range(0, NL<TestType>::digits + 1));
    uint8_t  y=sqrt(base,0,styleEnumMap[si]);
    uint8_t  t=std::lrintf(std::sqrtf(base));
    REQUIRE(y==t);
  }
}
TEMPLATE_TEST_CASE("bone 8", "", int8_t, uint8_t) {
  using Tu = std::make_unsigned_t<TestType>;
  using Tl = std::conditional_t<std::is_unsigned_v<TestType>, uint64_t, int64_t>;
  using Tt = std::conditional_t<std::is_unsigned_v<TestType>, uint16_t, int16_t>;
  SECTION("chngRdx") {
    uint8_t si = GENERATE(range(size_t{0}, std::size(styleEnumMap)));
    std::fesetround(styleMacroMap[si]);
    for (uint8_t radix0 = 0; radix0 <= NL<TestType>::digits; ++radix0) {
      for (uint8_t radix1 = 0; radix1 <= NL<TestType>::digits; ++radix1) {
        for (int32_t i = NL<TestType>::min(); i <= NL<TestType>::max(); ++i) {
          TestType a = i;
          auto b = chngRdx(a, radix0, radix1, styleEnumMap[si]);
          //CAPTURE(a, b, radix0, radix1, styleEnumMap[si]);
          if (radix0 < radix1) {
            REQUIRE(b == TestType(a << (radix1 - radix0)));
          } else if (radix1 < radix0) {
            TestType t = std::lrint(std::ldexpf(a, radix1 - radix0));
            REQUIRE(b == t);
          } else {
            REQUIRE(a == b);
          }
        }
      }
    }
  }

  SECTION("div") {
    uint8_t si = GENERATE(range(size_t{0}, std::size(styleEnumMap)));
    std::fesetround(styleMacroMap[si]);
    uint8_t radix = GENERATE(range(0, NL<TestType>::digits + 1));
    const float big = toF<float>(NL<TestType>::max(), radix, std::round_indeterminate), small = toF<float>(NL<TestType>::min(), radix, std::round_indeterminate);
    TestType dividend = GENERATE(range(Tt{NL<TestType>::min()}, Tt{NL<TestType>::max() + 1}));
    for (uint16_t i = 1; i <= NL<uint8_t>::max(); ++i) {
      TestType divisor = TestType(i);
      float a = float(dividend) / divisor; // must make sure the float type here have more than 2*8 digits. because br uses lrint, which introduces another rounding.
      if (a >= small && a <= big) {
        //CAPTURE(dividend, divisor, radix);
        TestType y0 = div(dividend, divisor, radix, styleEnumMap[si]);
        TestType y1 = div<Tl>(dividend, divisor, radix, styleEnumMap[si]);
        TestType t = br<TestType>(a, radix);
        REQUIRE(y0 == t);
        REQUIRE(y1 == t);
      }
    }
  }

  SECTION("remquo") {
    TestType dividend = GENERATE(range(Tt{NL<TestType>::min()}, Tt{NL<TestType>::max() + 1}));
    for (uint16_t i = 1; i <= NL<uint8_t>::max(); ++i) {
      TestType divisor = TestType(i);
      int qt;
      float rt = std::remquof(dividend, divisor, &qt);
      TestType qy;
      std::make_signed_t<TestType> ry;
      using A = afx_t<TestType, 0, std::round_indeterminate>;
      if constexpr (std::is_unsigned_v<TestType>) {
        bool of;
        auto [a, b] = A::raw(dividend).remQuo(A::raw(divisor), &of);
        REQUIRE(of == (rt > NL<int8_t>::max()));
        qy = a, ry = b.repr;
      } else {
        auto [a, b] = A::raw(dividend).remQuo(A::raw(divisor));
        qy = a, ry = b.repr;
      }
      REQUIRE((qt & 7) == (qy & 7));
      REQUIRE(rt == ry);
    }
  }

  SECTION("mul") {
    uint8_t si = GENERATE(range(size_t{0}, std::size(styleEnumMap)));
    std::fesetround(styleMacroMap[si]);
    uint8_t radix = GENERATE(range(0, NL<TestType>::digits + 1));
    TestType left = GENERATE(range(Tt{NL<TestType>::min()}, Tt{NL<TestType>::max() + 1}));
    TestType right = GENERATE(range(Tt{NL<TestType>::min()}, Tt{NL<TestType>::max() + 1}));
    Tt a = Tt(left) * right;
    if (a >= Tt(NL<TestType>::min())<<8&& a <=Tt(NL<TestType>::max())<<8) {
      TestType y0 = mul(left, right, radix, styleEnumMap[si]);
      TestType y1 = mul<Tl>(left, right, radix, styleEnumMap[si]);
      TestType t = std::lrintf(std::ldexpf(a, -radix));
      //CAPTURE(left, right, radix, si);
      REQUIRE(y0 == t);
      REQUIRE(y1 == t);
    }
  }
}
TEST_CASE("edge cases") {
  using usht=unsigned short;
  using uint=unsigned int;
  SECTION("unsigned short *") {
    usht a=NL<usht>::max();
    usht t=uint(a)*a;
    usht y=mul(a,a,0,std::round_indeterminate);
    REQUIRE(t==y);
  }
  SECTION("div by 0") {
    REQUIRE_THROWS_AS(div(666,0,1,std::round_indeterminate),std::domain_error);
    using A = afx_t<int, 0, std::round_indeterminate>;
    REQUIRE_THROWS_AS(A::raw(666).remQuo(A::raw(0)),std::domain_error);
  }
}
TEMPLATE_TEST_CASE("bone 16", "", int16_t, uint16_t) {
  using Tt = std::conditional_t<std::is_unsigned_v<TestType>, uint32_t, int32_t>;
  SECTION("toF") {
    uint8_t mode = GENERATE(range(size_t{0}, std::size(styleEnumMap)));
    for (Tt i = NL<TestType>::min(); i <= NL<TestType>::max(); ++i) {
      const TestType repr = i;
      for (int8_t radix = 0; radix <= NL<TestType>::digits; ++radix) {
        //CAPTURE(repr, radix, styleEnumMap[mode]);
        float t = std::ldexpf(repr, -radix);
        float y = toF<float>(repr, radix, styleEnumMap[mode]);
        REQUIRE(t == y);
      }
    }
  }
  /*SECTION("pow") {
    constexpr auto radixes = std::make_integer_sequence<uint8_t, NL<TestType>::digits+1>{};
    if constexpr(std::is_unsigned_v<TestType>) {
      [radixes]<uint8_t ... sis>(IntSeq<uint8_t, sis...>) {
        ([]<uint8_t ... r0>(IntSeq<uint8_t, r0...>, auto si) {
          constexpr std::float_round_style se = styleEnumMap[si];
          std::fesetround(styleMacroMap[si]);
          ([se](auto radix0) {
            using A = intToFpn<TestType, radix0, se>::type;
            if constexpr(radix0==0) {
              for (uint16_t _=0;_<1024;++_) {
                auto a=rg32();
                TestType base=a,e=a>>16;
                CAPTURE(base,e);
                TestType t=APowU<uint32_t>(base,e,1);
                TestType y=A(base).pow(e).repr;
                REQUIRE(t==y);
              }
            }else if constexpr(radix0==NL<TestType>::digits) {
              for (uint32_t e=1;e<=NL<uint16_t>::max();++e) {
                //for (uint8_t j=0;j<1;++j) {
                  auto a=rg32();
                {
                  TestType base=a>>=16;
                  A t=  APowU(A::raw(base),e,withId<A>::one);
                  // A t=A::raw(base);
                  // for (uint32_t i=1;i<e;++i)
                  //   t*=A::raw(base);
                  A y=A::raw(base).pow(e);
                  CAPTURE(base,e,t.repr,y.repr);

                  REQUIRE(std::abs(int16_t(t.repr-y.repr))<2);
                }
                // {
                //   TestType base=a;
                //   A t=A::raw(base);
                //   for (uint32_t i=1;i<e;++i)
                //     t*=A::raw(base);
                //   A y=A::raw(base).pow(e);
                //   CAPTURE(t.repr,y.repr);
                //   REQUIRE(std::abs(int16_t(t.repr-y.repr))<2);
                // }
                //}
              }
            }
          }(std::integral_constant<uint8_t, r0>{}), ...);
        }(radixes, std::integral_constant<int8_t, sis>{}), ...);
      }(std::make_integer_sequence<uint8_t, std::size(styleEnumMap)>{});
    }
  }*/
}
TEST_CASE("sqrt") {
  uint8_t si = GENERATE(range(size_t{0}, std::size(styleEnumMap)));
  std::fesetround(styleMacroMap[si]);
  SECTION("1 word") {
    uint8_t base=GENERATE(range(0,NL<uint8_t >::max()+1));
  uint8_t  y=sqrt(uint64_t(base),0,styleEnumMap[si]);
  uint8_t  t=std::lrintf(std::sqrtf(base));
  REQUIRE(y==t);
  }

  SECTION("1 word more") {
    for (unsigned int i=1;i<=NL<uint16_t>::max();++i) {
    uint16_t  y=uRoot2(uint16_t(i),styleEnumMap[si]);
    uint16_t  t=std::lrint(std::sqrt(i));
    REQUIRE(y==t);
    }
  }
  uint8_t radix=GENERATE(range(1,NL<uint8_t>::digits+1));
  SECTION("2 words") {
    uint8_t base=GENERATE(range(0,NL<uint8_t >::max()+1));
    uint8_t  y=sqrt(uint64_t(base),radix,styleEnumMap[si]);
    uint8_t  t=std::lrintf(std::sqrtf(base<<radix));
    CAPTURE(base,radix,styleEnumMap[si]);
    REQUIRE(y==t);
  }
  using u128=unsigned __int128;
  SECTION("2 words large") {
    uint64_t y=sqrt(NL<uint64_t>::max(),radix,styleEnumMap[si]);
    uint64_t t=uRoot2(u128(NL<uint64_t>::max())<<radix,styleEnumMap[si]);
    CAPTURE(radix,styleEnumMap[si]);
    REQUIRE(y==t);
  }
  constexpr uint16_t counts=4096;
  SECTION("2 words rand") {
    for (uint16_t i=0;i<counts;++i) {
      uint64_t base=uint64_t(rg32())<<32|rg32();
      base=base%NL<uint64_t>::max()+1;
      uint64_t y=sqrt(base,radix,styleEnumMap[si]);
      uint64_t t=uRoot2(u128(base)<<radix,styleEnumMap[si]);
      REQUIRE(y==t);
    }
  }
  //uint8_t radix = GENERATE(range(0, NL<TestType>::digits + 1));
}
TEMPLATE_TEST_CASE("bone 32","",int32_t,uint32_t) {
  constexpr uint16_t counts=2048;
  using ldbl=long double;
  using Tt=std::conditional_t<std::is_unsigned_v<TestType>,uint64_t,int64_t>;
  SECTION("div") {
    auto si=GENERATE(range(size_t{0}, std::size(styleEnumMap)));
    std::fesetround(styleMacroMap[si]);
    uint8_t radix=GENERATE(range(0,NL<TestType>::digits+1));
    const ldbl big=toF<ldbl>(NL<TestType>::max(),radix,std::round_indeterminate),small=toF<ldbl>(NL<TestType>::min(),radix,std::round_indeterminate);
    for (uint16_t i=0;i<counts;++i) {
      TestType dividend=rg32();
      TestType divisor=rg32()%NL<uint32_t>::max()+1;
      ldbl a=ldbl(dividend)/divisor;
      if (a>=small&&a<=big) {
        //CAPTURE(dividend,divisor,radix,styleEnumMap[si],std::ldexp(dividend,radix)/divisor);
        TestType y=div(dividend,divisor,radix,styleEnumMap[si]);
        TestType t=br<TestType>(a,radix);
        REQUIRE(y==t);
      }
    }
  }
  SECTION("mul") {//need long double with digits>=64
    uint8_t si = GENERATE(range(size_t{0}, std::size(styleEnumMap)));
    std::fesetround(styleMacroMap[si]);
    uint8_t radix = GENERATE(range(0, NL<TestType>::digits + 1));
    for (uint16_t i=0;i<counts;++i) {
    TestType left = rg32()/2;
    TestType right = rg32()/2;
    Tt a = Tt(left) * right;
    if (a >= Tt(NL<TestType>::min())<<32&& a <=Tt(NL<TestType>::max())<<32) {
      TestType y= mul(left, right, radix, styleEnumMap[si]);
      TestType t = std::llrint(std::ldexp(ldbl(a), -radix));
      //CAPTURE(left, right, radix, si);
      REQUIRE(y == t);
    }
    }
  }
}