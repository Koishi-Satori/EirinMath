#ifndef EIRIN_MATH_DETAIL_SIMD_FIXED_SIZE_HPP
#define EIRIN_MATH_DETAIL_SIMD_FIXED_SIZE_HPP

#pragma once

#include <cstddef>
#include <type_traits>
#include <bit>
#include "abi.hpp"
#include "../../macro.hpp"
#include "detail.hpp"

namespace eirin::simd_abi
{
template <typename T, std::size_t Remaining, typename Sequence>
struct fixed_abi_storage_builder;

template <typename T, typename... ABIs>
struct fixed_abi_storage_builder<T, 0, simd_sequence<T, ABIs...>>
{
    using type = simd_sequence<T, ABIs...>;
};

template <typename T, std::size_t Remaining, typename... ABIs>
requires(Remaining > 0)
struct fixed_abi_storage_builder<T, Remaining, simd_sequence<T, ABIs...>>
{
    using abi = typename all_native_abis::template best_abi<T, Remaining>;

    static constexpr std::size_t lanes = abi_size_v<T, abi>;
    static_assert(abi_valid_v<T, abi>);
    static_assert(lanes > 0 && lanes <= Remaining);

    using type = typename fixed_abi_storage_builder<T, Remaining - lanes, simd_sequence<T, ABIs..., abi>>::type;
};

template <typename T>
struct simd_sequence<T>
{
    using value_type = T;
    static constexpr std::size_t seq_size = 0;

    static constexpr std::size_t size()
    {
        return 0;
    }

    static constexpr std::size_t capacity()
    {
        return 0;
    }
};

namespace detail
{
    template <typename First, typename Remaining>
    struct simd_sequence_storage
    {
        First first;
        Remaining remaining;

        EIRIN_SIMD_INTRINSIC_DECLARE constexpr bool is_constant_p() const
        {
            if constexpr(std::is_class_v<First>)
                return first.is_constant_p() && remaining.is_constant_p();
            else
                return EIRIN_IS_CONSTANT_P(first) && remaining.is_constant_p();
        }
    };

    template <typename First, typename T>
    struct simd_sequence_storage<First, simd_sequence<T>>
    {
        First first;
        static constexpr simd_sequence<T> remaining = {};

        EIRIN_SIMD_INTRINSIC_DECLARE constexpr bool is_constant_p() const
        {
            if constexpr(std::is_class_v<First>)
                return first.is_constant_p();
            else
                return EIRIN_IS_CONSTANT_P(first);
        }
    };
} // namespace detail

template <typename T, typename ABI, typename... ABIs>
requires(!is_fixed_abi<ABI>)
struct simd_sequence<T, ABI, ABIs...> : detail::simd_sequence_storage<typename abi_traits<T, ABI>::member_type, simd_sequence<T, ABIs...>>
{
    using value_type = T;
    using first_type = typename abi_traits<T, ABI>::member_type;
    using first_abi = ABI;
    using remaining_type = simd_sequence<T, ABIs...>;
    static constexpr std::size_t seq_size = sizeof...(ABIs) + 1;

    static constexpr std::size_t size()
    {
        return seq_size;
    }

    static constexpr std::size_t capacity()
    {
        return abi_size_v<T, ABI> + remaining_type::capacity();
    }

    static constexpr std::size_t first_size = abi_size_v<T, ABI>;
    static constexpr bool is_homogeneous = (std::is_same_v<ABI, ABIs> && ...);
    using base = detail::simd_sequence_storage<typename abi_traits<T, ABI>::member_type, simd_sequence<T, ABIs...>>;
    using base::first;
    using base::remaining;

