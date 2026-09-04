#ifndef EIRIN_MATH_NUMERIC_HPP
#define EIRIN_MATH_NUMERIC_HPP

#pragma once

#include <numeric>
#include "detail/numeric_traits.hpp"
#include "fixed.hpp"
#include "math.hpp"
#include "numbers.hpp"

namespace eirin
{
namespace detail
{
    template <typename T, typename I>
    concept __has_enough_bits_imul = sizeof(I) >= sizeof(T) * 2;

    /**
     * @brief Detect overflow of `x + y` in the storage type `T`.
     *
     * @tparam T the storage type.
     * @tparam I the wider intermediate type.
     * @param x the first addend.
     * @param y the second addend.
     * @param z receives the sum truncated back to `T`. This is the exact sum
     *          when there is no overflow and the mod-2^N wrapped sum when
     *          there is, so `z` is always usable.
     * @return `true` iff `x + y` overflows `T`.
     */
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
        return res > static_cast<I>(detail::__any_int_traits<T>::max) ||
               res < static_cast<I>(detail::__any_int_traits<T>::min);
    }

    template <typename T, typename I>
    EIRIN_MATH_SMALL_FUNC_API bool __sub_overflow(T x, T y, T* z) noexcept
    {
        if constexpr(detail::is_signed_v<T> && has_make_unsigned_v<T>)
        {
            // use unsigned version to check.
            // neg - pos can neg overflow, pos - neg can pos overflow
            using u_type = detail::make_unsigned_t<T>;
            u_type res = static_cast<u_type>(x) - static_cast<u_type>(y);
            *z = static_cast<T>(res);
            constexpr u_type sign_bit = static_cast<u_type>(1) << (std::numeric_limits<u_type>::is_specialized ? std::numeric_limits<u_type>::digits - 1 : sizeof(T) * 8 - 1);
            const bool pos_overflow = (x >= 0 && y < 0 && (res & sign_bit));
            const bool neg_overflow = (x < 0 && y >= 0 && !(res & sign_bit));
            return pos_overflow || neg_overflow;
        }
        I res = static_cast<I>(x) - static_cast<I>(y);
        *z = static_cast<T>(res);
        return res > static_cast<I>(detail::__any_int_traits<T>::max) ||
               res < static_cast<I>(detail::__any_int_traits<T>::min);
    }

    template <typename T, typename I, bool r, unsigned int f>
    requires __has_enough_bits_imul<T, I>
    EIRIN_MATH_SMALL_FUNC_API I __mul_scaled(T x, T y) noexcept
    {
        const I p = static_cast<I>(x) * static_cast<I>(y);
        if constexpr(r)
        {
            constexpr I fraction_multiplier = I(1) << f;
            I v = p / (fraction_multiplier / 2);
            return (v + (v % 2)) >> 1;
        }
        else
        {
            return p >> f;
        }
    }

    template <typename T, typename I, bool r, unsigned int f>
    requires __has_enough_bits_imul<T, I>
    EIRIN_MATH_SMALL_FUNC_API bool __mul_overflow(T x, T y, T* z) noexcept
    {
        const I res = __mul_scaled<T, I, r, f>(x, y);
        *z = static_cast<T>(res);
        return res > static_cast<I>(detail::__any_int_traits<T>::max) ||
               res < static_cast<I>(detail::__any_int_traits<T>::min);
    }

    template <typename T, typename I, bool r, unsigned int f>
    requires __has_enough_bits_imul<T, I>
    EIRIN_MATH_SMALL_FUNC_API I __div_scaled(T x, T y) noexcept
    {
        const I num = static_cast<I>(x) << f; // x * 2^f
        if constexpr(r)
        {
            const I v = (num * 2) / y;
            return (v + (v % 2)) >> 1;
        }
        else
        {
            return num / y;
        }
    }

    template <typename T, typename I, bool r, unsigned int f, bool modwarp = true>
    requires __has_enough_bits_imul<T, I>
    EIRIN_MATH_SMALL_FUNC_API bool __div_overflow(T x, T y, T* z) noexcept
    {
        // division by zero has no fixed-point value, so always report overflow.
        if(y == 0)
        {
            *z = T(0);
            return true;
        }

        const I res = __div_scaled<T, I, r, f>(x, y);

        const bool overflow = res > static_cast<I>(detail::__any_int_traits<T>::max) ||
                              res < static_cast<I>(detail::__any_int_traits<T>::min);
        if(overflow)
        {
            if constexpr(modwarp)
                *z = static_cast<T>(res); // mod-2^N wrapped quotient
            else
                *z = T(0); // caller saturates and ignores z
        }
        else
        {
            *z = static_cast<T>(res);
        }
        return overflow;
    }
} // namespace detail

