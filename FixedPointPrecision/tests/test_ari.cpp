#pragma STDC FENV_ACCESS ON
#include "arithmetic.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include<vector>
#include<cstdint>
#include<ranges>
#include<stdexcept>
#include<cfenv>
#include<cmath>
import fixed;
namespace fpp_tests::arithmetic {
    namespace {
    constexpr int styleMacroMap[4]{FE_TOWARDZERO,FE_TONEAREST,FE_UPWARD,FE_DOWNWARD};
    constexpr std::float_round_style styleEnumMap[4] {
      std::round_toward_zero, std::round_to_nearest, std::round_toward_infinity, std::round_toward_neg_infinity
    };
    static_assert(std::size(styleEnumMap)==std::size(styleMacroMap));
        template<test_Tint T>
        std::vector<aint_dt<T> > sampleAint() noexcept {
            using Tt = typename rankOf<T>::two;
            std::vector<aint_dt<T> > r;
            if constexpr (std::is_signed_v<T>) {
                using Tu = std::make_unsigned_t<T>;
                constexpr T N = T{1} << (NL<Tu>::digits / 4 - 1);
                for (T i = 0; i < N; ++i) {
                    r.emplace_back(NL<Tt>::min() + i);
                    r.emplace_back(NL<Tt>::max() - i);
                }
                for (Tt i = 1; i <= N * 2; ++i) {
                    r.emplace_back(i << NL<Tu>::digits / 4);
                    r.emplace_back((-i) << NL<Tu>::digits / 4);
                }
                for (T i = 1; i < N; ++i) {
                    r.emplace_back(i);
                    r.emplace_back(static_cast<Tt>(-i));
                }
                r.emplace_back(Tt{0});
            } else {
                constexpr T N = T{1} << (NL<T>::digits / 4);
                for (T i = 0; i < N; ++i) {
                    r.emplace_back(i);
                    r.emplace_back(NL<Tt>::max() - i);
                }
                for (Tt i = 1; i <= 2 * N; ++i) {
                    r.emplace_back(i << NL<Tt>::digits / 4);
                }
            }
            r.shrink_to_fit();
            return r;
        }

        template<test_Tuint Tu>
        std::vector<Tu> sampleNDiv() noexcept {
            constexpr uint8_t d = NL<Tu>::digits;
            constexpr Tu N = Tu{1} << d / 2;
            std::vector<Tu> r(N * 2);
            for (Tu i = 0; i < N; ++i) {
                r[i * 2] = i;
                r[i * 2 + 1] = NL<std::make_signed_t<Tu> >::max() - i;
            }
            for (size_t i = 0; i < r.size(); ++i)
                r[i] |= Tu{1} << d - 1;
            return r;
        }

        template<test_Tint T>
        std::vector<T> sample() noexcept {
            std::vector<T> r;
            if constexpr (std::is_signed_v<T>) {
                using Tu = std::make_unsigned_t<T>;
                constexpr T N = T{1} << (NL<Tu>::digits / 2 - 1);
                for (T i = 0; i < N; ++i) {
                    r.emplace_back(NL<T>::min() + i);
                    r.emplace_back(NL<T>::max() - i);
                }
                for (T i = 1; i < N; ++i) {
                    r.emplace_back(i);
                    r.emplace_back(-i);
                }
                r.emplace_back(0);
            } else {
                constexpr T N = T{1} << NL<T>::digits / 2;
                for (T i = 0; i < N; ++i) {
                    r.emplace_back(i);
                    r.emplace_back(NL<T>::max() - i);
                }
            }
            r.shrink_to_fit();
            return r;
        }
        namespace SV = std::views;
    }


    template<std::forward_iterator T0, std::forward_iterator T1>
    struct CartIter {
        using value_type = std::tuple<typename T0::value_type, typename T1::value_type>;
        using difference_type = std::ptrdiff_t;

        struct Ender {
        };

        static constexpr Ender end{};

        CartIter() = default;

        constexpr
        CartIter(T0 &&it0b, T0 &&it0e, T1 &&it1b, T1 &&it1e)
            noexcept: b0(it0b), e0(it0e), e1(it1e), c1(it1b) {
            c0 = b0;
        }

        value_type operator*() const {
            return {*c0, *c1};
        }

        constexpr
        CartIter &operator++() {
            if (++c0 == e0) {
                ++c1;
                c0 = b0;
            }
            return *this;
        }

        constexpr
        CartIter operator++(int) const {
            CartIter a = *this;
            ++*this;
            return a;
        }

