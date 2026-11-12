#ifndef EIRIN_MATH_DETAIL_SIMD_ABI_HPP
#define EIRIN_MATH_DETAIL_SIMD_ABI_HPP

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <concepts>
#include "../../macro.hpp"
#include "detail.hpp"

#if defined(EIRIN_ARCH_X86) && EIRIN_PLATFORM_SIMD_HAS_SSE == EIRIN_TRUE
#    include <immintrin.h>
#endif

#if EIRIN_PLATFORM_SIMD_HAS_NEON == EIRIN_TRUE
#    include <arm_neon.h>
#endif

#if EIRIN_PLATFORM_SIMD_HAS_WASM_SIMD128 == EIRIN_TRUE
#    include <wasm_simd128.h>
#endif

namespace eirin::simd_abi
{
struct abi_scalar;

template <std::size_t N>
struct abi_fixed;

struct abi_sse2_128;

struct abi_avx2_256;

struct abi_avx512_512;

struct abi_neon_128;

struct abi_wasm_simd128;

template <typename T>
struct native_abi_selector;

template <typename T>
using native_abi_t = typename native_abi_selector<T>::type;

template <typename T, std::size_t N>
struct fixed_abi_selector;

template <typename T, typename... ABIs>
struct simd_sequence;

// Completed by detail/simd/fixed_size.hpp, which selects the concrete ABI chunks.
template <typename T, std::size_t N>
using fixed_abi_t = typename fixed_abi_selector<T, N>::type;

template <typename T>
struct is_fixed_size_abi : std::false_type
{};

template <std::size_t N>
struct is_fixed_size_abi<abi_fixed<N>> : std::true_type
{};

template <typename T>
inline constexpr bool is_fixed_size_abi_v = is_fixed_size_abi<T>::value;

template <typename T>
concept is_fixed_abi = is_fixed_size_abi_v<T>;

namespace detail
{
    template <typename T, bool IsArithmetic = std::is_arithmetic_v<T>>
    struct is_simd_lane : std::false_type
    {};

    template <typename T>
    struct is_simd_lane<T, true> : std::bool_constant<!std::is_same_v<T, bool> && (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8)>
    {};

    template <typename T>
    inline constexpr bool is_simd_lane_v = is_simd_lane<T>::value;

    template <typename T, bool IsLane = is_simd_lane_v<T>>
    struct is_x86_lane : std::false_type
    {};

    template <typename T>
    struct is_x86_lane<T, true> : std::bool_constant<!std::is_floating_point_v<T> || sizeof(T) == 4 || sizeof(T) == 8>
    {};

    template <typename T>
    inline constexpr bool is_x86_lane_v = is_x86_lane<T>::value;

    template <typename T, bool IsLane = is_simd_lane_v<T>>
    struct lane_info
    {
        static constexpr std::size_t lane_size = 0;
        static constexpr std::size_t alignment = 1;
    };

    template <typename T>
    struct lane_info<T, true>
    {
        static constexpr std::size_t lane_size = sizeof(T);
        static constexpr std::size_t alignment = alignof(T);
    };

    template <typename T, typename ABI, typename Native, typename Mask, std::size_t RegisterBits, bool PlatformAvailable>
    struct native_abi_traits
    {
        static constexpr std::size_t lane_size = lane_info<T>::lane_size;
        static constexpr bool has_valid_width =
            lane_size != 0 && RegisterBits % (lane_size * 8) == 0;
        static constexpr bool is_valid = PlatformAvailable && has_valid_width && !std::is_void_v<Native>;
        static constexpr std::size_t size = is_valid ? RegisterBits / (lane_size * 8) : 0;
        static constexpr std::size_t full_size = size;
        static constexpr std::size_t alignment = is_valid ? RegisterBits / 8 : 1;
        static constexpr bool is_partial = false;

        using value_type = T;
        using abi_type = ABI;
        using member_type = Native;
        using native_type = Native;
        using mask_type = Mask;
    };

    template <typename T, std::size_t N, bool IsLane = is_simd_lane_v<T>>
    struct fixed_member
    {
        using type = void;
    };

    template <typename T, std::size_t N>
    struct fixed_member<T, N, true>
    {
        using type = std::array<T, N>;
    };

#if defined(EIRIN_ARCH_X86) && EIRIN_PLATFORM_SIMD_HAS_SSE == EIRIN_TRUE
    template <typename T, std::size_t RegisterBits, bool IsLane = is_simd_lane_v<T>>
    struct x86_native_type
    {
        using type = void;
    };

    template <typename T, bool IsFloat = std::is_floating_point_v<T>>
    struct x86_native_128_impl
    {
        using type = __m128i;
    };

    template <typename T>
    struct x86_native_128_impl<T, true>
    {
        using type = __m128;
    };

    template <>
    struct x86_native_128_impl<double, true>
    {
        using type = __m128d;
    };

