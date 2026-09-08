#include <gtest/gtest.h>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <eirin/numeric.hpp>

using namespace eirin;

namespace
{
    template <typename T>
    constexpr bool is_signed_v = std::is_signed_v<T>;

    template <typename T>
    void check_boundaries()
    {
        using L = std::numeric_limits<T>;
        constexpr T min = L::min();
        constexpr T max = L::max();

        if constexpr(is_signed_v<T>)
        {
            EXPECT_EQ(saturating_add(max, T(1)), max);
            EXPECT_EQ(saturating_add(min, T(-1)), min);
            EXPECT_EQ(saturating_sub(max, T(-1)), max);
            EXPECT_EQ(saturating_sub(min, T(1)), min);
            EXPECT_EQ(saturating_mul(min, T(-1)), max);
            EXPECT_EQ(saturating_div(min, T(-1)), max);
            EXPECT_EQ(saturating_mul(min, T(1)), min);

            // mixed-sign product overflow clamps to the product's sign
            EXPECT_EQ(saturating_mul(T(-(max / 2) - 2), T(2)), min);
            EXPECT_EQ(saturating_mul(T(max / 2 + 2), T(-2)), min);
        }
        else
        {
            EXPECT_EQ(saturating_add(max, T(1)), max);
            EXPECT_EQ(saturating_sub(T(0), T(1)), T(0));
            EXPECT_EQ(saturating_mul(max, T(2)), max);
        }
    }

    // Random saturation reference for types of at most 32 bits: every
    // arithmetic result fits into 64-bit integers.
    template <typename T>
    void check_random32(std::mt19937_64& rng)
    {
        using L = std::numeric_limits<T>;
        constexpr T min = L::min();
        constexpr T max = L::max();

        for(int i = 0; i < 4000; ++i)
        {
            // casting a uniform 64-bit draw to T keeps the low N bits uniform
            const T x = static_cast<T>(rng());
            T y = static_cast<T>(rng());
            if(y == 0) // keep the divisor non-zero
                y = T(1);

            if constexpr(is_signed_v<T>)
            {
                const std::int64_t xl = x;
                const std::int64_t yl = y;
                constexpr std::int64_t lo = static_cast<std::int64_t>(min);
                constexpr std::int64_t hi = static_cast<std::int64_t>(max);
                auto clamp = [](std::int64_t v)
                {
                    return v > hi ? static_cast<T>(hi) : (v < lo ? static_cast<T>(lo) : static_cast<T>(v));
                };

                EXPECT_EQ(saturating_add(x, y), clamp(xl + yl));
                EXPECT_EQ(saturating_sub(x, y), clamp(xl - yl));
                EXPECT_EQ(saturating_mul(x, y), clamp(xl * yl));
                const T want_div = (x == min && y == T(-1)) ? max : static_cast<T>(xl / yl);
                EXPECT_EQ(saturating_div(x, y), want_div);
            }
            else
            {
                const std::uint64_t xl = x;
                const std::uint64_t yl = y;
                constexpr std::uint64_t hi = static_cast<std::uint64_t>(max);
                auto clamp = [](std::uint64_t v)
                {
                    return v > hi ? static_cast<T>(hi) : static_cast<T>(v);
                };

                EXPECT_EQ(saturating_add(x, y), clamp(xl + yl));
                EXPECT_EQ(saturating_sub(x, y), xl >= yl ? static_cast<T>(xl - yl) : T(0));
                EXPECT_EQ(saturating_mul(x, y), clamp(xl * yl));
                EXPECT_EQ(saturating_div(x, y), static_cast<T>(xl / yl));
            }
        }
    }

#if defined(__SIZEOF_INT128__)
    template <typename T>
    void check_random64(std::mt19937_64& rng)
    {
        using L = std::numeric_limits<T>;
        constexpr bool sign = is_signed_v<T>;
        constexpr auto lo = sign ? static_cast<__int128>(L::min()) : __int128(0);
        constexpr auto hi = sign ? static_cast<__int128>(L::max()) : static_cast<__int128>(L::max());

        for(int i = 0; i < 4000; ++i)
        {
            const std::uint64_t raw_x = rng();
            const std::uint64_t raw_y = rng();
            const T x = static_cast<T>(raw_x);
            T y = static_cast<T>(raw_y);
            if(y == 0)
                y = T(1);

            if constexpr(sign)
            {
                auto clamp = [](__int128 v)
                {
                    return v > hi ? static_cast<T>(hi) : (v < lo ? static_cast<T>(lo) : static_cast<T>(v));
                };
                EXPECT_EQ(saturating_add(x, y), clamp(static_cast<__int128>(x) + y));
                EXPECT_EQ(saturating_sub(x, y), clamp(static_cast<__int128>(x) - y));
                EXPECT_EQ(saturating_mul(x, y), clamp(static_cast<__int128>(x) * y));
                EXPECT_EQ(saturating_div(x, y), x == L::min() && y == T(-1) ? L::max() : static_cast<T>(x / y));
            }
            else
            {
                const auto raw_u = static_cast<unsigned __int128>(x);
                const auto raw_v = static_cast<unsigned __int128>(y);
                constexpr auto raw_mx = static_cast<unsigned __int128>(L::max());
                auto clamp = [](unsigned __int128 v)
                {
                    return v > raw_mx ? static_cast<T>(raw_mx) : static_cast<T>(v);
                };
                EXPECT_EQ(saturating_add(x, y), clamp(raw_u + raw_v));
                EXPECT_EQ(saturating_sub(x, y), raw_u >= raw_v ? static_cast<T>(raw_u - raw_v) : T(0));
                EXPECT_EQ(saturating_mul(x, y), clamp(raw_u * raw_v));
                EXPECT_EQ(saturating_div(x, y), static_cast<T>(raw_u / raw_v));
            }
        }
    }
#endif

