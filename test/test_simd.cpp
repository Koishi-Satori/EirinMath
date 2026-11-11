#include <gtest/gtest.h>
#include <eirin/fixed.hpp>
#include <eirin/detail/simd/abi.hpp>
#include <eirin/macro.hpp>

#include <array>
#include <type_traits>

#if defined(EIRIN_DEV_TEST_MODE) && EIRIN_PLATFORM_SIMD_HAS_AVX2 == EIRIN_TRUE && \
    EIRIN_SIMD_ENABLE == EIRIN_ENABLE && defined(EIRIN_MATH_HAS_INT128)
#    include <eirin/ext/simd_math.hpp>
#    include <eirin/math.hpp>
#    include <cstdint>
#    include <limits>
#    include <random>
#endif

TEST(SimdConfig, IsaMacrosAreBoolean)
{
    static_assert(EIRIN_PLATFORM_SIMD_HAS_SSE == EIRIN_TRUE || EIRIN_PLATFORM_SIMD_HAS_SSE == EIRIN_FALSE);
    static_assert(EIRIN_PLATFORM_SIMD_HAS_SSE2 == EIRIN_TRUE || EIRIN_PLATFORM_SIMD_HAS_SSE2 == EIRIN_FALSE);
    static_assert(EIRIN_PLATFORM_SIMD_HAS_AVX2 == EIRIN_TRUE || EIRIN_PLATFORM_SIMD_HAS_AVX2 == EIRIN_FALSE);
    static_assert(EIRIN_PLATFORM_SIMD_HAS_AVX512F == EIRIN_TRUE || EIRIN_PLATFORM_SIMD_HAS_AVX512F == EIRIN_FALSE);
    static_assert(EIRIN_PLATFORM_SIMD_HAS_AVX512DQ == EIRIN_TRUE || EIRIN_PLATFORM_SIMD_HAS_AVX512DQ == EIRIN_FALSE);
    static_assert(EIRIN_PLATFORM_SIMD_HAS_NEON == EIRIN_TRUE || EIRIN_PLATFORM_SIMD_HAS_NEON == EIRIN_FALSE);
    static_assert(EIRIN_PLATFORM_SIMD_HAS_WASM_SIMD128 == EIRIN_TRUE || EIRIN_PLATFORM_SIMD_HAS_WASM_SIMD128 == EIRIN_FALSE);
    static_assert(EIRIN_SIMD_ENABLE == EIRIN_ENABLE || EIRIN_SIMD_ENABLE == EIRIN_DISABLE);
}

TEST(SimdAbi, ExtractorBasics)
{
    using namespace eirin::simd_abi;

    struct unknown_abi
    {};

    static_assert(!abi_valid_v<int, unknown_abi>);
    static_assert(!abi_valid_v<bool, abi_scalar>);
    static_assert(!abi_valid_v<eirin::fixed32, abi_scalar>);
    static_assert(!abi_valid_v<float, abi_fixed<0>>);

    static_assert(abi_valid_v<int, abi_scalar>);
    static_assert(abi_size_v<int, abi_scalar> == 1);
    static_assert(abi_alignment_v<int, abi_scalar> == alignof(int));

    static_assert(abi_valid_v<float, abi_fixed<4>>);
    static_assert(abi_size_v<float, abi_fixed<4>> == 4);
    static_assert(abi_traits<float, abi_fixed<4>>::full_size == 4);
    static_assert(std::is_same_v<
                  typename abi_traits<float, abi_fixed<4>>::member_type,
                  std::array<float, 4>>);
}

