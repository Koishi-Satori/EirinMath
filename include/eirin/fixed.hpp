#ifndef EIRIN_MATH_FIXED_HPP
#define EIRIN_MATH_FIXED_HPP

#pragma once

#ifdef _MSC_VER
// C4244: conversion from 'type1' to 'type2', possible loss of data
// This is excepted, so disable it.
#    pragma warning(push)
#    pragma warning(disable : 4244)
#endif

#include <array>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ios>
#include <limits>
#include <istream>
#include <type_traits>
#include <concepts>
#include <iostream>
#include <iterator>
#include <bit>
#include "macro.hpp"
#include "detail/type_traits_impl.hpp"
#include "detail/int128.hpp"
#include "detail/numeric_traits.hpp"
#include "error.hpp"

namespace eirin
{
namespace detail
{
    template <typename T>
    consteval std::size_t __eval_max_bit_width() noexcept
    {
        if constexpr(std::numeric_limits<T>::is_specialized && std::numeric_limits<T>::radix == 2)
        {
            return std::numeric_limits<T>::digits;
        }
        else if(is_signed_v<T>)
        {
            return sizeof(T) * 8 - 1;
        }
        else
        {
            return sizeof(T) * 8;
        }
    }

    template <typename T>
    concept integral = is_integral_v<T>;
} // namespace detail

// if the type is unsigned type, then the fraction <= bit width, else <= bit width - 1.
template <typename Type, unsigned int fraction>
concept fixed_num_fraction = fraction > 0 && fraction <= static_cast<int>(detail::__eval_max_bit_width<Type>());

template <typename Type, typename IntermediateType>
concept fixed_num_size = sizeof(IntermediateType) > sizeof(Type);

template <typename Type, typename IntermediateType>
concept fixed_num_signness = detail::is_signed_v<IntermediateType> == detail::is_signed_v<Type>;

template <typename Type, typename IntermediateType, unsigned int fraction>
concept fixed_num_check = detail::is_integral_v<Type> && fixed_num_fraction<Type, fraction> && fixed_num_size<Type, IntermediateType> && fixed_num_signness<Type, IntermediateType>;

template <int scale>
concept fixed_format_check_scale = requires {
    scale == 2 || scale == 8 || scale == 10 || scale == 16;
};

template <typename Type, typename IntermediateType, unsigned int fraction, bool rounding>
requires fixed_num_check<Type, IntermediateType, fraction>
class fixed_num;

namespace detail
{
    // forward declaration: now we try to use hexfloat literals to represent some constants.
    template <typename CharT, typename T, typename I, unsigned int f, bool r>
    consteval fixed_num<T, I, f, r> eval_const(const CharT* str);
} // namespace detail

/**
     * @brief The fixed number.
     *
     * @tparam Type the store type.
     * @tparam IntermediateType used for calculation, must be larger than the store type.
     * @tparam fraction the fraction part of the fixed number.
     * @tparam rounding should round the fixed number, default false.
     */
template <typename Type, typename IntermediateType, unsigned int fraction, bool rounding = false>
requires fixed_num_check<Type, IntermediateType, fraction>
class fixed_num
{
public:
    static constexpr inline auto precision = fraction;
    static constexpr inline bool is_round_enable = rounding;
    // indicates the binary digits of the fixed number(internal representation)
    static constexpr inline auto digits = detail::__eval_max_bit_width<Type>();
    // indicates the binary digits of the integral part of the fixed number
    static constexpr inline auto digits_int = digits - fraction;

private:
    // just for raw value constructor call.
    struct raw_value_construct_tag
    {};

    static constexpr IntermediateType fraction_multiplier = IntermediateType(1) << fraction;

    // represent value 1.0, and for UQ0.n, this shoule always be 0.0.
    static constexpr Type raw_value_one = (precision == digits) ? static_cast<Type>(0) : static_cast<Type>(1) << fraction;

    static constexpr Type raw_value_max = detail::__any_int_traits<Type>::max;

    static constexpr Type raw_value_min = detail::__any_int_traits<Type>::min;

    constexpr inline fixed_num(Type val, raw_value_construct_tag) noexcept
        : m_value(val) {};

public:
    typedef Type value_type;
    typedef IntermediateType intermediate_type;

    inline fixed_num() noexcept = default;

    fixed_num(const fixed_num&) noexcept = default;

    /**
     * @brief Construct the fixed number from a integer value.
     *
     * @tparam T integral type.
     * @param val the input integer.
     * @return EIRIN_ALWAYS_INLINE constexpr the fixed number converted from the integer.
     */
    template <detail::integral T>
    EIRIN_ALWAYS_INLINE constexpr explicit fixed_num(T val) noexcept
        : m_value(static_cast<Type>(val) << fraction){};

    template <std::floating_point T>
    EIRIN_ALWAYS_INLINE constexpr explicit fixed_num(T val) noexcept
    {
        if constexpr(std::is_class_v<IntermediateType>)
        {
            // some class intermediate types (such as MSVC's std::_Signed128, boost::multiprecision
            // integers) have no double * IntermediateType operator, and the old
            // Type(val) * fraction_multiplier path truncated val to an integer first
            // (0.5 -> 0). This new implementation should solve both two issues?
            const T scaled = val * static_cast<T>(static_cast<Type>(fraction_multiplier));
            if constexpr(rounding)
                m_value = static_cast<Type>(scaled >= T{0} ? scaled + T{0.5} : scaled - T{0.5});
            else
                m_value = static_cast<Type>(scaled);
        }
        else
        {
            m_value = static_cast<Type>(
                rounding ? (val >= 0.0) ? (val * fraction_multiplier + T{0.5}) : (val * fraction_multiplier - T{0.5}) : (val * fraction_multiplier)
            );
        }
    };

    template <typename T, typename I, unsigned int f, bool r>
    EIRIN_ALWAYS_INLINE constexpr explicit fixed_num(fixed_num<T, I, f, r> fp) noexcept
        : m_value(from_fixed_num_value<f>(fp.internal_value()).internal_value())
    {}

    /**
     * @brief Get the inner value of the fixed number.
     * @note do not use unless you know what this function is and what are u doing.
     */
    EIRIN_ALWAYS_INLINE constexpr Type internal_value() const noexcept
    {
        return m_value;
    }

    /* constant defines */

    static constexpr fixed_num epsilon()
    {
        // 5/2^16 = 0.0000762939453125
        return detail::eval_const<char, Type, IntermediateType, fraction, rounding>("0x1.4p-14");
    }

    static constexpr fixed_num e()
    {
        // exact 61-bit dyadic approximation of e
        return detail::eval_const<char, Type, IntermediateType, fraction, rounding>("0x1.5bf0a8b145769534p+1");
    }

