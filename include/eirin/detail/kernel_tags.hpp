#ifndef EIRIN_MATH_DETAIL_KERNEL_TAGS_HPP
#define EIRIN_MATH_DETAIL_KERNEL_TAGS_HPP

#pragma once

#include <cstddef>

namespace eirin
{
namespace detail
{
    /* Instruction set levels, mirroring the EIRIN_PLATFORM_SIMD_* macros of
       macro.hpp.  A tag states which instructions a kernel may use; whether
       they are available at run time is a separate question and is answered
       inside the kernel (see ext/simd_math.hpp), never by the existence of a
       specialization. */
    struct arch_sse2
    {};

    struct arch_sse4_2
    {};

    struct arch_avx
    {};

    struct arch_avx2
    {};

    struct arch_neon
    {};

    struct arch_wasm_simd128
    {};

    /* Generic accelerated kernel tag, shared by the matrix and the vector
       functors: element type, instruction set level and lane count.

       A tag is a type (and not a bool such as GLM's `is_aligned`) because the
       dispatch axis is (shape, scalar type, instruction set): float, double and
       the fixed point types need different lanes and different multiply
       strategies, so a single flag could not name them. */
    template <typename T, typename Arch, std::size_t Lanes>
    struct simd_kernel
    {};
} // namespace detail
} // namespace eirin

#endif // EIRIN_MATH_DETAIL_KERNEL_TAGS_HPP
