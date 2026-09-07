#ifndef EIRIN_MATH_NUMERIC_HPP
#define EIRIN_MATH_NUMERIC_HPP

#pragma once

#if defined(_MSC_VER) && !defined(__clang__) && _MSC_VER >= 1937 && (defined(_M_IX86) || defined(_M_X64))
#    include "intrin.h"
#endif

#include <type_traits>
#include <memory>
#include "detail/numeric_traits.hpp"
#include "detail/sat_arith.hpp"
#include "fixed.hpp"
#include "math.hpp"
#include "numbers.hpp"

namespace eirin
{
/**
 * @brief Add two fixed-point numbers, with saturation in case of overflow.
 * 
 * @tparam T fixed point type, @see fixed_num
 * @param x operand 1
 * @param y operand 2
 * @return requires min/max if x + y may overflow, otherwise returns x + y
 */
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

/**
 * @brief Subtract one fixed-point number from another, with saturation in case of overflow.
 * 
 * @tparam T fixed point type, @see fixed_num
 * @param x operand 1
 * @param y operand 2
 * @return requires min/max if x - y may overflow, otherwise returns x - y
 */
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

/**
 * @brief Multiply two fixed-point numbers, with saturation in case of overflow.
 * 
 * @tparam T fixed point type, @see fixed_num
 * @param x operand 1
 * @param y operand 2
 * @return requires min/max if x * y may overflow, otherwise returns x * y
 */
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

/**
 * @brief Divide one fixed-point number from another, with saturation in case of overflow.
 * 
 * @tparam T fixed point type, @see fixed_num
 * @param x operand 1
 * @param y operand 2
 * @return requires min/max if x / y may overflow, otherwise returns x / y
 */
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
 * same formulas as `fixed_num::from_fixed_num_value` (arithmetic in the wider
 * of the two intermediate types, so a source storage wider than the
 * destination intermediate is never truncated), before saturation is applied.
 *
 * @tparam T fixed point type, @see fixed_num
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
    using src_intermediate_type = typename U::intermediate_type;
    using dst_intermediate_type = typename T::intermediate_type;
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

    // Work in the wider of the two intermediates so that a source storage
    // type wider than the destination intermediate is never truncated before
    // scaling and clamping have happened.
    using work_type = std::conditional_t<
        (sizeof(dst_intermediate_type) >= sizeof(src_intermediate_type)),
        dst_intermediate_type,
        src_intermediate_type>;

    const work_type xw = static_cast<work_type>(xi);

    work_type scaled;
    if constexpr(dst_fraction >= src_fraction)
    {
        constexpr auto shift = dst_fraction - src_fraction;
        if constexpr(shift != 0)
        {
            // Clamp the source value to the destination range before scaling
            // up, so the shifted value cannot overflow the work type.
            constexpr work_type high = static_cast<work_type>(max_res >> shift);
            if constexpr(detail::is_signed_v<work_type> && detail::is_signed_v<res_type>)
            {
                constexpr work_type low = static_cast<work_type>(min_res >> shift);
                scaled = xw < low ? low : (xw > high ? high : xw);
            }
            else
            {
                scaled = xw > high ? high : xw;
            }
            scaled <<= shift;
        }
        else
        {
            scaled = xw;
        }
    }
    else
    {
        constexpr auto shift = src_fraction - dst_fraction;
        if constexpr(T::is_round_enable)
        {
            scaled = xw / (work_type(1) << shift) +
                     (xw / (work_type(1) << (shift - 1))) % 2;
        }
        else
        {
            scaled = xw / (work_type(1) << shift);
        }
    }

    constexpr work_type hi = static_cast<work_type>(max_res);
    if constexpr(detail::is_signed_v<work_type> && detail::is_signed_v<res_type>)
    {
        constexpr work_type lo = static_cast<work_type>(min_res);
        scaled = scaled < lo ? lo : (scaled > hi ? hi : scaled);
    }
    else
    {
        // A negative source has already returned `T(0)` above, and an unsigned
        // source cannot be negative, so only the upper bound is needed here.
        scaled = scaled > hi ? hi : scaled;
    }

    return T::from_internal_value(static_cast<res_type>(scaled));
}

/// Add two integers, with saturation in case of overflow.
template <typename T>
requires detail::__saturating_arithmetic_type<T>
EIRIN_MATH_FUNC_API T saturating_add(T x, T y) noexcept
{
#if defined(_MSC_VER) || defined(__clang__)
    // according to the discussion with contributors of MSVC-STL,
    // to keep the same style of potential implementation style,
    // we use this kind of code.
    // it will produce 2 `add` + 1 `cmovno` to prevent branches on both MSVC and Clang.
    // branchless will perform one extra `add` but not depend on data.
    if constexpr(detail::is_signed_v<T> && detail::has_make_unsigned_v<T>)
    {
        constexpr auto shifts = detail::__any_int_traits<T>::digits;
        T saturated = (static_cast<detail::make_unsigned_t<T>>(y) >> shifts) + detail::__any_int_traits<T>::max;
        T z;
        bool overflowed = detail::__integral_add_overflow(x, y, std::addressof(z));
        return !overflowed ? z : saturated;
    }
#endif
    T z;
    if(!detail::__integral_add_overflow(x, y, std::addressof(z)))
        return z;
    if constexpr(detail::is_unsigned_v<T>)
        return detail::__any_int_traits<T>::max;
    else if(x < 0)
        return detail::__any_int_traits<T>::min;
    else
        return detail::__any_int_traits<T>::max;
}

