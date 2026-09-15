#ifndef EIRIN_MATH_DETAIL_TYPE_TVEC_HPP
#define EIRIN_MATH_DETAIL_TYPE_TVEC_HPP

#pragma once

#include <cstddef>
#include <concepts>
#include <type_traits>
#include <stdexcept>
#include "../macro.hpp"
#include "../fixed.hpp"
#include "../error.hpp"
#include "compute_vec_rel.hpp"

namespace eirin
{
namespace detail
{
    template <typename T, typename = void>
    struct is_vector_type : std::false_type
    {};

    template <typename T>
    struct is_vector_type<T, std::void_t<typename std::remove_cvref_t<T>::is_tvec_type>> : std::true_type
    {};

    template <typename T>
    inline constexpr bool is_vector_type_v = is_vector_type<T>::value;

    template <typename T>
    concept has_operator_bit_xor = requires(T a, T b) { a ^ b; };
    template <typename T>
    concept has_operator_bit_and = requires(T a, T b) { a & b; };
    template <typename T>
    concept has_operator_bit_or = requires(T a, T b) { a | b; };
    template <typename T>
    concept has_operator_bit_not = requires(T a) { ~a; };
    template <typename T>
    concept has_operator_left_shift = requires(T a, T b) { a << b; };
    template <typename T>
    concept has_operator_right_shift = requires(T a, T b) { a >> b; };
} // namespace detail

template <typename T>
concept vector_type = detail::is_vector_type_v<T>;

template <std::size_t N, typename T>
struct tvec;

template <std::size_t N, typename T, typename Derived>
struct tvec_base
{
    using size_type = std::size_t;
    using value_type = T;
    using is_tvec_type = std::true_type;

    [[nodiscard]]
    EIRIN_ALWAYS_INLINE constexpr static size_type size() noexcept
    {
        return N;
    }

    constexpr inline Derived operator+(const Derived& rhs) const noexcept
    {
        Derived tmp(derived());
        tmp += rhs;
        return tmp;
    }

    constexpr inline Derived operator-(const Derived& rhs) const noexcept
    {
        Derived tmp(derived());
        tmp -= rhs;
        return tmp;
    }

    constexpr inline Derived operator*(const Derived& rhs) const noexcept
    {
        Derived tmp(derived());
        tmp *= rhs;
        return tmp;
    }

    constexpr inline Derived operator/(const Derived& rhs) const noexcept
    {
        Derived tmp(derived());
        tmp /= rhs;
        return tmp;
    }

    constexpr inline Derived operator%(const Derived& rhs) const noexcept
    {
        Derived tmp(derived());
        tmp %= rhs;
        return tmp;
    }

    constexpr inline Derived operator^(const Derived& rhs) const noexcept
    {
        Derived tmp(derived());
        tmp ^= rhs;
        return tmp;
    }

    constexpr inline Derived operator&(const Derived& rhs) const noexcept
    {
        Derived tmp(derived());
        tmp &= rhs;
        return tmp;
    }

    constexpr inline Derived operator|(const Derived& rhs) const noexcept
    {
        Derived tmp(derived());
        tmp |= rhs;
        return tmp;
    }

    constexpr inline Derived operator<<(const Derived& rhs) const noexcept
    {
        Derived tmp(derived());
        tmp <<= rhs;
        return tmp;
    }

    constexpr inline Derived operator>>(const Derived& rhs) const noexcept
    {
        Derived tmp(derived());
        tmp >>= rhs;
        return tmp;
    }

    EIRIN_ALWAYS_INLINE constexpr value_type dot(const Derived& rhs) const noexcept
    {
        value_type res{0};
        Derived tmp(derived());
        for(std::size_t i = 0; i < size(); ++i)
            res += tmp[i] * rhs[i];
        return res;
    }

    EIRIN_ALWAYS_INLINE constexpr bool operator==(const Derived& rhs) const noexcept
    {
        return derived() == rhs;
    }

    EIRIN_ALWAYS_INLINE constexpr value_type& at(std::integral auto i)
    {
        size_type index = static_cast<size_type>(i);
        if(index >= this->size())
        {
            EIRIN_THROW_EXCEPTION(std::out_of_range, "vec index out of range.");
        }
        return derived()[index];
    }

    EIRIN_ALWAYS_INLINE constexpr const value_type& at(std::integral auto i) const
    {
        size_type index = static_cast<size_type>(i);
        if(index >= this->size())
        {
            EIRIN_THROW_EXCEPTION(std::out_of_range, "vec index out of range.");
        }
        return derived()[index];
    }

    EIRIN_ALWAYS_INLINE constexpr value_type& element(std::integral auto i) noexcept
    {
        return derived()[wrap_index(i)];
    }

    EIRIN_ALWAYS_INLINE constexpr const value_type& element(std::integral auto i) const noexcept
    {
        return derived()[wrap_index(i)];
    }

    EIRIN_ALWAYS_INLINE constexpr bool nearly_eq(const Derived& rhs) const noexcept
    {
        if constexpr(is_fixed_point_v<T>)
        {
            Derived tmp(derived());
            for(std::size_t i = 0; i < size(); ++i)
                if(!tmp[i].nearly_eq(rhs[i]))
                    return false;
            return true;
        }
        // return derived() == rhs;
        // for-each every element and compare with nearly_eq
        Derived tmp(derived());
        for(std::size_t i = 0; i < size(); ++i)
            if(!detail::compute_nearly_equal<T, true>::eval(tmp[i], rhs[i]))
                return false;
        return true;
    }

private:
    // mod-warp a (possibly negative) integral index into [0, N): negative values
    // count from the end, out-of-range values wrap around.
    [[nodiscard]]
    EIRIN_ALWAYS_INLINE constexpr static size_type wrap_index(std::integral auto i) noexcept
    {
        if constexpr(std::is_signed_v<decltype(i)>)
        {
            using signed_size = std::make_signed_t<size_type>;
            constexpr auto len = static_cast<signed_size>(N);
            const auto rem = static_cast<signed_size>(i) % len;
            return static_cast<size_type>(rem < 0 ? rem + len : rem);
        }
        else
        {
            return static_cast<size_type>(i) % N;
        }
    }

    EIRIN_ALWAYS_INLINE constexpr Derived& derived() noexcept
    {
        return static_cast<Derived&>(*this);
    }

    EIRIN_ALWAYS_INLINE constexpr const Derived& derived() const noexcept
    {
        return static_cast<const Derived&>(*this);
    }
};

#define EIRIN_TVEC2_2_MEMBERS(T, E0, E1)


} // namespace eirin

#endif
