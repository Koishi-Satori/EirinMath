#ifndef EIRIN_MATH_MAT_HPP
#define EIRIN_MATH_MAT_HPP

#pragma once

#include <cstddef>
#include <cstdint>
#include "ext/stdfloat.hpp"
#include "macro.hpp"
#include "detail/type_tmat.hpp"
#include "detail/type_tmat2x2.hpp"
#include "detail/type_tmat3x3.hpp"
#include "detail/type_tmat4x4.hpp"
#include "detail/type_tmat2x3.hpp"
#include "detail/type_tmat3x2.hpp"
#include "detail/int128.hpp"

namespace eirin
{
template <typename T>
using mat2 = tmat<2, 2, T>;
template <typename T>
using mat2x2 = tmat<2, 2, T>;

/* Native (platform) width types */

using mat2i = tmat<2, 2, int>;
using mat2u = tmat<2, 2, unsigned int>;
using mat2x2i = tmat<2, 2, int>;
using mat2x2u = tmat<2, 2, unsigned int>;

/* Sized signed/unsigned integer matrix */

using mat2i8 = tmat<2, 2, std::int8_t>;
using mat2u8 = tmat<2, 2, std::uint8_t>;
using mat2i16 = tmat<2, 2, std::int16_t>;
using mat2u16 = tmat<2, 2, std::uint16_t>;
using mat2i32 = tmat<2, 2, std::int32_t>;
using mat2u32 = tmat<2, 2, std::uint32_t>;
using mat2i64 = tmat<2, 2, std::int64_t>;
using mat2u64 = tmat<2, 2, std::uint64_t>;
using mat2x2i8 = tmat<2, 2, std::int8_t>;
using mat2x2u8 = tmat<2, 2, std::uint8_t>;
using mat2x2i16 = tmat<2, 2, std::int16_t>;
using mat2x2u16 = tmat<2, 2, std::uint16_t>;
using mat2x2i32 = tmat<2, 2, std::int32_t>;
using mat2x2u32 = tmat<2, 2, std::uint32_t>;
using mat2x2i64 = tmat<2, 2, std::int64_t>;
using mat2x2u64 = tmat<2, 2, std::uint64_t>;

/* Sized signed/unsigned integer matrix */

#ifdef EIRIN_MATH_HAS_INT128
using mat2i128 = tmat<2, 2, detail::int128_t>;
using mat2u128 = tmat<2, 2, detail::uint128_t>;
using mat2x2i128 = tmat<2, 2, detail::int128_t>;
using mat2x2u128 = tmat<2, 2, detail::uint128_t>;
#endif

/* Floating point matrix */

using mat2f = tmat<2, 2, float>;
using mat2x2f = tmat<2, 2, float>;
using mat2d = tmat<2, 2, double>;
using mat2x2d = tmat<2, 2, double>;

#ifdef __STDCPP_FLOAT16_T__
using mat2f16 = tmat<2, 2, std::float16_t>;
using mat2x2f16 = tmat<2, 2, std::float16_t>;
#endif

#ifdef __STDCPP_FLOAT32_T__
using mat2f32 = tmat<2, 2, std::float32_t>;
using mat2x2f32 = tmat<2, 2, std::float32_t>;
#endif

#ifdef __STDCPP_FLOAT64_T__
using mat2f64 = tmat<2, 2, std::float64_t>;
using mat2x2f64 = tmat<2, 2, std::float64_t>;
#endif

#ifdef __STDCPP_FLOAT128_T__
using mat2f128 = tmat<2, 2, std::float128_t>;
using mat2x2f128 = tmat<2, 2, std::float128_t>;
#endif

#ifdef __STDCPP_BFLOAT16_T__
using mat2bf16 = tmat<2, 2, std::bfloat16_t>;
using mat2x2bf16 = tmat<2, 2, std::bfloat16_t>;
#endif

/* Fixed point matrix */

// Generic fixed-point templates.
template <typename T, typename I, unsigned int f, bool r = false>
using mat2fixed = tmat<2, 2, fixed_num<T, I, f, r>>;
template <typename T, typename I, unsigned int f, bool r = false>
using mat2x2fixed = tmat<2, 2, fixed_num<T, I, f, r>>;

// Concrete fixed-point matrix.
using mat2fixed32 = tmat<2, 2, fixed32>;
using mat2x2fixed32 = tmat<2, 2, fixed32>;
#ifdef EIRIN_MATH_HAS_INT128
using mat2fixed64 = tmat<2, 2, fixed64>;
using mat2x2fixed64 = tmat<2, 2, fixed64>;
#endif

template <typename T>
using mat3 = tmat<3, 3, T>;
template <typename T>
using mat3x3 = tmat<3, 3, T>;

/* Native (platform) width types */

using mat3i = tmat<3, 3, int>;
using mat3u = tmat<3, 3, unsigned int>;
using mat3x3i = tmat<3, 3, int>;
using mat3x3u = tmat<3, 3, unsigned int>;

/* Sized signed/unsigned integer matrix */

using mat3i8 = tmat<3, 3, std::int8_t>;
using mat3u8 = tmat<3, 3, std::uint8_t>;
using mat3i16 = tmat<3, 3, std::int16_t>;
using mat3u16 = tmat<3, 3, std::uint16_t>;
using mat3i32 = tmat<3, 3, std::int32_t>;
using mat3u32 = tmat<3, 3, std::uint32_t>;
using mat3i64 = tmat<3, 3, std::int64_t>;
using mat3u64 = tmat<3, 3, std::uint64_t>;
using mat3x3i8 = tmat<3, 3, std::int8_t>;
using mat3x3u8 = tmat<3, 3, std::uint8_t>;
using mat3x3i16 = tmat<3, 3, std::int16_t>;
using mat3x3u16 = tmat<3, 3, std::uint16_t>;
using mat3x3i32 = tmat<3, 3, std::int32_t>;
using mat3x3u32 = tmat<3, 3, std::uint32_t>;
using mat3x3i64 = tmat<3, 3, std::int64_t>;
using mat3x3u64 = tmat<3, 3, std::uint64_t>;

#ifdef EIRIN_MATH_HAS_INT128
using mat3i128 = tmat<3, 3, detail::int128_t>;
using mat3u128 = tmat<3, 3, detail::uint128_t>;
using mat3x3i128 = tmat<3, 3, detail::int128_t>;
using mat3x3u128 = tmat<3, 3, detail::uint128_t>;
#endif

/* Floating point matrix */

using mat3f = tmat<3, 3, float>;
using mat3x3f = tmat<3, 3, float>;
using mat3d = tmat<3, 3, double>;
using mat3x3d = tmat<3, 3, double>;

#ifdef __STDCPP_FLOAT16_T__
using mat3f16 = tmat<3, 3, std::float16_t>;
using mat3x3f16 = tmat<3, 3, std::float16_t>;
#endif

#ifdef __STDCPP_FLOAT32_T__
using mat3f32 = tmat<3, 3, std::float32_t>;
using mat3x3f32 = tmat<3, 3, std::float32_t>;
#endif

#ifdef __STDCPP_FLOAT64_T__
using mat3f64 = tmat<3, 3, std::float64_t>;
using mat3x3f64 = tmat<3, 3, std::float64_t>;
#endif

#ifdef __STDCPP_FLOAT128_T__
using mat3f128 = tmat<3, 3, std::float128_t>;
using mat3x3f128 = tmat<3, 3, std::float128_t>;
#endif

#ifdef __STDCPP_BFLOAT16_T__
using mat3bf16 = tmat<3, 3, std::bfloat16_t>;
using mat3x3bf16 = tmat<3, 3, std::bfloat16_t>;
#endif

/* Fixed point matrix */

// Generic fixed-point templates.
template <typename T, typename I, unsigned int f, bool r = false>
using mat3fixed = tmat<3, 3, fixed_num<T, I, f, r>>;
template <typename T, typename I, unsigned int f, bool r = false>
using mat3x3fixed = tmat<3, 3, fixed_num<T, I, f, r>>;

// Concrete fixed-point matrix.
using mat3fixed32 = tmat<3, 3, fixed32>;
using mat3x3fixed32 = tmat<3, 3, fixed32>;
#ifdef EIRIN_MATH_HAS_INT128
using mat3fixed64 = tmat<3, 3, fixed64>;
using mat3x3fixed64 = tmat<3, 3, fixed64>;
#endif

template <typename T>
using mat4 = tmat<4, 4, T>;
template <typename T>
using mat4x4 = tmat<4, 4, T>;

/* Native (platform) width types */

using mat4i = tmat<4, 4, int>;
using mat4u = tmat<4, 4, unsigned int>;
using mat4x4i = tmat<4, 4, int>;
using mat4x4u = tmat<4, 4, unsigned int>;

/* Sized signed/unsigned integer matrix */

using mat4i8 = tmat<4, 4, std::int8_t>;
using mat4u8 = tmat<4, 4, std::uint8_t>;
using mat4i16 = tmat<4, 4, std::int16_t>;
using mat4u16 = tmat<4, 4, std::uint16_t>;
using mat4i32 = tmat<4, 4, std::int32_t>;
using mat4u32 = tmat<4, 4, std::uint32_t>;
using mat4i64 = tmat<4, 4, std::int64_t>;
using mat4u64 = tmat<4, 4, std::uint64_t>;
using mat4x4i8 = tmat<4, 4, std::int8_t>;
using mat4x4u8 = tmat<4, 4, std::uint8_t>;
using mat4x4i16 = tmat<4, 4, std::int16_t>;
using mat4x4u16 = tmat<4, 4, std::uint16_t>;
using mat4x4i32 = tmat<4, 4, std::int32_t>;
using mat4x4u32 = tmat<4, 4, std::uint32_t>;
using mat4x4i64 = tmat<4, 4, std::int64_t>;
using mat4x4u64 = tmat<4, 4, std::uint64_t>;

#ifdef EIRIN_MATH_HAS_INT128
using mat4i128 = tmat<4, 4, detail::int128_t>;
using mat4u128 = tmat<4, 4, detail::uint128_t>;
using mat4x4i128 = tmat<4, 4, detail::int128_t>;
using mat4x4u128 = tmat<4, 4, detail::uint128_t>;
#endif

/* Floating point matrix */

using mat4f = tmat<4, 4, float>;
using mat4x4f = tmat<4, 4, float>;
using mat4d = tmat<4, 4, double>;
using mat4x4d = tmat<4, 4, double>;

#ifdef __STDCPP_FLOAT16_T__
using mat4f16 = tmat<4, 4, std::float16_t>;
using mat4x4f16 = tmat<4, 4, std::float16_t>;
#endif

#ifdef __STDCPP_FLOAT32_T__
using mat4f32 = tmat<4, 4, std::float32_t>;
using mat4x4f32 = tmat<4, 4, std::float32_t>;
#endif

#ifdef __STDCPP_FLOAT64_T__
using mat4f64 = tmat<4, 4, std::float64_t>;
using mat4x4f64 = tmat<4, 4, std::float64_t>;
#endif

#ifdef __STDCPP_FLOAT128_T__
using mat4f128 = tmat<4, 4, std::float128_t>;
using mat4x4f128 = tmat<4, 4, std::float128_t>;
#endif

#ifdef __STDCPP_BFLOAT16_T__
using mat4bf16 = tmat<4, 4, std::bfloat16_t>;
using mat4x4bf16 = tmat<4, 4, std::bfloat16_t>;
#endif

/* Fixed point matrix */

// Generic fixed-point templates.
template <typename T, typename I, unsigned int f, bool r = false>
using mat4fixed = tmat<4, 4, fixed_num<T, I, f, r>>;
template <typename T, typename I, unsigned int f, bool r = false>
using mat4x4fixed = tmat<4, 4, fixed_num<T, I, f, r>>;

// Concrete fixed-point matrix.
using mat4fixed32 = tmat<4, 4, fixed32>;
using mat4x4fixed32 = tmat<4, 4, fixed32>;
#ifdef EIRIN_MATH_HAS_INT128
using mat4fixed64 = tmat<4, 4, fixed64>;
using mat4x4fixed64 = tmat<4, 4, fixed64>;
#endif

/* Non square shapes: matCxR is C columns and R rows. */

template <typename T>
using mat2x3 = tmat<2, 3, T>;
template <typename T>
using mat3x2 = tmat<3, 2, T>;

/* Native (platform) width types */

using mat2x3i = tmat<2, 3, int>;
using mat2x3u = tmat<2, 3, unsigned int>;
using mat3x2i = tmat<3, 2, int>;
using mat3x2u = tmat<3, 2, unsigned int>;

/* Sized signed/unsigned integer matrix */

using mat2x3i8 = tmat<2, 3, std::int8_t>;
using mat2x3u8 = tmat<2, 3, std::uint8_t>;
using mat2x3i16 = tmat<2, 3, std::int16_t>;
using mat2x3u16 = tmat<2, 3, std::uint16_t>;
using mat2x3i32 = tmat<2, 3, std::int32_t>;
using mat2x3u32 = tmat<2, 3, std::uint32_t>;
using mat2x3i64 = tmat<2, 3, std::int64_t>;
using mat2x3u64 = tmat<2, 3, std::uint64_t>;

using mat3x2i8 = tmat<3, 2, std::int8_t>;
using mat3x2u8 = tmat<3, 2, std::uint8_t>;
using mat3x2i16 = tmat<3, 2, std::int16_t>;
using mat3x2u16 = tmat<3, 2, std::uint16_t>;
using mat3x2i32 = tmat<3, 2, std::int32_t>;
using mat3x2u32 = tmat<3, 2, std::uint32_t>;
using mat3x2i64 = tmat<3, 2, std::int64_t>;
using mat3x2u64 = tmat<3, 2, std::uint64_t>;

#ifdef EIRIN_MATH_HAS_INT128
using mat2x3i128 = tmat<2, 3, detail::int128_t>;
using mat2x3u128 = tmat<2, 3, detail::uint128_t>;
using mat3x2i128 = tmat<3, 2, detail::int128_t>;
using mat3x2u128 = tmat<3, 2, detail::uint128_t>;
#endif

/* Floating point matrix */

using mat2x3f = tmat<2, 3, float>;
using mat2x3d = tmat<2, 3, double>;
using mat3x2f = tmat<3, 2, float>;
using mat3x2d = tmat<3, 2, double>;

#ifdef __STDCPP_FLOAT16_T__
using mat2x3f16 = tmat<2, 3, std::float16_t>;
using mat3x2f16 = tmat<3, 2, std::float16_t>;
#endif

#ifdef __STDCPP_FLOAT32_T__
using mat2x3f32 = tmat<2, 3, std::float32_t>;
using mat3x2f32 = tmat<3, 2, std::float32_t>;
#endif

#ifdef __STDCPP_FLOAT64_T__
using mat2x3f64 = tmat<2, 3, std::float64_t>;
using mat3x2f64 = tmat<3, 2, std::float64_t>;
#endif

#ifdef __STDCPP_FLOAT128_T__
using mat2x3f128 = tmat<2, 3, std::float128_t>;
using mat3x2f128 = tmat<3, 2, std::float128_t>;
#endif

#ifdef __STDCPP_BFLOAT16_T__
using mat2x3bf16 = tmat<2, 3, std::bfloat16_t>;
using mat3x2bf16 = tmat<3, 2, std::bfloat16_t>;
#endif

/* Fixed point matrix */

// Generic fixed-point templates.
template <typename T, typename I, unsigned int f, bool r = false>
using mat2x3fixed = tmat<2, 3, fixed_num<T, I, f, r>>;
template <typename T, typename I, unsigned int f, bool r = false>
using mat3x2fixed = tmat<3, 2, fixed_num<T, I, f, r>>;

// Concrete fixed-point matrix.
using mat2x3fixed32 = tmat<2, 3, fixed32>;
using mat3x2fixed32 = tmat<3, 2, fixed32>;
#ifdef EIRIN_MATH_HAS_INT128
using mat2x3fixed64 = tmat<2, 3, fixed64>;
using mat3x2fixed64 = tmat<3, 2, fixed64>;
#endif

} // namespace eirin

#endif
