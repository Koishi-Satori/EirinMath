#ifndef EIRIN_MATH_DETAIL_TYPE_TRAITS_IMPL_HPP
#define EIRIN_MATH_DETAIL_TYPE_TRAITS_IMPL_HPP

#pragma once

#include <type_traits>
#include <bit>
#include <cstdint>
#include <concepts>
#include <limits>
#include "int128.hpp"

namespace eirin
{
namespace detail
{
    /**
     * @brief The word types the word-splitting iterators may yield.
     *
     * `detail::bit_width` computes the total bit width by summing per-word
     * `std::bit_width` results, and `std::bit_width` only accepts the standard
     * unsigned integer types (it rejects `bool` and `unsigned __int128`). The
     * same requirement therefore applies to the generator word type. Today
     * this is exactly `unsigned char`, `unsigned short`, `unsigned int`,
     * `unsigned long` and `unsigned long long` (i.e. `uint8_t`/`uint16_t`/
     * `uint32_t`/`uint64_t` where available).
     *
     * @tparam T the candidate word type.
     */
    template <typename T>
    concept bit_width_word = std::is_unsigned_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool> && requires(T v) { std::bit_width(v); };

    /**
     * @brief Common base of `eirin::int_sequence_generator`.
     *
     * Holds the word type and the CRTP iterator interface. Both the primary
     * template (placeholder) and user specializations inherit from it, so
     * `word_type` remains visible to consumers such as `detail::bit_width`
     * without every specialization redeclaring it.
     *
     * The nested `iterator<Derived>` is an optional CRTP base. A
     * specialization may derive its own iterator from it and only implement
     * `move_forward()`, `move_backward()`, `equal_to()` and `get_ref()`, or it
     * may provide a completely independent iterator with `operator*`,
     * `operator++` and `operator!=`. Either way all operations should be
     * `constexpr` so that `detail::bit_width` keeps constant evaluation, and
     * the iterator should own the data it iterates over instead of pointing
     * into a by-value parameter.
     *
     * @tparam InType the big-integer type being split into words.
     * @tparam OutType the word type; must satisfy `detail::bit_width_word`.
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
} // namespace detail

/**
 * @brief Customization point that splits a user-defined big-integer type into
 * a sequence of words, consumed by `eirin::detail::bit_width`.
 *
 * The primary template is a placeholder: `is_specialized == false` means the
 * type is not split into words, and `detail::bit_width` falls back to its
 * generic shift loop. `begin()`/`end()` still form a well-defined empty range
 * (they compare equal) and dereferencing yields zero, so the placeholder is
 * safe to use directly.
 *
 * @tparam InType the big-integer type.
 * @tparam OutType the word type; must satisfy `detail::bit_width_word`.
 *
 * ## How to specialize
 *
 * Specialize the template for your big-integer type and inherit the common
 * base so `word_type` stays available:
 *
 * @code{.cpp}
 * template <eirin::detail::bit_width_word OutType>
 * requires std::is_same_v<OutType, std::uint64_t>
 * struct eirin::int_sequence_generator<MyBig, OutType>
 *     : eirin::detail::__int_sequence_generator_base<MyBig, OutType>
 * {
 *     using base_t = eirin::detail::__int_sequence_generator_base<MyBig, OutType>;
 *     static constexpr bool is_specialized = true;
 *
 *     struct iterator { ... }; // operator*, operator++, operator!=
 *     static constexpr iterator begin(const MyBig&) noexcept;
 *     static constexpr iterator end(const MyBig&) noexcept;
 * };
 * @endcode
 *
 * A full specialization `template <> struct int_sequence_generator<MyBig,
 * std::uint64_t> : <base>` inheriting the base is equally valid; the base can
 * be inherited by both kinds of specializations.
 *
 * ## Conventions
 *
 * - Words are yielded from the least significant to the most significant
 *   (little-endian order).
 * - The word width is part of the type contract: a specialization must use
 *   the word width the storage is actually sliced at. Do not write a generic
 *   partial specialization over every `OutType` unless the iterator really
 *   re-slices the raw storage at the requested width; otherwise the same
 *   value yields different bit widths for different word types.
 * - Mirror the primary template's constraint: use `bit_width_word` (or a
 *   stronger constraint such as `requires std::is_same_v<OutType,
 *   std::uint64_t>`). A weaker constraint still works for valid word types
 *   but produces less readable errors for invalid ones.
 * - The iterator needs `operator*`, `operator++` and `operator!=`; make them
 *   `constexpr` so `detail::bit_width` keeps constant evaluation. Keep the
 *   iterator self-contained (own the data it iterates over) instead of
 *   pointing into a by-value parameter: pointers into parameters break
 *   constant evaluation and are a lifetime hazard.
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

    // default for not specialized, shoule never available for begin->++->end loop.
    static constexpr const inline __default_iterator __iter_instance{};

public:
    static constexpr const inline bool is_specialized = false;

    static constexpr inline __default_iterator begin([[maybe_unused]] const InType& in) noexcept
    {
        return __iter_instance;
    }

    static constexpr inline __default_iterator end([[maybe_unused]] const InType& in) noexcept
    {
        return __iter_instance;
    }
};

/**
 * @brief Customization-point traits for `eirin::int_sequence_generator`,
 * analogous to `std::iterator_traits`.
 *
 * Consumers such as `detail::bit_width` query the generator through this
 * facade instead of poking the generator template directly, so probing code
 * stays stable even if the generator interface evolves.
 *
 * @tparam InType the big-integer type.
 * @tparam OutType the word type to query.
 */
template <typename InType, typename OutType>
struct int_sequence_generator_traits
{
private:
    using generator_type = int_sequence_generator<InType, OutType>;

public:
    static constexpr bool is_specialized = generator_type::is_specialized;

