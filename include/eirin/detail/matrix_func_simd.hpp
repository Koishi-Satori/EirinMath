#ifndef EIRIN_MATH_DETAIL_MATRIX_FUNC_SIMD_HPP
#define EIRIN_MATH_DETAIL_MATRIX_FUNC_SIMD_HPP

#pragma once

/* Accelerated matrix kernels.
 *
 * This header is included from matrix_func.hpp only when the platform reports
 * SIMD support (EIRIN_PLATFORM_HAS_SIMD) and EIRIN_MATRIX_ENABLE_SIMD is
 * enabled, so whether a specialization below exists is decided purely by
 * macros.  A runtime CPU probe (see ext/simd_math.hpp) may pick a path inside a
 * kernel, but it must never decide whether the specialization itself exists.
 *
 * A kernel is added in two steps:
 *   1. specialize detail::matrix_kernel_selector<C, R, T> in matrix_func.hpp to
 *      the matching tag, e.g. matrix_simd_kernel<float, arch_avx2, 8>;
 *   2. define the kernel here, e.g.
 *        template <typename T>
 *        struct compute_inverse<4, 4, T, matrix_simd_kernel<T, arch_avx2, 8>>
 *        {
 *            EIRIN_MATH_SMALL_FUNC_API static tmat<4, 4, T> eval(const tmat<4, 4, T>& mat) noexcept;
 *        };
 *
 * Every kernel must reproduce its scalar counterpart exactly: bit identical for
 * fixed point types (same widening multiply, same rounding, no contracted
 * multiply-add) and within the usual tolerance for floating point types.  The
 * scalar kernels are the reference implementation and the unit tests compare
 * both, so start from the scalar expansion and only then vectorize it.
 */

#endif // EIRIN_MATH_DETAIL_MATRIX_FUNC_SIMD_HPP