TEST(SimdAbi, X86Extractors)
{
    using namespace eirin::simd_abi;

#if defined(EIRIN_ARCH_X86) && EIRIN_PLATFORM_SIMD_HAS_FULL_SSE_ABI == EIRIN_TRUE
    static_assert(abi_valid_v<float, abi_sse2_128>);
    static_assert(abi_size_v<float, abi_sse2_128> == 4);
    static_assert(abi_size_v<double, abi_sse2_128> == 2);
    static_assert(abi_size_v<std::int64_t, abi_sse2_128> == 2);
    static_assert(abi_alignment_v<float, abi_sse2_128> == 16);
#endif

#if defined(EIRIN_ARCH_X86) && EIRIN_PLATFORM_SIMD_HAS_FULL_AVX_ABI == EIRIN_TRUE
    static_assert(abi_valid_v<float, abi_avx2_256>);
    static_assert(abi_size_v<float, abi_avx2_256> == 8);
    static_assert(abi_size_v<double, abi_avx2_256> == 4);
    static_assert(abi_size_v<std::int64_t, abi_avx2_256> == 4);
    static_assert(abi_alignment_v<float, abi_avx2_256> == 32);
#endif

#if defined(EIRIN_ARCH_X86) && EIRIN_PLATFORM_SIMD_HAS_AVX512_ABI == EIRIN_TRUE
    static_assert(abi_valid_v<float, abi_avx512_512>);
    static_assert(abi_size_v<float, abi_avx512_512> == 16);
    static_assert(abi_size_v<double, abi_avx512_512> == 8);
    static_assert(abi_size_v<std::int64_t, abi_avx512_512> == 8);
    static_assert(abi_alignment_v<float, abi_avx512_512> == 64);
#endif
}

TEST(SimdAbi, OtherArchitectureExtractors)
{
    using namespace eirin::simd_abi;

#if EIRIN_PLATFORM_SIMD_HAS_NEON == EIRIN_TRUE
    static_assert(abi_valid_v<float, abi_neon_128>);
    static_assert(abi_size_v<float, abi_neon_128> == 4);
    static_assert(abi_size_v<std::int64_t, abi_neon_128> == 2);
    static_assert(abi_alignment_v<float, abi_neon_128> == 16);
#endif

#if EIRIN_PLATFORM_SIMD_HAS_WASM_SIMD128 == EIRIN_TRUE
    static_assert(abi_valid_v<float, abi_wasm_simd128>);
    static_assert(abi_size_v<float, abi_wasm_simd128> == 4);
    static_assert(abi_size_v<std::int64_t, abi_wasm_simd128> == 2);
    static_assert(abi_alignment_v<float, abi_wasm_simd128> == 16);
#endif
}

#if defined(EIRIN_DEV_TEST_MODE) && EIRIN_PLATFORM_SIMD_HAS_AVX2 == EIRIN_TRUE && \
    EIRIN_SIMD_ENABLE == EIRIN_ENABLE && defined(EIRIN_MATH_HAS_INT128)
TEST(SimdFixed64, MultiplyMatchesScalar)
{
    std::mt19937_64 rng(0xE1A17);
    std::uniform_int_distribution<std::int64_t> dist(
        std::numeric_limits<std::int64_t>::min(),
        std::numeric_limits<std::int64_t>::max()
    );

    for(int round = 0; round < 2500; ++round)
    {
        std::array<eirin::fixed64, 4> lhs;
        std::array<eirin::fixed64, 4> rhs;
        for(std::size_t i = 0; i < lhs.size(); ++i)
        {
            lhs[i] = eirin::fixed64::from_internal_value(dist(rng));
            rhs[i] = eirin::fixed64::from_internal_value(dist(rng));
        }

        const auto result = eirin::simd::simd_fixed64_mul(lhs, rhs);
        for(std::size_t i = 0; i < lhs.size(); ++i)
        {
            EXPECT_EQ(result[i].internal_value(), (lhs[i] * rhs[i]).internal_value()) << "lane " << i;
        }
    }
}

TEST(SimdFixed64, ReduceAngleMatchesScalar)
{
    std::mt19937_64 rng(0xA17E);
    std::uniform_real_distribution<double> dist(-100.0, 100.0);
    const eirin::fixed64 one(1);
    const auto inv_two_pi = one / (eirin::numbers::pi_f64 * eirin::fixed64(2));
    const auto two_pi = eirin::numbers::pi_f64 * eirin::fixed64(2);

    for(int round = 0; round < 1000; ++round)
    {
        std::array<eirin::fixed64, 4> input;
        for(auto& value : input)
            value = eirin::fixed64(dist(rng));

        const auto result = eirin::simd::simd_reduce_angle(input);
        for(std::size_t i = 0; i < input.size(); ++i)
        {
            const auto expected = input[i] - eirin::floor(input[i] * inv_two_pi) * two_pi;
            EXPECT_EQ(result[i].internal_value(), expected.internal_value()) << "lane " << i;
        }
    }
}
#endif
