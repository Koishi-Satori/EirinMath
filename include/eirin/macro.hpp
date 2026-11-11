#ifndef EIRIN_MATH_MARCO_HPP
#define EIRIN_MATH_MARCO_HPP

#pragma once

#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#    include <utility>
#endif

#define EIRIN_ENABLE           1
#define EIRIN_DISABLE          0
#define EIRIN_TRUE             1
#define EIRIN_FALSE            0

#define EIRIN_OVERFLOW_DEFAULT 0
#define EIRIN_OVERFLOW_MODWRAP 1
#define EIRIN_OVERFLOW_SAT     2

#ifndef NDEBUG
#    define EIRIN_DEBUG
#endif

#ifdef __GNUC__
#    ifndef __clang__
#        define EIRIN_ALWAYS_INLINE __attribute__((always_inline)) inline
#    else
#        define EIRIN_ALWAYS_INLINE [[gnu::always_inline]] inline
#    endif
#elif defined _MSC_VER
#    define EIRIN_ALWAYS_INLINE __forceinline
#else
#    define EIRIN_ALWAYS_INLINE inline
#endif

#if defined(__cplusplus) && __cplusplus >= 202302L && defined(__cpp_consteval) && __cpp_consteval >= 202211L
#    define EIRIN_IF_CONSTEVAL if consteval
#else
#    include <type_traits>
#    define EIRIN_IF_CONSTEVAL if(std::is_constant_evaluated())
#endif

#if defined(__EXCEPTIONS) && __EXCEPTIONS != EIRIN_ENABLE
#    define EIRIN_NO_EXCEPTIONS
#endif

#if defined(_MSC_VER) && !defined(_CPPUNWIND)
#    define EIRIN_NO_EXCEPTIONS
#endif

// operate system detection
#if defined(_WIN32) || defined(_WIN64)
#    define EIRIN_OS_WINDOWS
#elif defined(__linux__) || defined(__linux)
#    define EIRIN_OS_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
#    define EIRIN_OS_MACOS
#else
#    define EIRIN_OS_UNKNOWN
#endif

// arch detection
#if defined(__pnacl__)
#    define EIRIN_ARCH_PNACL
#elif defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#    define EIRIN_ARCH_X86
#elif defined(__aarch64__) || defined(_M_ARM64) || defined(__arm__) || defined(_M_ARM)
#    define EIRIN_ARCH_ARM
#elif defined(__wasm__) || defined(__EMSCRIPTEN__)
#    define EIRIN_ARCH_WASM
#else
#    define EIRIN_ARCH_UNKNOWN
#endif

// ISA extension detection. The following defines all the EIRIN_PLATFORM_SIMD_HAS_XXX.
// ARM
#if defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#    define EIRIN_PLATFORM_SIMD_HAS_NEON EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_NEON EIRIN_FALSE
#endif

#if EIRIN_PLATFORM_SIMD_HAS_NEON == EIRIN_TRUE && !defined(__aarch64__) && !defined(_M_ARM64) && !defined(_M_ARM64EC)
#    define EIRIN_PLATFORM_SIMD_HAS_NEON_A32 EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_NEON_A32 EIRIN_FALSE
#endif

#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#    define EIRIN_PLATFORM_SIMD_HAS_NEON_A64 EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_NEON_A64 EIRIN_FALSE
#endif

// WebAssembly
#if defined(__wasm_simd128__)
#    define EIRIN_PLATFORM_SIMD_HAS_WASM_SIMD128 EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_WASM_SIMD128 EIRIN_FALSE
#endif

#if defined(__wasm_relaxed_simd__)
#    define EIRIN_PLATFORM_SIMD_HAS_WASM_RELAXED_SIMD EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_WASM_RELAXED_SIMD EIRIN_FALSE
#endif

// x86
#if defined(__MMX__)
#    define EIRIN_PLATFORM_SIMD_HAS_MMX EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_MMX EIRIN_FALSE
#endif

#if defined(__SSE__) || defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__) || \
    defined(_M_X64) || defined(_M_AMD64) || defined(_M_AVX) || defined(_M_AVX2) || \
    (defined(_M_IX86_FP) && (_M_IX86_FP >= 1))
