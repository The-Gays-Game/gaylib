#include "arithmetic.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include<vector>
#include<cstdint>
#include<ranges>
#include<stdexcept>

namespace fpp_tests::arithmetic
{
    namespace
    {
        template <test_Tint T>
        std::vector<aint_dt<T>> sampleAint() noexcept
        {
            using Tt = typename rankOf<T>::two;
            std::vector<aint_dt<T>> r;
            if constexpr (std::is_signed_v<T>)
            {
                using Tu = std::make_unsigned_t<T>;
                constexpr T N = T{1} << (std::numeric_limits<Tu>::digits / 4 - 1);
                for (T i = 0; i < N; ++i)
                {
                    r.emplace_back(std::numeric_limits<Tt>::min() + i);
                    r.emplace_back(std::numeric_limits<Tt>::max() - i);
                }
                for (Tt i = 1; i <= N * 2; ++i)
                {
                    r.emplace_back(i << std::numeric_limits<Tu>::digits / 4);
                    r.emplace_back((-i) << std::numeric_limits<Tu>::digits / 4);
                }
                for (T i = 1; i < N; ++i)
                {
                    r.emplace_back(i);
                    r.emplace_back(static_cast<Tt>(-i));
                }
                r.emplace_back(Tt{0});
            }
            else
            {
                constexpr T N = T{1} << (std::numeric_limits<T>::digits / 4);
                for (T i = 0; i < N; ++i)
                {
                    r.emplace_back(i);
                    r.emplace_back(std::numeric_limits<Tt>::max() - i);
                }
                for (Tt i = 1; i <= 2 * N; ++i)
                {
                    r.emplace_back(i << std::numeric_limits<Tt>::digits / 4);
                }
            }
            r.shrink_to_fit();
            return r;
        }

        template <test_Tuint Tu>
        std::vector<Tu> sampleNDiv() noexcept
        {
            constexpr uint8_t d = std::numeric_limits<Tu>::digits;
            constexpr Tu N = Tu{1} << d / 2;
            std::vector<Tu> r(N * 2);
            for (Tu i = 0; i < N; ++i)
            {
                r[i * 2] = i;
                r[i * 2 + 1] = std::numeric_limits<std::make_signed_t<Tu>>::max() - i;
            }
            for (size_t i = 0; i < r.size(); ++i)
                r[i] |= Tu{1} << d - 1;
            return r;
        }

        template <test_Tint T>
        std::vector<T> sample() noexcept
        {
            std::vector<T> r;
            if constexpr (std::is_signed_v<T>)
            {
                using Tu = std::make_unsigned_t<T>;
                constexpr T N = T{1} << (std::numeric_limits<Tu>::digits / 2 - 1);
                for (T i = 0; i < N; ++i)
                {
                    r.emplace_back(std::numeric_limits<T>::min() + i);
                    r.emplace_back(std::numeric_limits<T>::max() - i);
                }
                for (T i = 1; i < N; ++i)
                {
                    r.emplace_back(i);
                    r.emplace_back(-i);
                }
                r.emplace_back(0);
            }
            else
            {
                constexpr T N = T{1} << std::numeric_limits<T>::digits / 2;
                for (T i = 0; i < N; ++i)
                {
                    r.emplace_back(i);
                    r.emplace_back(std::numeric_limits<T>::max() - i);
                }
            }
            r.shrink_to_fit();
            return r;
        }
    }

    template <std::forward_iterator T0, std::forward_iterator T1>
    struct CartIter
    {
        using value_type = std::tuple<typename T0::value_type, typename T1::value_type>;
        using difference_type = std::ptrdiff_t;

        struct Ender
        {
        };

        static constexpr Ender end{};
        CartIter() = default;

        constexpr
        CartIter(T0&& it0b, T0&& it0e, T1&& it1b, T1&& it1e)
            noexcept: b0(it0b), e0(it0e), e1(it1e), c1(it1b)
        {
            c0 = b0;
        }

        value_type operator*() const
        {
            return {*c0, *c1};
        }

        constexpr
        CartIter& operator++()
        {
            if (++c0 == e0)
            {
                ++c1;
                c0 = b0;
            }
            return *this;
        }

        constexpr
        CartIter operator++(int) const
        {
            CartIter a = *this;
            ++*this;
            return a;
        }

        constexpr
        bool operator==(const CartIter& a) const
            noexcept
        {
            return c0 == a.c0 && c1 == a.c1;
        }

        constexpr
        bool operator==(Ender) const
            noexcept
        {
            return c1 == e1;
        }

