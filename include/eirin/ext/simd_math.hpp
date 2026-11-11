#ifndef EIRIN_MATH_EXT_SIMD_MATH_HPP
#define EIRIN_MATH_EXT_SIMD_MATH_HPP

#pragma once

#include "../macro.hpp"
#include "../fixed.hpp"
#include "../numbers.hpp"
#include <array>
#include <cstdint>

#if defined(EIRIN_ARCH_X86)
#    if defined(_MSC_VER) && !defined(__clang__)
#        include <intrin.h>
#    else
#        include <cpuid.h>
#    endif
#endif

#if EIRIN_SIMD_ENABLE == EIRIN_ENABLE && defined(EIRIN_ARCH_X86) && \
    (EIRIN_PLATFORM_SIMD_HAS_SSE == EIRIN_TRUE || EIRIN_PLATFORM_SIMD_HAS_AVX == EIRIN_TRUE)
#    include <immintrin.h>
#endif

namespace eirin::simd
{
/**
 * @brief Runtime ISA check auxiliary.
 */
class platform_support
{
    struct x86_features
    {
        bool sse2 = false;
        bool sse4_2 = false;
        bool avx = false;
        bool avx2 = false;
        bool avx512_f = false;
        bool avx512_dq = false;
    };

#if defined(EIRIN_ARCH_X86)
    static std::uint64_t read_xcr0() noexcept
    {
#    if defined(_MSC_VER) && !defined(__clang__)
        return _xgetbv(0);
#    else
        std::uint32_t eax;
        std::uint32_t edx;
        __asm__ volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
        return (static_cast<std::uint64_t>(edx) << 32) | eax;
#    endif
    }

    EIRIN_ALWAYS_INLINE static x86_features detect_features() noexcept
    {
        x86_features result;
        std::uint32_t eax = 0;
        std::uint32_t ebx = 0;
        std::uint32_t ecx = 0;
        std::uint32_t edx = 0;
        std::uint32_t max_leaf = 0;

#    if defined(_MSC_VER) && !defined(__clang__)
        int cpu_info[4];
        __cpuid(cpu_info, 0);
        max_leaf = static_cast<std::uint32_t>(cpu_info[0]);
        if(max_leaf < 1)
            return result;

        __cpuid(cpu_info, 1);
        ecx = static_cast<std::uint32_t>(cpu_info[2]);
        edx = static_cast<std::uint32_t>(cpu_info[3]);
#    else
        __cpuid(0, eax, ebx, ecx, edx);
        max_leaf = eax;
        if(max_leaf < 1)
            return result;

        __cpuid(1, eax, ebx, ecx, edx);
#    endif

        result.sse2 = (edx & (1U << 26)) != 0;
        result.sse4_2 = (ecx & (1U << 20)) != 0;

        const bool osxsave = (ecx & (1U << 27)) != 0;
        const bool cpu_avx = (ecx & (1U << 28)) != 0;
        const std::uint64_t xcr0 = osxsave ? read_xcr0() : 0;
        const bool avx_os = osxsave && ((xcr0 & 0x6) == 0x6);
        result.avx = cpu_avx && avx_os;

        if(max_leaf < 7)
            return result;

#    if defined(_MSC_VER) && !defined(__clang__)
        __cpuidex(cpu_info, 7, 0);
        eax = static_cast<std::uint32_t>(cpu_info[0]);
        ebx = static_cast<std::uint32_t>(cpu_info[1]);
        ecx = static_cast<std::uint32_t>(cpu_info[2]);
        edx = static_cast<std::uint32_t>(cpu_info[3]);
#    else
        __cpuid_count(7, 0, eax, ebx, ecx, edx);
#    endif

        result.avx2 = result.avx && (ebx & (1U << 5)) != 0;
        const bool avx512_os = avx_os && ((xcr0 & 0xe6) == 0xe6);
        result.avx512_f = avx512_os && (ebx & (1U << 16)) != 0;
        result.avx512_dq = result.avx512_f && (ebx & (1U << 17)) != 0;
        return result;
    }
#else
    EIRIN_ALWAYS_INLINE static x86_features detect_features() noexcept
    {
        return {};
    }
#endif

    static const x86_features& host_features() noexcept
    {
        static const x86_features features = detect_features();
        return features;
    }

public:
    [[nodiscard]] static bool supports_sse2() noexcept
    {
        return host_features().sse2;
    }

    [[nodiscard]] static bool supports_sse4_2() noexcept
    {
        return host_features().sse4_2;
    }