    template <typename From, typename To>
    To cast_ref(From v)
    {
        if constexpr(is_signed_v<To>)
        {
            if constexpr(is_signed_v<From>)
            {
                const std::int64_t vv = static_cast<std::int64_t>(v);
                const std::int64_t lo = static_cast<std::int64_t>(std::numeric_limits<To>::min());
                const std::int64_t hi = static_cast<std::int64_t>(std::numeric_limits<To>::max());
                return static_cast<To>(vv < lo ? lo : (vv > hi ? hi : vv));
            }
            else
            {
                const std::uint64_t hi = static_cast<std::uint64_t>(std::numeric_limits<To>::max());
                return static_cast<std::uint64_t>(v) > hi ? std::numeric_limits<To>::max() : static_cast<To>(v);
            }
        }
        else
        {
            const std::uint64_t hi = static_cast<std::uint64_t>(std::numeric_limits<To>::max());
            if constexpr(is_signed_v<From>)
            {
                if(v < 0)
                    return To(0);
            }
            return static_cast<std::uint64_t>(v) > hi ? std::numeric_limits<To>::max() : static_cast<To>(v);
        }
    }

    template <typename From, typename To>
    void check_cast_boundaries()
    {
        using L = std::numeric_limits<From>;
        constexpr From values[] = {L::min(), L::min() + 1, From(-1), From(0), From(1), L::max() - 1, L::max()};

        for(const From v : values)
            EXPECT_EQ((saturating_cast<To>(v)), (cast_ref<From, To>(v)));
    }

    template <typename From, typename To>
    void check_cast_random(std::mt19937_64& rng)
    {
        for(int i = 0; i < 4000; ++i)
        {
            const From v = static_cast<From>(rng());
            EXPECT_EQ((saturating_cast<To>(v)), (cast_ref<From, To>(v)));
        }
    }

    void run_std32_checks(std::mt19937_64& rng)
    {
        check_boundaries<std::int8_t>();
        check_boundaries<std::uint8_t>();
        check_boundaries<std::int16_t>();
        check_boundaries<std::uint16_t>();
        check_boundaries<std::int32_t>();
        check_boundaries<std::uint32_t>();

        check_random32<std::int8_t>(rng);
        check_random32<std::uint8_t>(rng);
        check_random32<std::int16_t>(rng);
        check_random32<std::uint16_t>(rng);
        check_random32<std::int32_t>(rng);
        check_random32<std::uint32_t>(rng);
    }
} // namespace

TEST(IntegralSaturating, BoundsAndRandom32)
{
    std::mt19937_64 rng(0x1A2B3C4D);
    run_std32_checks(rng);
}

