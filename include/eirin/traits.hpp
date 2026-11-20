#ifndef EIRIN_MATH_TRAITS_HPP
#define EIRIN_MATH_TRAITS_HPP

#pragma once

#include <bit>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include "macro.hpp"

namespace eirin::detail
{
/**
 * @brief Word types accepted by word-splitting big-integer generators.
 */
template <typename T>
concept bit_width_word =
    std::is_unsigned_v<T> &&
    !std::is_same_v<std::remove_cv_t<T>, bool> &&
    requires(T value) { std::bit_width(value); };

/**
 * @brief Common CRTP base for user-provided int_sequence_generator types.
 */
template <typename InType, bit_width_word OutType>
struct __int_sequence_generator_base
{
    using word_type = OutType;

    template <typename Derived>
    struct iterator
    {
        using value_type = OutType;
        using difference_type = std::size_t;
        using pointer = value_type*;
        using reference = const value_type&;

        constexpr iterator& operator++() noexcept
        {
            derived().move_forward();
            return *this;
        }

        constexpr iterator& operator--() noexcept
        {
            derived().move_backward();
            return *this;
        }

        constexpr bool operator==(const iterator& other) const noexcept
        {
            return derived().equal_to(other.derived());
        }

        constexpr bool operator!=(const iterator& other) const noexcept
        {
            return !(*this == other);
        }

        constexpr reference operator*() const noexcept
        {
            return derived().get_ref();
        }

    private:
        constexpr Derived& derived() noexcept
        {
            return static_cast<Derived&>(*this);
        }

        constexpr const Derived& derived() const noexcept
        {
            return static_cast<const Derived&>(*this);
        }
    };
};
} // namespace eirin::detail

namespace eirin::traits
{
template <typename T>
struct is_signed : public std::is_signed<T>
{};

template <typename T>
struct is_unsigned : public std::is_unsigned<T>
{};

template <typename T>
struct is_integral : public std::is_integral<T>
{};

template <typename T>
inline constexpr bool is_signed_v = is_signed<T>::value;

template <typename T>
inline constexpr bool is_unsigned_v = is_unsigned<T>::value;

template <typename T>
inline constexpr bool is_integral_v = is_integral<T>::value;

template <typename T, bool = is_unsigned_v<T>>
struct make_signed
{
    using type = T;
};

template <typename T>
struct make_signed<T, true>
{
    using type = std::make_signed_t<T>;
};

template <typename T>
using make_signed_t = typename make_signed<T>::type;

// std::make_unsigned is not SFINAE-friendly in libstdc++, so gate it first.
template <typename T, bool = (std::is_integral_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool>) || std::is_enum_v<T>>
struct __std_make_unsigned
{};

template <typename T>
struct __std_make_unsigned<T, true>
{
    using type = typename std::make_unsigned<T>::type;
};

template <typename T, typename = void>
struct make_unsigned
{};

template <typename T>
struct make_unsigned<T, std::void_t<typename __std_make_unsigned<T>::type>>
{
    using type = typename __std_make_unsigned<T>::type;
};

template <typename T>
using make_unsigned_t = typename make_unsigned<T>::type;

/**
 * @brief Epsilon used by element-wise nearly comparisons.
 *
 * Users may specialize this trait for their own scalar types.
 */
template <typename T>
struct element_compute_epsilon
{
    EIRIN_ALWAYS_INLINE constexpr static T eval()
    {
        return T{1e-6};
    }
};

/**
 * @brief Customization point for fractional scalar types.
 *
 * The library provides specializations for floating-point and fixed-point
 * types. Users may specialize this trait for additional scalar types.
 */
template <typename T>
struct is_fractional_scalar : std::false_type
{};

template <typename T>
requires std::floating_point<std::remove_cvref_t<T>>
struct is_fractional_scalar<T> : std::true_type
{};

template <typename T>
inline constexpr bool is_fractional_scalar_v = is_fractional_scalar<std::remove_cvref_t<T>>::value;

/**
 * @brief Customization point that splits a user-defined integer type into words.
 *
 * Specializations must yield words from least significant to most significant
 * and provide constexpr begin(const InType&) and end(const InType&) iterators.
 */
template <typename InType, detail::bit_width_word OutType>
struct int_sequence_generator : detail::__int_sequence_generator_base<InType, OutType>
{
    using word_type = typename detail::__int_sequence_generator_base<InType, OutType>::word_type;

private:
    struct __default_iterator : detail::__int_sequence_generator_base<InType, OutType>::template iterator<__default_iterator>
    {
        using value_type = OutType;
        using difference_type = std::size_t;
        using pointer = value_type*;
        using reference = const value_type&;

        constexpr void move_forward() noexcept {}

        constexpr void move_backward() noexcept {}

        constexpr bool equal_to([[maybe_unused]] const __default_iterator& other) const noexcept
        {
            return true;
        }

        constexpr reference get_ref() const noexcept
        {
            return zero_value;
        }

    private:
        value_type zero_value{0};
    };

    static constexpr const inline __default_iterator __iter_instance{};

public:
    static constexpr const inline bool is_specialized = false;

    static constexpr inline __default_iterator begin([[maybe_unused]] const InType& input) noexcept
    {
        return __iter_instance;
    }

    static constexpr inline __default_iterator end([[maybe_unused]] const InType& input) noexcept
    {
        return __iter_instance;
    }
};
} // namespace eirin::traits

#endif
