#ifndef EIRIN_MATH_DETAIL_SIMD_DETAIL_HPP
#define EIRIN_MATH_DETAIL_SIMD_DETAIL_HPP

#pragma once

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
} // namespace eirin::simd_abi

#endif