#if defined(__SIZEOF_INT128__)
TEST(IntegralSaturating, Random64)
{
    std::mt19937_64 rng(0x5A6B7C8D);
    check_boundaries<std::int64_t>();
    check_boundaries<std::uint64_t>();
    check_random64<std::int64_t>(rng);
    check_random64<std::uint64_t>(rng);
}
#else
TEST(IntegralSaturating, Random64)
{
    check_boundaries<std::int64_t>();
    check_boundaries<std::uint64_t>();
}
#endif

TEST(IntegralSaturating, ConstexprBoundaries)
{
    static_assert(saturating_add(std::int32_t{INT32_MAX}, std::int32_t{1}) == INT32_MAX);
    static_assert(saturating_add(std::uint32_t{UINT32_MAX}, std::uint32_t{1}) == UINT32_MAX);
    static_assert(saturating_sub(std::int32_t{INT32_MIN}, std::int32_t{1}) == INT32_MIN);
    static_assert(saturating_sub(std::uint32_t{0}, std::uint32_t{1}) == 0);
    static_assert(saturating_mul(std::int32_t{INT32_MIN}, std::int32_t{-1}) == INT32_MAX);
    static_assert(saturating_mul(std::int32_t{1000000000}, std::int32_t{-1000000000}) == INT32_MIN);
    static_assert(saturating_div(std::int32_t{INT32_MIN}, std::int32_t{-1}) == INT32_MAX);
    static_assert(saturating_cast<std::int8_t>(300) == 127);
    static_assert(saturating_cast<std::uint8_t>(-1) == 0);
}

TEST(IntegralSaturating, Cast)
{
    std::mt19937_64 rng(0x9A0B1C2D);
    check_cast_boundaries<std::int32_t, std::int8_t>();
    check_cast_boundaries<std::int64_t, std::uint8_t>();
    check_cast_boundaries<std::uint64_t, std::int8_t>();
    check_cast_boundaries<std::uint32_t, std::uint8_t>();

    check_cast_random<std::int32_t, std::int8_t>(rng);
    check_cast_random<std::int32_t, std::uint8_t>(rng);
    check_cast_random<std::uint32_t, std::int8_t>(rng);
    check_cast_random<std::uint32_t, std::uint8_t>(rng);
    check_cast_random<std::int32_t, std::int64_t>(rng);
    check_cast_random<std::int64_t, std::int32_t>(rng);
    check_cast_random<std::int64_t, std::uint32_t>(rng);
    check_cast_random<std::uint64_t, std::int32_t>(rng);
}

#if defined(__SIZEOF_INT128__)
TEST(IntegralSaturating, Int128)
{
    using i128 = eirin::detail::int128_t;
    using u128 = eirin::detail::uint128_t;
    constexpr i128 max128 = static_cast<i128>((static_cast<u128>(1) << 127) - 1);
    constexpr i128 min128 = static_cast<i128>(static_cast<u128>(1) << 127);
    constexpr u128 umax128 = ~static_cast<u128>(0);

    // gtest cannot stream __int128 values, so use EXPECT_TRUE for these.
    EXPECT_TRUE(saturating_add(max128, i128(1)) == max128);
    EXPECT_TRUE(saturating_add(min128, i128(-1)) == min128);
    EXPECT_TRUE(saturating_sub(max128, i128(-1)) == max128);
    EXPECT_TRUE(saturating_sub(min128, i128(1)) == min128);
    EXPECT_TRUE(saturating_mul(min128, i128(-1)) == max128);
    EXPECT_TRUE(saturating_mul(i128(-1000000000), i128(1000000000)) == i128(-1000000000000000000));
    EXPECT_TRUE(saturating_div(min128, i128(-1)) == max128);
    EXPECT_TRUE(saturating_add(umax128, u128(1)) == umax128);
    EXPECT_TRUE(saturating_sub(u128(0), u128(1)) == u128(0));
    EXPECT_TRUE(saturating_cast<i128>(std::int64_t{INT64_MAX}) == i128(INT64_MAX));
    EXPECT_TRUE(saturating_cast<std::int64_t>(max128) == INT64_MAX);
}
#endif
