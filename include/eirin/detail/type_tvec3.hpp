#ifndef EIRIN_MATH_DETAIL_TYPE_TVEC3_HPP
#define EIRIN_MATH_DETAIL_TYPE_TVEC3_HPP

#pragma once

#include <limits>
#include <eirin/detail/type_tvec.hpp>
#include <eirin/detail/type_tvec2.hpp>
#include <eirin/detail/type_tvec4.hpp>
#include <eirin/detail/compute_vec_rel.hpp>

namespace eirin
{
template <typename T>
struct tvec<3, T> : public tvec_base<3, T, tvec<3, T>>
{
    using value_type = T;
    using type = tvec<3, T>;
    using bool_type = tvec<3, bool>;

    T x, y, z;

    EIRIN_ALWAYS_INLINE constexpr tvec() = default;
    EIRIN_ALWAYS_INLINE constexpr tvec(const tvec& v) = default;

    EIRIN_ALWAYS_INLINE explicit constexpr tvec(T scalar)
        : x(scalar), y(scalar), z(scalar){};

    EIRIN_ALWAYS_INLINE constexpr tvec(T _x, T _y, T _z)
        : x(_x), y(_y), z(_z){};

    /// Explicit conversions (like GLSL)
    /// Explicit construct from x,y,z
    template <typename A, typename B, typename C>
    EIRIN_ALWAYS_INLINE constexpr tvec(A _x, B _y, C _z)
        : x(static_cast<T>(_x)), y(static_cast<T>(_y)), z(static_cast<T>(_z)){};
    /// Explicit construct from xy,z
    template <typename A, typename B>
    EIRIN_ALWAYS_INLINE constexpr tvec(tvec<2, A> _xy, B _z)
        : x(static_cast<T>(_xy.x)), y(static_cast<T>(_xy.y)), z(static_cast<T>(_z)){};
    /// Explicit construct from x,yz
    template <typename A, typename B>
    EIRIN_ALWAYS_INLINE constexpr tvec(A _x, tvec<2, B> _yz)
        : x(static_cast<T>(_x)), y(static_cast<T>(_yz.x)), z(static_cast<T>(_yz.y)){};
    /// Explicit construct from xyz
    template <typename A>
    EIRIN_ALWAYS_INLINE constexpr tvec(tvec<3, A> _xyz)
        : x(static_cast<T>(_xyz.x)), y(static_cast<T>(_xyz.y)), z(static_cast<T>(_xyz.z)){};
    /// Explicit construct from xyzw
    template <typename A>
    EIRIN_ALWAYS_INLINE constexpr tvec(tvec<4, A> _xyzw)
        : x(static_cast<T>(_xyzw.x)), y(static_cast<T>(_xyzw.y)), z(static_cast<T>(_xyzw.z)){};

    tvec& operator=(const tvec& other) noexcept = default;

    constexpr inline T& operator[](std::size_t i) noexcept
    {
        switch(i)
        {
        default:
        case 2:
            return z;
        case 1:
            return y;
        case 0:
            return x;
        }
    }

    constexpr inline const T& operator[](std::size_t i) const noexcept
    {
        switch(i)
        {
        default:
        case 2:
            return z;
        case 1:
            return y;
        case 0:
            return x;
        }
    }

