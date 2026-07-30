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
#include <algorithm>
#include <iterator>
#include "macro.hpp"
#include "detail/int128.hpp"
#include "error.hpp"

namespace eirin
{
namespace detail
{
    template <typename T, unsigned int F, int E>
    struct sqrt_init_value
    {
        static constexpr T value() noexcept
        {
            constexpr int half_exp = E / 2;
            constexpr bool adjust = (E % 2) != 0;

            constexpr T base = static_cast<T>(1) << half_exp;
            constexpr T value = adjust ? (base | (base >> 1)) : base;

            return value << (F / 2 + F % 2);
        }
    };

    template <typename T, unsigned int F, typename S = std::make_index_sequence<sizeof(T) * 8>>
    struct sqrt_lookup_table;

    template <typename T, unsigned int F, size_t... I>
    struct sqrt_lookup_table<T, F, std::index_sequence<I...>>
    {
        static constexpr std::array<T, sizeof...(I)> generate() noexcept
        {
            return {{sqrt_init_value<T, F, I>::value()...}};
        }
    };

    template <typename T>
    constexpr int find_msb(T value) noexcept
    {
        if(value == 0)
            return -1;
        if constexpr(std::is_signed_v<T>)
        {
            using U = std::make_unsigned_t<T>;
            auto abs_v = value < 0 ? -value : value;
            return find_msb(static_cast<U>(abs_v));
        }
        else
        {
            int bits = 0;
            while(value >>= 1)
                ++bits;
            return bits;
        }
    }

    template <typename T>
    struct is_signed : public std::is_signed<T>
    {};

#ifdef EIRIN_MATH_HAS_INT128
    template <>
    struct is_signed<detail::int128_t> : public std::true_type
    {};
#endif

    template <typename T>
    struct __eval_max_bit_width_helper
    {
        EIRIN_ALWAYS_INLINE constexpr const std::size_t eval() const noexcept
        {
            if constexpr (std::numeric_limits<T>::is_specialized)
            {
                return std::numeric_limits<T>::digits;
            }
            else
            {
                return sizeof(T) * 8 - 1;
            }
        }
    };

    template <typename T>
    constexpr inline std::size_t eval_max_bit_width = __eval_max_bit_width_helper<T>::eval();
} // namespace detail

template <typename Type, unsigned int fraction>
concept fixed_num_fraction = fraction > 0 && fraction <= sizeof(Type) * 8 - 1;

template <typename Type, typename IntermediateType>
concept fixed_num_size = sizeof(IntermediateType) > sizeof(Type);

template <typename Type, typename IntermediateType>
concept fixed_num_signness = detail::is_signed<IntermediateType>::value == detail::is_signed<Type>::value;

template <typename Type, typename IntermediateType, unsigned int fraction>
concept fixed_num_check = std::is_integral_v<Type> && fixed_num_fraction<Type, fraction> && fixed_num_size<Type, IntermediateType> && fixed_num_signness<Type, IntermediateType>;

template <int scale>
concept fixed_format_check_scale = requires {
    scale == 2 || scale == 8 || scale == 10 || scale == 16;
};

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
    // static_assert(std::is_integral_v<Type>, "The store type must be a integral type.");
    // static_assert(fraction > 0, "The fraction must be greater than zero.");
    // static_assert(fraction <= sizeof(Type) * 8 - 1, "the type must be able to hold the entire fractional part.");
    // static_assert(sizeof(IntermediateType) > sizeof(Type), "The intermediate type must be larger than the store type.");
    // static_assert(std::is_signed<IntermediateType>::value == std::is_signed<Type>::value, "The intermediate type must have the same signness as the store type.");

    // just for raw value constructor call.
    struct raw_value_construct_tag
    {};

    static constexpr IntermediateType fraction_multiplier = IntermediateType(1) << fraction;

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
    template <std::integral T>
    EIRIN_ALWAYS_INLINE constexpr explicit fixed_num(T val) noexcept
        : m_value(static_cast<Type>(val) << fraction){};

