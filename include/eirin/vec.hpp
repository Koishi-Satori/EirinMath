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
} // namespace eirin

#endif
