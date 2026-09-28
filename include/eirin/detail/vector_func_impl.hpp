#ifndef EIRIN_MATH_DETAIL_VECTOR_FUNC_IMPL_HPP
#define EIRIN_MATH_DETAIL_VECTOR_FUNC_IMPL_HPP

#pragma once

#include "vector_func.hpp"

namespace eirin::detail
{
/* Unrolled scalar kernels; the generic reference implementation is the primary
   template in vector_func.hpp.  Accelerated kernels are added as
   `compute_dot<N, T, vector_simd_kernel<...>>` specializations in a header that
   is only compiled when the platform and the user switch agree. */
template <typename T>
struct compute_dot<2, T, vector_scalar_kernel>
{
    EIRIN_ALWAYS_INLINE constexpr static T eval(const tvec<2, T>& lhs, const tvec<2, T>& rhs) noexcept
    {
        return lhs.x * rhs.x + lhs.y * rhs.y;
    }
};

template <typename T>
struct compute_dot<3, T, vector_scalar_kernel>
{
    EIRIN_ALWAYS_INLINE constexpr static T eval(const tvec<3, T>& lhs, const tvec<3, T>& rhs) noexcept
    {
        return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
    }
};

template <typename T>
struct compute_dot<4, T, vector_scalar_kernel>
{
    EIRIN_ALWAYS_INLINE constexpr static T eval(const tvec<4, T>& lhs, const tvec<4, T>& rhs) noexcept
    {
        return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
    }
};
} // namespace eirin::detail

#endif
