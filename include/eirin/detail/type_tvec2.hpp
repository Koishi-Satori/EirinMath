#ifndef EIRIN_MATH_DETAIL_TYPE_TVEC2_HPP
#define EIRIN_MATH_DETAIL_TYPE_TVEC2_HPP

#pragma once

#include <limits>
#include <eirin/detail/type_tvec.hpp>
#include <eirin/detail/type_tvec3.hpp>
#include <eirin/detail/type_tvec4.hpp>
#include <eirin/detail/compute_vec_rel.hpp>
#include <eirin/detail/vec_swizzle.hpp>

namespace eirin
{
template <typename T>
struct tvec<2, T> : public tvec_base<2, T, tvec<2, T>>
{
    using value_type = T;
    using type = tvec<2, T>;
    using bool_type = tvec<2, bool>;

    T x, y;

    EIRIN_ALWAYS_INLINE constexpr tvec() = default;
    EIRIN_ALWAYS_INLINE constexpr tvec(const tvec& v) = default;
    EIRIN_ALWAYS_INLINE explicit constexpr tvec(T scalar)
        : x(scalar), y(scalar){};
    EIRIN_ALWAYS_INLINE constexpr tvec(T _x, T _y)
        : x(_x), y(_y){};

#if EIRIN_VEC_SWIZZLE_ENABLE == EIRIN_ENABLE
    EIRIN_TVEC_SWIZZLE2_4_MEMBERS_DECL(T, x, y)
    EIRIN_TVEC_SWIZZLE2_3_MEMBERS_DECL(T, x, y)
    EIRIN_TVEC_SWIZZLE2_2_MEMBERS_DECL(T, x, y)
#endif