    template <std::floating_point T>
    EIRIN_ALWAYS_INLINE constexpr explicit fixed_num(T val) noexcept
    {
        if constexpr(std::is_class_v<IntermediateType>)
        {
            if constexpr(rounding)
            {
                m_value = static_cast<Type>(
                    val >= 0.0 ?
                        (val * T{0.5} * fraction_multiplier) :
                        (val * fraction_multiplier - T{0.5})
                );
            }
            else
            {
                m_value = static_cast<Type>(
                    Type(val) * fraction_multiplier
                );
            }
        }
        else
        {
            m_value = static_cast<Type>(
                rounding ? (val >= 0.0) ? (val * fraction_multiplier * T{0.5}) : (val * fraction_multiplier - T{0.5}) : (val * fraction_multiplier)
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
        return from_fixed_num_value<64>(0x5000000000000ll);
    }

    static constexpr fixed_num e()
    {
        return from_fixed_num_value<61>(0x56FC2A2C515DA54Dll);
    }

    static constexpr fixed_num pi()
    {
        return from_fixed_num_value<61>(0x6487ED5110B4611All);
    }

    static constexpr fixed_num pi_2()
    {
        // pi / 2 just need shift the inner value right 1 bit.
        // so here just set the fraction to 1 less.
        return from_fixed_num_value<62>(0x6487ED5110B4611All);
    }

    static constexpr fixed_num pi_4()
    {
        // pi / 2 just need shift the inner value right 2 bit.
        // so here just set the fraction to 2 less.
        return from_fixed_num_value<63>(0x6487ED5110B4611All);
    }

    static constexpr fixed_num double_pi()
    {
        // pi / 2 just need shift the inner value left 1 bit.
        // so here just set the fraction to 1 more.
        return from_fixed_num_value<60>(0x6487ED5110B4611All);
    }

    static constexpr fixed_num nearly_compare_epsilon()
    {
        return from_fixed_num_value<64>(0x5000000000000ll);
    }

    static constexpr inline auto precision = fraction;

    EIRIN_ALWAYS_INLINE static constexpr Type signbit_mask() noexcept
    {
        return static_cast<Type>(1) << (sizeof(Type) * 8 - 1);
    }

    EIRIN_ALWAYS_INLINE friend constexpr bool signbit(const fixed_num& f) noexcept
    {
        if constexpr(std::is_signed_v<Type>)
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

    template <bool IgnoreSignBit = true>
    EIRIN_ALWAYS_INLINE constexpr std::size_t bit_width() const noexcept
    {
        if constexpr (!IgnoreSignBit)
            return sizeof(Type);
        const Type mask = m_value >> (sizeof(Type) * 8 - 1);
        Type u_value = (m_value ^ mask) - mask;
        for (int i = sizeof(Type) * 8 - 1; i >= 1; --i)
        {
            if ((u_value >> static_cast<Type>(i)) & static_cast<Type>(0x1))
                return i + 1;
        }
        return 0;
    }

    /* operator override functions */

    fixed_num& operator=(const fixed_num&) noexcept = default;

    template <std::integral T>
    constexpr inline explicit operator T() const noexcept
    {
        return static_cast<T>(m_value >> fraction);
    }

    template <std::floating_point T>
    constexpr inline explicit operator T() const noexcept
    {
        // MSVC might warn about precision loss here, but it's expected.
        // SO I JUST FUCK IT BEFORE.
        return static_cast<T>(static_cast<value_type>(m_value / fraction_multiplier));
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

    constexpr inline fixed_num& operator+=(const std::integral auto& val) noexcept
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

    constexpr inline fixed_num& operator-=(const std::integral auto& val) noexcept
    {
        m_value -= static_cast<Type>(val) << fraction;
        return *this;
    }

    constexpr inline fixed_num operator*(const fixed_num& other) const noexcept
    {
        if constexpr(rounding)
        {
            auto _value = (static_cast<IntermediateType>(m_value) * other.m_value) / (fraction_multiplier / 2);
            return fixed_num(static_cast<Type>(_value + (_value % 2)), raw_value_construct_tag{});
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
            m_value = static_cast<Type>(_value + (_value % 2));
        }
        else
        {
            auto _value = (static_cast<IntermediateType>(m_value) * other.m_value) >> fraction;
            m_value = static_cast<Type>(_value);
        }
        return *this;
    }

    constexpr inline fixed_num& operator*=(const std::integral auto& val) noexcept
    {
        m_value *= val;
        return *this;
    }

    constexpr inline fixed_num operator/(const fixed_num& other) const noexcept
    {
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

    constexpr inline fixed_num& operator/=(const fixed_num& other) noexcept
    {
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

    constexpr inline fixed_num& operator/=(const std::integral auto& val) noexcept
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

    constexpr inline fixed_num& operator~() noexcept
    {
        m_value = ~m_value;
        return *this;
    }
    /**
     * @brief Left Shifting the internal representation of the fixed point with val bits.
     *        To be noticed that, this operator does not checks overflow and range of n bits.
     * @see shl
     * 
     * @param val n bits to left shift.
     * @return constexpr fixed_num compute result.
     */
    constexpr inline fixed_num operator<<(const std::integral auto& val) const noexcept
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
    constexpr inline fixed_num& operator<<=(const std::integral auto& val) noexcept
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
    constexpr inline fixed_num operator>>(const std::integral auto& val) const noexcept
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
    constexpr inline fixed_num& operator>>=(const std::integral auto& val) noexcept
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
        m_value += Type(1) << fraction;
        return *this;
    }

    constexpr inline fixed_num operator++(int) noexcept
    {
        fixed_num temp = *this;
        m_value += Type(1) << fraction;
        return temp;
    }

    constexpr inline fixed_num& operator--() noexcept
    {
        m_value -= Type(1) << fraction;
        return *this;
    }

    constexpr inline fixed_num operator--(int) noexcept
    {
        fixed_num temp = *this;
        m_value -= Type(1) << fraction;
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

    // generate lookup table for sqrt calc.
    static constexpr auto sqrt_init_table = detail::sqrt_lookup_table<Type, fraction>::generate();

    static constexpr Type get_sqrt_init_value(int exponent) noexcept
    {
        constexpr int max_exponent = sizeof(Type) * 8 - 1;
        const int clamped = std::clamp(exponent, 0, max_exponent);
        return sqrt_init_table[clamped];
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

    EIRIN_ALWAYS_INLINE constexpr fixed_num divide(const std::integral auto& val) const
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

    EIRIN_ALWAYS_INLINE constexpr fixed_num& divide_by(const std::integral auto& val)
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

    EIRIN_ALWAYS_INLINE constexpr fixed_num shl(const std::integral auto& val) const
    {
        if(val < 0) [[unlikely]]
            EIRIN_THROW_EXCEPTION(std::range_error, "n bits to left shift should be greater than or equal to 0.");
        auto n_bits = static_cast<decltype(val)>(bit_width());
        constexpr auto max_bits = static_cast<decltype(val)>(sizeof(Type) * 8 - 1);
        if (val >= max_bits)
            EIRIN_THROW_EXCEPTION(std::overflow_error, "left shift bits larger than bit width is undefined.");
        if (n_bits != 0 && n_bits + val > max_bits)
            EIRIN_THROW_EXCEPTION(std::overflow_error, "will cause overflow after left shifts n bits.");
        return fixed_num(m_value << val, raw_value_construct_tag{});
    }

    EIRIN_ALWAYS_INLINE constexpr fixed_num& shl_by(const std::integral auto& val)
    {
        if(val < 0) [[unlikely]]
            EIRIN_THROW_EXCEPTION(std::range_error, "n bits to left shift should be greater than or equal to 0.");
        auto n_bits = static_cast<decltype(val)>(bit_width());
        constexpr auto max_bits = static_cast<decltype(val)>(sizeof(Type) * 8 - 1);
        if (val >= max_bits)
            EIRIN_THROW_EXCEPTION(std::overflow_error, "left shift bits larger than bit width is undefined.");
        if (n_bits != 0 && n_bits + val > max_bits)
            EIRIN_THROW_EXCEPTION(std::overflow_error, "will cause overflow after left shifts n bits.");
        m_value <<= val;
        return *this;
    }

    EIRIN_ALWAYS_INLINE constexpr fixed_num shr(const std::integral auto& val) const
    {
        if(val < 0) [[unlikely]]
            EIRIN_THROW_EXCEPTION(std::range_error, "n bits to left shift should be greater than or equal to 0.");
        auto n_bits = static_cast<decltype(val)>(bit_width());
        constexpr auto max_bits = static_cast<decltype(val)>(sizeof(Type) * 8 - 1);
        if (val >= max_bits)
            EIRIN_THROW_EXCEPTION(std::overflow_error, "left shift bits larger than bit width is undefined.");
        return fixed_num(m_value >> val, raw_value_construct_tag{});
    }

    EIRIN_ALWAYS_INLINE constexpr fixed_num& shr_by(const std::integral auto& val) const
    {
        if(val < 0) [[unlikely]]
            EIRIN_THROW_EXCEPTION(std::range_error, "n bits to left shift should be greater than or equal to 0.");
        auto n_bits = static_cast<decltype(val)>(bit_width());
        constexpr auto max_bits = static_cast<decltype(val)>(sizeof(Type) * 8 - 1);
        if (val >= max_bits)
            EIRIN_THROW_EXCEPTION(std::overflow_error, "left shift bits larger than bit width is undefined.");
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

    template <typename CharT, typename T, typename I, unsigned int f, bool r>
    constexpr bool parse(const CharT* str, size_t len, fixed_num<T, I, f, r>& fp) noexcept
    {
        using fixed = fixed_num<T, I, f, r>;
        size_t pos = 0;
        bool negative = false;

        auto check_ch = [](char ch) -> bool
        {
            // This should be faster than isdigit
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

        I int_part = I(0), dec_part = I(0);
        // parse the integer part.
        while(pos < len && str[pos] != '.')
        {
            if(!check_ch(str[pos]))
                return false;
            int_part = int_part * 10 + (next() - '0');
        }

        I fixed_value;
        // parse the decimal part.
        if(pos < len && str[pos] == '.')
        {
            ++pos;
            constexpr auto max_fraction = ((std::int64_t)1 << fixed::precision) - 1;
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

        // check overflow
        if(fixed_value > static_cast<I>(std::numeric_limits<T>::max()) || fixed_value < static_cast<I>(std::numeric_limits<T>::min()))
        {
            return false;
        }

        fp = fixed::from_internal_value(static_cast<T>(fixed_value));
        if(negative)
            fp = -fp;
        return true;
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
constexpr inline fixed_num<T, I, f, r> operator+(const fixed_num<T, I, f, r>& fp, const std::integral auto& val) noexcept
{
    return fixed_num<T, I, f, r>(fp) += val;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator+(const std::integral auto& val, const fixed_num<T, I, f, r>& fp) noexcept
{
    return fixed_num<T, I, f, r>(fp) += val;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator-(const fixed_num<T, I, f, r>& fp, const std::integral auto& val) noexcept
{
    return fixed_num<T, I, f, r>(fp) -= val;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator-(const std::integral auto& val, const fixed_num<T, I, f, r>& fp) noexcept
{
    return fixed_num<T, I, f, r>(val) -= fp;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator*(const fixed_num<T, I, f, r>& fp, const std::integral auto& val) noexcept
{
    return fixed_num<T, I, f, r>(fp) *= val;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator*(const std::integral auto& val, const fixed_num<T, I, f, r>& fp) noexcept
{
    return fixed_num<T, I, f, r>(fp) *= val;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator/(const fixed_num<T, I, f, r>& fp, const std::integral auto& val)
{
    return fixed_num<T, I, f, r>(fp) /= val;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator/(const std::integral auto& val, const fixed_num<T, I, f, r>& fp)
{
    return fixed_num<T, I, f, r>(val) /= fp;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator%(const fixed_num<T, I, f, r>& fp, const std::integral auto& val) noexcept
{
    return fixed_num<T, I, f, r>(fp) %= val;
}

template <typename T, typename I, unsigned int f, bool r>
constexpr inline fixed_num<T, I, f, r> operator%(const std::integral auto& val, const fixed_num<T, I, f, r>& fp) noexcept
{
    return fixed_num<T, I, f, r>(val) %= fp;
}

inline bool f32_from_cstring(const char* str, size_t len, fixed32& fp) noexcept
{
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

    int64_t int_part = 0, dec_part = 0;
    // parse the integer part.
    while(has_next() && peek() != '.')
    {
        if(!check_ch(peek()))
        {
            return false;
        }
        int_part = int_part * 10 + (next() - '0');
    }
    int64_t fixed_value;
    // parse the decimal part.
    if(has_next() && peek() == '.')
    {
        ++pos;
        constexpr auto max_fraction = (1 << fixed32::precision) - 1;
        int64_t divisor = 1;
        while(has_next())
        {
            if(!check_ch(peek()))
            {
                return false;
            }
            if(dec_part > max_fraction / 10)
            {
                break;
            }
            auto digit = next() - '0';
            dec_part = dec_part * 10 + digit;
            divisor *= 10;
        }
        fixed_value = (int_part << fixed32::precision) + (dec_part << fixed32::precision) / divisor;
    }
    else
    {
        fixed_value = int_part << fixed32::precision;
    }

    // check overflow
    if(fixed_value > static_cast<int64_t>(std::numeric_limits<int32_t>::max()) || fixed_value < static_cast<int64_t>(std::numeric_limits<int32_t>::min()))
    {
        return false;
    }

    fp = fixed32::from_internal_value(static_cast<int32_t>(fixed_value));
    if(negative)
        fp = -fp;
    return true;
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
    if(fixed_value > static_cast<I>(std::numeric_limits<T>::max()) || fixed_value < static_cast<I>(std::numeric_limits<T>::min()))
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

    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = true;
    static constexpr bool is_integer = false;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;

    static constexpr fixed_type radix = fixed_type(2);
    // 1 bit sign + fractions bits + integer bits
    static constexpr fixed_type digits = fixed_type(sizeof(T) * 8 - 1);
    // integer bits = sizeof(T) * 8 - 1 - f
    // fraction bits = f
    // digits10 = integer bits * log10(2) + fraction bits * log10(2)
    // we can use log10(2) = 643 / 2136
    static constexpr fixed_type digits10 = fixed_type(static_cast<T>((sizeof(T) * 8 - 1) * 643L / 2136));
    static constexpr fixed_type min_exponent = fixed_type(0);
    static constexpr fixed_type min_exponent10 = fixed_type(10);

    static constexpr float_denorm_style has_denorm = denorm_absent;
    static constexpr bool has_denorm_loss = false;

    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = false;

    static constexpr bool traps = true;
    static constexpr bool tinyness_before = false;
    static constexpr float_round_style round_style = round_toward_zero;

#define EIRIN_SHORT_IMPL(name)         \
    static constexpr fixed_type name() \
    {                                  \
        return fixed_type::name();     \
    }

#define EIRIN_DIRECT_IMPL(name, value) \
    static constexpr fixed_type name() \
    {                                  \
        return fixed_type(value);      \
    }

    EIRIN_SHORT_IMPL(epsilon)
    EIRIN_DIRECT_IMPL(round_error, 0)
    EIRIN_DIRECT_IMPL(infinity, 0)
    EIRIN_DIRECT_IMPL(quiet_NaN, 0)
    EIRIN_DIRECT_IMPL(signaling_NaN, 0)
    EIRIN_DIRECT_IMPL(denorm_min, 0)

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

#undef EIRIN_SHORT_IMPL
#undef EIRIN_DIRECT_IMPL
};
} // namespace std

#ifdef _MSC_VER
#    pragma warning(pop)
#endif

#endif // EIRIN_MATH_FIXED_HPP