    static constexpr fixed_num pi()
    {
        // exact 61-bit dyadic approximation of pi
        return detail::eval_const<char, Type, IntermediateType, fraction, rounding>("0x1.921fb54442d18468p+1");
    }

    static constexpr fixed_num pi_2()
    {
        // exact 61-bit dyadic approximation of pi / 2
        return detail::eval_const<char, Type, IntermediateType, fraction, rounding>("0x1.921fb54442d18468p+0");
    }

    static constexpr fixed_num pi_4()
    {
        // exact 61-bit dyadic approximation of pi / 4
        return detail::eval_const<char, Type, IntermediateType, fraction, rounding>("0x1.921fb54442d18468p-1");
    }

    static constexpr fixed_num double_pi()
    {
        // exact 61-bit dyadic approximation of 2 * pi
        return detail::eval_const<char, Type, IntermediateType, fraction, rounding>("0x1.921fb54442d18468p+2");
    }

    /**
     * @brief Get the epsilon used by the nearly_* comparison functions.
     *
     * Defaults to 5/2^16 = 0.0000762939453125. Specialize
     * `eirin::detail::nearly_compare_epsilon<fixed_num>` to customize the
     * epsilon for a fixed point type.
     *
     * @return the comparison epsilon of this fixed point type.
     */
    static constexpr fixed_num nearly_compare_epsilon()
    {
        using custom_epsilon = detail::nearly_compare_epsilon<fixed_num>;
        if constexpr(custom_epsilon::is_specialized)
            return custom_epsilon::value;
        // 5/2^16 = 0.0000762939453125
        return detail::eval_const<char, Type, IntermediateType, fraction, rounding>("0x1.4p-14");
    }

    EIRIN_ALWAYS_INLINE static constexpr Type signbit_mask() noexcept
    {
        // for signed type, it should be 1 << (sizeof(Type) * 8 - 1)
        // for unsigned type, it should be 0.
        if constexpr(detail::is_unsigned_v<Type>)
            return 0;
        else
            return static_cast<Type>(1) << (sizeof(Type) * 8 - 1);
    }

    EIRIN_ALWAYS_INLINE friend constexpr bool signbit(const fixed_num& f) noexcept
    {
        if constexpr(detail::is_signed_v<Type>)
            return f.m_value & signbit_mask();
        else // unsigned value
            return false;
    }

    EIRIN_ALWAYS_INLINE constexpr Type raw_integral_part() const noexcept
    {
        Type result = m_value;
        result &= ~signbit_mask(); // Remove signbit
        result >>= fraction; // Drop fractional part
        return result;
    }

    EIRIN_ALWAYS_INLINE constexpr Type integral_part() const noexcept
    {
        Type result = m_value;
        if(signbit(*this))
        {
            result = ~result;
            result += static_cast<Type>(1) << fraction;
        }
        result >>= fraction; // Drop fractional part

        return result;
    }

    EIRIN_ALWAYS_INLINE constexpr Type fractional_part() const noexcept
    {
        return m_value % (static_cast<Type>(1) << fraction);
    }

    /**
     * @brief Evaluate the bit width of the fixed number, similar to std::bit_width, but support any fixed-point number.
     * 
     * @tparam IgnoreSignBit should ignore the sign bit, default true. If false, the bit width will be the minimum bit width for two's complement representation.
     * @return lzcnt of the fixed-point number.
     * @note For a custom fixed-point storage type, provide a specialization of
     *       `eirin::int_sequence_generator_traits`: it should produce a
     *       begin/end iterator that yields every integral "word" of the
     *       storage type, so the bit width can be computed faster by applying
     *       `std::bit_width` to each word. Without such a specialization, the
     *       algorithm falls back to a slower method.
     */
    template <bool IgnoreSignBit = true>
    EIRIN_ALWAYS_INLINE constexpr std::size_t bit_width() const noexcept
    {
        using u_type = detail::make_unsigned_t<Type>;
        if constexpr(!IgnoreSignBit)
        {
            // actual bit width with sign bit(minimum bit width for two's complement representation)
            // for positive number or zero, res = bit_width(x) + 1 (x=0 -> 1)
            // for negative number, set y = -x
            //                      res = bit_width(y), if y is power of 2
            //                      res = bit_width(y) + 1, else cases.
            if(m_value >= 0)
            {
                const u_type u_val = static_cast<u_type>(m_value);
                return detail::bit_width(u_val) + 1;
            }
            else
            {
                const u_type u_abs = static_cast<u_type>(-(m_value + 1)) + 1;
                const std::size_t w = detail::bit_width(u_abs);
                if(std::has_single_bit(u_abs)) // power of 2
                    return w;
                return w + 1;
            }
        }

        // return the actual bit width of absolute value of m_value, ignore the sign bit.
        const u_type u = static_cast<u_type>(m_value);
        const u_type mask = m_value < 0 ? static_cast<u_type>(~u_type(0)) : static_cast<u_type>(0);
        u_type u_value = (u ^ mask) - mask;

        return detail::bit_width(u_value);
    }

    /* operator override functions */

    fixed_num& operator=(const fixed_num&) noexcept = default;

    template <detail::integral T>
    constexpr inline explicit operator T() const noexcept
    {
        return static_cast<T>(m_value >> fraction);
    }

    template <std::floating_point T>
    constexpr inline explicit operator T() const noexcept
    {
        // MSVC might warn about precision loss here, but it's expected.
        // MSVC's 128-bit integer-class type has no conversion to floating point,
        // so split into integral and fractional parts in value_type arithmetic.
        const value_type quot = static_cast<value_type>(m_value / fraction_multiplier);
        const value_type rem = static_cast<value_type>(m_value % fraction_multiplier);
        T divisor = T(1);
        for(unsigned int i = 0; i < fraction; ++i)
        {
            divisor *= T(2);
        }
        return static_cast<T>(quot) + static_cast<T>(rem) / divisor;
    }

    constexpr inline fixed_num operator+(const fixed_num& other) const noexcept
    {
        return fixed_num(m_value + other.m_value, raw_value_construct_tag{});
    }

    constexpr inline fixed_num& operator+=(const fixed_num& other) noexcept
    {
        m_value += other.m_value;
        return *this;
    }

    constexpr inline fixed_num& operator+=(const detail::integral auto& val) noexcept
    {
        m_value += static_cast<Type>(val) << fraction;
        return *this;
    }

    constexpr inline fixed_num operator-(const fixed_num& other) const noexcept
    {
        return fixed_num(m_value - other.m_value, raw_value_construct_tag{});
    }

