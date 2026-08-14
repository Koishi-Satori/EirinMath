#ifndef EIRIN_MATH_DETAIL_TYPE_TRAITS_IMPL_HPP
#define EIRIN_MATH_DETAIL_TYPE_TRAITS_IMPL_HPP

#pragma once

#include <type_traits>
#include <bit>
#include "int128.hpp"

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

template <typename T>
struct make_unsigned : public std::make_unsigned<T>
{};

template <typename T>
using make_unsigned_t = typename make_unsigned<T>::type;

// std::bit_width only accepts standard unsigned integer types; fall back to a
// constexpr loop for 128-bit and arbitrary-precision integers.
template <typename T>
constexpr std::size_t bit_width(T x) noexcept
{
    if constexpr(std::is_integral_v<T> && !std::is_same_v<T, detail::int128_t> && !std::is_same_v<T, detail::uint128_t>)
    {
        return std::bit_width(x);
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
