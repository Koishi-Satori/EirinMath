#ifndef EIRIN_MATH_DETAIL_COMPUTE_VEC_REL_HPP
#define EIRIN_MATH_DETAIL_COMPUTE_VEC_REL_HPP

#pragma once

#include "../macro.hpp"
#include "../fixed.hpp"
#include <cstdlib>

namespace eirin
{
namespace detail
{
    // the following is a helper struct to compute the epsilon value for compute_nearly_equal.
    // for those who want to override the epsilon value, they can specialize this struct for their own type.
    template <typename T>
    struct compute_epsilon
    {
        EIRIN_ALWAYS_INLINE constexpr static T eval()
        {
            return T{1e-6};
        }
    };

    template <typename T, bool isFloat>
    struct compute_equal
    {
        EIRIN_ALWAYS_INLINE constexpr static bool eval(T a, T b)
        {
            return a == b;
        }
    };

    // for floating point types, we need to consider the precision issues
    template <typename T, bool override>
    struct compute_nearly_equal
    {
        EIRIN_ALWAYS_INLINE constexpr static bool eval(T a, T b)
        {
            if constexpr(std::numeric_limits<T>::is_iec559)
            {
                using std::abs;
                return abs(a - b) <= compute_epsilon<T>::eval();
            }
            else
            {
                return a == b;
            }
        }
    };

    // template <typename T, typename I, unsigned int f, bool r>
    // struct compute_epsilon<fixed_num<T, I, f, r>>
    // {
    //     EIRIN_ALWAYS_INLINE constexpr static fixed_num<T, I, f, r> eval()
    //     {
    //         return std::numeric_limits<fixed_num<T, I, f, r>>::epsilon();
    //     }
    // };

    // template <typename T, typename I, unsigned int f, bool r>
    // struct compute_equal<fixed_num<T, I, f, r>, false>
    // {
    //     using fixed = fixed_num<T, I, f, r>;

    //     EIRIN_ALWAYS_INLINE constexpr static bool eval(fixed a, fixed b)
    //     {
    //         using std::abs;
    //         return abs(a - b) <= compute_epsilon<fixed>::eval();
    //     }
    // };
} // namespace detail
} // namespace eirin

#endif