    private:
        const T0 b0, e0;
        const T1 e1;
        T0 c0;
        T1 c1;
    };

    TEMPLATE_TEST_CASE("general", "[wide]", uint16_t, int16_t)
    {
        using Tt = rankOf<TestType>::two;
        const std::vector<TestType> samples = sample<TestType>();
        SECTION("wideMul")
        {
            for (auto it = CartIter(samples.begin(), samples.end(), samples.begin(), samples.end()); it != it.end; ++it)
            {
                const auto [l,r] = *it;
                CAPTURE(l, r);
                Tt t = Tt(l) * r;
                Tt y = wideMul(l, r).merge();
                REQUIRE(t==y);
            }
        }
        SECTION("wideLS")
        {
            const auto byIt = std::views::iota(uint8_t{1}, uint8_t(sizeof(TestType) * CHAR_BIT + 1));
            for (auto it = CartIter(samples.begin(), samples.end(), byIt.begin(), byIt.end()); it != it.end; ++it)
            {
                const auto [l,r] = *it;
                CAPTURE(l, r);
                Tt t = Tt(l) << r;
                Tt y = wideLS(l, r).merge();
                REQUIRE(t==y);
            }
            REQUIRE_THROWS_AS(wideLS(0,0), std::domain_error);
        }
        SECTION("aint_dt op+=") {
            for (const auto &l:samples) {
                using Th=typename rankOf<TestType>::half;
                const TestType a=std::min<Tt>(Tt{std::numeric_limits<TestType>::max()}-l,std::numeric_limits<std::make_unsigned_t<Th>>::max())+1;
                for (TestType r=0;r<a;++r) {
                    CAPTURE(l,r);
                    TestType t=l+r;
                    aint_dt<Th> b(l);
                    b+=r;
                    TestType y=b.merge();
                    REQUIRE(t==y);
                }
            }
        }
        if constexpr (std::is_unsigned_v<TestType>)
        {
            SECTION("u212Div")
            {
                const std::vector<aint_dt<TestType>> aintSamples = sampleAint<TestType>();
                const auto nDivSamples = sampleNDiv<TestType>();
                for (auto it = CartIter(aintSamples.begin(), aintSamples.end(), nDivSamples.begin(), nDivSamples.end()); it != it.end; ++it)
                {
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
    TEMPLATE_TEST_CASE("edge", "[wide]", int, unsigned int)
    {
        using Tt = rankOf<TestType>::two;
        std::vector<TestType> samples{std::numeric_limits<TestType>::max()};
        if constexpr (std::is_signed_v<TestType>)
            samples.emplace_back(std::numeric_limits<TestType>::min());
        else
        {
            SECTION("u212Div")
            {
                Tt dividend = std::numeric_limits<Tt>::max();
                TestType divisor = TestType{1} << std::numeric_limits<TestType>::digits - 1;
                CAPTURE(dividend, divisor);
                Tt tq = dividend / divisor, tr = dividend % divisor;
                auto [a,yr] = u212Div(aint_dt<TestType>(dividend), divisor);
                Tt yq = a.merge();
                REQUIRE(tq==yq);
                REQUIRE(tr==yr);
            }
        }
        SECTION("aint_dt op+=") {
            using Tu=std::make_unsigned_t<TestType>;
            for (auto it = CartIter(samples.begin(), samples.end(), samples.begin(), samples.end()); it != it.end; ++it) {
                const auto [a,b] = *it;
                Tt l=static_cast<Tt>(a)*b;
                Tu r=Tu{1}<<std::numeric_limits<Tu>::digits-1;
                aint_dt<TestType> c(l);
                c+=r;
                Tt t=l+r;
                Tt y=c.merge();
                REQUIRE(t==y);
            }
        }
        SECTION("wideMul")
        {
            for (auto it = CartIter(samples.begin(), samples.end(), samples.begin(), samples.end()); it != it.end; ++it)
            {
                const auto [l,r] = *it;
                CAPTURE(l, r);
                Tt t = Tt(l) * r;
                Tt y = wideMul(l, r).merge();
                REQUIRE(t==y);
            }
        }
        SECTION("wideLS")
        {
            const auto byIt = std::views::iota(uint8_t{1}, uint8_t(sizeof(TestType) * CHAR_BIT + 1));
            for (auto it = CartIter(samples.begin(), samples.end(), byIt.begin(), byIt.end()); it != it.end; ++it)
            {
                const auto [l,r] = *it;
                CAPTURE(l, r);
                Tt t = Tt(l) << r;
                Tt y = wideLS(l, r).merge();
                REQUIRE(t==y);
            }
        }

    }
}
