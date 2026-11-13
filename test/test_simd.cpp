#include <gtest/gtest.h>
#include <eirin/fixed.hpp>
#include <eirin/detail/simd/abi.hpp>
#include <eirin/detail/simd/fixed_size.hpp>
#include <eirin/macro.hpp>

#include <array>
#include <cstring>
#include <type_traits>

#if defined(EIRIN_DEV_TEST_MODE) && EIRIN_PLATFORM_SIMD_HAS_AVX2 == EIRIN_TRUE && \
    EIRIN_SIMD_ENABLE == EIRIN_ENABLE && defined(EIRIN_MATH_HAS_INT128)
#    include <eirin/ext/simd_math.hpp>
#    include <eirin/math.hpp>
#    include <cstdint>
#    include <limits>
#    include <random>
#endif

namespace
{
template <typename T, typename = void>
struct has_native_abi : std::false_type
{};

template <typename T>
struct has_native_abi<T, std::void_t<eirin::simd_abi::native_abi_t<T>>> : std::true_type
{};

template <typename T, std::size_t N>
void expect_storage_round_trip()
{
    using storage = eirin::simd_abi::fixed_storage<T, N>;

    std::array<T, N> input{};
    for(std::size_t i = 0; i < N; ++i)
        input[i] = static_cast<T>(i + 1);

    const auto loaded = storage::load(input.data());

    std::array<T, N + 4> output;
    output.fill(static_cast<T>(99));
    loaded.store(output.data() + 2);

    EXPECT_EQ(output[0], static_cast<T>(99));
    EXPECT_EQ(output[1], static_cast<T>(99));
    for(std::size_t i = 0; i < N; ++i)
        EXPECT_EQ(output[i + 2], input[i]) << "lane " << i;
    for(std::size_t i = N + 2; i < output.size(); ++i)
        EXPECT_EQ(output[i], static_cast<T>(99)) << "tail lane " << i;

    std::array<T, storage::capacity> padded{};
    static_assert(sizeof(loaded) == sizeof(padded));
    std::memcpy(padded.data(), &loaded, sizeof(padded));
    for(std::size_t i = N; i < padded.size(); ++i)
        EXPECT_EQ(padded[i], T{}) << "padding lane " << i;
}
} // namespace

#if defined(EIRIN_SIMD_TEST_EXPECT_AVX2)
static_assert(EIRIN_PLATFORM_SIMD_HAS_AVX2 == EIRIN_TRUE);
static_assert(EIRIN_PLATFORM_SIMD_HAS_AVX512F == EIRIN_FALSE);
static_assert(eirin::simd::simd_backend<eirin::simd_abi::abi_avx2_256>::is_implemented);

static_assert(std::is_same_v<
              typename eirin::simd_abi::fixed_abi_t<float, 5>::first_abi,
              eirin::simd_abi::abi_avx2_256>);
static_assert(std::is_same_v<
              typename eirin::simd_abi::fixed_abi_t<float, 9>::first_abi,
              eirin::simd_abi::abi_avx2_256>);
static_assert(std::is_same_v<
              typename eirin::simd_abi::fixed_abi_t<double, 5>::first_abi,
              eirin::simd_abi::abi_avx2_256>);
static_assert(std::is_same_v<
              typename eirin::simd_abi::fixed_abi_t<std::int64_t, 9>::first_abi,
              eirin::simd_abi::abi_avx2_256>);

static_assert(eirin::simd_abi::fixed_abi_t<float, 5>::size() == 1);
static_assert(eirin::simd_abi::fixed_abi_t<float, 9>::size() == 2);
static_assert(eirin::simd_abi::fixed_abi_t<double, 5>::size() == 2);
static_assert(eirin::simd_abi::fixed_abi_t<std::int64_t, 9>::size() == 4);

static_assert(std::is_same_v<
              typename eirin::simd_abi::fixed_abi_t<float, 3>::first_abi,
              eirin::simd_abi::abi_sse2_128>);
static_assert(std::is_same_v<
              typename eirin::simd_abi::fixed_abi_t<float, 2>::first_abi,
              eirin::simd_abi::abi_scalar>);
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

