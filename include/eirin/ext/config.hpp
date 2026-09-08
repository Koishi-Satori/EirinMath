/**
 * @file config.hpp
 * @author KKoishi_
 * @brief Eirin Math Setup Config
 * @date 2026-09-08
 * @note DO NOT directly include this file in your code.
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef EIRIN_MATH_EXT_CONFIG_HPP
#define EIRIN_MATH_EXT_CONFIG_HPP

#pragma once

#include "../macro.hpp"

#ifndef EIRIN_MATH_MARCO_HPP
#warning "This file is not included after macro.hpp or directly included in user code." 
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

// Feature: Vector Swizzle.
// This feature provides a swizzle proxy type for tvec, which allows for swizzle operations like GLSL.
// e.g. `v.xyzw()` returns a swizzle proxy that can be used to access or modify the components of the vector `v` in a different order.
// Noticed that the swizzle proxy is a temporary object, so it should not be used after the original vector is destroyed or modified.
#ifndef EIRIN_VEC_SWIZZLE_ENABLE
#    define EIRIN_VEC_SWIZZLE_ENABLE EIRIN_ENABLE
#endif

#ifndef EIRIN_VEC_SWIZZLE_FORCE_INLINE
#    define EIRIN_VEC_SWIZZLE_FORCE_INLINE EIRIN_DISABLE
#endif

#if EIRIN_VEC_SWIZZLE_FORCE_INLINE == EIRIN_DISABLE
#    define EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL inline constexpr
#else
#    define EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL EIRIN_ALWAYS_INLINE constexpr
#endif

// Vector chained swizzle, e.g. `v.xyzw().xy()`: enabled by default.
// Define EIRIN_VEC_DISABLE_SWIZZLE_CHAIN before including any EIRIN header to
// disable chaining, or set EIRIN_VEC_SWIZZLE_CHAIN directly.
#ifdef EIRIN_VEC_DISABLE_SWIZZLE_CHAIN
#    define EIRIN_VEC_SWIZZLE_CHAIN EIRIN_DISABLE
#endif

#ifndef EIRIN_VEC_SWIZZLE_CHAIN
#    define EIRIN_VEC_SWIZZLE_CHAIN EIRIN_ENABLE
#endif

// Chained swizzle as a writable lvalue, e.g. `v.xy().yx() = ...`: disabled by
// default, so a chained result is read-only while the first hop from a vector
// (`v.xy() = ...`) stays writable.  Define
// EIRIN_VEC_ENABLE_SWIZZLE_CHAIN_AS_LVALUE before including any EIRIN header
// to allow chained writes, or set EIRIN_VEC_SWIZZLE_CHAIN_AS_LVALUE directly.
#ifdef EIRIN_VEC_ENABLE_SWIZZLE_CHAIN_AS_LVALUE
#    if EIRIN_VEC_SWIZZLE_CHAIN == EIRIN_ENABLE
#        define EIRIN_VEC_SWIZZLE_CHAIN_AS_LVALUE EIRIN_ENABLE
#    endif
#endif

#ifndef EIRIN_VEC_SWIZZLE_CHAIN_AS_LVALUE
#    define EIRIN_VEC_SWIZZLE_CHAIN_AS_LVALUE EIRIN_DISABLE
#endif

#define EIRIN_MATH_HAS_INCLUDE_CONFIG

#endif
