#ifndef EIRIN_MATH_DETAIL_SIMD_BACKEND_HPP
#define EIRIN_MATH_DETAIL_SIMD_BACKEND_HPP

#pragma once

#include <type_traits>
#include "../../macro.hpp"
#if defined(EIRIN_ARCH_X86) && EIRIN_PLATFORM_SIMD_HAS_SSE == EIRIN_TRUE
#    include <immintrin.h>
#endif

#if EIRIN_PLATFORM_SIMD_HAS_NEON == EIRIN_TRUE
#    include <arm_neon.h>
#endif

#if EIRIN_PLATFORM_SIMD_HAS_WASM_SIMD128 == EIRIN_TRUE
#    include <wasm_simd128.h>
#endif
#include "abi.hpp"

namespace eirin::simd
{
template <typename ABI>
struct simd_backend
{
    using backend_abi = ABI;
    template <typename T>
    using storage_type = typename ::eirin::simd_abi::abi_traits<T, backend_abi>::member_type;

    static constexpr bool is_implemented = false;
};

#define EIRIN_SIMD_BACKEND_REQUIRES          requires(::eirin::simd_abi::abi_valid_v<T, backend_abi>)
#define EIRIN_SIMD_BACKEND_DECLARE           EIRIN_SIMD_BACKEND_REQUIRES EIRIN_SIMD_INTRINSIC_DECLARE
#define EIRIN_SIMD_BACKEND_DECLARE_NODISCARD EIRIN_SIMD_BACKEND_REQUIRES [[nodiscard]] EIRIN_SIMD_INTRINSIC_DECLARE

template <>
struct simd_backend<::eirin::simd_abi::abi_scalar>
{
    using backend_abi = ::eirin::simd_abi::abi_scalar;
    template <typename T>
    using storage_type = typename ::eirin::simd_abi::abi_traits<T, backend_abi>::member_type;

    static constexpr bool is_implemented = true;

    template <typename T>
    EIRIN_SIMD_BACKEND_DECLARE_NODISCARD static storage_type<T> load(const T* src) noexcept
    {
        return *src;
    }

    template <typename T>
    EIRIN_SIMD_BACKEND_DECLARE static void store(T* dest, storage_type<T> src) noexcept
    {
        *dest = src;
    }
};

#if defined(EIRIN_ARCH_X86) && EIRIN_PLATFORM_SIMD_HAS_SSE == EIRIN_TRUE
template <>
struct simd_backend<::eirin::simd_abi::abi_sse2_128>
{
    using backend_abi = ::eirin::simd_abi::abi_sse2_128;
    template <typename T>
    using storage_type = typename ::eirin::simd_abi::abi_traits<T, backend_abi>::member_type;

    static constexpr bool is_implemented = true;

    template <typename T>
    EIRIN_SIMD_BACKEND_DECLARE_NODISCARD static storage_type<T> load(const T* src) noexcept
    {
        if constexpr(std::is_same_v<T, float>)
            return _mm_loadu_ps(src);
        else if constexpr(std::is_same_v<T, double>)
            return _mm_loadu_pd(src);
        else
            return _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));
    }

    template <typename T>
    EIRIN_SIMD_BACKEND_DECLARE static void store(T* dest, storage_type<T> src) noexcept
    {
        if constexpr(std::is_same_v<T, float>)
            _mm_storeu_ps(dest, src);
        else if constexpr(std::is_same_v<T, double>)
            _mm_storeu_pd(dest, src);
        else
            _mm_storeu_si128(reinterpret_cast<__m128i*>(dest), src);
    }
};
#endif

#if defined(EIRIN_ARCH_X86) && EIRIN_PLATFORM_SIMD_HAS_FULL_AVX_ABI == EIRIN_TRUE
template <>
struct simd_backend<::eirin::simd_abi::abi_avx2_256>
{
    using backend_abi = ::eirin::simd_abi::abi_avx2_256;
    template <typename T>
    using storage_type = typename ::eirin::simd_abi::abi_traits<T, backend_abi>::member_type;

    static constexpr bool is_implemented = true;

    template <typename T>
    EIRIN_SIMD_BACKEND_DECLARE_NODISCARD static storage_type<T> load(const T* src) noexcept
    {
        using lane_type = std::remove_cv_t<T>;

        if constexpr(std::is_same_v<lane_type, float>)
        {
            return _mm256_loadu_ps(src);
        }
        else if constexpr(std::is_same_v<lane_type, double>)
        {
            return _mm256_loadu_pd(src);
        }
        else
        {
            static_assert(
                std::is_integral_v<lane_type>,
                "AVX2 backend only supports float, double, and integral lanes."
            );
            return _mm256_loadu_si256(
                reinterpret_cast<const __m256i*>(src)
            );
        }
    }

    template <typename T>
    EIRIN_SIMD_BACKEND_DECLARE static void store(T* dest, storage_type<T> src) noexcept
    {
        using lane_type = std::remove_cv_t<T>;

        if constexpr(std::is_same_v<lane_type, float>)
        {
            _mm256_storeu_ps(dest, src);
        }
        else if constexpr(std::is_same_v<lane_type, double>)
        {
            _mm256_storeu_pd(dest, src);
        }
        else
        {
            static_assert(
                std::is_integral_v<lane_type>,
                "AVX2 backend only supports float, double, and integral lanes."
            );
            _mm256_storeu_si256(
                reinterpret_cast<__m256i*>(dest),
                src
            );
        }
    }
};
#endif

#undef EIRIN_SIMD_BACKEND_REQUIRES
#undef EIRIN_SIMD_BACKEND_DECLARE
#undef EIRIN_SIMD_BACKEND_DECLARE_NODISCARD
} // namespace eirin::simd

#endif