template <typename T>
requires fixed_point<T>
EIRIN_MATH_FUNC_API T saturating_add(T x, T y) noexcept
{
    using type = typename T::value_type;
    using intermediate_type = typename T::intermediate_type;
    type z;
    if(!detail::__add_overflow<type, intermediate_type>(x.internal_value(), y.internal_value(), &z))
        return T::from_internal_value(z);
    if constexpr(detail::is_unsigned_v<type>)
        return T::from_internal_value(detail::__any_int_traits<type>::max);
    else if(x < T(0))
        return T::from_internal_value(detail::__any_int_traits<type>::min);
    else
        return T::from_internal_value(detail::__any_int_traits<type>::max);
}

template <typename T>
requires fixed_point<T>
EIRIN_MATH_FUNC_API T saturating_sub(T x, T y) noexcept
{
    using type = typename T::value_type;
    using intermediate_type = typename T::intermediate_type;
    type z;
    if(!detail::__sub_overflow<type, intermediate_type>(x.internal_value(), y.internal_value(), &z))
        return T::from_internal_value(z);
    if constexpr(detail::is_unsigned_v<type>)
        return T::from_internal_value(static_cast<type>(0));
    else if(x < T(0)) // neg - pos can neg overflow
        return T::from_internal_value(detail::__any_int_traits<type>::min);
    else
        return T::from_internal_value(detail::__any_int_traits<type>::max);
}

template <typename T>
requires fixed_point<T>
EIRIN_MATH_FUNC_API T saturating_mul(T x, T y) noexcept
{
    using type = typename T::value_type;
    using intermediate_type = typename T::intermediate_type;
    constexpr bool rounding = T::is_round_enable;
    constexpr auto fraction = T::precision;

    const intermediate_type res = detail::__mul_scaled<type, intermediate_type, rounding, fraction>(
        x.internal_value(), y.internal_value()
    );

    if constexpr(detail::is_unsigned_v<type>)
    {
        const intermediate_type max_v = static_cast<intermediate_type>(detail::__any_int_traits<type>::max);
        return T::from_internal_value(static_cast<type>(res > max_v ? max_v : res));
    }
    else
    {
        const intermediate_type min_v = static_cast<intermediate_type>(detail::__any_int_traits<type>::min);
        const intermediate_type max_v = static_cast<intermediate_type>(detail::__any_int_traits<type>::max);
        const intermediate_type clamped = res < min_v ? min_v : (res > max_v ? max_v : res);
        return T::from_internal_value(static_cast<type>(clamped));
    }
}

template <typename T>
requires fixed_point<T>
EIRIN_MATH_FUNC_API T saturating_div(T x, T y) noexcept
{
    using type = typename T::value_type;
    using intermediate_type = typename T::intermediate_type;
    constexpr bool rounding = T::is_round_enable;
    constexpr auto fraction = T::precision;

    const type xi = x.internal_value();
    const type yi = y.internal_value();

    if constexpr(T::digits_int != 0)
    {
        constexpr type value_one = static_cast<type>(1) << fraction;
        if constexpr(detail::is_signed_v<type>)
        {
            constexpr type neg_value_one = -value_one;
            if(yi >= value_one || yi <= neg_value_one)
            {
                return x / y;
            }
        }
        else if(yi >= value_one)
        {
            return x / y;
        }
    }

    const intermediate_type res = detail::__div_scaled<type, intermediate_type, rounding, fraction>(xi, yi);

    if constexpr(detail::is_unsigned_v<type>)
    {
        const intermediate_type max_v = static_cast<intermediate_type>(detail::__any_int_traits<type>::max);
        return T::from_internal_value(static_cast<type>(res > max_v ? max_v : res));
    }
    else
    {
        const intermediate_type min_v = static_cast<intermediate_type>(detail::__any_int_traits<type>::min);
        const intermediate_type max_v = static_cast<intermediate_type>(detail::__any_int_traits<type>::max);
        const intermediate_type clamped = res < min_v ? min_v : (res > max_v ? max_v : res);
        return T::from_internal_value(static_cast<type>(clamped));
    }
}

/**
 * @brief Saturating value cast between two fixed-point types.
 *
 * Returns the value of `x` represented in `T`, clamped to the storage range
 * of `T` when it does not fit. Cross-fraction conversion preserves the value:
 * the source internal value is scaled to the destination fraction using the
 * same formulas as `fixed_num::from_fixed_num_value` (arithmetic in the
 * destination intermediate type), before saturation is applied.
 *
 * @tparam T the destination fixed-point type.
 * @tparam U the source fixed-point type.
 * @param x the source value.
 * @return `x` saturated to the representable range of `T`.
 */

