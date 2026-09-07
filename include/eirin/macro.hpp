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

// if the ++/-- operator should mod-warp or sat the value when meet UQ(N) or Q(N-1).
// for best performance, default option is no-op.

#ifdef EIRIN_MODWRAP_FIXED_INC_DEC
#    define EIRIN_FIXED_NUM_SELF_INC_OVERFLOW EIRIN_OVERFLOW_MODWRAP
#    define EIRIN_FIXED_NUM_SELF_DEC_OVERFLOW EIRIN_OVERFLOW_MODWRAP
#endif
#ifdef EIRIN_SAT_FIXED_INC_DEC
#    define EIRIN_FIXED_NUM_SELF_INC_OVERFLOW EIRIN_OVERFLOW_SAT
#    define EIRIN_FIXED_NUM_SELF_DEC_OVERFLOW EIRIN_OVERFLOW_SAT
#endif

#ifndef EIRIN_FIXED_NUM_SELF_INC_OVERFLOW
#    define EIRIN_FIXED_NUM_SELF_INC_OVERFLOW EIRIN_OVERFLOW_DEFAULT
#endif
#ifndef EIRIN_FIXED_NUM_SELF_DEC_OVERFLOW
#    define EIRIN_FIXED_NUM_SELF_DEC_OVERFLOW EIRIN_OVERFLOW_DEFAULT
#endif

// use which type of int128, builtin or extention
// in MSVC, the default is builtin int128 due to performance issue of _Signed128

#ifndef EIRIN_USE_EXT_BUILTIN_INT128
#    ifdef _MSC_VER
#        define EIRIN_FORCE_EXT_BUILTIN_INT128
#    endif
#endif

#ifdef EIRIN_FORCE_EXT_BUILTIN_INT128
#    define EIRIN_USE_EXT_BUILTIN_INT128 EIRIN_ENABLE
#endif

#ifndef EIRIN_USE_EXT_BUILTIN_INT128
#    define EIRIN_USE_EXT_BUILTIN_INT128 EIRIN_DISABLE
#endif

// exposed math functions
#ifdef EIRIN_MATH_FUNC_FORCE_INLINE
#    define EIRIN_MATH_FUNC_API EIRIN_ALWAYS_INLINE constexpr
#else
#    define EIRIN_MATH_FUNC_API inline constexpr
#endif
// small func can force inline.
#define EIRIN_MATH_SMALL_FUNC_API EIRIN_ALWAYS_INLINE constexpr

#endif // EIRIN_MATH_MARCO_HPP
