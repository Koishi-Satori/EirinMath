#ifndef EIRIN_MATH_DETAIL_SIMD_DETAIL_HPP
#define EIRIN_MATH_DETAIL_SIMD_DETAIL_HPP

#pragma once

#include <cstddef>
#include <type_traits>
#include "../../macro.hpp"

namespace eirin::simd_abi
{
constexpr inline bool platform_has_mmx = EIRIN_PLATFORM_SIMD_HAS_MMX;
constexpr inline bool platform_has_sse = EIRIN_PLATFORM_SIMD_HAS_SSE;
constexpr inline bool platform_has_sse2 = EIRIN_PLATFORM_SIMD_HAS_SSE2;
constexpr inline bool platform_has_sse3 = EIRIN_PLATFORM_SIMD_HAS_SSE3;
constexpr inline bool platform_has_ssse3 = EIRIN_PLATFORM_SIMD_HAS_SSSE3;
constexpr inline bool platform_has_sse4_1 = EIRIN_PLATFORM_SIMD_HAS_SSE4_1;
constexpr inline bool platform_has_sse4_2 = EIRIN_PLATFORM_SIMD_HAS_SSE4_2;
constexpr inline bool platform_has_xop = EIRIN_PLATFORM_SIMD_HAS_XOP;
constexpr inline bool platform_has_avx = EIRIN_PLATFORM_SIMD_HAS_AVX;
constexpr inline bool platform_has_avx2 = EIRIN_PLATFORM_SIMD_HAS_AVX2;
constexpr inline bool platform_has_bmi = EIRIN_PLATFORM_SIMD_HAS_BMI1;
constexpr inline bool platform_has_bmi2 = EIRIN_PLATFORM_SIMD_HAS_BMI2;
constexpr inline bool platform_has_lzcnt = EIRIN_PLATFORM_SIMD_HAS_LZCNT;
constexpr inline bool platform_has_sse4a = EIRIN_PLATFORM_SIMD_HAS_SSE4A;
constexpr inline bool platform_has_fma = EIRIN_PLATFORM_SIMD_HAS_FMA;
constexpr inline bool platform_has_fma4 = EIRIN_PLATFORM_SIMD_HAS_FMA4;
constexpr inline bool platform_has_f16c = EIRIN_PLATFORM_SIMD_HAS_F16C;
constexpr inline bool platform_has_popcnt = EIRIN_PLATFORM_SIMD_HAS_POPCNT;
constexpr inline bool platform_has_avx512f = EIRIN_PLATFORM_SIMD_HAS_AVX512F;
constexpr inline bool platform_has_avx512dq = EIRIN_PLATFORM_SIMD_HAS_AVX512DQ;
constexpr inline bool platform_has_avx512vl = EIRIN_PLATFORM_SIMD_HAS_AVX512VL;
constexpr inline bool platform_has_avx512bw = EIRIN_PLATFORM_SIMD_HAS_AVX512BW;
constexpr inline bool platform_has_avx512dq_vl = platform_has_avx512dq && platform_has_avx512vl;
constexpr inline bool platform_has_avx512bw_vl = platform_has_avx512bw && platform_has_avx512vl;

constexpr inline bool platform_has_neon = EIRIN_PLATFORM_SIMD_HAS_NEON;
constexpr inline bool platform_has_neon_a32 = EIRIN_PLATFORM_SIMD_HAS_NEON_A32;
constexpr inline bool platform_has_neon_a64 = EIRIN_PLATFORM_SIMD_HAS_NEON_A64;

constexpr inline bool platform_has_sse_abi = EIRIN_PLATFORM_SIMD_HAS_SSE_ABI;
constexpr inline bool platform_has_full_sse_abi = EIRIN_PLATFORM_SIMD_HAS_FULL_SSE_ABI;
constexpr inline bool platform_has_avx_abi = EIRIN_PLATFORM_SIMD_HAS_AVX_ABI;
constexpr inline bool platform_has_full_avx_abi = EIRIN_PLATFORM_SIMD_HAS_FULL_AVX_ABI;
constexpr inline bool platform_has_avx512_abi = EIRIN_PLATFORM_SIMD_HAS_AVX512_ABI;
constexpr inline bool platform_has_full_avx512_abi = EIRIN_PLATFORM_SIMD_HAS_FULL_AVX512_ABI;

constexpr inline bool platform_support_neon_float =
#if defined __GCC_IEC_559
    __GCC_IEC_559 == 0;
#elif defined __FAST_MATH__
    true;
#else
    false;
#endif

template <typename T>
constexpr std::size_t vectorized_sizeof()
{
    // if constexpr // TODO: maybe we need a type_traits to check if T is a valid type for vectorization...

    if constexpr(sizeof(T) <= 8)
    {
        // X86:
        if constexpr(platform_has_avx512bw)
            return 64;
        if constexpr(platform_has_avx512f && sizeof(T) >= 4)
            return 64;
        if constexpr(platform_has_avx2)
            return 32;
        if constexpr(platform_has_avx && std::is_floating_point_v<T>)
            return 32;
        if constexpr(platform_has_sse2)
            return 16;
        if constexpr(platform_has_sse && std::is_same_v<T, float>)
            return 16;
        /* The following is too much trouble because of mixed MMX and x87 code.
	 * While nothing here explicitly calls MMX instructions of registers,
	 * they are still emitted but no EMMS cleanup is done.
	 * if constexpr (platform_has_mmx && sizeof(T) <= 4 && std::is_integral_v<T>)
	 *  return 8;
	 */

        // PowerPC:
        // if constexpr (platform_has_power8vec
        // 	      || (platform_has_power_vmx && (sizeof(T) < 8))
        // 	      || (platform_has_power_vsx && std::is_floating_point_v<T>) )
        //   return 16;

        // ARM:
        if constexpr(platform_has_neon_a64 || (platform_has_neon_a32 && !std::is_same_v<T, double>))
            return 16;
        if constexpr(platform_has_neon && sizeof(T) < 8
                     // Only allow fp if the user allows non-ICE559 fp (e.g.
                     // via -ffast-math). ARMv7 NEON fp is not conforming to
                     // IEC559.
                     && (platform_support_neon_float || !std::is_floating_point_v<T>))
            return 16;

        // WebAssembly SIMD:
        if constexpr(EIRIN_PLATFORM_SIMD_HAS_WASM_SIMD128 == EIRIN_TRUE)
            return 16;
    }
    return sizeof(T);
}

} // namespace eirin::simd_abi

#endif
