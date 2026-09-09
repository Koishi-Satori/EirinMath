#ifndef EIRIN_MATH_VEC_HPP
#define EIRIN_MATH_VEC_HPP

#pragma once

#include <cstddef>
#include <cstdint>
#if defined(__has_include)
#    if __has_include(<stdfloat>)
#        include <stdfloat>
#    endif
#endif
#include "macro.hpp"
#include "detail/type_tvec2.hpp"
#include "detail/type_tvec3.hpp"
#include "detail/type_tvec4.hpp"
#include "detail/int128.hpp"

namespace eirin
{
template <typename T>
using vec2 = tvec<2, T>;
template <typename T>
using vec3 = tvec<3, T>;
template <typename T>
using vec4 = tvec<4, T>;

/* Native (platform) width types */

using vec2i = tvec<2, int>;
using vec2u = tvec<2, unsigned int>;
using vec3i = tvec<3, int>;
using vec3u = tvec<3, unsigned int>;
using vec4i = tvec<4, int>;
using vec4u = tvec<4, unsigned int>;

/* Sized signed/unsigned integer vectors */

using vec2i8 = tvec<2, std::int8_t>;
using vec2u8 = tvec<2, std::uint8_t>;
using vec2i16 = tvec<2, std::int16_t>;
using vec2u16 = tvec<2, std::uint16_t>;
using vec2i32 = tvec<2, std::int32_t>;
using vec2u32 = tvec<2, std::uint32_t>;
using vec2i64 = tvec<2, std::int64_t>;
using vec2u64 = tvec<2, std::uint64_t>;

using vec3i8 = tvec<3, std::int8_t>;
using vec3u8 = tvec<3, std::uint8_t>;
using vec3i16 = tvec<3, std::int16_t>;
using vec3u16 = tvec<3, std::uint16_t>;
using vec3i32 = tvec<3, std::int32_t>;
using vec3u32 = tvec<3, std::uint32_t>;
using vec3i64 = tvec<3, std::int64_t>;
using vec3u64 = tvec<3, std::uint64_t>;

using vec4i8 = tvec<4, std::int8_t>;
using vec4u8 = tvec<4, std::uint8_t>;
using vec4i16 = tvec<4, std::int16_t>;
using vec4u16 = tvec<4, std::uint16_t>;
using vec4i32 = tvec<4, std::int32_t>;
using vec4u32 = tvec<4, std::uint32_t>;
using vec4i64 = tvec<4, std::int64_t>;
using vec4u64 = tvec<4, std::uint64_t>;

#ifdef EIRIN_MATH_HAS_INT128
using vec2i128 = tvec<2, detail::int128_t>;
using vec2u128 = tvec<2, detail::uint128_t>;
using vec3i128 = tvec<3, detail::int128_t>;
using vec3u128 = tvec<3, detail::uint128_t>;
using vec4i128 = tvec<4, detail::int128_t>;
using vec4u128 = tvec<4, detail::uint128_t>;
#endif

/* Floating point vectors */

using vec2f = tvec<2, float>;
using vec3f = tvec<3, float>;
using vec4f = tvec<4, float>;
using vec2d = tvec<2, double>;
using vec3d = tvec<3, double>;
using vec4d = tvec<4, double>;

#ifdef __STDCPP_FLOAT16_T__
using vec2f16 = tvec<2, std::float16_t>;
using vec3f16 = tvec<3, std::float16_t>;
using vec4f16 = tvec<4, std::float16_t>;
#endif

#ifdef __STDCPP_FLOAT32_T__
using vec2f32 = tvec<2, std::float32_t>;
using vec3f32 = tvec<3, std::float32_t>;
using vec4f32 = tvec<4, std::float32_t>;
#endif

#ifdef __STDCPP_FLOAT64_T__
using vec2f64 = tvec<2, std::float64_t>;
using vec3f64 = tvec<3, std::float64_t>;
using vec4f64 = tvec<4, std::float64_t>;
#endif

#ifdef __STDCPP_FLOAT128_T__
using vec2f128 = tvec<2, std::float128_t>;
using vec3f128 = tvec<3, std::float128_t>;
using vec4f128 = tvec<4, std::float128_t>;
#endif

#ifdef __STDCPP_BFLOAT16_T__
using vec2bf16 = tvec<2, std::bfloat16_t>;
using vec3bf16 = tvec<3, std::bfloat16_t>;
using vec4bf16 = tvec<4, std::bfloat16_t>;
#endif

/* Fixed point vectors */

// Generic fixed-point templates.
template <typename T, typename I, unsigned int f, bool r = false>
using vec2fixed = tvec<2, fixed_num<T, I, f, r>>;
template <typename T, typename I, unsigned int f, bool r = false>
using vec3fixed = tvec<3, fixed_num<T, I, f, r>>;
template <typename T, typename I, unsigned int f, bool r = false>
using vec4fixed = tvec<4, fixed_num<T, I, f, r>>;

// Concrete fixed-point vectors.
using vec2fixed32 = tvec<2, fixed32>;
using vec2fixed64 = tvec<2, fixed64>;
using vec3fixed32 = tvec<3, fixed32>;
using vec3fixed64 = tvec<3, fixed64>;
using vec4fixed32 = tvec<4, fixed32>;
using vec4fixed64 = tvec<4, fixed64>;
} // namespace eirin

#endif