TEST(SimdAbi, NativeSelection)
{
    using namespace eirin::simd_abi;

    static_assert(has_native_abi<float>::value);
    static_assert(has_native_abi<std::int64_t>::value);
    static_assert(!has_native_abi<eirin::fixed32>::value);

#if EIRIN_SIMD_ENABLE == EIRIN_DISABLE
    static_assert(std::is_same_v<native_abi_t<float>, abi_scalar>);
    static_assert(std::is_same_v<native_abi_t<std::int64_t>, abi_scalar>);
#elif defined(EIRIN_ARCH_X86)
#    if EIRIN_PLATFORM_SIMD_HAS_AVX512F == EIRIN_TRUE
    static_assert(std::is_same_v<native_abi_t<float>, abi_avx512_512>);
    static_assert(std::is_same_v<native_abi_t<std::int64_t>, abi_avx512_512>);
#    elif EIRIN_PLATFORM_SIMD_HAS_FULL_AVX_ABI == EIRIN_TRUE
    static_assert(std::is_same_v<native_abi_t<float>, abi_avx2_256>);
    static_assert(std::is_same_v<native_abi_t<std::int64_t>, abi_avx2_256>);
#    elif EIRIN_PLATFORM_SIMD_HAS_FULL_SSE_ABI == EIRIN_TRUE
    static_assert(std::is_same_v<native_abi_t<float>, abi_sse2_128>);
    static_assert(std::is_same_v<native_abi_t<std::int64_t>, abi_sse2_128>);
#    else
    static_assert(std::is_same_v<native_abi_t<float>, abi_scalar>);
    static_assert(std::is_same_v<native_abi_t<std::int64_t>, abi_scalar>);
#    endif
#elif EIRIN_PLATFORM_SIMD_HAS_NEON == EIRIN_TRUE
    static_assert(std::is_same_v<native_abi_t<float>, abi_neon_128>);
    static_assert(std::is_same_v<native_abi_t<std::int64_t>, abi_neon_128>);
#elif EIRIN_PLATFORM_SIMD_HAS_WASM_SIMD128 == EIRIN_TRUE
    static_assert(std::is_same_v<native_abi_t<float>, abi_wasm_simd128>);
    static_assert(std::is_same_v<native_abi_t<std::int64_t>, abi_wasm_simd128>);
#else
    static_assert(std::is_same_v<native_abi_t<float>, abi_scalar>);
    static_assert(std::is_same_v<native_abi_t<std::int64_t>, abi_scalar>);
#endif
}

TEST(SimdAbi, FixedStorage)
{
    using namespace eirin::simd_abi;

    using selected = fixed_abi_t<float, 5>;
    using storage = fixed_storage<const float, 5>;

    static_assert(fixed_abi_t<float, 3>::capacity() == 4);
    static_assert(fixed_abi_t<float, 9>::capacity() == 16);
    static_assert(std::is_same_v<typename selected::value_type, float>);
    static_assert(selected::capacity() == 8);

    static_assert(std::is_same_v<typename storage::value_type, float>);
    static_assert(std::is_same_v<typename storage::storage_type, selected>);
    static_assert(std::is_base_of_v<selected, storage>);
    static_assert(storage::logical_size == 5);
    static_assert(storage::capacity == 8);
    static_assert(storage::padding_size == 3);
    static_assert(storage::size() == 5);
    static_assert(storage::storage_size() == 8);
    static_assert(storage::chunk_count >= 1);
    static_assert(storage::chunk_count <= storage::capacity);
    static_assert(std::is_trivially_copyable_v<storage>);
    static_assert(sizeof(storage) >= storage::capacity * sizeof(float));

    fixed_storage<int, 1> scalar(42);
    EXPECT_EQ(scalar.at<0>(), 42);
    EXPECT_EQ(scalar.storage().capacity(), 1);

    std::array<float, 5> input{1.f, 2.f, 3.f, 4.f, 5.f};
    const auto loaded = fixed_storage<float, 5>::load(input.data());
    std::array<float, 5> output{};
    loaded.store(output.data());
    EXPECT_EQ(output, input);
}

TEST(SimdStorage, RoundTripAndTailSafety)
{
    expect_storage_round_trip<int, 1>();
    expect_storage_round_trip<float, 2>();
    expect_storage_round_trip<float, 3>();
    expect_storage_round_trip<float, 5>();
    expect_storage_round_trip<float, 9>();
    expect_storage_round_trip<double, 5>();
    expect_storage_round_trip<std::int64_t, 9>();
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