        constexpr
        bool operator==(const CartIter &a) const
            noexcept {
            return c0 == a.c0 && c1 == a.c1;
        }

        constexpr
        bool operator==(Ender) const
            noexcept {
            return c1 == e1;
        }

    private:
        const T0 b0, e0;
        const T1 e1;
        T0 c0;
        T1 c1;
    };

    TEMPLATE_TEST_CASE("direct", "", uint8_t, int8_t) {
        using Tt = rankOf<TestType>::two;
        const auto dividendSamples = SV::iota(Tt{NL<TestType>::min()}, Tt{NL<TestType>::max() + 1});
        std::vector<TestType> divisorSamples;
        for (TestType i = NL<TestType>::min(); i < 0; ++i)
            divisorSamples.emplace_back(i);
        for (Tt i = 1; i <= NL<TestType>::max(); ++i)
            divisorSamples.emplace_back(i);
        divisorSamples.shrink_to_fit();
        SECTION("divRnd") {
            for (size_t i = 0; i < std::size(styleMacroMap); ++i) {
                std::fesetround(styleMacroMap[i]);
                for (auto it = CartIter(dividendSamples.begin(), dividendSamples.end(), divisorSamples.begin(),
                                        divisorSamples.end()); it != it.end; ++it) {
                    const auto [a,r] = *it;
                    TestType l = a;
                    CAPTURE(l, r, styleEnumMap[i]);
                    TestType t = std::lrintf(float(l) / r);
                    TestType y = divRnd(l, r, styleEnumMap[i]);
                    REQUIRE(t==y);
                }
            }
        }
        if constexpr (std::is_signed_v<TestType>) {
            SECTION("lsDivRnd") {
                for (uint8_t by = 1; by < sizeof(TestType) * CHAR_BIT; ++by) {
                    for (size_t i = 0; i < std::size(styleMacroMap); ++i) {
                        std::fesetround(styleMacroMap[i]);
                        for (auto it = CartIter(dividendSamples.begin(), dividendSamples.end(), divisorSamples.begin(),
                                                divisorSamples.end()); it != it.end; ++it) {
                            const auto [l,r] = *it;
                            if (wideLS(l, by).h >= r)
                                continue;
                            CAPTURE(l, r, by, styleEnumMap[i]);
                            TestType t = std::lrintf(l * float(1 << by) / r);
                            TestType y = lsDivRnd(l, Tt(r), by, styleEnumMap[i]);
                            REQUIRE(t==y);
                        }
                    }
                }
            }
        }
    }