    constexpr inline fixed_num& operator-=(const fixed_num& other) noexcept
    {
        m_value -= other.m_value;
        return *this;
    }

    constexpr inline fixed_num& operator-=(const detail::integral auto& val) noexcept
    {
        m_value -= static_cast<Type>(val) << fraction;
        return *this;
    }

    constexpr inline fixed_num operator*(const fixed_num& other) const noexcept
    {
        if constexpr(rounding)
        {
            auto _value = (static_cast<IntermediateType>(m_value) * other.m_value) / (fraction_multiplier / 2);
            _value = (_value + (_value % 2)) >> 1;
            return fixed_num(static_cast<Type>(_value), raw_value_construct_tag{});
        }
        else
        {
            return fixed_num(static_cast<Type>((static_cast<IntermediateType>(m_value) * other.m_value) >> fraction), raw_value_construct_tag{});
        }
    }

    constexpr inline fixed_num& operator*=(const fixed_num& other) noexcept
    {
        if constexpr(rounding)
        {
            auto _value = (static_cast<IntermediateType>(m_value) * other.m_value) / (fraction_multiplier / 2);
            _value = (_value + (_value % 2)) >> 1;
            m_value = static_cast<Type>(_value);
        }
        else
        {
            auto _value = (static_cast<IntermediateType>(m_value) * other.m_value) >> fraction;
            m_value = static_cast<Type>(_value);
        }
        return *this;
    }

    constexpr inline fixed_num& operator*=(const detail::integral auto& val) noexcept
    {
        m_value *= val;
        return *this;
    }

    constexpr inline fixed_num operator/(const fixed_num& other) const noexcept
    {
        if constexpr(rounding)
        {
            auto _value = ((static_cast<IntermediateType>(m_value) << fraction) * 2) / other.m_value;
            _value = (_value + (_value % 2)) >> 1;
            return fixed_num(static_cast<Type>(_value), raw_value_construct_tag{});
        }
        else
        {
            return fixed_num(static_cast<Type>((static_cast<IntermediateType>(m_value) << fraction) / other.m_value), raw_value_construct_tag{});
        }
    }

    constexpr inline fixed_num& operator/=(const fixed_num& other) noexcept
    {
        if constexpr(rounding)
        {
            auto _value = ((static_cast<IntermediateType>(m_value) << fraction) * 2) / other.m_value;
            _value = (_value + (_value % 2)) >> 1;
            m_value = static_cast<Type>(_value);
        }
        else
        {
            m_value = static_cast<Type>((static_cast<IntermediateType>(m_value) << fraction) / other.m_value);
        }
        return *this;
    }

    constexpr inline fixed_num& operator/=(const detail::integral auto& val) noexcept
    {
        m_value /= val;
        return *this;
    }

    constexpr inline fixed_num operator%(const fixed_num& other) const noexcept
    {
        return fixed_num(m_value % other.m_value, raw_value_construct_tag{});
    }

    constexpr inline fixed_num& operator%=(const fixed_num& other) noexcept
    {
        m_value %= other.m_value;
        return *this;
    }

    // BEGIN bitwise
    // Fixed point bitwise operation has no math meaning, but still need it.
    // Just operate the internal representation can be good.

    constexpr inline fixed_num operator^(const fixed_num& other) const noexcept
    {
        return fixed_num(m_value ^ other.m_value, raw_value_construct_tag{});
    }

    constexpr inline fixed_num& operator^=(const fixed_num& other) noexcept
    {
        m_value ^= other.m_value;
        return *this;
    }

    constexpr inline fixed_num operator&(const fixed_num& other) const noexcept
    {
        return fixed_num(m_value & other.m_value, raw_value_construct_tag{});
    }

    constexpr inline fixed_num& operator&=(const fixed_num& other) noexcept
    {
        m_value &= other.m_value;
        return *this;
    }

    constexpr inline fixed_num operator|(const fixed_num& other) const noexcept
    {
        return fixed_num(m_value | other.m_value, raw_value_construct_tag{});
    }

    constexpr inline fixed_num& operator|=(const fixed_num& other) noexcept
    {
        m_value |= other.m_value;
        return *this;
    }

    constexpr inline fixed_num operator~() noexcept
    {
        return fixed_num(~m_value, raw_value_construct_tag{});
    }

    /**
     * @brief Left Shifting the internal representation of the fixed point with val bits.
     *        To be noticed that, this operator does not checks overflow and range of n bits.
     * @see shl
     *
     * @param val n bits to left shift.
     * @return constexpr fixed_num compute result.
     */
    constexpr inline fixed_num operator<<(const detail::integral auto& val) const noexcept
    {
        return fixed_num(m_value << val, raw_value_construct_tag{});
    }

    /**
     * @brief Left Shifting the internal representation of the fixed point with val bits.
     *        To be noticed that, this operator does not checks overflow and range of n bits.
     * @see shl_by
     *
     * @param val n bits to left shift.
     * @return constexpr fixed_num compute result.
     */
    constexpr inline fixed_num& operator<<=(const detail::integral auto& val) noexcept
    {
        m_value <<= val;
        return *this;
    }

    /**
     * @brief Right Shifting the internal representation of the fixed point with val bits.
     *        To be noticed that, this operator does not checks overflow and range of n bits.
     * @see shr
     *
     * @param val n bits to right shift.
     * @return constexpr fixed_num compute result.
     */
    constexpr inline fixed_num operator>>(const detail::integral auto& val) const noexcept
    {
        return fixed_num(m_value >> val, raw_value_construct_tag{});
    }

    /**
     * @brief Right Shifting the internal representation of the fixed point with val bits.
     *        To be noticed that, this operator does not checks overflow and range of n bits.
     * @see shr_by
     *
     * @param val n bits to right shift.
     * @return constexpr fixed_num compute result.
     */
    constexpr inline fixed_num& operator>>=(const detail::integral auto& val) noexcept
    {
        m_value >>= val;
        return *this;
    }

    // END bitwise

    constexpr inline fixed_num operator-() const noexcept
    {
        return fixed_num(-m_value, raw_value_construct_tag{});
    }

    constexpr inline fixed_num& operator++() noexcept
    {
        if constexpr(detail::is_signed_v<Type> && digits == fraction)
        {
#if EIRIN_FIXED_NUM_SELF_INC_OVERFLOW == EIRIN_OVERFLOW_SAT
            if(m_value < 0)
                m_value = static_cast<Type>(static_cast<IntermediateType>(m_value) + fraction_multiplier);
            else
                m_value = raw_value_max;
#elif EIRIN_FIXED_NUM_SELF_INC_OVERFLOW == EIRIN_OVERFLOW_MODWRAP
            m_value = static_cast<Type>(static_cast<IntermediateType>(m_value) + fraction_multiplier); // mod 2^N
#else
            // do nothing here, just no-op
            return *this;
#endif
        }
        m_value += raw_value_one;
        return *this;
    }