    [[nodiscard]] static bool supports_avx() noexcept
    {
        return host_features().avx;
    }

    [[nodiscard]] static bool supports_avx2() noexcept
    {
        return host_features().avx2;
    }

    [[nodiscard]] static bool supports_avx512_f() noexcept
    {
        return host_features().avx512_f;
    }

    [[nodiscard]] static bool supports_avx512_dq() noexcept
    {
        return host_features().avx512_dq;
    }
};

#if EIRIN_SIMD_ENABLE == EIRIN_ENABLE && defined(EIRIN_ARCH_X86) && \
    EIRIN_PLATFORM_SIMD_HAS_AVX2 == EIRIN_TRUE && defined(EIRIN_MATH_HAS_INT128)
namespace detail
{
    /**
     * @brief Signed fixed64 multiplication with a binary-point shift of 32.
     *
     * AVX2 has no 64x64->128 multiply instruction. The operands are split into
     * 32-bit limbs in unsigned magnitude form, then the required bits
     * [32, 96) of the 128-bit product are reconstructed with carries. This
     * matches fixed64's scalar `(int128)a * b >> 32` result, including the
     * final modulo-2^64 conversion.
     */
    EIRIN_ALWAYS_INLINE __m256i avx_fixed64_mul_shift32(__m256i lhs, __m256i rhs) noexcept
    {
        const __m256i zero = _mm256_setzero_si256();
        const __m256i low_mask = _mm256_set1_epi64x(0xFFFFFFFFULL);

        const __m256i lhs_sign = _mm256_sub_epi64(zero, _mm256_srli_epi64(lhs, 63));
        const __m256i rhs_sign = _mm256_sub_epi64(zero, _mm256_srli_epi64(rhs, 63));
        const __m256i lhs_abs = _mm256_sub_epi64(_mm256_xor_si256(lhs, lhs_sign), lhs_sign);
        const __m256i rhs_abs = _mm256_sub_epi64(_mm256_xor_si256(rhs, rhs_sign), rhs_sign);

        const __m256i lhs_lo = _mm256_and_si256(lhs_abs, low_mask);
        const __m256i lhs_hi = _mm256_srli_epi64(lhs_abs, 32);
        const __m256i rhs_lo = _mm256_and_si256(rhs_abs, low_mask);
        const __m256i rhs_hi = _mm256_srli_epi64(rhs_abs, 32);

        const __m256i p00 = _mm256_mul_epu32(lhs_lo, rhs_lo);
        const __m256i p01 = _mm256_mul_epu32(lhs_lo, rhs_hi);
        const __m256i p10 = _mm256_mul_epu32(lhs_hi, rhs_lo);
        const __m256i p11 = _mm256_mul_epu32(lhs_hi, rhs_hi);

        const __m256i p00_lo = _mm256_and_si256(p00, low_mask);
        const __m256i p00_hi = _mm256_srli_epi64(p00, 32);
        const __m256i p01_lo = _mm256_and_si256(p01, low_mask);
        const __m256i p01_hi = _mm256_srli_epi64(p01, 32);
        const __m256i p10_lo = _mm256_and_si256(p10, low_mask);
        const __m256i p10_hi = _mm256_srli_epi64(p10, 32);

        const __m256i cross_lo_sum = _mm256_add_epi64(p01_lo, p10_lo);
        const __m256i cross_lo = _mm256_and_si256(cross_lo_sum, low_mask);
        const __m256i cross_lo_carry = _mm256_srli_epi64(cross_lo_sum, 32);
        const __m256i cross_hi = _mm256_add_epi64(
            _mm256_add_epi64(p01_hi, p10_hi),
            cross_lo_carry
        );

        const __m256i word1_sum = _mm256_add_epi64(p00_hi, cross_lo);
        const __m256i word1 = _mm256_and_si256(word1_sum, low_mask);
        const __m256i word1_carry = _mm256_srli_epi64(word1_sum, 32);
        const __m256i word2 = _mm256_add_epi64(
            _mm256_add_epi64(cross_hi, _mm256_and_si256(p11, low_mask)),
            word1_carry
        );

        __m256i result = _mm256_or_si256(word1, _mm256_slli_epi64(word2, 32));
        const __m256i sign = _mm256_xor_si256(lhs_sign, rhs_sign);
        result = _mm256_sub_epi64(_mm256_xor_si256(result, sign), sign);

        // Arithmetic right shift rounds negative values toward negative
        // infinity, while the unsigned magnitude path truncates toward zero.
        const __m256i p00_lo_nonzero = _mm256_xor_si256(
            _mm256_cmpeq_epi64(p00_lo, zero),
            _mm256_set1_epi64x(-1)
        );
        const __m256i negative_adjust = _mm256_and_si256(
            _mm256_and_si256(sign, p00_lo_nonzero),
            _mm256_set1_epi64x(1)
        );
        result = _mm256_sub_epi64(result, negative_adjust);
        return result;
    }