    template <typename U>
    constexpr inline tvec& operator=(const tvec<3, U>& rhs) noexcept
    {
        this->x = static_cast<T>(rhs.x);
        this->y = static_cast<T>(rhs.y);
        this->z = static_cast<T>(rhs.z);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator+=(U scalar) noexcept
    {
        this->x += static_cast<T>(scalar);
        this->y += static_cast<T>(scalar);
        this->z += static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator+=(const tvec<3, U>& rhs) noexcept
    {
        this->x += static_cast<T>(rhs.x);
        this->y += static_cast<T>(rhs.y);
        this->z += static_cast<T>(rhs.z);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator-=(U scalar) noexcept
    {
        this->x -= static_cast<T>(scalar);
        this->y -= static_cast<T>(scalar);
        this->z -= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator-=(const tvec<3, U>& rhs) noexcept
    {
        this->x -= static_cast<T>(rhs.x);
        this->y -= static_cast<T>(rhs.y);
        this->z -= static_cast<T>(rhs.z);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator*=(U scalar) noexcept
    {
        this->x *= static_cast<T>(scalar);
        this->y *= static_cast<T>(scalar);
        this->z *= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator*=(const tvec<3, U>& rhs) noexcept
    {
        this->x *= static_cast<T>(rhs.x);
        this->y *= static_cast<T>(rhs.y);
        this->z *= static_cast<T>(rhs.z);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator/=(U scalar) noexcept
    {
        this->x /= static_cast<T>(scalar);
        this->y /= static_cast<T>(scalar);
        this->z /= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator/=(const tvec<3, U>& rhs) noexcept
    {
        this->x /= static_cast<T>(rhs.x);
        this->y /= static_cast<T>(rhs.y);
        this->z /= static_cast<T>(rhs.z);
        return *this;
    }

    constexpr inline tvec& operator++() noexcept
    {
        ++this->x;
        ++this->y;
        ++this->z;
        return *this;
    }

    constexpr inline tvec& operator--() noexcept
    {
        --this->x;
        --this->y;
        --this->z;
        return *this;
    }

    constexpr inline tvec operator++(int) noexcept
    {
        tvec res(*this);
        ++*this;
        return res;
    }

    constexpr inline tvec operator--(int) noexcept
    {
        tvec res(*this);
        --*this;
        return res;
    }

    template <typename U>
    constexpr inline tvec& operator%=(U scalar) noexcept
    {
        this->x %= static_cast<T>(scalar);
        this->y %= static_cast<T>(scalar);
        this->z %= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator%=(const tvec<3, U>& rhs) noexcept
    {
        this->x %= static_cast<T>(rhs.x);
        this->y %= static_cast<T>(rhs.y);
        this->z %= static_cast<T>(rhs.z);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator^=(U scalar) noexcept
    {
        this->x ^= static_cast<T>(scalar);
        this->y ^= static_cast<T>(scalar);
        this->z ^= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_xor<U> && detail::has_operator_bit_xor<T>
    constexpr inline tvec& operator^=(const tvec<3, U>& rhs) noexcept
    {
        this->x ^= rhs.x;
        this->y ^= rhs.y;
        this->z ^= rhs.z;
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator&=(U scalar) noexcept
    {
        this->x &= static_cast<T>(scalar);
        this->y &= static_cast<T>(scalar);
        this->z &= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_and<U> && detail::has_operator_bit_xor<T>
    constexpr inline tvec& operator&=(const tvec<3, U>& rhs) noexcept
    {
        this->x &= rhs.x;
        this->y &= rhs.y;
        this->z &= rhs.z;
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator|=(U scalar) noexcept
    {
        this->x |= static_cast<T>(scalar);
        this->y |= static_cast<T>(scalar);
        this->z |= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_or<U> && detail::has_operator_bit_xor<T>
    constexpr inline tvec& operator|=(const tvec<3, U>& rhs) noexcept
    {
        this->x |= rhs.x;
        this->y |= rhs.y;
        this->z |= rhs.z;
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator<<=(U scalar) noexcept
    {
        this->x <<= static_cast<T>(scalar);
        this->y <<= static_cast<T>(scalar);
        this->z <<= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_or<U> && detail::has_operator_bit_xor<T>
    constexpr inline tvec& operator<<=(const tvec<3, U>& rhs) noexcept
    {
        this->x <<= rhs.x;
        this->y <<= rhs.y;
        this->z <<= rhs.z;
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator>>=(U scalar) noexcept
    {
        this->x >>= static_cast<T>(scalar);
        this->y >>= static_cast<T>(scalar);
        this->z >>= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_or<U> && detail::has_operator_bit_xor<T>
    constexpr inline tvec& operator>>=(const tvec<3, U>& rhs) noexcept
    {
        this->x >>= rhs.x;
        this->y >>= rhs.y;
        this->z >>= rhs.z;
        return *this;
    }

    template <typename U = T>
    requires detail::has_operator_bit_not<T> && std::same_as<U, T>
    constexpr inline tvec operator~() noexcept
    {
        return tvec{~this->x, ~this->y, ~this->z};
    }

    EIRIN_ALWAYS_INLINE constexpr tvec cross(const tvec& rhs) const noexcept
    {
        return tvec(
            y * rhs.z - z * rhs.y,
            z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x
        );
    }

    constexpr inline bool_type operator&&(const tvec& rhs) noexcept
    {
        return bool_type{this->x && rhs.x, this->y && rhs.y, this->z && rhs.z};
    }

    constexpr inline bool_type operator||(const tvec& rhs) noexcept
    {
        return bool_type{this->x || rhs.x, this->y || rhs.y, this->z || rhs.z};
    }
};

// unary operators for vec3
template <typename T>
constexpr inline tvec<3, T> operator+(const tvec<3, T>& v)
{
    return v;
}

template <typename T>
constexpr inline tvec<3, T> operator-(const tvec<3, T>& v)
{
    return tvec<3, T>(-v.x, -v.y, -v.z);
}

// binary operators for vec3

template <typename T>
constexpr inline tvec<3, T> operator+(const tvec<3, T>& v, T scalar)
{
    return tvec<3, T>(v.x + scalar, v.y + scalar, v.z + scalar);
}

template <typename T>
constexpr inline tvec<3, T> operator+(T scalar, const tvec<3, T>& v)
{
    return tvec<3, T>(scalar + v.x, scalar + v.y, scalar + v.z);
}

template <typename T>
constexpr inline tvec<3, T> operator-(const tvec<3, T>& v, T scalar)
{
    return tvec<3, T>(v.x - scalar, v.y - scalar, v.z - scalar);
}

template <typename T>
constexpr inline tvec<3, T> operator-(T scalar, const tvec<3, T>& v)
{
    return tvec<3, T>(scalar - v.x, scalar - v.y, scalar - v.z);
}

template <typename T>
constexpr inline tvec<3, T> operator*(const tvec<3, T>& v, T scalar)
{
    return tvec<3, T>(v.x * scalar, v.y * scalar, v.z * scalar);
}

template <typename T>
constexpr inline tvec<3, T> operator*(T scalar, const tvec<3, T>& v)
{
    return tvec<3, T>(scalar * v.x, scalar * v.y, scalar * v.z);
}

template <typename T>
constexpr inline tvec<3, T> operator/(const tvec<3, T>& v, T scalar)
{
    return tvec<3, T>(v.x / scalar, v.y / scalar, v.z / scalar);
}

template <typename T>
constexpr inline tvec<3, T> operator/(T scalar, const tvec<3, T>& v)
{
    return tvec<3, T>(scalar / v.x, scalar / v.y, scalar / v.z);
}

template <typename T>
constexpr inline tvec<3, T> operator%(const tvec<3, T>& v, T scalar)
{
    return tvec<3, T>(v.x % scalar, v.y % scalar, v.z % scalar);
}

template <typename T>
constexpr inline tvec<3, T> operator%(T scalar, const tvec<3, T>& v)
{
    return tvec<3, T>(scalar % v.x, scalar % v.y, scalar % v.z);
}

template <typename T>
requires detail::has_operator_bit_xor<T>
constexpr inline tvec<3, T> operator^(const tvec<3, T>& v, T scalar)
{
    return tvec<3, T>(v.x ^ scalar, v.y ^ scalar, v.z ^ scalar);
}

template <typename T>
requires detail::has_operator_bit_xor<T>
constexpr inline tvec<3, T> operator^(T scalar, const tvec<3, T>& v)
{
    return tvec<3, T>(scalar ^ v.x, scalar ^ v.y, scalar ^ v.z);
}

template <typename T>
requires detail::has_operator_bit_and<T>
constexpr inline tvec<3, T> operator&(const tvec<3, T>& v, T scalar)
{
    return tvec<3, T>(v.x & scalar, v.y & scalar, v.z & scalar);
}

template <typename T>
requires detail::has_operator_bit_and<T>
constexpr inline tvec<3, T> operator&(T scalar, const tvec<3, T>& v)
{
    return tvec<3, T>(scalar & v.x, scalar & v.y, scalar & v.z);
}

template <typename T>
requires detail::has_operator_bit_or<T>
constexpr inline tvec<3, T> operator|(const tvec<3, T>& v, T scalar)
{
    return tvec<3, T>(v.x | scalar, v.y | scalar, v.z | scalar);
}

template <typename T>
requires detail::has_operator_bit_or<T>
constexpr inline tvec<3, T> operator|(T scalar, const tvec<3, T>& v)
{
    return tvec<3, T>(scalar | v.x, scalar | v.y, scalar | v.z);
}

template <typename T>
requires detail::has_operator_left_shift<T>
constexpr inline tvec<3, T> operator<<(const tvec<3, T>& v, T scalar)
{
    return tvec<3, T>(v.x << scalar, v.y << scalar, v.z << scalar);
}

template <typename T>
requires detail::has_operator_left_shift<T>
constexpr inline tvec<3, T> operator<<(T scalar, const tvec<3, T>& v)
{
    return tvec<3, T>(scalar << v.x, scalar << v.y, scalar << v.z);
}

template <typename T>
requires detail::has_operator_right_shift<T>
constexpr inline tvec<3, T> operator>>(const tvec<3, T>& v, T scalar)
{
    return tvec<3, T>(v.x >> scalar, v.y >> scalar, v.z >> scalar);
}

template <typename T>
requires detail::has_operator_right_shift<T>
constexpr inline tvec<3, T> operator>>(T scalar, const tvec<3, T>& v)
{
    return tvec<3, T>(scalar >> v.x, scalar >> v.y, scalar >> v.z);
}

template <typename T>
EIRIN_ALWAYS_INLINE constexpr bool operator==(const tvec<3, T>& v1, const tvec<3, T>& v2)
{
    return detail::compute_equal<T, std::numeric_limits<T>::is_iec559>::eval(v1.x, v2.x) &&
           detail::compute_equal<T, std::numeric_limits<T>::is_iec559>::eval(v1.y, v2.y) &&
           detail::compute_equal<T, std::numeric_limits<T>::is_iec559>::eval(v1.z, v2.z);
}
} // namespace eirin

#endif