template <typename T, typename U>
requires fixed_point<T> && fixed_point<U>
EIRIN_MATH_FUNC_API T saturating_cast(U x) noexcept
{
    using type = typename U::value_type;
    using res_type = typename T::value_type;
    using intermediate_type = typename T::intermediate_type;
    constexpr auto src_fraction = U::precision;
    constexpr auto dst_fraction = T::precision;
    constexpr res_type min_res = detail::__any_int_traits<res_type>::min;
    constexpr res_type max_res = detail::__any_int_traits<res_type>::max;
    const auto xi = x.internal_value();

    if constexpr(detail::is_signed_v<type> && !detail::is_signed_v<res_type>)
    {
        if(xi < 0)
            return T(0);
    }

    intermediate_type scaled;
    if constexpr(dst_fraction >= src_fraction)
    {
        scaled = static_cast<intermediate_type>(xi) << (dst_fraction - src_fraction);
    }
    else
    {
        constexpr auto shift = src_fraction - dst_fraction;
        const intermediate_type num = static_cast<intermediate_type>(xi);
        if constexpr(T::is_round_enable)
        {
            scaled = num / (intermediate_type(1) << shift) +
                     (num / (intermediate_type(1) << (shift - 1))) % 2;
        }
        else
        {
            scaled = num / (intermediate_type(1) << shift);
        }
    }

    if constexpr(detail::is_signed_v<res_type>)
    {
        const intermediate_type lo = static_cast<intermediate_type>(min_res);
        const intermediate_type hi = static_cast<intermediate_type>(max_res);
        scaled = scaled < lo ? lo : (scaled > hi ? hi : scaled);
    }
    else
    {
        const intermediate_type hi = static_cast<intermediate_type>(max_res);
        scaled = scaled < intermediate_type(0) ? intermediate_type(0) : (scaled > hi ? hi : scaled);
    }

    return T::from_internal_value(static_cast<res_type>(scaled));
}

/**
 * @brief Add two fixed-point numbers with defined modulo-2^N wrapping.
 *
 * On overflow the result wraps around exactly like two's-complement addition,
 * computed through the wider intermediate type so no undefined behavior is
 * invoked (unlike plain `operator+`, which overflows the storage type).
 *
 * @tparam T @see fixed_num
 * @param x the first addend.
 * @param y the second addend.
 * @return `(x + y) mod 2^N`, where `N` is the bit width of the storage type.
 */
template <typename T>
requires fixed_point<T>
EIRIN_MATH_FUNC_API T modwarp_add(T x, T y) noexcept
{
    using type = typename T::value_type;
    using intermediate_type = typename T::intermediate_type;
    type z;
    detail::__add_overflow<type, intermediate_type>(x.internal_value(), y.internal_value(), &z);
    return T::from_internal_value(z);
}

/**
 * @brief Subtract two fixed-point numbers with defined modulo-2^N wrapping.
 *
 * @tparam T @see fixed_num
 * @param x the minuend.
 * @param y the subtrahend.
 * @return `(x - y) mod 2^N`, where `N` is the bit width of the storage type.
 */
template <typename T>
requires fixed_point<T>
EIRIN_MATH_FUNC_API T modwarp_sub(T x, T y) noexcept
{
    using type = typename T::value_type;
    using intermediate_type = typename T::intermediate_type;
    type z;
    detail::__sub_overflow<type, intermediate_type>(x.internal_value(), y.internal_value(), &z);
    return T::from_internal_value(z);
}

/**
 * @brief Multiply two fixed-point numbers with defined modulo-2^N wrapping.
 *
 * @tparam T @see fixed_num
 * @param x the first factor.
 * @param y the second factor.
 * @return `(x * y) mod 2^N`, where `N` is the bit width of the storage type.
 */
template <typename T>
requires fixed_point<T>
EIRIN_MATH_FUNC_API T modwarp_mul(T x, T y) noexcept
{
    using type = typename T::value_type;
    using intermediate_type = typename T::intermediate_type;
    constexpr bool rounding = T::is_round_enable;
    constexpr auto fraction = T::precision;
    type z;
    detail::__mul_overflow<type, intermediate_type, rounding, fraction>(x.internal_value(), y.internal_value(), &z);
    return T::from_internal_value(z);
}

/**
 * @brief Divide two fixed-point numbers with defined modulo-2^N wrapping.
 *
 * The divisor must not be zero (division by zero stays undefined behavior,
 * as with `operator/`).
 *
 * @tparam T @see fixed_num
 * @param x the dividend.
 * @param y the divisor, must not be zero.
 * @return `(x / y) mod 2^N`, where `N` is the bit width of the storage type.
 */
template <typename T>
requires fixed_point<T>
EIRIN_MATH_FUNC_API T modwarp_div(T x, T y) noexcept
{
    using type = typename T::value_type;
    using intermediate_type = typename T::intermediate_type;
    constexpr bool rounding = T::is_round_enable;
    constexpr auto fraction = T::precision;
    type z;
    detail::__div_overflow<type, intermediate_type, rounding, fraction>(x.internal_value(), y.internal_value(), &z);
    return T::from_internal_value(z);
}
} // namespace eirin

#endif