    constexpr inline fixed_num operator++(int) noexcept
    {
        fixed_num temp = *this;
        if constexpr(detail::is_signed_v<Type> && digits == fraction)
        {
#if EIRIN_FIXED_NUM_SELF_INC_OVERFLOW == EIRIN_OVERFLOW_SAT
            if(m_value < 0)
                m_value = static_cast<Type>(static_cast<IntermediateType>(m_value) + fraction_multiplier);
            else
                m_value = raw_value_max;
#elif EIRIN_FIXED_NUM_SELF_INC_OVERFLOW == EIRIN_OVERFLOW_MODWRAP
            m_value = static_cast<Type>(static_cast<IntermediateType>(m_value) + fraction_multiplier); // mod 2^N
#else
            // do nothing here, just no-op
            return *this;
#endif
        }
        m_value += raw_value_one;
        return temp;
    }

    constexpr inline fixed_num& operator--() noexcept
    {
        if constexpr(detail::is_signed_v<Type> && digits == fraction)
        {
#if EIRIN_FIXED_NUM_SELF_INC_OVERFLOW == EIRIN_OVERFLOW_SAT
            if(m_value < 0)
                m_value = static_cast<Type>(static_cast<IntermediateType>(m_value) - fraction_multiplier);
            else
                m_value = raw_value_min;
#elif EIRIN_FIXED_NUM_SELF_INC_OVERFLOW == EIRIN_OVERFLOW_MODWRAP
            m_value = static_cast<Type>(static_cast<IntermediateType>(m_value) - fraction_multiplier); // mod 2^N
#else
            // do nothing here, just no-op
            return *this;
#endif
        }
        m_value -= raw_value_one;
        return *this;
    }

    constexpr inline fixed_num operator--(int) noexcept
    {
        fixed_num temp = *this;
        if constexpr(detail::is_signed_v<Type> && digits == fraction)
        {
#if EIRIN_FIXED_NUM_SELF_INC_OVERFLOW == EIRIN_OVERFLOW_SAT
            if(m_value < 0)
                m_value = static_cast<Type>(static_cast<IntermediateType>(m_value) - fraction_multiplier);
            else
                m_value = raw_value_min
#elif EIRIN_FIXED_NUM_SELF_INC_OVERFLOW == EIRIN_OVERFLOW_MODWRAP
            m_value = static_cast<Type>(static_cast<IntermediateType>(m_value) - fraction_multiplier); // mod 2^N
#else
            // do nothing here, just no-op
            return *this;
#endif
        }
        m_value -= raw_value_one;
        return temp;
    }

    constexpr friend bool operator==(const fixed_num& lhs, const fixed_num& rhs) noexcept = default;

    constexpr std::strong_ordering operator<=>(const fixed_num& rhs) const noexcept
    {
        return m_value <=> rhs.m_value;
    }

    constexpr inline bool operator&&(const fixed_num& other) const noexcept
    {
        return m_value && other.m_value;
    }

    constexpr inline bool operator||(const fixed_num& other) const noexcept
    {
        return m_value || other.m_value;
    }

    /* nearly compare */

    constexpr inline bool nearly_eq(const fixed_num& other) const noexcept
    {
        auto div = m_value - other.m_value;
        return div >= -nearly_compare_epsilon().m_value && div <= nearly_compare_epsilon().m_value;
    }

    constexpr inline bool nearly_ne(const fixed_num& other) const noexcept
    {
        auto div = m_value - other.m_value;
        return div < -nearly_compare_epsilon().m_value || div > nearly_compare_epsilon().m_value;
    }

    constexpr inline bool nearly_gt(const fixed_num& other) const noexcept
    {
        auto div = m_value - other.m_value;
        return div > nearly_compare_epsilon().m_value;
    }

    constexpr inline bool nearly_lt(const fixed_num& other) const noexcept
    {
        auto div = m_value - other.m_value;
        return div < -nearly_compare_epsilon().m_value;
    }

    constexpr inline bool nearly_gt_eq(const fixed_num& other) const noexcept
    {
        return !(nearly_lt(other));
    }

    constexpr inline bool nearly_lt_eq(const fixed_num& other) const noexcept
    {
        return !(nearly_gt(other));
    }

    /* convert functions */

    template <unsigned int _fraction, typename T, typename std::enable_if_t<(_fraction > fraction), T*> = nullptr>
    EIRIN_ALWAYS_INLINE static constexpr fixed_num from_fixed_num_value(T inner_value) noexcept
    {
        return rounding ?
                   fixed_num(static_cast<Type>(inner_value / (T(1) << (_fraction - fraction)) + (inner_value / (T(1) << (_fraction - fraction - 1)) % 2)), raw_value_construct_tag{}) :
                   fixed_num(static_cast<Type>(inner_value / (T(1) << (_fraction - fraction))), raw_value_construct_tag{});
    }

    template <unsigned int _fraction, typename T, typename std::enable_if_t<(_fraction <= fraction), T*> = nullptr>
    EIRIN_ALWAYS_INLINE static constexpr fixed_num from_fixed_num_value(T inner_value) noexcept
    {
        return fixed_num(static_cast<Type>(inner_value * (T(1) << (fraction - _fraction))), raw_value_construct_tag{});
    }

    static constexpr fixed_num from_internal_value(Type internal_value) noexcept
    {
        return fixed_num(internal_value, raw_value_construct_tag{});
    }

    template <typename OutputIter>
    static constexpr OutputIter copy_as_chars_to(
        const fixed_num& val,
        OutputIter out,
        int base = 10,
        bool uppercase = false
    )
    {
        const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
        Type divisor = static_cast<Type>(1) << fraction;
        auto put_char = [&out](const char c)
        {
            *out = c;
            ++out;
        };

        auto value = val.m_value;
        Type int_part;
        if(value == val.signbit_mask())
        {
            put_char('-');
            int_part = ~(value >> fraction) + 1;
            value = 0;
        }
        else
        {
            if(value < 0)
            {
                put_char('-');
                value = -value;
            }
            int_part = value >> fraction;
        }
        value %= divisor;
        std::array<char, 514> buffer;
        auto iter = buffer.begin();

        if(int_part == 0)
        {
            put_char('0');
        }
        else
        {
            while(int_part > 0)
            {
                auto digit = int_part % base;
                *iter++ = digits[digit];
                int_part /= base;
                if(iter == buffer.end())
                {
                    while(iter-- != buffer.begin())
                        put_char(*iter);
                    iter = buffer.begin();
                }
            }
        }
        while(iter-- != buffer.begin())
            put_char(*iter);

        if(value != 0)
        {
            put_char('.');
            for(unsigned int i = 0; i < fraction; ++i)
            {
                if(value == 0)
                {
                    break;
                }
                if(divisor % base == 0)
                {
                    divisor /= base;
                }
                else
                {
                    value *= base;
                }

                auto digit = (value / divisor) % base;
                put_char(digits[digit]);
                value %= divisor;
            }
        }

        return out;
    }

