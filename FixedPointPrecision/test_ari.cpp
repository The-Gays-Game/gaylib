#include "arithmetic.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include<vector>
#include<cstdint>
#include<ranges>

namespace fpp_tests::arithmetic
{
    namespace
    {
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
            noexcept: b0(it0b), e0(it0e), c1(it1b), e1(it1e)
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
        }
    }

    TEMPLATE_TEST_CASE("edge", "[wide]", int, unsigned int)
    {
        std::vector<TestType> samples{std::numeric_limits<TestType>::max()};
        if constexpr (std::is_signed_v<TestType>)
            samples.emplace_back(std::numeric_limits<TestType>::min());
        using Tt = rankOf<TestType>::two;
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
