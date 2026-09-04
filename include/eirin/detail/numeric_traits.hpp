#ifndef EIRIN_MATH_DETAIL_NUMERIC_TRAITS_HPP
#define EIRIN_MATH_DETAIL_NUMERIC_TRAITS_HPP

#pragma once

#include <limits>
#include "./type_traits_impl.hpp"

namespace eirin::detail
{
/**
 * @brief An integral trait for any possible fixed storage type, similar to `__gnu_cxx::__int_traits`.
 * 
 * @tparam T the integral type to query.
 */
template <typename T>
struct __any_int_traits
{
private:
    static consteval bool __eval_signed() noexcept
    {
        if constexpr(is_signed_v<T>)
        {
            return true;
        }
        else if constexpr(is_unsigned_v<T>)
        {
            return false;
        }
        else if constexpr(requires { static_cast<T>(-1) < static_cast<T>(0); })
        {
            return static_cast<T>(-1) < static_cast<T>(0);
        }
        else
        {
            return false;
        }
    }

    static consteval int __eval_digits() noexcept
    {
        if constexpr(std::numeric_limits<T>::is_specialized)
        {
            return std::numeric_limits<T>::digits;
        }
        else if constexpr(__eval_signed())
        {
            return sizeof(T) * 8 - 1;
        }
        else
        {
            return sizeof(T) * 8;
        }
    }

    // 2^digits - 1 without ever forming 2^digits: max = (2^(digits-1)-1) + 2^(digits-1).
    static consteval T __eval_max_impl() noexcept
    {
        constexpr T half = static_cast<T>(1) << (__eval_digits() - 1);
        return half - static_cast<T>(1) + half;
    }

    static consteval T __eval_max() noexcept
    {
        if constexpr(std::numeric_limits<T>::is_specialized)
        {
            return std::numeric_limits<T>::max();
        }
        else
        {
            return __eval_max_impl();
        }
    }

    static consteval T __eval_min() noexcept
    {
        if constexpr(std::numeric_limits<T>::is_specialized)
        {
            return std::numeric_limits<T>::min();
        }
        else if constexpr(__eval_signed())
        {
            // min == -(2^digits) == -max - 1; no bitwise complement required.
            return -__eval_max() - static_cast<T>(1);
        }
        else
        {
            return static_cast<T>(0);
        }
    }

public:
    static constexpr bool is_signed = __eval_signed();
    static constexpr int digits = __eval_digits();
    static constexpr T max = __eval_max();
    static constexpr T min = __eval_min();
};
} // namespace eirin::detail

#endif