    template <typename T, bool IsFloat = std::is_floating_point_v<T>>
    struct x86_native_256_impl
    {
        using type = __m256i;
    };

    template <typename T>
    struct x86_native_256_impl<T, true>
    {
        using type = __m256;
    };

    template <>
    struct x86_native_256_impl<double, true>
    {
        using type = __m256d;
    };

    template <typename T, bool IsFloat = std::is_floating_point_v<T>>
    struct x86_native_512_impl
    {
        using type = __m512i;
    };

    template <typename T>
    struct x86_native_512_impl<T, true>
    {
        using type = __m512;
    };

    template <>
    struct x86_native_512_impl<double, true>
    {
        using type = __m512d;
    };

    template <typename T>
    struct x86_native_type<T, 128, true> : x86_native_128_impl<T>
    {
    };

    template <typename T>
    struct x86_native_type<T, 256, true> : x86_native_256_impl<T>
    {
    };

    template <typename T>
    struct x86_native_type<T, 512, true> : x86_native_512_impl<T>
    {
    };

    template <std::size_t Lanes>
    using x86_mask_type = std::conditional_t<
        Lanes <= 8,
        __mmask8,
        std::conditional_t<Lanes <= 16, __mmask16, std::conditional_t<Lanes <= 32, __mmask32, __mmask64>>>;

    template <typename T, typename ABI, std::size_t RegisterBits, bool PlatformAvailable>
    struct x86_abi_traits : native_abi_traits<T, ABI, typename x86_native_type<T, RegisterBits>::type, typename x86_native_type<T, RegisterBits>::type, RegisterBits, PlatformAvailable && is_x86_lane_v<T>>
    {};
#endif

#if EIRIN_PLATFORM_SIMD_HAS_NEON == EIRIN_TRUE
    template <typename T>
    struct neon_native_type
    {
        using type = void;
    };

#    define EIRIN_DEFINE_NEON_NATIVE(T, Native) \
        template <>                             \
        struct neon_native_type<T>              \
        {                                       \
            using type = Native;                \
        }

    EIRIN_DEFINE_NEON_NATIVE(std::int8_t, int8x16_t);
    EIRIN_DEFINE_NEON_NATIVE(std::uint8_t, uint8x16_t);
    EIRIN_DEFINE_NEON_NATIVE(std::int16_t, int16x8_t);
    EIRIN_DEFINE_NEON_NATIVE(std::uint16_t, uint16x8_t);
    EIRIN_DEFINE_NEON_NATIVE(std::int32_t, int32x4_t);
    EIRIN_DEFINE_NEON_NATIVE(std::uint32_t, uint32x4_t);
    EIRIN_DEFINE_NEON_NATIVE(std::int64_t, int64x2_t);
    EIRIN_DEFINE_NEON_NATIVE(std::uint64_t, uint64x2_t);
    EIRIN_DEFINE_NEON_NATIVE(float, float32x4_t);

#    if EIRIN_PLATFORM_SIMD_HAS_NEON_A64 == EIRIN_TRUE
    EIRIN_DEFINE_NEON_NATIVE(double, float64x2_t);
#    endif

#    undef EIRIN_DEFINE_NEON_NATIVE

    template <typename T>
    struct neon_abi_traits : native_abi_traits<T, abi_neon_128, typename neon_native_type<T>::type, typename neon_native_type<T>::type, 128, true>
    {};
#endif

} // namespace detail

template <typename T, typename ABI>
struct abi_traits
{
    static constexpr bool is_valid = false;
    static constexpr std::size_t size = 0;
    static constexpr std::size_t full_size = 0;
    static constexpr std::size_t alignment = 1;
    static constexpr bool is_partial = false;

    using value_type = T;
    using abi_type = ABI;
    using member_type = void;
    using native_type = void;
    using mask_type = void;
};

struct abi_scalar
{};

template <std::size_t N>
struct abi_fixed
{};

struct abi_sse2_128
{};

struct abi_avx2_256
{};

struct abi_avx512_512
{};

struct abi_neon_128
{};

struct abi_wasm_simd128
{};

template <typename T, std::size_t N>
struct fixed_abi_selector
{
};

template <typename T>
struct abi_traits<T, abi_scalar>
{
    static constexpr bool is_valid = detail::is_simd_lane_v<T>;
    static constexpr std::size_t size = is_valid ? 1 : 0;
    static constexpr std::size_t full_size = size;
    static constexpr std::size_t alignment = is_valid ? detail::lane_info<T>::alignment : 1;
    static constexpr bool is_partial = false;

    using value_type = T;
    using abi_type = abi_scalar;
    using member_type = std::conditional_t<is_valid, T, void>;
    using native_type = member_type;
    using mask_type = std::conditional_t<is_valid, bool, void>;
};

template <typename T, std::size_t N>
struct abi_traits<T, abi_fixed<N>>
{
    static constexpr bool is_valid = detail::is_simd_lane_v<T> && N > 0;
    static constexpr std::size_t size = is_valid ? N : 0;
    static constexpr std::size_t full_size = size;
    static constexpr std::size_t alignment = is_valid ? detail::lane_info<T>::alignment : 1;
    static constexpr bool is_partial = false;

