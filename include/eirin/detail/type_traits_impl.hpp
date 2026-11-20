#ifndef EIRIN_MATH_DETAIL_TYPE_TRAITS_IMPL_HPP
#define EIRIN_MATH_DETAIL_TYPE_TRAITS_IMPL_HPP

#pragma once

#include <bit>
#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>
#include "../traits.hpp"
#include "int128.hpp"

namespace eirin::detail
{
/**
 * @brief Read-only traits facade for `eirin::traits::int_sequence_generator`.
 */
template <typename InType, typename OutType>
struct int_sequence_generator_traits
{
private:
    using generator_type = ::eirin::traits::int_sequence_generator<InType, OutType>;

public:
    static constexpr bool is_specialized = generator_type::is_specialized;

    using word_type = typename generator_type::word_type;

    static constexpr std::size_t word_bits = std::numeric_limits<word_type>::digits;
};

// detector: true iff an unsigned counterpart exists
template <typename T, typename = void>
struct has_make_unsigned : std::false_type
{};

template <typename T>
struct has_make_unsigned<T, std::void_t<typename traits::make_unsigned<T>::type>> : std::true_type
{};

template <typename T>
inline constexpr bool has_make_unsigned_v = has_make_unsigned<T>::value;

// A dependent `false`, due to a plain `static_assert(false, ...)` is rejected
// by GCC as soon as the template is parsed, even if no specialization is ever
// instantiated.
template <typename...>
inline constexpr bool always_false = false;

// Not used yet, maybe later.
template <typename...>
inline constexpr bool always_true = true;

/**
 * @brief Picks the first word type, in priority order, with a generator.
 */
template <typename T, typename... Words>
struct generator_word_selector;

template <typename T>
struct generator_word_selector<T>
{
    static constexpr bool specialized = false;
    using word_type = void;
};

template <typename T, typename Word, typename... Rest>
struct generator_word_selector<T, Word, Rest...>
{
    static constexpr bool specialized =
        int_sequence_generator_traits<T, Word>::is_specialized ||
        generator_word_selector<T, Rest...>::specialized;

    using word_type = std::conditional_t<
        int_sequence_generator_traits<T, Word>::is_specialized,
        Word,
        typename generator_word_selector<T, Rest...>::word_type>;
};

template <typename T, typename = void>
struct can_apply_bit_width : std::false_type
{};

template <typename T>
struct can_apply_bit_width<T, std::void_t<decltype(std::bit_width(std::declval<T>()))>> : std::true_type
{};

/**
 * @brief Return the number of bits needed to represent `value`.
 *
 * Standard unsigned integers use `std::bit_width`. Types with a specialized
 * `eirin::traits::int_sequence_generator` are split into words; all other
 * types fall back to a generic shift loop.
 */
template <typename T>
EIRIN_MATH_FUNC_API std::size_t bit_width(T value) noexcept
{
    if constexpr(can_apply_bit_width<T>::value)
    {
        return std::bit_width(value);
    }
    else if constexpr(generator_word_selector<
                          T,
                          unsigned long long,
                          unsigned long,
                          unsigned int,
                          unsigned short,
                          unsigned char>::specialized)
    {
        using selector = generator_word_selector<
            T,
            unsigned long long,
            unsigned long,
            unsigned int,
            unsigned short,
            unsigned char>;
        using generator_traits = int_sequence_generator_traits<T, typename selector::word_type>;
        using generator = ::eirin::traits::int_sequence_generator<T, typename selector::word_type>;
        constexpr std::size_t word_bits = generator_traits::word_bits;
        auto iter = generator::begin(value);
        const auto end = generator::end(value);
        std::size_t width = 0u;
        std::size_t index = 0u;
        while(iter != end)
        {
            if(auto word = *iter; word != 0)
                width = index * word_bits + std::bit_width(word);
            ++index;
            ++iter;
        }
        return width;
    }
    else
    {
        std::size_t width = 0;
        while(value != 0)
        {
            ++width;
            value >>= 1;
        }
        return width;
    }
}
} // namespace eirin::detail

#ifdef EIRIN_MATH_HAS_INT128
namespace eirin::traits
{
template <>
struct is_signed<detail::int128_t> : public std::true_type
{};

template <>
struct is_integral<detail::int128_t> : public std::true_type
{};

template <>
struct is_unsigned<detail::uint128_t> : public std::true_type
{};

template <>
struct is_integral<detail::uint128_t> : public std::true_type
{};

template <>
struct make_signed<detail::uint128_t, true>
{
    using type = detail::int128_t;
};

template <>
struct make_unsigned<detail::int128_t>
{
    using type = detail::uint128_t;
};

template <>
struct make_unsigned<detail::uint128_t>
{
    using type = detail::uint128_t;
};
} // namespace eirin::traits
#endif

#endif
