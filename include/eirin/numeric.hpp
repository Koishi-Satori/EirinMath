#ifndef EIRIN_MATH_NUMERIC_HPP
#define EIRIN_MATH_NUMERIC_HPP

#pragma once

#include "fixed.hpp"
#include "math.hpp"
#include "numbers.hpp"

namespace eirin
{
namespace detail
{
    template <typename T, typename I>
    EIRIN_MATH_SMALL_FUNC_API bool __add_overflow(T x, T y, T* z) noexcept
    {
        if constexpr(detail::is_signed_v<T> && has_make_unsigned_v<T>)
        {
            // use unsigned version to check.
            using u_type = detail::make_unsigned_t<T>;
            u_type res = static_cast<u_type>(x) + static_cast<u_type>(y);
            *z = static_cast<T>(res);
            constexpr u_type sign_bit = static_cast<u_type>(1) << (std::numeric_limits<u_type>::is_specialized ? std::numeric_limits<u_type>::digits - 1 : sizeof(T) * 8 - 1);
            const bool pos_overflow = (x > 0 && y > 0 && (res & sign_bit));
            const bool neg_overflow = (x < 0 && y < 0 && !(res & sign_bit));
            return pos_overflow || neg_overflow;
        }
        I res = static_cast<I>(x) + static_cast<I>(y);
        *z = static_cast<T>(res);
        return res > static_cast<I>(detail::__max_value<T, I>) ||
               res < static_cast<I>(detail::__min_value<T, I>);
    }
} // namespace detail

template <typename T>
requires fixed_point<T>
EIRIN_MATH_SMALL_FUNC_API T add_sat(T x, T y) noexcept
{
    using type = typename T::value_type;
    using intermediate_type = typename T::intermediate_type;
    type z;
    if(!detail::__add_overflow<type, intermediate_type>(x.internal_value(), y.internal_value(), &z))
        return T::from_internal_value(z);
    if constexpr(detail::is_unsigned_v<type>)
        return T::from_internal_value(detail::__max_value<type, intermediate_type>);
    else if(x < T(0))
        return T::from_internal_value(detail::__min_value<type, intermediate_type>);
    else
        return T::from_internal_value(detail::__max_value<type, intermediate_type>);
}

// template <typename T>
// requires fixed_point<T>
// EIRIN_MATH_SMALL_FUNC_API T add_modwarp(T x, T y) noexcept
// {
//     using value_type = typename T::value_type;
//     using intermediate_type = typename T::intermediate_type;
//     constexpr auto fraction = T::precision;
// }
} // namespace eirin

#endif
