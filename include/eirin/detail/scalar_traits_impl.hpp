#ifndef EIRIN_MATH_DETAIL_SCALAR_TRAITS_IMPL_HPP
#define EIRIN_MATH_DETAIL_SCALAR_TRAITS_IMPL_HPP

#pragma once

#include <type_traits>
#include <concepts>
#include "../traits.hpp"
#include "../fixed.hpp"
#include "type_tmat.hpp"
#include "type_tvec.hpp"
#include "vec_swizzle.hpp"

namespace eirin
{
namespace traits
{
    template <typename T>
    requires is_fixed_point_v<std::remove_cvref_t<T>>
    struct is_fractional_scalar<T> : std::true_type
    {};
} // namespace traits

template <typename T>
concept fractional_scalar = traits::is_fractional_scalar_v<T>;

template <typename T>
concept scalar_type = !matrix_type<T> && !vector_type<T> && !detail::is_swizzle_proxy<T>;
} // namespace eirin

#endif
