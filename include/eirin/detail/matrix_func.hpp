#ifndef EIRIN_MATH_DETAIL_MATRIX_FUNC_HPP
#define EIRIN_MATH_DETAIL_MATRIX_FUNC_HPP

#pragma once

#include <cstddef>
#include <concepts>
#include <limits>
#include <type_traits>
#include "../macro.hpp"
#include "type_tmat.hpp"
#include "scalar_traits_impl.hpp"
#include "type_traits_impl.hpp"
#include "../fixed.hpp"

namespace eirin
{
namespace detail
{
    template <std::size_t C, std::size_t R, typename T>
    struct compute_inverse
    {
        EIRIN_MATH_FUNC_API static tmat<C, R, T> eval([[maybe_unused]] const tmat<C, R, T>& mat) noexcept
        {
            static_assert(__always_false<T>, "this operation is not allowed for this shape of mat.");
        }
    };

    template <std::size_t C, std::size_t R, typename T>
    struct compute_transpose
    {
        EIRIN_MATH_FUNC_API static typename tmat<C, R, T>::transpose_type eval([[maybe_unused]] const tmat<C, R, T>& mat) noexcept
        {
            static_assert(__always_false<T>, "this operation is not allowed for this shape of mat.");
        }
    };

    template <std::size_t C, std::size_t R, typename T>
    struct compute_determinant
    {
        EIRIN_MATH_FUNC_API static T eval([[maybe_unused]] const tmat<C, R, T>& mat) noexcept
        {
            static_assert(__always_false<T>, "this operation is not allowed for this shape of mat.");
        }
    };
} // namespace detail

/**
 * @brief Calc the inverse matrix of the given matrix `mat`.
 * 
 * @tparam C cols of matrix
 * @tparam R rows of matrix
 * @tparam T matrix value type
 * @param mat input matrix.
 * @return inverse matrix of `mat`.
 * @note inverse on a Singular Matrix is UB.
 */
template <std::size_t C, std::size_t R, typename T>
requires fractional_scalar<T> && (C == R)
EIRIN_MATH_FUNC_API tmat<C, R, T> inverse(const tmat<C, R, T>& mat) noexcept
{
    return detail::compute_inverse<C, R, T>::eval(mat);
}

template <std::size_t C, std::size_t R, typename T>
EIRIN_MATH_FUNC_API typename tmat<C, R, T>::transpose_type transpose(const tmat<C, R, T>& mat) noexcept
{
    return detail::compute_transpose<C, R, T>::eval(mat);
}

template <std::size_t C, std::size_t R, typename T>
requires (C == R)
EIRIN_MATH_FUNC_API T determinant(const tmat<C, R, T>& mat) noexcept
{
    return detail::compute_determinant<C, R, T>::eval(mat);
}
} // namespace eirin

// include eval struct implementations
#include "matrix_func_impl.hpp"

#endif
