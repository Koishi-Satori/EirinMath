#ifndef EIRIN_MATH_VEC_HPP
#define EIRIN_MATH_VEC_HPP

#include <concepts>
#include <cstddef>
#include <eirin/macro.hpp>

namespace eirin
{
namespace detail
{
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

template <std::size_t N, typename T>
struct tvec;

template <std::size_t N, typename T, typename Derived>
struct tvec_base
{
    using size_type = std::size_t;
    using value_type = T;

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

    EIRIN_ALWAYS_INLINE constexpr value_type dot(const Derived& rhs) const noexcept
    {
        value_type res{0};
        Derived tmp(derived());
        for(std::size_t i = 0; i < size(); ++i)
            res += tmp[i] * rhs[i];
        return res;
    }

private:
    Derived& derived() noexcept
    {
        return static_cast<Derived&>(*this);
    }

    const Derived& derived() const noexcept
    {
        return static_cast<const Derived&>(*this);
    }
};

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
    requires(!std::same_as<T, U>)
    constexpr inline tvec& operator=(const tvec<2, U>& rhs) noexcept
    {
        this->x = static_cast<T>(rhs.x);
        this->y = static_cast<T>(rhs.y);
        return *this;
    }

    constexpr inline tvec& operator=(const tvec&& rhs) noexcept
    {
        this->x = static_cast<T>(rhs.x);
        this->y = static_cast<T>(rhs.y);
        return *this;
    }

    template <typename U>
    requires(!std::same_as<T, U>)
    constexpr inline tvec& operator+=(const tvec<2, U>& rhs) noexcept
    {
        this->x += static_cast<T>(rhs.x);
        this->y += static_cast<T>(rhs.y);
        return *this;
    }

    constexpr inline tvec& operator+=(const tvec& rhs) noexcept
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    template <typename U>
    requires(!std::same_as<T, U>)
    constexpr inline tvec& operator-=(const tvec<2, U>& rhs) noexcept
    {
        this->x -= static_cast<T>(rhs.x);
        this->y -= static_cast<T>(rhs.y);
        return *this;
    }

    constexpr inline tvec& operator-=(const tvec& rhs) noexcept
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    template <typename U>
    requires(!std::same_as<T, U>)
    constexpr inline tvec& operator*=(const tvec<2, U>& rhs) noexcept
    {
        this->x *= static_cast<T>(rhs.x);
        this->y *= static_cast<T>(rhs.y);
        return *this;
    }

    constexpr inline tvec& operator*=(const tvec& rhs) noexcept
    {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    template <typename U>
    requires(!std::same_as<T, U>)
    constexpr inline tvec& operator/=(const tvec<2, U>& rhs) noexcept
    {
        this->x /= static_cast<T>(rhs.x);
        this->y /= static_cast<T>(rhs.y);
        return *this;
    }

    constexpr inline tvec& operator/=(const tvec& rhs) noexcept
    {
        x /= rhs.x;
        y /= rhs.y;
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
    requires(!std::same_as<T, U>)
    constexpr inline tvec& operator%=(const tvec<2, U>& rhs) noexcept
    {
        this->x %= static_cast<T>(rhs.x);
        this->y %= static_cast<T>(rhs.y);
        return *this;
    }

    constexpr inline tvec& operator%=(const tvec& rhs) noexcept
    {
        x %= rhs.x;
        y %= rhs.y;
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_xor<U> && (!std::same_as<T, U>)
    constexpr inline tvec& operator^=(const tvec& rhs) noexcept
    {
        x ^= rhs.x;
        y ^= rhs.y;
        return *this;
    }

    template <typename U = T>
    requires detail::has_operator_bit_xor<U>
    constexpr inline tvec& operator^=(const tvec& rhs) noexcept
    {
        x ^= rhs.x;
        y ^= rhs.y;
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_and<U> && (!std::same_as<T, U>)
    constexpr inline tvec& operator&=(const tvec& rhs) noexcept
    {
        x &= rhs.x;
        y &= rhs.y;
        return *this;
    }

    template <typename U = T>
    requires detail::has_operator_bit_and<U>
    constexpr inline tvec& operator&=(const tvec& rhs) noexcept
    {
        x &= rhs.x;
        y &= rhs.y;
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_or<U> && (!std::same_as<T, U>)
    constexpr inline tvec& operator|=(const tvec& rhs) noexcept
    {
        x |= rhs.x;
        y |= rhs.y;
        return *this;
    }

    template <typename U = T>
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
} // namespace eirin

#endif
