#ifndef EIRIN_MATH_MARCO_HPP
#define EIRIN_MATH_MARCO_HPP

#pragma once

#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#    include <utility>
#endif

#define EIRIN_ENABLE           1
#define EIRIN_DISABLE          0

#define EIRIN_OVERFLOW_DEFAULT 0
#define EIRIN_OVERFLOW_MODWRAP 1
#define EIRIN_OVERFLOW_SAT     2

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

#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#    define EIRIN_UNREACHABLE std::unreachable()
#elif defined(_MSC_VER) && !defined(__clang__)
#    define EIRIN_UNREACHABLE __assume(false)
#else
#    define EIRIN_UNREACHABLE __builtin_unreachable()
#endif

// must include this, as config header.
#include "ext/config.hpp"

#ifdef EIRIN_MATH_HAS_INCLUDE_CONFIG
// do nothing here, just for silence code analysis warning.
#endif

#endif // EIRIN_MATH_MARCO_HPP