    TEMPLATE_TEST_CASE("general", "", uint16_t, int16_t) {
        using Tt = rankOf<TestType>::two;
        const std::vector<TestType> samples = sample<TestType>();
        SECTION("wideMul") {
            for (auto it = CartIter(samples.begin(), samples.end(), samples.begin(), samples.end()); it != it.end; ++
                 it) {
                const auto [l,r] = *it;
                CAPTURE(l, r);
                Tt t = Tt(l) * r;
                Tt y = wideMul(l, r).merge();
                REQUIRE(t==y);
            }
        }
        SECTION("wideLS") {
            const auto byIt = SV::iota(uint8_t{1}, static_cast<uint8_t>(sizeof(TestType) * CHAR_BIT + 1));
            for (auto it = CartIter(samples.begin(), samples.end(), byIt.begin(), byIt.end()); it != it.end; ++it) {
                const auto [l,r] = *it;
                CAPTURE(l, r);
                Tt t = Tt(l) << r;
                Tt y = wideLS(l, r).merge();
                REQUIRE(t==y);
            }
            REQUIRE_THROWS_AS(wideLS(0,0), std::domain_error);
        }
        using Th = typename rankOf<TestType>::half;
        SECTION("aint_dt op+=") {
            for (const auto &l: samples) {
                const TestType a = std::min<Tt>(Tt{NL<TestType>::max()} - l, NL<std::make_unsigned_t<Th> >::max()) + 1;
                for (TestType r = 0; r < a; ++r) {
                    CAPTURE(l, r);
                    TestType t = l + r;
                    aint_dt<Th> b(l);
                    b += r;
                    TestType y = b.merge();
                    REQUIRE(t==y);
                }
            }
        }
        SECTION("aint_dt op>>=") {
            const auto byIt = SV::iota(uint8_t{0}, static_cast<uint8_t>(sizeof(TestType) * CHAR_BIT));
            for (auto it = CartIter(samples.begin(), samples.end(), byIt.begin(), byIt.end()); it != it.end; ++it) {
                const auto [l,r] = *it;
                CAPTURE(l, r);
                Tt t = Tt(l) >> r;
                aint_dt<Th> a(l);
                a >>= r;
                Tt y = a.merge();
                REQUIRE(t==y);
            }
        }

        SECTION("aint_dt.narrowRnd") {
          REQUIRE_THROWS_AS(aint_dt<Th>().narrowRnd(0,styleEnumMap[0]), std::domain_error);
            const auto byIt = SV::iota(uint8_t{1}, static_cast<uint8_t>(sizeof(Th) * CHAR_BIT + 1));
          for (size_t i = 0; i < std::size(styleMacroMap); ++i) {
                std::fesetround(styleMacroMap[i]);
                for (auto it = CartIter(samples.begin() + 1, samples.end(), byIt.begin(), byIt.end()); it != it.end; ++
                     it) {
                    const auto [l,r] = *it;
                    CAPTURE(l, r, styleEnumMap[i]);
                    TestType t = std::lrintf(std::ldexpf(l, -r));
                    if (t >= NL<Th>::min() && t <= NL<Th>::max()) {
                        TestType y = aint_dt<Th>(l).narrowRnd(r, styleEnumMap[i]);
                        REQUIRE(t==y);
                    }
                }
            }
        }
      SECTION("rnd") {
          const auto byIt = SV::iota(uint8_t{0}, static_cast<uint8_t>(sizeof(Th) * CHAR_BIT + 1));
          for (size_t i = 0; i < std::size(styleMacroMap); ++i) {
            std::fesetround(styleMacroMap[i]);
            for (auto it = CartIter(samples.begin() + 1, samples.end(), byIt.begin(), byIt.end()); it != it.end; ++
                 it) {
              const auto [l,r] = *it;
              CAPTURE(l, r, styleEnumMap[i]);
              TestType t = std::lrintf(std::ldexpf(l, -r));
                TestType y = rnd(l,r, styleEnumMap[i]);
                REQUIRE(t==y);

                 }
          }
        }
        if constexpr (std::is_unsigned_v<TestType>) {
            SECTION("u212Div") {
                const std::vector<aint_dt<TestType> > aintSamples = sampleAint<TestType>();
                const std::vector<TestType> nDivSamples = sampleNDiv<TestType>();
                for (auto it = CartIter(aintSamples.begin(), aintSamples.end(), nDivSamples.begin(), nDivSamples.end());
                     it != it.end; ++it) {
                    const auto [l,r] = *it;
                    Tt a = l.merge();
                    CAPTURE(a, r);
                    Tt tq = a / r, tr = a % r;
                    auto [b,yr] = u212Div(l, r);
                    Tt yq = b.merge();
                    REQUIRE(tq==yq);
                    REQUIRE(tr==yr);
                }
            }
        }
    }