#    define EIRIN_PLATFORM_SIMD_HAS_SSE EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_SSE EIRIN_FALSE
#endif

#if defined(__SSE2__) || defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__) || \
    defined(_M_X64) || defined(_M_AMD64) || defined(_M_AVX) || defined(_M_AVX2) || \
    (defined(_M_IX86_FP) && (_M_IX86_FP >= 2))
#    define EIRIN_PLATFORM_SIMD_HAS_SSE2 EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_SSE2 EIRIN_FALSE
#endif

#if defined(__SSE3__) || defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__) || \
    defined(_M_AVX) || defined(_M_AVX2)
#    define EIRIN_PLATFORM_SIMD_HAS_SSE3 EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_SSE3 EIRIN_FALSE
#endif

#if defined(__SSSE3__) || defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__) || \
    defined(_M_AVX) || defined(_M_AVX2)
#    define EIRIN_PLATFORM_SIMD_HAS_SSSE3 EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_SSSE3 EIRIN_FALSE
#endif

#if defined(__SSE4_1__) || defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__) || \
    defined(_M_AVX) || defined(_M_AVX2)
#    define EIRIN_PLATFORM_SIMD_HAS_SSE4_1 EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_SSE4_1 EIRIN_FALSE
#endif

#if defined(__SSE4_2__) || defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__) || \
    defined(_M_AVX) || defined(_M_AVX2)
#    define EIRIN_PLATFORM_SIMD_HAS_SSE4_2 EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_SSE4_2 EIRIN_FALSE
#endif

#if defined(__SSE4A__)
#    define EIRIN_PLATFORM_SIMD_HAS_SSE4A EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_SSE4A EIRIN_FALSE
#endif

#if defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__) || defined(_M_AVX) || defined(_M_AVX2)
#    define EIRIN_PLATFORM_SIMD_HAS_AVX EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_AVX EIRIN_FALSE
#endif

#if defined(__AVX2__) || defined(__AVX512F__) || defined(_M_AVX2)
#    define EIRIN_PLATFORM_SIMD_HAS_AVX2 EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_AVX2 EIRIN_FALSE
#endif

#if defined(__AVX512F__)
#    define EIRIN_PLATFORM_SIMD_HAS_AVX512F EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_AVX512F EIRIN_FALSE
#endif

#if defined(__AVX512DQ__)
#    define EIRIN_PLATFORM_SIMD_HAS_AVX512DQ EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_AVX512DQ EIRIN_FALSE
#endif

#if defined(__AVX512VL__)
#    define EIRIN_PLATFORM_SIMD_HAS_AVX512VL EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_AVX512VL EIRIN_FALSE
#endif

#if defined(__AVX512BW__)
#    define EIRIN_PLATFORM_SIMD_HAS_AVX512BW EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_AVX512BW EIRIN_FALSE
#endif

#if defined(__AVX512CD__)
#    define EIRIN_PLATFORM_SIMD_HAS_AVX512CD EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_AVX512CD EIRIN_FALSE
#endif

#if defined(__BMI__)
#    define EIRIN_PLATFORM_SIMD_HAS_BMI1 EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_BMI1 EIRIN_FALSE
#endif

#if defined(__BMI2__)
#    define EIRIN_PLATFORM_SIMD_HAS_BMI2 EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_BMI2 EIRIN_FALSE
#endif

#if defined(__LZCNT__)
#    define EIRIN_PLATFORM_SIMD_HAS_LZCNT EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_LZCNT EIRIN_FALSE
#endif

#if defined(__XOP__)
#    define EIRIN_PLATFORM_SIMD_HAS_XOP EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_XOP EIRIN_FALSE
#endif

#if defined(__FMA__)
#    define EIRIN_PLATFORM_SIMD_HAS_FMA EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_FMA EIRIN_FALSE
#endif

#if defined(__FMA4__)
#    define EIRIN_PLATFORM_SIMD_HAS_FMA4 EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_FMA4 EIRIN_FALSE
#endif

#if defined(__F16C__)
#    define EIRIN_PLATFORM_SIMD_HAS_F16C EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_F16C EIRIN_FALSE
#endif

