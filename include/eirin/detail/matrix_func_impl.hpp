#ifndef EIRIN_MATH_DETAIL_MATRIX_FUNC_IMPL_HPP
#define EIRIN_MATH_DETAIL_MATRIX_FUNC_IMPL_HPP

#pragma once

#include "matrix_func.hpp"

namespace eirin::detail
{
template <typename T>
struct compute_inverse<2, 2, T>
{
    EIRIN_MATH_SMALL_FUNC_API static tmat<2, 2, T> eval(const tmat<2, 2, T>& mat) noexcept
    {
        T one_over_det = static_cast<T>(1) / (mat[0][0] * mat[1][1] - mat[1][0] * mat[0][1]);
        tmat<2, 2, T> inverse_mat(mat[1][1] * one_over_det, -mat[0][1] * one_over_det, -mat[1][0] * one_over_det, mat[0][0] * one_over_det);

        return inverse_mat;
    }
};

template <typename T>
struct compute_transpose<2, 2, T>
{
    EIRIN_MATH_SMALL_FUNC_API static typename tmat<2, 2, T>::transpose_type eval(const tmat<2, 2, T>& mat) noexcept
    {
        using transpose_type = typename tmat<2, 2, T>::transpose_type;
        transpose_type result;
        result[0][0] = mat[0][0];
        result[0][1] = mat[1][0];
        result[1][0] = mat[0][1];
        result[1][1] = mat[1][1];
        return result;
    }
};

template <typename T>
struct compute_determinant<2, 2, T>
{
    EIRIN_MATH_SMALL_FUNC_API static T eval(const tmat<2, 2, T>& mat) noexcept
    {
        return mat[0][0] * mat[1][1] - mat[1][0] * mat[0][1];
    }
};
} // namespace eirin::detail

#endif