    EIRIN_ALWAYS_INLINE constexpr fixed_num divide(const detail::integral auto& val) const
    {
        if(val == 0) [[unlikely]]
            EIRIN_THROW_EXCEPTION(divide_by_zero);

        return fixed_num(m_value / val, raw_value_construct_tag{});
    }

    EIRIN_ALWAYS_INLINE constexpr fixed_num divide(const fixed_num& other) const
    {
        if(other.m_value == 0) [[unlikely]]
            EIRIN_THROW_EXCEPTION(divide_by_zero);

        if constexpr(rounding)
        {
            auto _value = ((static_cast<IntermediateType>(m_value) << fraction) * 2) / other.m_value;
            return fixed_num(static_cast<Type>(_value + (_value % 2)), raw_value_construct_tag{});
        }
        else
        {
            return fixed_num(static_cast<Type>((static_cast<IntermediateType>(m_value) << fraction) / other.m_value), raw_value_construct_tag{});
        }
    }

    EIRIN_ALWAYS_INLINE constexpr fixed_num& divide_by(const detail::integral auto& val)
    {
        if(val == 0) [[unlikely]]
            EIRIN_THROW_EXCEPTION(divide_by_zero);

        m_value /= val;
        return *this;
    }

    EIRIN_ALWAYS_INLINE constexpr fixed_num& divide_by(const fixed_num& other)
    {
        if(other.m_value == 0) [[unlikely]]
            EIRIN_THROW_EXCEPTION(divide_by_zero);

        if constexpr(rounding)
        {
            auto _value = ((static_cast<IntermediateType>(m_value) << fraction) * 2) / other.m_value;
            m_value = static_cast<Type>(_value + (_value % 2));
        }
        else
        {
            m_value = static_cast<Type>((static_cast<IntermediateType>(m_value) << fraction) / other.m_value);
        }
        return *this;
    }

    EIRIN_ALWAYS_INLINE constexpr fixed_num shl(const detail::integral auto& val) const
    {
        if(val < 0) [[unlikely]]
            EIRIN_THROW_EXCEPTION(std::range_error, "n bits to left shift should be greater than or equal to 0.");
        auto n_bits = static_cast<decltype(val)>(bit_width());
        constexpr auto max_bits = static_cast<decltype(val)>(sizeof(Type) * 8 - 1);
        if(val >= max_bits)
            EIRIN_THROW_EXCEPTION(std::overflow_error, "left shift bits larger than bit width is undefined.");
        if(n_bits != 0 && n_bits + val > max_bits)
            EIRIN_THROW_EXCEPTION(std::overflow_error, "will cause overflow after left shifts n bits.");
        return fixed_num(m_value << val, raw_value_construct_tag{});
    }

    EIRIN_ALWAYS_INLINE constexpr fixed_num& shl_by(const detail::integral auto& val)
    {
        if(val < 0) [[unlikely]]
            EIRIN_THROW_EXCEPTION(std::range_error, "n bits to left shift should be greater than or equal to 0.");
        auto n_bits = static_cast<decltype(val)>(bit_width());
        constexpr auto max_bits = static_cast<decltype(val)>(sizeof(Type) * 8 - 1);
        if(val >= max_bits)
            EIRIN_THROW_EXCEPTION(std::overflow_error, "left shift bits larger than bit width is undefined.");
        if(n_bits != 0 && n_bits + val > max_bits)
            EIRIN_THROW_EXCEPTION(std::overflow_error, "will cause overflow after left shifts n bits.");
        m_value <<= val;
        return *this;
    }

    EIRIN_ALWAYS_INLINE constexpr fixed_num shr(const detail::integral auto& val) const
    {
        if(val < 0) [[unlikely]]
            EIRIN_THROW_EXCEPTION(std::range_error, "n bits to right shift should be greater than or equal to 0.");
        auto n_bits = static_cast<decltype(val)>(bit_width());
        constexpr auto max_bits = static_cast<decltype(val)>(sizeof(Type) * 8 - 1);
        if(val >= max_bits)
            EIRIN_THROW_EXCEPTION(std::overflow_error, "right shift bits larger than bit width is undefined.");
        return fixed_num(m_value >> val, raw_value_construct_tag{});
    }

    EIRIN_ALWAYS_INLINE constexpr fixed_num& shr_by(const detail::integral auto& val)
    {
        if(val < 0) [[unlikely]]
            EIRIN_THROW_EXCEPTION(std::range_error, "n bits to right shift should be greater than or equal to 0.");
        auto n_bits = static_cast<decltype(val)>(bit_width());
        constexpr auto max_bits = static_cast<decltype(val)>(sizeof(Type) * 8 - 1);
        if(val >= max_bits)
            EIRIN_THROW_EXCEPTION(std::overflow_error, "right shift bits larger than bit width is undefined.");
        m_value >>= val;
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const fixed_num& fp)
    {
        auto iter = std::ostream_iterator<char>(os);
        auto flags = os.flags();
        int base = (flags & std::ios_base::hex) ? 16 : ((flags & std::ios_base::dec) ? 10 : ((flags & std::ios_base::oct) ? 8 : 2));
        bool uppercase = (flags & std::ios_base::uppercase) != 0;
        fixed_num::copy_as_chars_to(fp, iter, base, uppercase);
        os.flush();
        return os;
    }

    /**
     * @brief internal value, just for NTTP, do not use it.
     *
     */
    Type m_value;
};

namespace detail
{
    template <typename T>
    struct is_fixed_point : std::false_type
    {};

    template <typename T, typename I, unsigned int f, bool r>
    struct is_fixed_point<fixed_num<T, I, f, r>> : public std::true_type
    {};

    // round-half-away signed right shift, used by hexfloat parsing.
    template <typename V>
    constexpr V hexfloat_rshift_round(V v, unsigned int sh) noexcept
    {
        if(sh == 0)
            return v;
        const V half = static_cast<V>(1) << (sh - 1);
        if(v >= 0)
            return (v + half) >> sh;
        return -(((-v) + half) >> sh);
    }

