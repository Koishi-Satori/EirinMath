#ifndef EIRIN_MATH_DETAIL_MATRIX_FUNC_IMPL_HPP
#define EIRIN_MATH_DETAIL_MATRIX_FUNC_IMPL_HPP

#pragma once

#include "matrix_func.hpp"

namespace eirin::detail
{
template <typename T>
struct compute_inverse<2, 2, T, matrix_scalar_kernel>
{
    EIRIN_MATH_SMALL_FUNC_API static tmat<2, 2, T> eval(const tmat<2, 2, T>& mat) noexcept
    {
        T one_over_det = static_cast<T>(1) / (mat[0][0] * mat[1][1] - mat[1][0] * mat[0][1]);
        tmat<2, 2, T> inverse_mat(mat[1][1] * one_over_det, -mat[0][1] * one_over_det, -mat[1][0] * one_over_det, mat[0][0] * one_over_det);

        return inverse_mat;
    }
};

template <typename T>
struct compute_inverse<3, 3, T, matrix_scalar_kernel>
{
    EIRIN_MATH_SMALL_FUNC_API static tmat<3, 3, T> eval(const tmat<3, 3, T>& mat) noexcept
    {
        // @see: https://www.onlinemathstutor.org/post/3x3_inverses
        auto row0 = mat[1].cross(mat[2]);
        auto row1 = mat[2].cross(mat[0]);
        auto row2 = mat[0].cross(mat[1]);
        T one_over_det = static_cast<T>(1) / (dot(mat[0], row0));
        // clang-format off
        return tmat<3, 3, T>(
            tvec<3, T>(row0[0], row1[0], row2[0]) * one_over_det,
            tvec<3, T>(row0[1], row1[1], row2[1]) * one_over_det,
            tvec<3, T>(row0[2], row1[2], row2[2]) * one_over_det
        );
        // clang-format on
    }
};

template <typename T>
struct compute_inverse<4, 4, T, matrix_scalar_kernel>
{
    EIRIN_MATH_SMALL_FUNC_API static tmat<4, 4, T> eval(const tmat<4, 4, T>& mat) noexcept
    {
        T c00 = mat[2][2] * mat[3][3] - mat[3][2] * mat[2][3];
        T c02 = mat[1][2] * mat[3][3] - mat[3][2] * mat[1][3];
        T c03 = mat[1][2] * mat[2][3] - mat[2][2] * mat[1][3];

        T c04 = mat[2][1] * mat[3][3] - mat[3][1] * mat[2][3];
        T c06 = mat[1][1] * mat[3][3] - mat[3][1] * mat[1][3];
        T c07 = mat[1][1] * mat[2][3] - mat[2][1] * mat[1][3];

        T c08 = mat[2][1] * mat[3][2] - mat[3][1] * mat[2][2];
        T c10 = mat[1][1] * mat[3][2] - mat[3][1] * mat[1][2];
        T c11 = mat[1][1] * mat[2][2] - mat[2][1] * mat[1][2];

        T c12 = mat[2][0] * mat[3][3] - mat[3][0] * mat[2][3];
        T c14 = mat[1][0] * mat[3][3] - mat[3][0] * mat[1][3];
        T c15 = mat[1][0] * mat[2][3] - mat[2][0] * mat[1][3];

        T c16 = mat[2][0] * mat[3][2] - mat[3][0] * mat[2][2];
        T c18 = mat[1][0] * mat[3][2] - mat[3][0] * mat[1][2];
        T c19 = mat[1][0] * mat[2][2] - mat[2][0] * mat[1][2];

        T c20 = mat[2][0] * mat[3][1] - mat[3][0] * mat[2][1];
        T c22 = mat[1][0] * mat[3][1] - mat[3][0] * mat[1][1];
        T c23 = mat[1][0] * mat[2][1] - mat[2][0] * mat[1][1];

        tvec<4, T> f0(c00, c00, c02, c03);
        tvec<4, T> f1(c04, c04, c06, c07);
        tvec<4, T> f2(c08, c08, c10, c11);
        tvec<4, T> f3(c12, c12, c14, c15);
        tvec<4, T> f4(c16, c16, c18, c19);
        tvec<4, T> f5(c20, c20, c22, c23);

        tvec<4, T> v0(mat[1][0], mat[0][0], mat[0][0], mat[0][0]);
        tvec<4, T> v1(mat[1][1], mat[0][1], mat[0][1], mat[0][1]);
        tvec<4, T> v2(mat[1][2], mat[0][2], mat[0][2], mat[0][2]);
        tvec<4, T> v3(mat[1][3], mat[0][3], mat[0][3], mat[0][3]);

        tvec<4, T> inv0(v1 * f0 - v2 * f1 + v3 * f2);
        tvec<4, T> inv1(v0 * f0 - v2 * f3 + v3 * f4);
        tvec<4, T> inv2(v0 * f1 - v1 * f3 + v3 * f5);
        tvec<4, T> inv3(v0 * f2 - v1 * f4 + v2 * f5);

        tvec<4, T> sign1(1, -1, 1, -1);
        tvec<4, T> sign2(-1, 1, -1, 1);
        tmat<4, 4, T> inverse(inv0 * sign1, inv1 * sign2, inv2 * sign1, inv3 * sign2);

        tvec<4, T> r0(inverse[0][0], inverse[1][0], inverse[2][0], inverse[3][0]);

        tvec<4, T> dot0(mat[0] * r0);
        T dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

        T one_over_det = static_cast<T>(1) / dot1;

        return inverse * one_over_det;
    }
};

template <typename T>
struct compute_transpose<2, 2, T, matrix_scalar_kernel>
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
struct compute_transpose<3, 3, T, matrix_scalar_kernel>
{
    EIRIN_MATH_SMALL_FUNC_API static typename tmat<3, 3, T>::transpose_type eval(const tmat<3, 3, T>& mat) noexcept
    {
        using transpose_type = typename tmat<3, 3, T>::transpose_type;
        transpose_type result;
        // transpose: T[c][r] = M[r][c]
        result[0][0] = mat[0][0];
        result[0][1] = mat[1][0];
        result[0][2] = mat[2][0];

        result[1][0] = mat[0][1];
        result[1][1] = mat[1][1];
        result[1][2] = mat[2][1];

        result[2][0] = mat[0][2];
        result[2][1] = mat[1][2];
        result[2][2] = mat[2][2];
        return result;
    }
};

template <typename T>
struct compute_transpose<4, 4, T, matrix_scalar_kernel>
{
    EIRIN_MATH_SMALL_FUNC_API static typename tmat<4, 4, T>::transpose_type eval(const tmat<4, 4, T>& mat) noexcept
    {
        using transpose_type = typename tmat<4, 4, T>::transpose_type;
        transpose_type result;
        // transpose: T[c][r] = M[r][c]
        result[0][0] = mat[0][0];
        result[0][1] = mat[1][0];
        result[0][2] = mat[2][0];
        result[0][3] = mat[3][0];

        result[1][0] = mat[0][1];
        result[1][1] = mat[1][1];
        result[1][2] = mat[2][1];
        result[1][3] = mat[3][1];

        result[2][0] = mat[0][2];
        result[2][1] = mat[1][2];
        result[2][2] = mat[2][2];
        result[2][3] = mat[3][2];

        result[3][0] = mat[0][3];
        result[3][1] = mat[1][3];
        result[3][2] = mat[2][3];
        result[3][3] = mat[3][3];
        return result;
    }
};

template <typename T>
struct compute_determinant<2, 2, T, matrix_scalar_kernel>
{
    EIRIN_MATH_SMALL_FUNC_API static T eval(const tmat<2, 2, T>& mat) noexcept
    {
        return mat[0][0] * mat[1][1] - mat[1][0] * mat[0][1];
    }
};

template <typename T>
struct compute_determinant<3, 3, T, matrix_scalar_kernel>
{
    EIRIN_MATH_SMALL_FUNC_API static T eval(const tmat<3, 3, T>& mat) noexcept
    {
        // clang-format off
        return mat[0][0] * (mat[1][1] * mat[2][2] - mat[2][1] * mat[1][2])
				- mat[1][0] * (mat[0][1] * mat[2][2] - mat[2][1] * mat[0][2])
				+ mat[2][0] * (mat[0][1] * mat[1][2] - mat[1][1] * mat[0][2]);
        // clang-format on
    }
};

template <typename T>
struct compute_determinant<4, 4, T, matrix_scalar_kernel>
{
    EIRIN_MATH_SMALL_FUNC_API static T eval(const tmat<4, 4, T>& mat) noexcept
    {
        T f00 = mat[2][2] * mat[3][3] - mat[3][2] * mat[2][3];
        T f01 = mat[2][1] * mat[3][3] - mat[3][1] * mat[2][3];
        T f02 = mat[2][1] * mat[3][2] - mat[3][1] * mat[2][2];
        T f03 = mat[2][0] * mat[3][3] - mat[3][0] * mat[2][3];
        T f04 = mat[2][0] * mat[3][2] - mat[3][0] * mat[2][2];
        T f05 = mat[2][0] * mat[3][1] - mat[3][0] * mat[2][1];

        tvec<4, T> cof(
            (mat[1][1] * f00 - mat[1][2] * f01 + mat[1][3] * f02),
            -(mat[1][0] * f00 - mat[1][2] * f03 + mat[1][3] * f04),
            (mat[1][0] * f01 - mat[1][1] * f03 + mat[1][3] * f05),
            -(mat[1][0] * f02 - mat[1][1] * f04 + mat[1][2] * f05)
        );

        return mat[0][0] * cof[0] + mat[0][1] * cof[1] +
               mat[0][2] * cof[2] + mat[0][3] * cof[3];
    }
};
} // namespace eirin::detail

#endif