#if defined(__POPCNT__)
#    define EIRIN_PLATFORM_SIMD_HAS_POPCNT EIRIN_TRUE
#else
#    define EIRIN_PLATFORM_SIMD_HAS_POPCNT EIRIN_FALSE
#endif

#if EIRIN_PLATFORM_SIMD_HAS_SSE
#define EIRIN_PLATFORM_SIMD_HAS_SSE_ABI EIRIN_TRUE
#else
#define EIRIN_PLATFORM_SIMD_HAS_SSE_ABI EIRIN_FALSE
#endif
#if EIRIN_PLATFORM_SIMD_HAS_SSE2
#define EIRIN_PLATFORM_SIMD_HAS_FULL_SSE_ABI EIRIN_TRUE
#else
#define EIRIN_PLATFORM_SIMD_HAS_FULL_SSE_ABI EIRIN_FALSE
#endif

#if EIRIN_PLATFORM_SIMD_HAS_AVX
#define EIRIN_PLATFORM_SIMD_HAS_AVX_ABI EIRIN_TRUE
#else
#define EIRIN_PLATFORM_SIMD_HAS_AVX_ABI EIRIN_FALSE
#endif
#if EIRIN_PLATFORM_SIMD_HAS_AVX2
#define EIRIN_PLATFORM_SIMD_HAS_FULL_AVX_ABI EIRIN_TRUE
#else
#define EIRIN_PLATFORM_SIMD_HAS_FULL_AVX_ABI EIRIN_FALSE
#endif

#if EIRIN_PLATFORM_SIMD_HAS_AVX512F
#define EIRIN_PLATFORM_SIMD_HAS_AVX512_ABI EIRIN_TRUE
#else
#define EIRIN_PLATFORM_SIMD_HAS_AVX512_ABI EIRIN_FALSE
#endif
#if EIRIN_PLATFORM_SIMD_HAS_AVX512BW
#define EIRIN_PLATFORM_SIMD_HAS_FULL_AVX512_ABI EIRIN_TRUE
#else
#define EIRIN_PLATFORM_SIMD_HAS_FULL_AVX512_ABI EIRIN_FALSE
#endif

// Global SIMD feature gatekeeper. It is enabled by default when the target exposes at
// least one SIMD ISA and can be overridden before including EirinMath header files.
#ifndef EIRIN_SIMD_ENABLE
#    if defined(EIRIN_MATH_NO_SIMD)
#        define EIRIN_SIMD_ENABLE EIRIN_DISABLE
#    elif EIRIN_PLATFORM_SIMD_HAS_SSE == EIRIN_TRUE || \
        EIRIN_PLATFORM_SIMD_HAS_NEON == EIRIN_TRUE || \
        EIRIN_PLATFORM_SIMD_HAS_WASM_SIMD128 == EIRIN_TRUE
#        define EIRIN_SIMD_ENABLE EIRIN_ENABLE
#    else
#        define EIRIN_SIMD_ENABLE EIRIN_DISABLE
#    endif
#endif

#if EIRIN_SIMD_ENABLE != EIRIN_ENABLE && EIRIN_SIMD_ENABLE != EIRIN_DISABLE
#    error "EIRIN_SIMD_ENABLE must be EIRIN_ENABLE or EIRIN_DISABLE."
#endif

#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#    define EIRIN_UNREACHABLE std::unreachable()
#elif defined(_MSC_VER) && !defined(__clang__)
#    define EIRIN_UNREACHABLE __assume(false)
#else
#    define EIRIN_UNREACHABLE __builtin_unreachable()
#endif

#ifdef EIRIN_DEBUG
#    include <cassert>
#    define EIRIN_INDEX_LENGTH_ASSERT(i, max) (assert((i) < (max)))
#else
#    define EIRIN_INDEX_LENGTH_ASSERT(i, max)
#endif

#if defined(__cpp_multidimensional_subscript) && __cpp_multidimensional_subscript >= 202110L
#define EIRIN_HAS_CXX_FEATURE_MULTIDIM_SUBSCRIPT
#endif

// must include this, as config header.
#include "ext/config.hpp"

#ifdef EIRIN_MATH_HAS_INCLUDE_CONFIG
// do nothing here, just for silence code analysis warning.
#endif

#endif // EIRIN_MATH_MARCO_HPP