    using value_type = T;
    using abi_type = abi_fixed<N>;
    using member_type = typename detail::fixed_member<T, N>::type;
    using native_type = std::conditional_t<is_valid, T, void>;
    using mask_type = std::conditional_t<is_valid, bool, void>;
};

#if defined(EIRIN_ARCH_X86) && EIRIN_PLATFORM_SIMD_HAS_SSE == EIRIN_TRUE
template <typename T>
struct abi_traits<T, abi_sse2_128> : detail::x86_abi_traits<T, abi_sse2_128, 128, platform_has_full_sse_abi>
{};

template <typename T>
struct abi_traits<T, abi_avx2_256> : detail::x86_abi_traits<T, abi_avx2_256, 256, platform_has_full_avx_abi>
{};

template <typename T>
struct abi_traits<T, abi_avx512_512>
{
    using native_type = typename detail::x86_native_type<T, 512>::type;
    static constexpr bool is_valid = detail::native_abi_traits <
                                     T,
                          abi_avx512_512,
                          native_type,
                          native_type,
                          512,
                          platform_has_avx512_abi &&detail::is_x86_lane_v < T >> ::is_valid;
    static constexpr std::size_t size = is_valid ? 512 / (detail::lane_info<T>::lane_size * 8) : 0;
    static constexpr std::size_t full_size = size;
    static constexpr std::size_t alignment = is_valid ? 64 : 1;
    static constexpr bool is_partial = false;

    using value_type = T;
    using abi_type = abi_avx512_512;
    using member_type = native_type;
    using mask_type = detail::x86_mask_type<size>;
};
#endif

#if EIRIN_PLATFORM_SIMD_HAS_NEON == EIRIN_TRUE
template <typename T>
struct abi_traits<T, abi_neon_128> : detail::neon_abi_traits<T>
{};
#endif

#if EIRIN_PLATFORM_SIMD_HAS_WASM_SIMD128 == EIRIN_TRUE
template <typename T>
struct abi_traits<T, abi_wasm_simd128> : detail::native_abi_traits<T, abi_wasm_simd128, v128_t, v128_t, 128, true>
{};
#endif

template <typename T, typename ABI>
inline constexpr bool abi_valid_v = abi_traits<T, ABI>::is_valid;

template <typename T, typename ABI>
inline constexpr std::size_t abi_size_v = abi_traits<T, ABI>::size;

template <typename T, typename ABI>
inline constexpr std::size_t abi_alignment_v = abi_traits<T, ABI>::alignment;

namespace detail
{
    template <typename T>
    constexpr auto determine_native_abi()
    {
        using lane_type = std::remove_cv_t<T>;
        static_assert(is_simd_lane_v<lane_type>, "native ABI deduction requires a supported SIMD lane type.");

        if constexpr(EIRIN_SIMD_ENABLE == EIRIN_DISABLE)
        {
            return static_cast<abi_scalar*>(nullptr);
        }
#if defined(EIRIN_ARCH_X86)
        else
        {
            constexpr std::size_t bytes = vectorized_sizeof<lane_type>();

            if constexpr(bytes >= 64 && abi_traits<lane_type, abi_avx512_512>::is_valid)
                return static_cast<abi_avx512_512*>(nullptr);
            else if constexpr(bytes >= 32 && abi_traits<lane_type, abi_avx2_256>::is_valid)
                return static_cast<abi_avx2_256*>(nullptr);
            else if constexpr(bytes >= 16 && abi_traits<lane_type, abi_sse2_128>::is_valid)
                return static_cast<abi_sse2_128*>(nullptr);
            else
                return static_cast<abi_scalar*>(nullptr);
        }
#elif EIRIN_PLATFORM_SIMD_HAS_NEON == EIRIN_TRUE
        else if constexpr(abi_traits<lane_type, abi_neon_128>::is_valid)
        {
            return static_cast<abi_neon_128*>(nullptr);
        }
        else
        {
            return static_cast<abi_scalar*>(nullptr);
        }
#elif EIRIN_PLATFORM_SIMD_HAS_WASM_SIMD128 == EIRIN_TRUE
        else if constexpr(abi_traits<lane_type, abi_wasm_simd128>::is_valid)
        {
            return static_cast<abi_wasm_simd128*>(nullptr);
        }
        else
        {
            return static_cast<abi_scalar*>(nullptr);
        }
#else
        else
        {
            return static_cast<abi_scalar*>(nullptr);
        }
#endif
    }
} // namespace detail

template <typename T>
requires detail::is_simd_lane_v<T>
struct native_abi_selector<T>
{
    using type = std::remove_pointer_t<decltype(detail::determine_native_abi<T>())>;
};
} // namespace eirin::simd_abi

#endif