    /// Explicit conversions (like GLSL)
    /// Explicit construct from x,y
    template <typename A, typename B>
    EIRIN_ALWAYS_INLINE constexpr tvec(A _x, B _y)
        : x(static_cast<T>(_x)), y(static_cast<T>(_y)){};
    /// Explicit construct from xy
    template <typename A>
    EIRIN_ALWAYS_INLINE constexpr tvec(tvec<2, A> _xy)
        : x(static_cast<T>(_xy.x)), y(static_cast<T>(_xy.y)){};
    /// Explicit construct from xyz
    template <typename A>
    EIRIN_ALWAYS_INLINE constexpr tvec(tvec<3, A> _xyz)
        : x(static_cast<T>(_xyz.x)), y(static_cast<T>(_xyz.y)){};
    /// Explicit construct from xyzw
    template <typename A>
    EIRIN_ALWAYS_INLINE constexpr tvec(tvec<4, A> _xyzw)
        : x(static_cast<T>(_xyzw.x)), y(static_cast<T>(_xyzw.y)){};

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
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator+=(U scalar) noexcept
    {
        this->x += static_cast<T>(scalar);
        this->y += static_cast<T>(scalar);
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
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator-=(U scalar) noexcept
    {
        this->x -= static_cast<T>(scalar);
        this->y -= static_cast<T>(scalar);
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
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator*=(U scalar) noexcept
    {
        this->x *= static_cast<T>(scalar);
        this->y *= static_cast<T>(scalar);
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
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator/=(U scalar) noexcept
    {
        this->x /= static_cast<T>(scalar);
        this->y /= static_cast<T>(scalar);
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
        --this->x;
        --this->y;
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
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec operator%=(U scalar) noexcept
    {
        this->x %= static_cast<T>(scalar);
        this->y %= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator%=(const tvec<2, U>& rhs) noexcept
    {
        this->x %= static_cast<T>(rhs.x);
        this->y %= static_cast<T>(rhs.y);
        return *this;
    }

    template <typename U>
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator^=(U scalar) noexcept
    {
        this->x ^= static_cast<T>(scalar);
        this->y ^= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_xor<U> && detail::has_operator_bit_xor<T>
    constexpr inline tvec& operator^=(const tvec<2, U>& rhs) noexcept
    {
        this->x ^= static_cast<T>(rhs.x);
        this->y ^= static_cast<T>(rhs.y);
        return *this;
    }

    template <typename U>
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator&=(U scalar) noexcept
    {
        this->x &= static_cast<T>(scalar);
        this->y &= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_and<U> && detail::has_operator_bit_and<T>
    constexpr inline tvec& operator&=(const tvec<2, U>& rhs) noexcept
    {
        this->x &= static_cast<T>(rhs.x);
        this->y &= static_cast<T>(rhs.y);
        return *this;
    }

    template <typename U>
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator|=(U scalar) noexcept
    {
        this->x |= static_cast<T>(scalar);
        this->y |= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_or<U> && detail::has_operator_bit_or<T>
    constexpr inline tvec& operator|=(const tvec<2, U>& rhs) noexcept
    {
        this->x |= static_cast<T>(rhs.x);
        this->y |= static_cast<T>(rhs.y);
        return *this;
    }

    template <typename U>
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator<<=(U scalar) noexcept
    {
        this->x <<= static_cast<T>(scalar);
        this->y <<= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    requires detail::has_operator_left_shift<U> && detail::has_operator_left_shift<T>
    constexpr inline tvec& operator<<=(const tvec<2, U>& rhs) noexcept
    {
        this->x <<= static_cast<T>(rhs.x);
        this->y <<= static_cast<T>(rhs.y);
        return *this;
    }

    template <typename U>
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator>>=(U scalar) noexcept
    {
        this->x >>= static_cast<T>(scalar);
        this->y >>= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    requires detail::has_operator_right_shift<U> && detail::has_operator_right_shift<T>
    constexpr inline tvec& operator>>=(const tvec<2, U>& rhs) noexcept
    {
        this->x >>= static_cast<T>(rhs.x);
        this->y >>= static_cast<T>(rhs.y);
        return *this;
    }

    template <typename U = T>
    requires detail::has_operator_bit_not<T> && std::same_as<U, T>
    constexpr inline tvec operator~() const noexcept
    {
        return tvec{~this->x, ~this->y};
    }

    EIRIN_ALWAYS_INLINE constexpr value_type cross(const tvec& rhs) const noexcept
    {
        auto res = x * rhs.y;
        res -= y * rhs.x;
        return res;
    }

    constexpr inline bool_type operator&&(const tvec& rhs) noexcept
    {
        return bool_type{this->x && rhs.x, this->y && rhs.y};
    }

    constexpr inline bool_type operator||(const tvec& rhs) noexcept
    {
        return bool_type{this->x || rhs.x, this->y || rhs.y};
    }

    /* These functions are defined for conversion from swizzle_proxy to tvec. 
       Should contains: +, -, *, /, %, ^, &, |, <<, >>
    */
#if EIRIN_VEC_SWIZZLE_ENABLE == EIRIN_ENABLE
#    define EIRIN_VEC_SWIZZLE_CONVERSION_OP_FUNC(op)                     \
        constexpr inline tvec& operator op##=(const tvec & rhs) noexcept \
        {                                                                \
            this->x op## = static_cast<T>(rhs.x);                        \
            this->y op## = static_cast<T>(rhs.y);                        \
            return *this;                                                \
        }
#    define EIRIN_VEC_SWIZZLE_CONVERSION_OP_FUNC_WITH_REQUIRE(op, name) \
        template <typename U = T>                                       \
        requires detail::has_operator_                                  \
        ##name<T>&& std::same_as<U, T>                                  \
            EIRIN_VEC_SWIZZLE_CONVERSION_OP_FUNC(op)

    EIRIN_VEC_SWIZZLE_CONVERSION_OP_FUNC(+)
    EIRIN_VEC_SWIZZLE_CONVERSION_OP_FUNC(-)
    EIRIN_VEC_SWIZZLE_CONVERSION_OP_FUNC(*)
    EIRIN_VEC_SWIZZLE_CONVERSION_OP_FUNC(/)
    EIRIN_VEC_SWIZZLE_CONVERSION_OP_FUNC(%)
    EIRIN_VEC_SWIZZLE_CONVERSION_OP_FUNC_WITH_REQUIRE(^, bit_xor)
    EIRIN_VEC_SWIZZLE_CONVERSION_OP_FUNC_WITH_REQUIRE(&, bit_and)
    EIRIN_VEC_SWIZZLE_CONVERSION_OP_FUNC_WITH_REQUIRE(|, bit_or)
    EIRIN_VEC_SWIZZLE_CONVERSION_OP_FUNC_WITH_REQUIRE(<<, left_shift)
    EIRIN_VEC_SWIZZLE_CONVERSION_OP_FUNC_WITH_REQUIRE(>>, right_shift)

#    undef EIRIN_VEC_SWIZZLE_CONVERSION_OP_FUNC
#    undef EIRIN_VEC_SWIZZLE_CONVERSION_OP_FUNC_WITH_REQUIRE
#endif
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
EIRIN_ALWAYS_INLINE constexpr bool operator==(const tvec<2, T>& v1, const tvec<2, T>& v2)
{
    return detail::compute_equal<T, std::numeric_limits<T>::is_iec559>::eval(v1.x, v2.x) &&
           detail::compute_equal<T, std::numeric_limits<T>::is_iec559>::eval(v1.y, v2.y);
}
} // namespace eirin

#endif
