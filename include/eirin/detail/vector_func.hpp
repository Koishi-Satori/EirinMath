#ifndef EIRIN_MATH_DETAIL_VECTOR_FUNC_HPP
#define EIRIN_MATH_DETAIL_VECTOR_FUNC_HPP

#pragma once

#include <concepts>
#include "kernel_tags.hpp"
#include "type_tvec.hpp"

namespace eirin
{
namespace detail
{
    // clang-format off

    /* Compute Kernel */

    // Vector kernel tags. `vector_scalar_kernel` marks the scalar eval
    // implementation, `vector_simd_kernel` marks SIMD accelerated one.
    struct vector_scalar_kernel {};

    template <typename T, typename Arch, std::size_t Lanes>
    using vector_simd_kernel = simd_kernel<T, Arch, Lanes>;

    /* Compute Kernel selection table for the vector functors.

       To add new accelerated kernel, you need to specialize this table for the matching
       (N, T), e.g.
           template <> struct vector_kernel_selector<4, float>
           { using type = vector_simd_kernel<float, arch_avx2, 8>; };
       and add the matching `compute_*(..., vector_simd_kernel<...>)`
       specializations where they belong (a header that is only compiled when
       the platform and the user switch agree, see matrix_func_simd.hpp for the
       pattern).

       Contract: an accelerated kernel must be numerically identical to the
       scalar one - bit identical for fixed point (same widening, same rounding,
       no contracted multiply-add) and equal within the usual tolerance for
       floating point. The scalar kernels are the reference implementation and
       the differential tests compare both. */
    template <std::size_t N, typename T>
    struct vector_kernel_selector
    {
        using type = vector_scalar_kernel; // TODO: SIMD support further.
    };

    // clang-format on

    template <std::size_t N, typename T, typename Kernel = typename vector_kernel_selector<N, T>::type>
    struct compute_dot
    {
        EIRIN_ALWAYS_INLINE constexpr static T eval(const tvec<N, T>& lhs, const tvec<N, T>& rhs) noexcept
        {
            return lhs.dot(rhs);
        }
    };

    template <std::size_t N, typename T, typename Kernel = typename vector_kernel_selector<N, T>::type>
    struct compute_cross
    {
        EIRIN_ALWAYS_INLINE constexpr static auto eval(const tvec<N, T>& lhs, const tvec<N, T>& rhs) noexcept
        {
            // GLSL only defines cross() for 3 component vectors; the 2 component
            // form is the exterior product (a scalar) and is offered as an
            // extension.
            // other dimension is not supported(tvec7) or not well-defined in math(tvec4).
            static_assert(N == 2 || N == 3, "cross is only well-defined for 2 component (exterior product) and 3 component vectors.");
            return lhs.cross(rhs);
        }
    };

    template <std::size_t N, typename T, typename Kernel = typename vector_kernel_selector<N, T>::type>
    struct compute_splat
    {
        EIRIN_ALWAYS_INLINE constexpr static tvec<N, T> eval(const tvec<N, T>& vec, std::size_t idx) noexcept
        {
            EIRIN_INDEX_LENGTH_ASSERT(idx, N);
            tvec<N, T> res(T{0});
            for(std::size_t i = 0; i < N; ++i)
                res[i] = vec[idx];
            return res;
        }
    };
} // namespace detail

template <std::size_t N, typename T>
EIRIN_MATH_FUNC_API T dot(const tvec<N, T>& lhs, const tvec<N, T>& rhs) noexcept
{
    return detail::compute_dot<N, T>::eval(lhs, rhs);
}

template <std::size_t N, typename T>
EIRIN_MATH_FUNC_API auto cross(const tvec<N, T>& lhs, const tvec<N, T>& rhs) noexcept
{
    return detail::compute_cross<N, T>::eval(lhs, rhs);
}

template <std::size_t N, typename T>
EIRIN_MATH_FUNC_API tvec<N, T> splat(const tvec<N, T>& vec, std::size_t idx) noexcept
{
    return detail::compute_splat<N, T>::eval(vec, idx);
}
} // namespace eirin

#include "vector_func_impl.hpp"

#endif
