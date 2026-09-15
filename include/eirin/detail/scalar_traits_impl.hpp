#ifndef EIRIN_MATH_DETAIL_SCALAR_TRAITS_IMPL_HPP
#define EIRIN_MATH_DETAIL_SCALAR_TRAITS_IMPL_HPP

#pragma once

#include <type_traits>
#include <concepts>
#include "../fixed.hpp"
#include "type_tmat.hpp"
#include "type_tvec.hpp"
#include "vec_swizzle.hpp"

namespace eirin
{
namespace detail
{
    template <typename T>
    struct is_fractional_scalar : std::false_type
    {};

    template <typename T>
    requires std::floating_point<std::remove_cvref_t<T>>
    struct is_fractional_scalar<T> : std::true_type
    {};

    template <typename T>
    requires is_fixed_point_v<std::remove_cvref_t<T>>
    struct is_fractional_scalar<T> : std::true_type
    {};

    template <typename T>
    inline constexpr bool is_fractional_scalar_v = is_fractional_scalar<std::remove_cvref_t<T>>::value;
} // namespace detail

template <typename T>
concept fractional_scalar = detail::is_fractional_scalar_v<T>;

template <typename T>
concept scalar_type = !matrix_type<T> && !vector_type<T> && !detail::is_swizzle_proxy<T>;
} // namespace eirin

#endif