    // parse a C99-style hexfloat literal "0x<hex>[.<hex>][p<exp>]" (no leading
    // sign) into the raw fixed-point value.
    template <typename I>
    constexpr bool parse_hexfloat(
        const char* str,
        size_t len,
        unsigned int fraction,
        I& raw,
        bool round_half_away = true
    ) noexcept
    {
        size_t pos = 0;
        if(pos + 1 >= len || str[pos] != '0' || (str[pos + 1] != 'x' && str[pos + 1] != 'X'))
            return false;
        pos += 2;

        // Effective value bits of the intermediate. sizeof() is not reliable
        // for class types: boost::multiprecision::int256_t has padding, so
        // sizeof * 8 overestimates and a 1 << (bits-4) cap would wrap to 0.
        constexpr unsigned int I_bits = []() constexpr -> unsigned int
        {
            if constexpr(std::numeric_limits<I>::is_specialized && std::numeric_limits<I>::radix == 2)
                return static_cast<unsigned int>(std::numeric_limits<I>::digits);
            else
                return static_cast<unsigned int>(sizeof(I) * 8);
        }();
        constexpr unsigned int max_mant_bits = I_bits > 2 ? I_bits - 2 : 1;

        auto hex_val = [](char c) -> int
        {
            if('0' <= c && c <= '9')
                return c - '0';
            if('a' <= c && c <= 'f')
                return c - 'a' + 10;
            if('A' <= c && c <= 'F')
                return c - 'A' + 10;
            return -1;
        };

        I mant = 0;
        bool any_digit = false;
        bool truncated = false;
        unsigned int dropped_nibbles = 0;
        unsigned int frac_hex = 0;
        auto append_hex_digit = [&](int d)
        {
            if(mant != 0 && mant > (static_cast<I>(1) << (max_mant_bits - 4)))
            {
                truncated |= (mant & 0xF) != 0;
                mant >>= 4;
                ++dropped_nibbles;
            }
            mant = mant * 16 + d;
        };

        while(pos < len)
        {
            const int d = hex_val(str[pos]);
            if(d < 0)
                break;
            append_hex_digit(d);
            ++pos;
            any_digit = true;
        }
        if(pos < len && str[pos] == '.')
        {
            ++pos;
            while(pos < len)
            {
                const int d = hex_val(str[pos]);
                if(d < 0)
                    break;
                append_hex_digit(d);
                ++frac_hex;
                ++pos;
                any_digit = true;
            }
        }
        if(!any_digit)
            return false;

        int exp2 = 0;
        if(pos < len && (str[pos] == 'p' || str[pos] == 'P'))
        {
            ++pos;
            bool exp_negative = false;
            if(pos < len && (str[pos] == '+' || str[pos] == '-'))
            {
                exp_negative = str[pos] == '-';
                ++pos;
            }
            if(pos >= len || !('0' <= str[pos] && str[pos] <= '9'))
                return false;
            while(pos < len && '0' <= str[pos] && str[pos] <= '9')
            {
                exp2 = exp2 * 10 + (str[pos] - '0');
                if(exp2 > 4096)
                    return false;
                ++pos;
            }
            if(exp_negative)
                exp2 = -exp2;
        }

        // raw = mant * 2^(fraction + exp2 - 4 * frac_hex + 4 * dropped_nibbles)
        const long long sh = static_cast<long long>(fraction) + exp2 -
                             4LL * static_cast<long long>(frac_hex) +
                             4LL * static_cast<long long>(dropped_nibbles);
        if(sh >= 0)
        {
            if(sh > static_cast<long long>(I_bits))
                return false;
            raw = mant << static_cast<unsigned int>(sh);
        }
        else
        {
            const unsigned int rsh = static_cast<unsigned int>(-sh);
            if(rsh > I_bits)
                return false;
            raw = round_half_away ? hexfloat_rshift_round(mant, rsh) : static_cast<I>(mant >> rsh);
        }
        (void)truncated;
        return pos == len;
    }

    // parse hexfloat literals format string and store as const.
    template <typename I, unsigned int fraction>
    constexpr I eval_dyadic(const char* str)
    {
        std::size_t len = 0;
        while(str[len] != '\0')
            ++len;
        std::size_t pos = 0;
        bool negative = false;
        if(pos < len && (str[pos] == '-' || str[pos] == '+'))
        {
            negative = str[pos] == '-';
            ++pos;
        }
        I raw = 0;
        if(!parse_hexfloat(str + pos, len - pos, fraction, raw))
            return I(0);
        return negative ? -raw : raw;
    }

    template <typename CharT, typename T, typename I, unsigned int f, bool r>
    constexpr bool parse(const CharT* str, size_t len, fixed_num<T, I, f, r>& fp) noexcept
    {
        using fixed = fixed_num<T, I, f, r>;
        size_t pos = 0;
        bool negative = false;

        auto check_ch = [](char ch) -> bool
        {
            return '0' <= ch && ch <= '9';
        };

        auto next = [&]() -> CharT
        {
            return str[pos++];
        };

        if(pos < len && str[pos] == '-')
        {
            negative = true;
            next();
        }

        I fixed_value;
        if(pos + 1 < len && str[pos] == '0' && (str[pos + 1] == 'x' || str[pos + 1] == 'X'))
        {
            // we guess this is hexfloat literals format string.
            if(!parse_hexfloat(str + pos, len - pos, fixed::precision, fixed_value, fixed::is_round_enable))
                return false;
        }
        else
        {
            I int_part = I(0), dec_part = I(0);
            // parse the integer part.
            while(pos < len && str[pos] != '.')
            {
                if(!check_ch(str[pos]))
                    return false;
                int_part = int_part * 10 + (next() - '0');
            }

            // parse the decimal part.
            if(pos < len && str[pos] == '.')
            {
                ++pos;
                constexpr auto max_fraction = (static_cast<I>(1) << fixed::precision) - 1;
                I divisor = I(1);
                while(pos < len)
                {
                    if(!check_ch(str[pos]))
                        return false;
                    if(dec_part > max_fraction / 10)
                    {
                        break;
                    }
                    auto digit = next() - '0';
                    dec_part = dec_part * 10 + digit;
                    divisor *= 10;
                }
                fixed_value = (int_part << fixed::precision) + (dec_part << fixed::precision) / divisor;
            }
            else
            {
                fixed_value = int_part << fixed::precision;
            }
        }

        // check overflow
        if(fixed_value > static_cast<I>(detail::__any_int_traits<T>::max) || fixed_value < static_cast<I>(detail::__any_int_traits<T>::min))
        {
            return false;
        }

        fp = fixed::from_internal_value(static_cast<T>(fixed_value));
        if(negative)
            fp = -fp;
        return true;
    }