/// Subtract one integer from another, with saturation in case of overflow.
template <typename T>
requires detail::__saturating_arithmetic_type<T>
EIRIN_MATH_FUNC_API T saturating_sub(T x, T y) noexcept
{
#if defined(_MSC_VER) || defined(__clang__)
    if constexpr(detail::is_signed_v<T> && detail::has_make_unsigned_v<T>)
    {
        T saturated = x >= y ? detail::__any_int_traits<T>::max : detail::__any_int_traits<T>::min;
        T z;
        bool overflowed = detail::__integral_sub_overflow(x, y, std::addressof(z));
        return !overflowed ? z : saturated;
    }
#endif
    T z;
    if(!detail::__integral_sub_overflow(x, y, std::addressof(z)))
        return z;
    if constexpr(detail::is_unsigned_v<T>)
        return detail::__any_int_traits<T>::min;
    else if(x < 0)
        return detail::__any_int_traits<T>::min;
    else
        return detail::__any_int_traits<T>::max;
}

/// Multiply two integers, with saturation in case of overflow.
template <typename T>
requires detail::__saturating_arithmetic_type<T>
EIRIN_MATH_FUNC_API T saturating_mul(T x, T y) noexcept
{
#if defined(_MSC_VER) || defined(__clang__)
    if constexpr(detail::is_signed_v<T> && detail::has_make_unsigned_v<T>)
    {
        T saturated = (x < 0) ^ (y < 0) ? detail::__any_int_traits<T>::min : detail::__any_int_traits<T>::max;
        T z;
        bool overflowed = detail::__integral_mul_overflow(x, y, std::addressof(z));
        return !overflowed ? z : saturated;
    }
#endif
    T z;
    if(!detail::__integral_mul_overflow(x, y, std::addressof(z)))
        return z;
    if constexpr(detail::is_unsigned_v<T>)
        return detail::__any_int_traits<T>::max;
    else if((x < 0) ^ (y < 0))
        return detail::__any_int_traits<T>::min;
    else
        return detail::__any_int_traits<T>::max;
}

/// Divide one integer from another, with saturation in case of overflow.
template <typename T>
requires detail::__saturating_arithmetic_type<T>
EIRIN_MATH_FUNC_API T saturating_div(T x, T y) noexcept
{
    if constexpr(detail::is_unsigned_v<T>)
        return x / y;
    else if(x == detail::__any_int_traits<T>::min && y == -1)
        return detail::__any_int_traits<T>::max;
    else
        return x / y;
}

/// Type casting, with saturation in case of overflow.
template <typename Res, typename T>
requires detail::__saturating_arithmetic_type<T> && detail::__saturating_arithmetic_type<Res>
EIRIN_MATH_FUNC_API Res saturating_cast(T x) noexcept
{
    constexpr auto digits_res = detail::__any_int_traits<Res>::digits;
    constexpr auto digits_in = detail::__any_int_traits<T>::digits;
    constexpr Res max_res = detail::__any_int_traits<Res>::max;

    if constexpr(detail::is_signed_v<Res> && detail::is_signed_v<T>)
    {
        if constexpr(digits_res < digits_in)
        {
            constexpr Res min_res = detail::__any_int_traits<Res>::min;

            if(x < static_cast<T>(min_res))
                return min_res;
            else if(x > static_cast<T>(max_res))
                return max_res;
        }
    }
    else if constexpr(detail::is_signed_v<T>)
    {
        if(x < 0)
            return 0;
        else if(detail::make_unsigned_t<T>(x) > max_res)
            return max_res;
    }
    else
    {
        if(x > detail::make_unsigned_t<Res>(max_res))
            return max_res;
    }
    return static_cast<Res>(x);
}

/**
 * @brief Add two fixed-point numbers with defined modulo-2^N wrapping.
 *
 * On overflow the result wraps around exactly like two's-complement addition,
 * computed through the wider intermediate type so no undefined behavior is
 * invoked (unlike plain `operator+`, which overflows the storage type).
 *
 * @tparam T fixed point type, @see fixed_num
 * @param x operand 1
 * @param y operand 2
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
 * @tparam T fixed point type, @see fixed_num
 * @param x operand 1
 * @param y operand 2
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
 * @tparam T fixed point type, @see fixed_num
 * @param x operand 1
 * @param y operand 2
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
 * @tparam T fixed point type, @see fixed_num
 * @param x operand 1
 * @param y operand 2
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
