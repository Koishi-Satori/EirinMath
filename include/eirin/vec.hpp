#ifndef EIRIN_MATH_VEC_HPP
#define EIRIN_MATH_VEC_HPP

#pragma once

#include <cstddef>
#include <eirin/macro.hpp>
#include <eirin/detail/type_tvec2.hpp>
#include <eirin/detail/type_tvec3.hpp>
#include <eirin/detail/type_tvec4.hpp>

namespace eirin
{
template <typename T>
using vec2 = tvec<2, T>;
template <typename T>
using vec3 = tvec<3, T>;
template <typename T>
using vec4 = tvec<4, T>;
using ivec2 = vec2<int>;
using ivec4 = vec4<int>;
using f32vec4 = vec4<fixed32>;
using f64vec4 = vec4<fixed64>;
} // namespace eirin

#endif