    EIRIN_ALWAYS_INLINE __m256i avx_fixed64_floor(__m256i value) noexcept
    {
        const __m256i zero = _mm256_setzero_si256();
        const __m256i low_mask = _mm256_set1_epi64x(0xFFFFFFFFULL);
        const __m256i one = _mm256_set1_epi64x(1LL << 32);
        const __m256i sign = _mm256_sub_epi64(zero, _mm256_srli_epi64(value, 63));
        const __m256i magnitude = _mm256_sub_epi64(_mm256_xor_si256(value, sign), sign);
        const __m256i fraction = _mm256_and_si256(magnitude, low_mask);
        const __m256i fraction_nonzero = _mm256_xor_si256(
            _mm256_cmpeq_epi64(fraction, zero),
            _mm256_set1_epi64x(-1)
        );
        const __m256i truncated_magnitude = _mm256_slli_epi64(_mm256_srli_epi64(magnitude, 32), 32);
        const __m256i truncated = _mm256_sub_epi64(
            _mm256_xor_si256(truncated_magnitude, sign),
            sign
        );
        const __m256i adjust = _mm256_and_si256(_mm256_and_si256(sign, fraction_nonzero), one);
        return _mm256_sub_epi64(truncated, adjust);
    }

    EIRIN_ALWAYS_INLINE __m256i avx_reduce_angle_fixed64(__m256i value) noexcept
    {
        const __m256i two_pi = _mm256_set1_epi64x((numbers::pi_f64 * 2_f64).internal_value());
        const __m256i inv_two_pi = _mm256_set1_epi64x((1_f64 / (numbers::pi_f64 * 2_f64)).internal_value());
        const __m256i quotient = avx_fixed64_mul_shift32(value, inv_two_pi);
        const __m256i period_count = avx_fixed64_floor(quotient);
        return _mm256_sub_epi64(value, avx_fixed64_mul_shift32(period_count, two_pi));
    }
} // namespace detail

[[nodiscard]] EIRIN_ALWAYS_INLINE std::array<fixed64, 4> simd_fixed64_mul(
    const std::array<fixed64, 4>& lhs,
    const std::array<fixed64, 4>& rhs
) noexcept
{
    const __m256i lhs_v = _mm256_set_epi64x(
        lhs[3].internal_value(),
        lhs[2].internal_value(),
        lhs[1].internal_value(),
        lhs[0].internal_value()
    );
    const __m256i rhs_v = _mm256_set_epi64x(
        rhs[3].internal_value(),
        rhs[2].internal_value(),
        rhs[1].internal_value(),
        rhs[0].internal_value()
    );
    const __m256i result = detail::avx_fixed64_mul_shift32(lhs_v, rhs_v);

    alignas(32) std::int64_t values[4];
    _mm256_store_si256(reinterpret_cast<__m256i*>(values), result);
    return {
        fixed64::from_internal_value(values[0]),
        fixed64::from_internal_value(values[1]),
        fixed64::from_internal_value(values[2]),
        fixed64::from_internal_value(values[3]),
    };
}

[[nodiscard]] EIRIN_ALWAYS_INLINE std::array<fixed64, 4> simd_reduce_angle(
    const std::array<fixed64, 4>& values
) noexcept
{
    const __m256i values_v = _mm256_set_epi64x(
        values[3].internal_value(),
        values[2].internal_value(),
        values[1].internal_value(),
        values[0].internal_value()
    );
    const __m256i result = detail::avx_reduce_angle_fixed64(values_v);

    alignas(32) std::int64_t reduced[4];
    _mm256_store_si256(reinterpret_cast<__m256i*>(reduced), result);
    return {
        fixed64::from_internal_value(reduced[0]),
        fixed64::from_internal_value(reduced[1]),
        fixed64::from_internal_value(reduced[2]),
        fixed64::from_internal_value(reduced[3]),
    };
}
#endif
} // namespace eirin::simd

#endif
