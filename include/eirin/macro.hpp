#ifndef EIRIN_MATH_MARCO_HPP
#define EIRIN_MATH_MARCO_HPP

#pragma once

#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#    include <utility>
#endif

#define EIRIN_ENABLE  1
#define EIRIN_DISABLE 0

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
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#    define EIRIN_ARCH_X86
#elif defined(__aarch64__) || defined(_M_ARM64) || defined(__arm__) || defined(_M_ARM)
#    define EIRIN_ARCH_ARM
#elif defined(__wasm__) || defined(__EMSCRIPTEN__)
#    define EIRIN_ARCH_WASM
#else
#    define EIRIN_ARCH_UNKNOWN
#endif

// check if SIMD is enabled
#ifdef __AVX2__
#    define EIRIN_PLATFORM_HAS_SIMD
#    define EIRIN_PLATFORM_SIMD_AVX2
#elif defined(__AVX__)
#    define EIRIN_PLATFORM_HAS_SIMD
#    define EIRIN_PLATFORM_SIMD_AVX
#elif defined(__SSE4_2__)
#    define EIRIN_PLATFORM_HAS_SIMD
#    define EIRIN_PLATFORM_SIMD_SSE4_2
#elif defined(__SSE2__)
#    define EIRIN_PLATFORM_HAS_SIMD
#    define EIRIN_PLATFORM_SIMD_SSE2
#else
#    define EIRIN_MATH_NO_SIMD
#endif

#ifndef EIRIN_VEC_SWIZZLE_ENABLE
#    define EIRIN_VEC_SWIZZLE_ENABLE EIRIN_ENABLE
#endif

#ifndef EIRIN_VEC_SWIZZLE_FORCE_INLINE
#    define EIRIN_VEC_SWIZZLE_FORCE_INLINE EIRIN_DISABLE
#endif

#if EIRIN_VEC_SWIZZLE_FORCE_INLINE == EIRIN_DISABLE
#    define EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL constexpr inline
#else
#    define EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL constexpr EIRIN_ALWAYS_INLINE
#endif

#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#    define EIRIN_UNREACHABLE std::unreachable()
#elif defined(_MSC_VER) && !defined(__clang__)
#    define EIRIN_UNREACHABLE __assume(false)
#else
#    define EIRIN_UNREACHABLE __builtin_unreachable()
#endif

#endif // EIRIN_MATH_MARCO_HPP