    TEMPLATE_TEST_CASE("promotion", "", int, unsigned int) {
        using Tt = rankOf<TestType>::two;
        std::vector<TestType> samples{NL<TestType>::max()};

        if constexpr (std::is_signed_v<TestType>) {
            samples.emplace_back(NL<TestType>::min());
            const std::vector<TestType> dividendSamples({NL<TestType>::max() - 1, NL<TestType>::min() + 1});

            SECTION("lsDivRnd") {
                constexpr uint8_t by = sizeof(TestType) * CHAR_BIT - 1;
                for (auto it = CartIter(dividendSamples.begin(), dividendSamples.end(), samples.begin(), samples.end());
                     it != it.end; ++it) {
                    for (size_t i = 0; i < std::size(styleMacroMap); ++i) {
                        std::fesetround(styleMacroMap[i]);
                        const auto [l,r] = *it;
                        CAPTURE(l, r, styleEnumMap[i]);
                        Tt t = std::llrint(l * ldexp(1, by) / r);
                        TestType y = lsDivRnd(l, r, by, styleEnumMap[i]);
                        REQUIRE(t==y);
                    }
                }
            }
        } else {
            SECTION("u212Div") {
                Tt dividend = NL<Tt>::max();
                TestType divisor = TestType{1} << NL<TestType>::digits - 1;
                CAPTURE(dividend, divisor);
                Tt tq = dividend / divisor, tr = dividend % divisor;
                auto [a,yr] = u212Div(aint_dt<TestType>(dividend), divisor);
                Tt yq = a.merge();
                REQUIRE(tq==yq);
                REQUIRE(tr==yr);
            }
        }
        SECTION("aint_dt op>>=") {
            using Th = typename rankOf<TestType>::half;
            const std::vector<uint8_t> byIt({0, sizeof(Th) * CHAR_BIT, sizeof(TestType) * CHAR_BIT - 1});
            for (auto it = CartIter(samples.begin(), samples.end(), byIt.begin(), byIt.end()); it != it.end; ++it) {
                const auto [l,r] = *it;
                CAPTURE(l, r);
                TestType t = l >> r;
                aint_dt<Th> a(l);
                a >>= r;
                TestType y = a.merge();
                REQUIRE(t==y);
            }
        }
        SECTION("aint_dt op+=") {
            using Tu = std::make_unsigned_t<TestType>;
            for (auto it = CartIter(samples.begin(), samples.end(), samples.begin(), samples.end()); it != it.end; ++
                 it) {
                const auto [a,b] = *it;
                Tt l = static_cast<Tt>(a) * b;
                Tu r = Tu{1} << NL<Tu>::digits - 1;
                aint_dt<TestType> c(l);
                c += r;
                Tt t = l + r;
                Tt y = c.merge();
                REQUIRE(t==y);
            }
        }
        SECTION("wideMul") {
            for (auto it = CartIter(samples.begin(), samples.end(), samples.begin(), samples.end()); it != it.end; ++
                 it) {
                const auto [l,r] = *it;
                CAPTURE(l, r);
                Tt t = Tt(l) * r;
                Tt y = wideMul(l, r).merge();
                REQUIRE(t==y);
            }
        }
        SECTION("wideLS") {
            const auto byIt = SV::iota(uint8_t{1}, uint8_t(sizeof(TestType) * CHAR_BIT + 1));
            for (auto it = CartIter(samples.begin(), samples.end(), byIt.begin(), byIt.end()); it != it.end; ++it) {
                const auto [l,r] = *it;
                CAPTURE(l, r);
                Tt t = Tt(l) << r;
                Tt y = wideLS(l, r).merge();
                REQUIRE(t==y);
            }
        }
        SECTION("aint_dt.narrowRnd") {
            const auto byIt = SV::iota(uint8_t{1}, static_cast<uint8_t>(sizeof(TestType) * CHAR_BIT + 1));
            for (size_t i = 0; i < std::size(styleMacroMap); ++i) {
                std::fesetround(styleMacroMap[i]);
                for (auto it = CartIter(samples.begin(), samples.end(), byIt.begin(), byIt.end()); it != it.end; ++it) {
                    const auto [l,r] = *it;
                    CAPTURE(l, r, styleEnumMap[i]);
                    Tt t = std::llrint(std::ldexp(l, -r));
                    if (t >= NL<TestType>::min() && t <= NL<TestType>::max()) {
                        TestType y = aint_dt<TestType>(l).narrowRnd(r, styleEnumMap[i]);
                        REQUIRE(t==y);
                    }
                }
            }
        }
      SECTION("rnd") {
          const auto byIt = SV::iota(uint8_t{0}, static_cast<uint8_t>(sizeof(TestType) * CHAR_BIT + 1));
          for (size_t i = 0; i < std::size(styleMacroMap); ++i) {
            std::fesetround(styleMacroMap[i]);
            for (auto it = CartIter(samples.begin(), samples.end(), byIt.begin(), byIt.end()); it != it.end; ++it) {
              const auto [l,r] = *it;
              CAPTURE(l, r, styleEnumMap[i]);
              Tt t = std::llrint(std::ldexp(l, -r));
                TestType y = rnd(l,r, styleEnumMap[i]);
                REQUIRE(t==y);

            }
          }
        }
        SECTION("divRnd") {
            std::vector<TestType> divisorSamples = samples;
            divisorSamples.emplace_back(TestType{1});
            std::vector<TestType> dividendSamples = samples;
            dividendSamples.emplace_back(NL<TestType>::max() - 1);
            if constexpr (std::is_signed_v<TestType>) {
                //divisorSamples.emplace_back(TestType{-1});
                dividendSamples.emplace_back(NL<TestType>::min() + 1);
            }

            for (size_t i = 0; i < std::size(styleMacroMap); ++i) {
                std::fesetround(styleMacroMap[i]);
                for (auto it = CartIter(dividendSamples.begin(), dividendSamples.end(), divisorSamples.begin(),
                                        divisorSamples.end()); it != it.end; ++it) {
                    const auto [a,r] = *it;
                    TestType l = a;
                    CAPTURE(l, r, styleEnumMap[i]);
                    Tt t = std::llrint(double(l) / r);
                    TestType y = divRnd(l, r, styleEnumMap[i]);
                    REQUIRE(t==y);
                }
            }
        }
    }
}