    using word_type = typename generator_type::word_type;

    // Number of value bits per word, matching `std::bit_width` semantics and
    // safe against padding bits (unlike `sizeof(word_type) * 8`).
    static constexpr std::size_t word_bits = std::numeric_limits<word_type>::digits;
};
} // namespace eirin

namespace eirin::detail
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

// detector: true iff an unsigned counterpart exists
template <typename T, typename = void>
struct has_make_unsigned : std::false_type
{};

template <typename T>
struct has_make_unsigned<T, std::void_t<typename make_unsigned<T>::type>> : std::true_type
{};

template <typename T>
inline constexpr bool has_make_unsigned_v = has_make_unsigned<T>::value;

/**
 * @brief Picks the first word type, in priority order (wider preferred), for
 * which a generator specialization exists.
 *
 * Used by `detail::bit_width` so it works no matter which standard unsigned
 * word width a user chose for their big-integer type. The candidate list
 * covers exactly the types accepted by `detail::bit_width_word`.
 *
 * @tparam T the big-integer type.
 * @tparam Words the candidate word types, most preferred first.
 */
template <typename T, typename... Words>
struct __generator_word_selector;

template <typename T>
struct __generator_word_selector<T>
{
    static constexpr bool specialized = false;
    using word_type = void;
};

template <typename T, typename Word, typename... Rest>
struct __generator_word_selector<T, Word, Rest...>
{
    static constexpr bool specialized =
        ::eirin::int_sequence_generator_traits<T, Word>::is_specialized ||
        __generator_word_selector<T, Rest...>::specialized;

    using word_type = std::conditional_t<
        ::eirin::int_sequence_generator_traits<T, Word>::is_specialized,
        Word,
        typename __generator_word_selector<T, Rest...>::word_type>;
};

/**
 * @brief Return the number of bits needed to represent `x`.
 *
 * Dispatch rules:
 * - Standard unsigned integer types: `std::bit_width`.
 * - Types with a specialized `eirin::int_sequence_generator`: iterates all
 *   the "words" of `T` (yielded least significant first) and computes
 *   `highest_nonzero_word_index * word_bits + std::bit_width(top_word)`,
 *   which is usually faster than the generic loop.
 * - Everything else (e.g. `int128_t`/`uint128_t`): a generic shift loop.
 *
 * @tparam T the value type.
 * @param x the input value.
 * @return the bit width; `0` for zero.
 */
template <typename T>
EIRIN_MATH_FUNC_API std::size_t bit_width(T x) noexcept
{
    if constexpr(std::is_integral_v<T> && !std::is_same_v<T, detail::int128_t> && !std::is_same_v<T, detail::uint128_t>)
    {
        return std::bit_width(x);
    }
    else if constexpr(detail::__generator_word_selector<
                          T,
                          unsigned long long,
                          unsigned long,
                          unsigned int,
                          unsigned short,
                          unsigned char>::specialized)
    {
        using selector = detail::__generator_word_selector<
            T,
            unsigned long long,
            unsigned long,
            unsigned int,
            unsigned short,
            unsigned char>;
        using traits = ::eirin::int_sequence_generator_traits<T, typename selector::word_type>;
        using generator = ::eirin::int_sequence_generator<T, typename selector::word_type>;
        constexpr std::size_t word_bits = traits::word_bits;
        auto iter = generator::begin(x);
        const auto end = generator::end(x);
        std::size_t bw = 0u;
        std::size_t idx = 0u;
        // Words are yielded from the least significant to the most significant.
        // The total bit width is `highest_nonzero_word_index * word_bits +
        // bit_width(top_word)`; zero words below the top still contribute their
        // full word shift, so recompute for every nonzero word.
        while(iter != end)
        {
            if(auto word = *iter; word != 0)
                bw = idx * word_bits + std::bit_width(word);
            ++idx;
            ++iter;
        }
        return bw;
    }
    else
    {
        std::size_t w = 0;
        while(x != 0)
        {
            ++w;
            x >>= 1;
        }
        return w;
    }
}

#ifdef EIRIN_MATH_HAS_INT128
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

#endif
} // namespace eirin::detail

#endif
