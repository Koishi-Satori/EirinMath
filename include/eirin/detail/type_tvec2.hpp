#ifndef EIRIN_MATH_DETAIL_TYPE_TVEC2_HPP
#define EIRIN_MATH_DETAIL_TYPE_TVEC2_HPP

#pragma once

#include <limits>
#include <eirin/detail/type_tvec.hpp>
#include <eirin/detail/compute_vec_rel.hpp>

namespace eirin
{
template <typename T>
struct tvec<2, T> : public tvec_base<2, T, tvec<2, T>>
{
    using value_type = T;

    // union data_type
    // {
    //     T x, y;
    // };

    // data_type data;

    T x, y;

    EIRIN_ALWAYS_INLINE constexpr tvec() = default;
    EIRIN_ALWAYS_INLINE constexpr tvec(const tvec& v) = default;
    EIRIN_ALWAYS_INLINE explicit constexpr tvec(T scalar)
        : x(scalar), y(scalar){};
    EIRIN_ALWAYS_INLINE constexpr tvec(T _x, T _y)
        : x(_x), y(_y){};
    template <typename A, typename B>
    EIRIN_ALWAYS_INLINE constexpr tvec(A _x, B _y)
        : x(static_cast<T>(_x)), y(static_cast<T>(_y)){};

    tvec& operator=(const tvec& other) noexcept = default;

    constexpr inline T& operator[](std::size_t i) noexcept
    {
        if(i == 0) return x;
        return y;
    }

    constexpr inline const T& operator[](std::size_t i) const noexcept
    {
        if(i == 0) return x;
        return y;
    }