    EIRIN_SIMD_INTRINSIC_DECLARE constexpr simd_sequence() = default;
    EIRIN_SIMD_INTRINSIC_DECLARE constexpr simd_sequence(const simd_sequence&) = default;
    EIRIN_SIMD_INTRINSIC_DECLARE constexpr simd_sequence(simd_sequence&&) = default;
    EIRIN_SIMD_INTRINSIC_DECLARE constexpr simd_sequence& operator=(const simd_sequence&) = default;
    EIRIN_SIMD_INTRINSIC_DECLARE constexpr simd_sequence& operator=(simd_sequence&&) = default;

    template <typename U>
    EIRIN_SIMD_INTRINSIC_DECLARE constexpr simd_sequence(U&& a)
        : base{static_cast<U&&>(a)} {};

    template <typename U, typename U2>
    EIRIN_SIMD_INTRINSIC_DECLARE constexpr simd_sequence(U&& a, U2&& b)
        : base{static_cast<U&&>(a), static_cast<U2&&>(b)} {};

    template <typename U>
    EIRIN_SIMD_INTRINSIC_DECLARE constexpr simd_sequence(U&& a, simd_sequence<T>)
        : base{static_cast<U&&>(a)} {};

    template <std::size_t N>
    EIRIN_SIMD_INTRINSIC_DECLARE constexpr auto& at()
    {
        if constexpr(N == 0)
            return first;
        else
            return remaining.template at<N - 1>();
    }

    template <std::size_t N>
    EIRIN_SIMD_INTRINSIC_DECLARE constexpr const auto& at() const
    {
        if constexpr(N == 0)
            return first;
        else
            return remaining.template at<N - 1>();
    }
};

template <typename T, std::size_t N>
requires(N > 0 && detail::is_simd_lane_v<std::remove_cv_t<T>>)
struct fixed_abi_selector<T, N>
{
    using value_type = std::remove_cv_t<T>;
    static constexpr std::size_t capacity = std::bit_ceil(N);
    using type = typename fixed_abi_storage_builder<value_type, capacity, simd_sequence<value_type>>::type;

    static_assert(std::is_same_v<typename type::value_type, value_type>);
    static_assert(type::capacity() == capacity);
};

template <typename T, std::size_t N>
requires(N > 0 && detail::is_simd_lane_v<std::remove_cv_t<T>>)
struct fixed_storage : fixed_abi_t<std::remove_cv_t<T>, N>
{
    using value_type = std::remove_cv_t<T>;
    using abi_type = abi_fixed<N>;
    using storage_type = fixed_abi_t<value_type, N>;

    static constexpr std::size_t logical_size = N;
    static constexpr std::size_t capacity = std::bit_ceil(N);
    static constexpr std::size_t padding_size = capacity - logical_size;
    static constexpr std::size_t chunk_count = storage_type::size();
    static constexpr bool is_homogeneous = storage_type::is_homogeneous;
    static constexpr std::size_t first_chunk_size = storage_type::first_size;

    static_assert(storage_type::capacity() == capacity);

    using storage_type::at;
    using storage_type::storage_type;

    EIRIN_SIMD_INTRINSIC_DECLARE constexpr fixed_storage() = default;
    EIRIN_SIMD_INTRINSIC_DECLARE constexpr fixed_storage(const fixed_storage&) = default;
    EIRIN_SIMD_INTRINSIC_DECLARE constexpr fixed_storage(fixed_storage&&) = default;
    EIRIN_SIMD_INTRINSIC_DECLARE constexpr fixed_storage& operator=(const fixed_storage&) = default;
    EIRIN_SIMD_INTRINSIC_DECLARE constexpr fixed_storage& operator=(fixed_storage&&) = default;

    static constexpr std::size_t size() noexcept
    {
        return logical_size;
    }

    static constexpr std::size_t storage_size() noexcept
    {
        return storage_type::capacity();
    }

    EIRIN_SIMD_INTRINSIC_DECLARE constexpr storage_type& storage() noexcept
    {
        return *this;
    }

    EIRIN_SIMD_INTRINSIC_DECLARE constexpr const storage_type& storage() const noexcept
    {
        return *this;
    }
};
} // namespace eirin::simd_abi

#endif
