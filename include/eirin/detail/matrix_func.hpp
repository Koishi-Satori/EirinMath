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
#include "kernel_tags.hpp"
#include "../fixed.hpp"

namespace eirin
{
namespace detail
{
    // clang-format off

    /* Compute Kernel */

    // Vector kernel tags. `matrix_scalar_kernel` marks the scalar eval
    // implementation, `matrix_simd_kernel` marks SIMD accelerated one.
    struct matrix_scalar_kernel {};

    template <typename T, typename Arch, std::size_t Lanes>
    using matrix_simd_kernel = simd_kernel<T, Arch, Lanes>;

    /* Compute Kernel selection table for the matrix functors.

       To add an accelerated kernel:
         1. specialize this table, e.g.
              template <> struct matrix_kernel_selector<4, 4, float>
              { using type = matrix_simd_kernel<float, arch_avx2, 8>; };
         2. add the matching `compute_*(..., matrix_simd_kernel<float, arch_avx2, 8>)`
            specializations in matrix_func_simd.hpp (which is only compiled when
            both EIRIN_PLATFORM_HAS_SIMD and EIRIN_MATRIX_ENABLE_SIMD are set).

       Contract: a SIMD kernel must be numerically identical to the scalar
       kernel of the same shape - bit identical for fixed point (same widening,
       same rounding, no contracted multiply-add) and equal up to the usual
       floating point tolerance otherwise.  The scalar kernels are the reference
       implementation and the differential tests compare both. */
    template <std::size_t C, std::size_t R, typename T>
    struct matrix_kernel_selector
    {
          using type = matrix_scalar_kernel;   // TODO: SIMD support further.
    };

    // clang-format on

    template <std::size_t C, std::size_t R, typename T, typename Kernel = typename matrix_kernel_selector<C, R, T>::type>
    requires fractional_scalar<T> && square_matrix<C, R>
    struct compute_inverse
    {
        EIRIN_MATH_FUNC_API static tmat<C, R, T> eval([[maybe_unused]] const tmat<C, R, T>& mat) noexcept
        {
            static_assert(__always_false<T>, "this operation is not allowed for this shape of mat.");
        }
    };

    template <std::size_t C, std::size_t R, typename T, typename Kernel = typename matrix_kernel_selector<C, R, T>::type>
    struct compute_transpose
    {
        EIRIN_MATH_FUNC_API static typename tmat<C, R, T>::transpose_type eval([[maybe_unused]] const tmat<C, R, T>& mat) noexcept
        {
            static_assert(__always_false<T>, "this operation is not allowed for this shape of mat.");
        }
    };

    template <std::size_t C, std::size_t R, typename T, typename Kernel = typename matrix_kernel_selector<C, R, T>::type>
    requires square_matrix<C, R>
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
requires fractional_scalar<T> && detail::square_matrix<C, R>
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
requires detail::square_matrix<C, R>
EIRIN_MATH_FUNC_API T determinant(const tmat<C, R, T>& mat) noexcept
{
    return detail::compute_determinant<C, R, T>::eval(mat);
}
} // namespace eirin

// include eval struct implementations
#include "matrix_func_impl.hpp"

// include SIMD eval struct implementations
#if defined(EIRIN_PLATFORM_HAS_SIMD) && (EIRIN_MATRIX_ENABLE_SIMD == EIRIN_ENABLE)
#    include "matrix_func_simd.hpp"
#endif

#endif