    template <typename U>
    constexpr inline tvec& operator=(const tvec<2, U>& rhs) noexcept
    {
        this->x = static_cast<T>(rhs.x);
        this->y = static_cast<T>(rhs.y);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator+=(const tvec<2, U>& rhs) noexcept
    {
        this->x += static_cast<T>(rhs.x);
        this->y += static_cast<T>(rhs.y);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator-=(const tvec<2, U>& rhs) noexcept
    {
        this->x -= static_cast<T>(rhs.x);
        this->y -= static_cast<T>(rhs.y);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator*=(const tvec<2, U>& rhs) noexcept
    {
        this->x *= static_cast<T>(rhs.x);
        this->y *= static_cast<T>(rhs.y);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator/=(const tvec<2, U>& rhs) noexcept
    {
        this->x /= static_cast<T>(rhs.x);
        this->y /= static_cast<T>(rhs.y);
        return *this;
    }

    constexpr inline tvec& operator++() noexcept
    {
        ++this->x;
        ++this->y;
        return *this;
    }

    constexpr inline tvec& operator--() noexcept
    {
        ++this->x;
        ++this->y;
        return *this;
    }

    constexpr inline tvec& operator++(int) noexcept
    {
        tvec res(*this);
        ++*this;
        return res;
    }

    constexpr inline tvec& operator--(int) noexcept
    {
        tvec res(*this);
        --*this;
        return res;
    }

    template <typename U>
    constexpr inline tvec& operator%=(const tvec<2, U>& rhs) noexcept
    {
        this->x %= static_cast<T>(rhs.x);
        this->y %= static_cast<T>(rhs.y);
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_xor<U>
    constexpr inline tvec& operator^=(const tvec& rhs) noexcept
    {
        x ^= rhs.x;
        y ^= rhs.y;
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_and<U>
    constexpr inline tvec& operator&=(const tvec& rhs) noexcept
    {
        x &= rhs.x;
        y &= rhs.y;
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_or<U>
    constexpr inline tvec& operator|=(const tvec& rhs) noexcept
    {
        x |= rhs.x;
        y |= rhs.y;
        return *this;
    }

    template <typename U = T>
    requires detail::has_operator_bit_not<U>
    constexpr inline tvec& operator~() noexcept
    {
        return tvec{~this->x, ~this->y};
    }

    EIRIN_ALWAYS_INLINE constexpr value_type cross(const tvec& rhs) const noexcept
    {
        auto res = x * rhs.y;
        res -= y * rhs.x;
        return res;
    }

    constexpr inline tvec<2, bool> operator&&(const tvec& rhs) noexcept
    {
        return tvec<2, bool>{this->x && rhs.x, this->y && rhs.y};
    }

    constexpr inline tvec<2, bool> operator||(const tvec& rhs) noexcept
    {
        return tvec<2, bool>{this->x || rhs.x, this->y || rhs.y};
    }
};

// unary operators for vec2
template <typename T>
constexpr inline tvec<2, T> operator+(const tvec<2, T>& v)
{
    return v;
}

template <typename T>
constexpr inline tvec<2, T> operator-(const tvec<2, T>& v)
{
    return tvec<2, T>(-v.x, -v.y);
}

// binary operators for vec2

template <typename T>
constexpr inline tvec<2, T> operator+(const tvec<2, T>& v, T scalar)
{
    return tvec<2, T>(v.x + scalar, v.y + scalar);
}

template <typename T>
constexpr inline tvec<2, T> operator+(T scalar, const tvec<2, T>& v)
{
    return tvec<2, T>(v.x + scalar, v.y + scalar);
}

template <typename T>
constexpr inline tvec<2, T> operator+(const tvec<2, T>& v1, const tvec<2, T>& v2)
{
    return tvec<2, T>(v1.x + v2.x, v1.y + v2.y);
}

template <typename T>
constexpr inline tvec<2, T> operator-(const tvec<2, T>& v, T scalar)
{
    return tvec<2, T>(v.x - scalar, v.y - scalar);
}

template <typename T>
constexpr inline tvec<2, T> operator-(T scalar, const tvec<2, T>& v)
{
    return tvec<2, T>(scalar - v.x, scalar - v.y);
}

template <typename T>
constexpr inline tvec<2, T> operator-(const tvec<2, T>& v1, const tvec<2, T>& v2)
{
    return tvec<2, T>(v1.x - v2.x, v1.y - v2.y);
}

template <typename T>
constexpr inline tvec<2, T> operator*(const tvec<2, T>& v, T scalar)
{
    return tvec<2, T>(v.x * scalar, v.y * scalar);
}

template <typename T>
constexpr inline tvec<2, T> operator*(T scalar, const tvec<2, T>& v)
{
    return tvec<2, T>(v.x * scalar, v.y * scalar);
}

template <typename T>
constexpr inline tvec<2, T> operator*(const tvec<2, T>& v1, const tvec<2, T>& v2)
{
    return tvec<2, T>(v1.x * v2.x, v1.y * v2.y);
}

template <typename T>
constexpr inline tvec<2, T> operator/(const tvec<2, T>& v, T scalar)
{
    return tvec<2, T>(v.x / scalar, v.y / scalar);
}

template <typename T>
constexpr inline tvec<2, T> operator/(T scalar, const tvec<2, T>& v)
{
    return tvec<2, T>(scalar / v.x, scalar / v.y);
}

template <typename T>
constexpr inline tvec<2, T> operator/(const tvec<2, T>& v1, const tvec<2, T>& v2)
{
    return tvec<2, T>(v1.x / v2.x, v1.y / v2.y);
}

template <typename T>
constexpr inline tvec<2, T> operator%(const tvec<2, T>& v, T scalar)
{
    return tvec<2, T>(v.x % scalar, v.y % scalar);
}

template <typename T>
constexpr inline tvec<2, T> operator%(T scalar, const tvec<2, T>& v)
{
    return tvec<2, T>(scalar % v.x, scalar % v.y);
}

template <typename T>
constexpr inline tvec<2, T> operator%(const tvec<2, T>& v1, const tvec<2, T>& v2)
{
    return tvec<2, T>(v1.x % v2.x, v1.y % v2.y);
}

template <typename T>
requires detail::has_operator_bit_xor<T>
constexpr inline tvec<2, T> operator^(const tvec<2, T>& v, T scalar)
{
    return tvec<2, T>(v.x ^ scalar, v.y ^ scalar);
}

template <typename T>
requires detail::has_operator_bit_xor<T>
constexpr inline tvec<2, T> operator^(T scalar, const tvec<2, T>& v)
{
    return tvec<2, T>(scalar ^ v.x, scalar ^ v.y);
}

template <typename T>
requires detail::has_operator_bit_xor<T>
constexpr inline tvec<2, T> operator^(const tvec<2, T>& v1, const tvec<2, T>& v2)
{
    return tvec<2, T>(v1.x ^ v2.x, v1.y ^ v2.y);
}

template <typename T>
requires detail::has_operator_bit_and<T>
constexpr inline tvec<2, T> operator&(const tvec<2, T>& v, T scalar)
{
    return tvec<2, T>(v.x & scalar, v.y & scalar);
}

template <typename T>
requires detail::has_operator_bit_and<T>
constexpr inline tvec<2, T> operator&(T scalar, const tvec<2, T>& v)
{
    return tvec<2, T>(scalar & v.x, scalar & v.y);
}

template <typename T>
requires detail::has_operator_bit_and<T>
constexpr inline tvec<2, T> operator&(const tvec<2, T>& v1, const tvec<2, T>& v2)
{
    return tvec<2, T>(v1.x & v2.x, v1.y & v2.y);
}

template <typename T>
requires detail::has_operator_bit_or<T>
constexpr inline tvec<2, T> operator|(const tvec<2, T>& v, T scalar)
{
    return tvec<2, T>(v.x | scalar, v.y | scalar);
}

template <typename T>
requires detail::has_operator_bit_or<T>
constexpr inline tvec<2, T> operator|(T scalar, const tvec<2, T>& v)
{
    return tvec<2, T>(scalar | v.x, scalar | v.y);
}

template <typename T>
requires detail::has_operator_bit_or<T>
constexpr inline tvec<2, T> operator|(const tvec<2, T>& v1, const tvec<2, T>& v2)
{
    return tvec<2, T>(v1.x | v2.x, v1.y | v2.y);
}

template <typename T>
requires detail::has_operator_left_shift<T>
constexpr inline tvec<2, T> operator<<(const tvec<2, T>& v, T scalar)
{
    return tvec<2, T>(v.x << scalar, v.y << scalar);
}

template <typename T>
requires detail::has_operator_left_shift<T>
constexpr inline tvec<2, T> operator<<(T scalar, const tvec<2, T>& v)
{
    return tvec<2, T>(scalar << v.x, scalar << v.y);
}

template <typename T>
requires detail::has_operator_left_shift<T>
constexpr inline tvec<2, T> operator<<(const tvec<2, T>& v1, const tvec<2, T>& v2)
{
    return tvec<2, T>(v1.x << v2.x, v1.y << v2.y);
}

template <typename T>
requires detail::has_operator_right_shift<T>
constexpr inline tvec<2, T> operator>>(const tvec<2, T>& v, T scalar)
{
    return tvec<2, T>(v.x >> scalar, v.y >> scalar);
}

template <typename T>
requires detail::has_operator_right_shift<T>
constexpr inline tvec<2, T> operator>>(T scalar, const tvec<2, T>& v)
{
    return tvec<2, T>(scalar >> v.x, scalar >> v.y);
}

template <typename T>
requires detail::has_operator_right_shift<T>
constexpr inline tvec<2, T> operator>>(const tvec<2, T>& v1, const tvec<2, T>& v2)
{
    return tvec<2, T>(v1.x >> v2.x, v1.y >> v2.y);
}

template <typename T>
EIRIN_ALWAYS_INLINE constexpr bool operator==(const tvec<2, T>& v1, const tvec<2, T>& v2)
{
    return detail::compute_equal<T, std::numeric_limits<T>::is_iec559>::eval(v1.x, v2.x) &&
           detail::compute_equal<T, std::numeric_limits<T>::is_iec559>::eval(v1.y, v2.y);
}
} // namespace eirin

#endif