    template <typename CharT, typename T, typename I, unsigned int f, bool r>
    consteval fixed_num<T, I, f, r> eval_const(const CharT* str)
    {
        // eval string size in compile time.
        // must ensure the string is null terminated, and only has ascii chars.
        std::size_t len = 0;
        while(str[len] != '\0')
            ++len;

        fixed_num<T, I, f, r> fp;
        if(!parse(str, len, fp))
        {
            return fixed_num<T, I, f, r>();
        }
        return fp;
    }

    template <typename Fixed>
    consteval std::size_t eval_integral_part_max_digits10() noexcept
    {
        using T = typename Fixed::value_type;
        using U = detail::make_unsigned_t<T>;
        // calculate the max integral part digits10 of the max value of Fixed.
        // we can use length = floor((bit_width - 1) * log10(2)) to calculate the max digits10 of the integral part.
        constexpr std::size_t bit_width = sizeof(T) * 8;
        constexpr std::size_t precision = Fixed::precision;
        constexpr bool is_signed = detail::is_signed_v<T>;
        // we need to calculate the max value if using std::numeric_limits::max
        // due to the fixed has precision bits, the max value of the integral part is not equal to std::numeric_limits::max.
        constexpr U max_unsigned = []() constexpr
        {
            if constexpr(std::numeric_limits<T>::is_specialized)
            {
                return static_cast<U>(std::numeric_limits<T>::max());
            }
            else if constexpr(is_signed)
            {
                // 2^(bit_width-1) - 1, computed in unsigned arithmetic.
                return (U(1) << (bit_width - 1)) - U(1);
            }
            else
            {
                // 2^bit_width - 1.
                return ~U(0);
            }
        }();
        constexpr U max_int_part = max_unsigned >> precision;
        constexpr T max_value = static_cast<T>(max_int_part);
        std::size_t digits = 0;
        T pow10 = 1;
        while(pow10 <= max_value)
        {
            pow10 *= 10;
            ++digits;
        }
        return digits;
    }
} // namespace detail

template <typename T>
inline constexpr bool is_fixed_point_v = detail::is_fixed_point<std::remove_cv_t<T>>::value;

template <typename T>
concept fixed_point = detail::is_fixed_point<std::remove_cv_t<T>>::value;

/**
 * @brief Predefined fixed32 type, with 16 bits fraction, 15 bits intergal, and 1 bit sign.
 *        This type uses int32_t as store type, and int64_t as intermediate type.
 *
 */
using fixed32 = fixed_num<int32_t, int64_t, 16, false>;
#ifdef EIRIN_MATH_HAS_INT128
/**
 * @brief Predefined fixed64 type, with 32 bits fraction, 16 bits intergal, and 1 bit sign.
 *        This type uses int64_t as store type, and 128-bits intergal as intermediate type.
 *
 */
using fixed64 = fixed_num<int64_t, detail::int128_t, 32, false>;
#endif

inline namespace literals
{
    constexpr inline fixed32 operator""_f32(unsigned long long val)
    {
        return fixed32(val);
    }

    constexpr inline fixed32 operator""_f32(long double val)
    {
        return fixed32(val);
    }

    constexpr inline fixed32 operator""_f32(const char* str, size_t len)
    {
        fixed32 fp;
        detail::parse(str, len, fp);
        return fp;
    }

    template <char... chars>
    constexpr inline fixed32 operator""_f32()
    {
        auto len = sizeof...(chars);
        const char str[] = {chars...};
        fixed32 fp;
        detail::parse(str, len, fp);
        return fp;
    }

#ifdef EIRIN_MATH_HAS_INT128
    constexpr inline fixed64 operator""_f64(const char* str, size_t len)
    {
        fixed64 fp;
        detail::parse(str, len, fp);
        return fp;
    }

    template <char... chars>
    constexpr inline fixed64 operator""_f64()
    {
        auto len = sizeof...(chars);
        const char str[] = {chars...};
        fixed64 fp;
        detail::parse(str, len, fp);
        return fp;
    }
#endif
} // namespace literals

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator+(const fixed_num<T, I, f, r>& fp, const detail::integral auto& val) noexcept
{
    return fixed_num<T, I, f, r>(fp) += val;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator+(const detail::integral auto& val, const fixed_num<T, I, f, r>& fp) noexcept
{
    return fixed_num<T, I, f, r>(fp) += val;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator-(const fixed_num<T, I, f, r>& fp, const detail::integral auto& val) noexcept
{
    return fixed_num<T, I, f, r>(fp) -= val;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator-(const detail::integral auto& val, const fixed_num<T, I, f, r>& fp) noexcept
{
    return fixed_num<T, I, f, r>(val) -= fp;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator*(const fixed_num<T, I, f, r>& fp, const detail::integral auto& val) noexcept
{
    return fixed_num<T, I, f, r>(fp) *= val;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator*(const detail::integral auto& val, const fixed_num<T, I, f, r>& fp) noexcept
{
    return fixed_num<T, I, f, r>(fp) *= val;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator/(const fixed_num<T, I, f, r>& fp, const detail::integral auto& val)
{
    return fixed_num<T, I, f, r>(fp) /= val;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator/(const detail::integral auto& val, const fixed_num<T, I, f, r>& fp)
{
    return fixed_num<T, I, f, r>(val) /= fp;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator%(const fixed_num<T, I, f, r>& fp, const detail::integral auto& val) noexcept
{
    return fixed_num<T, I, f, r>(fp) %= val;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator%(const detail::integral auto& val, const fixed_num<T, I, f, r>& fp) noexcept
{
    return fixed_num<T, I, f, r>(val) %= fp;
}

inline bool f32_from_cstring(const char* str, size_t len, fixed32& fp) noexcept
{
    // detail::parse should has same behavior as f32_from_cstring.
    return detail::parse(str, len, fp);
}

template <typename T, typename I, unsigned int f, bool r>
constexpr bool fixed_from_cstring(const char* str, size_t len, fixed_num<T, I, f, r>& fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    size_t pos = 0;
    bool negative = false;
    auto peek = [&]() -> char
    {
        return str[pos];
    };
    auto next = [&]() -> char
    {
        return str[pos++];
    };
    auto has_next = [&]() -> bool
    {
        return pos < len;
    };
    auto check_ch = [](char ch) -> bool
    {
        // This should be faster than isdigit
        return '0' <= ch && ch <= '9';
    };

    if(has_next() && peek() == '-')
    {
        negative = true;
        next();
    }

    I int_part = I(0), dec_part = I(0);
    // parse the integer part.
    while(has_next() && peek() != '.')
    {
        if(!check_ch(peek()))
            break;
        int_part = int_part * 10 + (next() - '0');
    }
    I fixed_value = I(0);
    // parse the decimal part.
    if(has_next() && peek() == '.')
    {
        next();
        constexpr auto max_fraction = (T(1) << f) - T(1);
        I divisor = I(1);
        while(has_next())
        {
            if(dec_part > max_fraction / 10 || !check_ch(peek()))
                break;
            auto digit = next() - '0';
            dec_part = dec_part * 10 + digit;
            divisor *= 10;
        }
        fixed_value = (int_part << f) + (dec_part << f) / divisor;
    }
    else
    {
        fixed_value = (int_part << f);
    }

    // check overflow
    if(fixed_value > static_cast<I>(detail::__any_int_traits<T>::max) || fixed_value < static_cast<I>(detail::__any_int_traits<T>::min))
    {
        return false;
    }

    fp = fixed::from_internal_value(static_cast<T>(fixed_value));
    if(negative)
        fp = -fp;
    return true;
}

template <typename CharT, class Traits, typename T, typename I, unsigned int f, bool r>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, fixed_num<T, I, f, r>& fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    bool negative = false;
    auto peek = [&]() -> CharT
    {
        return is.peek();
    };
    auto next = [&]() -> CharT
    {
        return is.get();
    };
    auto has_next = [&]() -> bool
    {
        auto ch = is.peek();
        return !is.eof();
    };
    if(!has_next())
    {
        return is;
    }
    else if(peek() == '-')
    {
        negative = true;
        next();
    }

    T int_part = T(0), dec_part = T(0);
    // parse the integer part.
    while(has_next() && peek() != '.')
    {
        if(!isdigit(peek()))
            break;
        int_part = int_part * 10 + (next() - '0');
    }
    // parse the decimal part.
    if(has_next() && peek() == '.')
    {
        next();
        constexpr auto max_fraction = (T(1) << f) - T(1);
        T scale = T(1), divisor = T(1);
        while(has_next())
        {
            if(dec_part > max_fraction / 10 || !isdigit(peek()))
                break;
            auto digit = next() - '0';
            dec_part = dec_part * 10 + digit;
            divisor *= 10;
        }
        fp = fixed::from_internal_value((int_part << f) + (dec_part << f) / divisor);
    }
    else
    {
        fp = fixed::from_internal_value(int_part << f);
    }
    if(negative)
        fp = -fp;
    return is;
}

/**
 * @brief The default hash function for fixed point type.
 *
 * @tparam FixedType fixed point type.
 */
template <typename FixedType>
requires is_fixed_point_v<FixedType>
struct fixed_hash
{
    std::size_t operator()(const FixedType& k) const noexcept
    {
        return std::hash<typename FixedType::value_type>()(k.internal_value());
    }
};
} // namespace eirin

namespace std
{
template <typename T, typename I, unsigned int f, bool r>
struct numeric_limits<eirin::fixed_num<T, I, f, r>>
{
    using fixed_type = eirin::fixed_num<T, I, f, r>;

    static constexpr int calc_digits10() noexcept
    {
        // digits10 = floor((sizeof(T) * 8 - 1) * log10(2)).
        // Use the rational approximation log10(2) ≈ 643 / 2136 (error ~3.3e-8).
        // For every bit width that can occur in practice (n <= 1024),
        // n * log10(2) is at least 1.4e-3 away from the nearest integer,
        // far larger than the accumulated approximation error, so the
        // integer division yields the exact floor value.
        constexpr int n = static_cast<int>(eirin::detail::__eval_max_bit_width<T>());
        return n * 643 / 2136;
    }

    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = is_signed_v<T>;
    static constexpr bool is_integer = false;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;

    static constexpr int radix = 2;
    // 1 bit sign + fractions bits + integer bits
    static constexpr int digits = static_cast<int>(fixed_type::digits);
    static constexpr int digits10 = calc_digits10();
    static constexpr int min_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent = 0;
    static constexpr int max_exponent10 = 0;

    static constexpr float_denorm_style has_denorm = denorm_absent;
    static constexpr bool has_denorm_loss = false;

    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = is_unsigned_v<T>;

#if defined(EIRIN_ARCH_X86) || defined(EIRIN_ARCH_PNACL) || defined(EIRIN_ARCH_WASM)
    // Used to describe the if using the fixed point type as operator number might cause hardware traps, for example, divide by zero.
    // The fixed_num::divide function will check and throw exception, but operator/ will not check, and might cause hardware traps on some architecture.
    static constexpr bool traps = true;
#else
    // Used to describe the if using the fixed point type as operator number might cause hardware traps, for example, divide by zero.
    // In current compile platform, both operator/ and fixed_num::divide will not cause hardware traps.
    // The fixed_num::divide function will check and throw exception, and operator/ will not trap (such as ARM architecture).
    static constexpr bool traps = false;
#endif
    static constexpr bool tinyness_before = false;
    static constexpr float_round_style round_style = r ? round_to_nearest : round_toward_zero;

#define EIRIN_DIRECT_IMPL(name, value) \
    static constexpr fixed_type name() \
    {                                  \
        return fixed_type(value);      \
    }

    EIRIN_DIRECT_IMPL(infinity, 0)
    EIRIN_DIRECT_IMPL(quiet_NaN, 0)
    EIRIN_DIRECT_IMPL(signaling_NaN, 0)
    EIRIN_DIRECT_IMPL(denorm_min, 0)

    static constexpr fixed_type epsilon() noexcept
    {
        return fixed_type::from_internal_value(T(1));
    }

    static constexpr fixed_type round_error() noexcept
    {
        return fixed_type::from_internal_value(T(1));
    }

    static constexpr fixed_type min() noexcept
    {
        if constexpr(std::numeric_limits<T>::is_specialized)
        {
            return fixed_type::from_internal_value(std::numeric_limits<T>::min());
        }
        else
        {
            constexpr fixed_type min_value = fixed_type::from_internal_value(fixed_type::signbit_mask());
            return min_value;
        }
    }

    static constexpr fixed_type max() noexcept
    {
        if constexpr(std::numeric_limits<T>::is_specialized)
        {
            return fixed_type::from_internal_value(std::numeric_limits<T>::max());
        }
        else
        {
            constexpr fixed_type max_value = fixed_type::from_internal_value(~fixed_type::signbit_mask());
            return max_value;
        }
    }

    static constexpr fixed_type lowest() noexcept
    {
        return min();
    }

#undef EIRIN_DIRECT_IMPL
};
} // namespace std

#ifdef _MSC_VER
#    pragma warning(pop)
#endif

#endif // EIRIN_MATH_FIXED_HPP
