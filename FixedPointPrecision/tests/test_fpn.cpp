#pragma STDC FENV_ACCESS ON
#include "arithmetic.h"

#include <bit>
#include <catch2/catch_get_random_seed.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <cfenv>
#include <cmath>
#include <random>
#include <utility>
#include<ranges>
import fixed;
import helpers;
namespace {

template <class, uint8_t, std::float_round_style>
struct intToFpn {
};

template <std::signed_integral T0, uint8_t V0, std::float_round_style V1>
struct intToFpn<T0, V0, V1> {
  using type = fx<T0, V0, V1>;
};

template <std::unsigned_integral T0, uint8_t V0, std::float_round_style V1>
struct intToFpn<T0, V0, V1> {
  using type = ufx<T0, V0, V1>;
};

template<class A>
struct withId: A{
  inline static const withId one{};
  constexpr
  withId operator *=(const withId &o)noexcept {
    if (this==&one)
      *this=o;
    else if (&o!=&one)
      static_cast<A&>(*this)*=o;
    return *this;
  }
  withId operator *=(A o)noexcept {
    static_cast<A&>(*this)*=o;
    return *this;
  }
  //explicit constexpr withId(A a):A(a){}
};
} // namespace
template <std::integral T, T... Ints>
using IntSeq = std::integer_sequence<T, Ints...>;
TEMPLATE_TEST_CASE("bone 16", "", int16_t, uint16_t) {
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
  SECTION("pow") {
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
                TestType t=intPow<uint32_t>(base,e,1);
                TestType y=A(base).pow(e).repr;
                REQUIRE(t==y);
              }
            }else if constexpr(radix0==NL<TestType>::digits) {
              for (uint32_t e=1;e<=NL<uint16_t>::max();++e) {
                //for (uint8_t j=0;j<1;++j) {
                  auto a=rg32();
                {
                  TestType base=a>>=16;
                  A t=  intPow(A::raw(base),e,withId<A>::one);
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
  }
  constexpr auto radixes = std::make_integer_sequence<uint8_t, NL<TestType>::digits>{};
  SECTION("op %") {
    for (const auto _:std::ranges::views::iota(0,1<<16)) {
      []<uint8_t... r0>(IntSeq<uint8_t, r0...> radixes){
        ([](auto radix0) {
          uint32_t rv=rg32();
          using A=intToFpn<TestType, radix0,std::round_indeterminate>::type;
          A dividend=A::raw(rv),divisor=A::raw(rv>>16);
          if (divisor==A(0))
            return;
          CAPTURE(float(dividend),float(divisor));
          A y=dividend%divisor;
          float t=std::fmodf(dividend,divisor);
          REQUIRE(t==float(y));
        }(std::integral_constant<uint8_t,r0>{}),...);
      }(radixes);
    }
  }
  using Tu = std::make_unsigned_t<TestType>;
  SECTION("ctor change radix") {
    []<uint8_t... r0>(IntSeq<uint8_t, r0...> radixes) {
      ([]<uint8_t... r1>(IntSeq<uint8_t, r1...>, auto radix0) {
          ([radix0]<int8_t... ss>(IntSeq<int8_t, ss...>, auto radix1) {
              ([radix0, radix1](auto s) {
                  constexpr auto se = static_cast<std::float_round_style>(s());
                  using A = intToFpn<TestType, radix0, se>::type;
                  using B = intToFpn<TestType, radix1, se>::type;
                  for (uint16_t i = 0; i < 2048; ++i) {
                    auto a = A::raw(rg32());
                    auto b = B(a);
                    auto c = ufx<Tu, radix0, std::round_indeterminate>::raw(a.repr);
                    auto d = ufx<Tu, radix1, std::round_indeterminate>(c);
                    CAPTURE(a.repr, b.repr, radix0, radix1, se);
                    if ((std::is_unsigned_v<TestType> || c.repr >> NL<Tu>::digits - 1 == d.repr >> NL<Tu>::digits - 1) && std::popcount(Tu(a.repr)) == std::popcount(d.repr)) {
                      REQUIRE(A(b).repr == a.repr);
                      REQUIRE(static_cast<float>(a) == static_cast<float>(b));
                    } else {
                      if constexpr(radix1 > radix0) {
                        REQUIRE(b.repr == TestType(a.repr << (radix1 - radix0)));
                      } else if constexpr (radix1 < radix0) {
                        REQUIRE(b.repr == rnd(a.repr, radix0 - radix1, se));
                      }
                    }
                  }
                }(std::integral_constant<int8_t, ss>{}),
                ...);
            }(styleEnumSeq, std::integral_constant<uint8_t, r1>{}),
            ...);
        }(radixes, std::integral_constant<uint8_t, r0>{}),
        ...);
    }(radixes);
  }
  SECTION("to int") {
    []<uint8_t... r0>(IntSeq<uint8_t, r0...>) {
      ([](auto radix0) {
          using A = intToFpn<TestType, radix0, std::round_toward_zero>::type;
          for (Tt i = NL<TestType>::min(); i <= NL<TestType>::max(); ++i) {
            auto a = A::raw(i);
            float b = a;
            CAPTURE(b);
            REQUIRE(static_cast<TestType>(a) == static_cast<TestType>(b));
          }
        }(std::integral_constant<uint8_t, r0>{}),
        ...);
    }(radixes);
  }

  SECTION("op /") {
    [radixes]<uint8_t ... sis>(IntSeq<uint8_t, sis...>) {
      ([]<uint8_t ... r0>(IntSeq<uint8_t, r0...>, auto si) {
        constexpr std::float_round_style se = styleEnumMap[si];
        std::fesetround(styleMacroMap[si]);
        ([se](auto radix0) {
          using A = intToFpn<TestType, radix0, se>::type;
          const float fpnMax = A::raw(NL<TestType>::max()), fpnMin = A::raw(NL<TestType>::min());
          for (uint16_t i = 0; i < 1 << 15; ++i) {
            uint32_t rv = rg32();
            auto l = A::raw(rv), r = A::raw(rv >> 16);
            double a = double(l) / double(r);
            /*
             *why can't we use `float a=float(l)/float(r)` here? fpn division result is correctly rounded. this means q=round(a*2^radix/b). single operation
             *of `float` is also correctly rounded, which means <0.5(for none round-to-nearest it's 1ulp, which makes matter worse) ulp, but how is `t` calculated? `t=lrint(round(l/r)*2^radix)`, note ldexp is exact, so
             *round(a*2^radix)=a*2^radix=ldexp(a,radix). lrint introduces another rounding, so the ulp distance sums up to <1. `float` has 24 digits and
             *`fx` has 16 digits, so 1 ulp in `float` can be >0.5 ulp in `fx`, this means rounding to the incorrect direction of the int, differing by 1. however,
             *`double` has 53 digits, so 1 ulp in `double` is <0.5 ulp in `fx`, this means correct rounding.
             */
            CAPTURE(l.repr, r.repr, radix0, se);
            if (a <= fpnMax && a >= fpnMin) {
              A y = l / r;
              auto t = A::br(a);
              REQUIRE(t.repr==y.repr);
            }
          }
        }(std::integral_constant<uint8_t, r0>{}), ...);
      }(radixes, std::integral_constant<int8_t, sis>{}), ...);
    }(std::make_integer_sequence<uint8_t, std::size(styleEnumMap)>{});
  }

  SECTION("op *") {
    [radixes]<uint8_t ... sis>(IntSeq<uint8_t, sis...>) {
      ([]<uint8_t ... r0>(IntSeq<uint8_t, r0...>, auto si) {
        constexpr std::float_round_style se = styleEnumMap[si];
        std::fesetround(styleMacroMap[si]);
        ([se](auto radix0) {
          using A = intToFpn<TestType, radix0, se>::type;
          const float fpnMax = A::raw(NL<TestType>::max()), fpnMin = A::raw(NL<TestType>::min());
          for (uint16_t i = 0; i < 1 << 15; ++i) {
            uint32_t rv = rg32();
            auto l = A::raw(rv), r = A::raw(rv >> 16);
            double a = double(l) * double(r);
            CAPTURE(l.repr, r.repr, radix0, se);
            if (a <= fpnMax && a >= fpnMin) {
              A y = l * r;
              auto t = A::br(a);
              REQUIRE(t.repr==y.repr);
            }
          }
        }(std::integral_constant<uint8_t, r0>{}), ...);
      }(radixes, std::integral_constant<int8_t, sis>{}), ...);
    }(std::make_integer_sequence<uint8_t, std::size(styleEnumMap)>{});
  }

  SECTION("remQuo") {
    for (const auto _:std::ranges::views::iota(0,1<<16)) {
      []<uint8_t... r0>(IntSeq<uint8_t, r0...> radixes){
        ([](auto radix0) {
          uint32_t rv=rg32();
          using A=intToFpn<TestType, radix0,std::round_indeterminate>::type;
          const float fpnMax = A::raw(NL<TestType>::max()), fpnMin = A::raw(NL<TestType>::min());
          A dividend=A::raw(rv),divisor=A::raw(rv>>16);
          int qt;
          float rt=std::remquo(float(dividend),float(divisor),&qt);
          if (float q0=float(dividend)/float(divisor);q0<=fpnMax&&q0>=fpnMin) {
            qt&=(1<<3)-1;
            auto [qy,ry]=dividend.remQuo(divisor);
            qy&=(1<<3)-1;
            REQUIRE(qt==qy);
            REQUIRE(rt==float(ry));
          }
        }(std::integral_constant<uint8_t,r0>{}),...);
      }(radixes);
    }
  }
}

TEMPLATE_TEST_CASE("bone 32", "", int32_t, uint32_t) {
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

TEST_CASE("fast path") {
  SECTION("toF") {
    for (const auto _:std::ranges::views::iota(0,1<<18)) {
      for (int8_t radix:std::ranges::views::iota(int8_t{0},int8_t{64})) {
        uint64_t repr=uint64_t(rg32())<<32|rg32();
        double t=std::ldexp(repr, -radix);
        double y=toF<double>(repr,radix,std::round_indeterminate);
        REQUIRE(t==y);
      }
    }
  }
  SECTION("fromF") {
    for (const auto _:std::ranges::views::iota(0,1<<18)) {
      for (int8_t radix:std::ranges::views::iota(int8_t{0},int8_t{64})) {
        double a=std::bit_cast<double>(static_cast<uint64_t>(rg32()>>1) <<32|rg32());
        //CAPTURE(uint16_t(radix),a);
        const double fpnMax=toF<double>(NL<uint64_t>::max(),radix,std::round_indeterminate);
        if (a>fpnMax||a<0||std::isnan(a)) {
          continue;
        }
        uint64_t t=std::ldexp(a, radix);
        uint64_t y=fromF<uint64_t>(a,radix);

        REQUIRE(t==y);
      }
    }
  }
}

TEMPLATE_TEST_CASE("bone 128", "", __int128, unsigned __int128) {
  rg32.seed(Catch::getSeed());
  SECTION("toF") {
    auto mode = GENERATE(range(size_t{0}, std::size(styleEnumMap)));
    std::fesetround(styleMacroMap[mode]);
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

  constexpr auto radixes = std::make_integer_sequence<uint8_t, NL<TestType>::digits>{};
  using calcType = std::conditional_t<std::is_unsigned_v<TestType>, uint16_t, int16_t>;
  SECTION("op /") {
    [radixes]<uint8_t ... sis>(IntSeq<uint8_t, sis...>) {
      ([]<uint8_t ... r0>(IntSeq<uint8_t, r0...>, auto si) {
        constexpr std::float_round_style se = styleEnumMap[si];
        std::fesetround(styleMacroMap[si]);
        ([](auto radix0) {
          using A = intToFpn<TestType, radix0, se>::type;
          const float fpnMax = A::raw(NL<calcType>::max()), fpnMin = A::raw(NL<calcType>::min());
          for (uint16_t i = 0; i < 1 << 13; ++i) {
            uint32_t rv = rg32();
            auto l = A::raw(calcType(rv)), r = A::raw(rv >> 16);
            double a = double(l) / double(r);
            //CAPTURE(calcType(l.repr),calcType(r.repr),radix0,se);//gcc's ld(linker) isn't happy with this line.
            if (a <= fpnMax && a >= fpnMin) {
              A y = l / r;
              auto t=A::raw(std::lrint(std::ldexp(a,radix0)));
              REQUIRE(t.repr==y.repr);
            }
          }
        }(std::integral_constant<uint8_t, r0>{}), ...);
      }(radixes, std::integral_constant<int8_t, sis>{}), ...);
    }(std::make_integer_sequence<uint8_t, std::size(styleEnumMap)>{});
  }
  SECTION("op *") {
    [radixes]<uint8_t ... sis>(IntSeq<uint8_t, sis...>) {
      ([]<uint8_t ... r0>(IntSeq<uint8_t, r0...>, auto si) {
        constexpr std::float_round_style se = styleEnumMap[si];
        std::fesetround(styleMacroMap[si]);
        ([se](auto radix0) {
          using A = intToFpn<TestType, radix0, se>::type;
          const float fpnMax = A::raw(NL<calcType>::max()), fpnMin = A::raw(NL<calcType>::min());
          for (uint16_t i = 0; i < 1 << 13; ++i) {
            uint32_t rv = rg32();
            auto l = A::raw(calcType(rv)), r = A::raw(rv >> 16);
            double a = static_cast<double>(l) * static_cast<double>(r);
            CAPTURE(calcType(l.repr),calcType(r.repr),int16_t(radix0),se);
            if (a <= fpnMax && a >= fpnMin) {
              A y = l * r;
              auto t=A::raw(std::lrint(std::ldexp(a,radix0)));
              REQUIRE(calcType(t.repr)==calcType(y.repr));
            }
          }
        }(std::integral_constant<uint8_t, r0>{}), ...);
      }(radixes, std::integral_constant<int8_t, sis>{}), ...);
    }(std::make_integer_sequence<uint8_t, std::size(styleEnumMap)>{});
  }
}