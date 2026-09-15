#ifndef EIRIN_MATH_MAT_HPP
#define EIRIN_MATH_MAT_HPP

#pragma once

#include <cstddef>
#include <cstdint>
#if defined(__has_include)
#    if __has_include(<stdfloat>)
#        include <stdfloat>
#    endif
#endif
#include "macro.hpp"
#include "detail/type_tmat.hpp"
#include "detail/type_tmat2x2.hpp"
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

/* Sized signed/unsigned integer matrix */

#ifdef EIRIN_MATH_HAS_INT128
using mat2i128 = tmat<2, 2, detail::int128_t>;
using mat2u128 = tmat<2, 2, detail::uint128_t>;
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

} // namespace eirin

#endif
