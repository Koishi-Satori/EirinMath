#ifndef EIRIN_MATH_VEC_HPP
#define EIRIN_MATH_VEC_HPP

#pragma once

#include <cstddef>
#include <eirin/macro.hpp>
#include <eirin/detail/type_tvec2.hpp>

namespace eirin
{
template <typename T>
using vec2 = tvec<2, T>;
} // namespace eirin

#endif
