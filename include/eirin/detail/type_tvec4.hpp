#ifndef EIRIN_MATH_DETAIL_TYPE_TVEC4_HPP
#define EIRIN_MATH_DETAIL_TYPE_TVEC4_HPP

#pragma once

#include <limits>
#include <eirin/detail/type_tvec.hpp>
#include <eirin/detail/type_tvec2.hpp>
#include <eirin/detail/type_tvec3.hpp>
#include <eirin/detail/compute_vec_rel.hpp>
#include <eirin/detail/vec_swizzle.hpp>

namespace eirin
{
template <typename T>
struct tvec<4, T> : public tvec_base<4, T, tvec<4, T>>
{
    using value_type = T;
    using type = tvec<4, T>;
    using bool_type = tvec<4, bool>;

    T x, y, z, w;


#if EIRIN_VEC_SWIZZLE_ENABLE == EIRIN_ENABLE
    EIRIN_TVEC_SWIZZLE4_4_MEMBERS_DECL(T, x, y, z, w)
    EIRIN_TVEC_SWIZZLE4_3_MEMBERS_DECL(T, x, y, z, w)
    EIRIN_TVEC_SWIZZLE4_2_MEMBERS_DECL(T, x, y, z, w)
#endif

    EIRIN_ALWAYS_INLINE constexpr tvec() = default;
    EIRIN_ALWAYS_INLINE constexpr tvec(const tvec& v) = default;

    EIRIN_ALWAYS_INLINE explicit constexpr tvec(T scalar)
        : x(scalar), y(scalar), z(scalar), w(scalar){};

    EIRIN_ALWAYS_INLINE constexpr tvec(T _x, T _y, T _z, T _w)
        : x(_x), y(_y), z(_z), w(_w){};

    /// Explicit conversions (like GLSL)
    /// Explicit construct from x,y,z,w
    template <typename X, typename Y, typename Z, typename W>
    EIRIN_ALWAYS_INLINE constexpr tvec(X _x, Y _y, Z _z, W _w)
        : x(static_cast<T>(_x)), y(static_cast<T>(_y)), z(static_cast<T>(_z)), w(static_cast<T>(_w)){};
    /// Explicit construct from xy,z,w
    template <typename A, typename B, typename C>
    EIRIN_ALWAYS_INLINE constexpr tvec(const tvec<2, A>& _xy, B _z, C _w)
        : x(static_cast<T>(_xy.x)), y(static_cast<T>(_xy.y)), z(static_cast<T>(_z)), w(static_cast<T>(_w)){};
    /// Explicit construct from x,yz,w
    template <typename A, typename B, typename C>
    EIRIN_ALWAYS_INLINE constexpr tvec(A _x, const tvec<2, B>& _yz, C _w)
        : x(static_cast<T>(_x)), y(static_cast<T>(_yz.x)), z(static_cast<T>(_yz.y)), w(static_cast<T>(_w)){};
    /// Explicit construct from x,y,zw
    template <typename A, typename B, typename C>
    EIRIN_ALWAYS_INLINE constexpr tvec(A _x, B _y, const tvec<2, C>& _zw)
        : x(static_cast<T>(_x)), y(static_cast<T>(_y)), z(static_cast<T>(_zw.x)), w(static_cast<T>(_zw.y)){};
    /// Explicit construct from xy,zw
    template <typename A, typename B>
    EIRIN_ALWAYS_INLINE constexpr tvec(const tvec<2, A>& _xy, const tvec<2, B>& _zw)
        : x(static_cast<T>(_xy.x)), y(static_cast<T>(_xy.y)), z(static_cast<T>(_zw.x)), w(static_cast<T>(_zw.y)){};
    /// Explicit construct from xyz,w
    template <typename A, typename B>
    EIRIN_ALWAYS_INLINE constexpr tvec(const tvec<3, A>& _xyz, B _w)
        : x(static_cast<T>(_xyz.x)), y(static_cast<T>(_xyz.y)), z(static_cast<T>(_xyz.z)), w(static_cast<T>(_w)){};
    /// Explicit construct from x, yzw
    template <typename A, typename B>
    EIRIN_ALWAYS_INLINE constexpr tvec(A _x, const tvec<3, B>& _yzw)
        : x(static_cast<T>(_x)), y(static_cast<T>(_yzw.x)), z(static_cast<T>(_yzw.y)), w(static_cast<T>(_yzw.z)){};
    /// Explicit construct from xyzw
    template <typename U>
    EIRIN_ALWAYS_INLINE constexpr tvec(const tvec<4, U>& v)
        : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)), z(static_cast<T>(v.z)), w(static_cast<T>(v.w)){};

    tvec& operator=(const tvec& other) noexcept = default;

    constexpr inline T& operator[](std::size_t i) noexcept
    {
        switch(i)
        {
        case 3:
            return w;
        case 2:
            return z;
        case 1:
            return y;
        case 0:
            return x;
        default:
            EIRIN_UNREACHABLE;
        }
    }

    constexpr inline const T& operator[](std::size_t i) const noexcept
    {
        switch(i)
        {
        case 3:
            return w;
        case 2:
            return z;
        case 1:
            return y;
        case 0:
            return x;
        default:
            EIRIN_UNREACHABLE;
        }
    }

    template <typename U>
    constexpr inline tvec& operator=(const tvec<4, U>& rhs) noexcept
    {
        this->x = static_cast<T>(rhs.x);
        this->y = static_cast<T>(rhs.y);
        this->z = static_cast<T>(rhs.z);
        this->w = static_cast<T>(rhs.w);
        return *this;
    }

    template <typename U>
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator+=(U scalar) noexcept
    {
        this->x += static_cast<T>(scalar);
        this->y += static_cast<T>(scalar);
        this->z += static_cast<T>(scalar);
        this->w += static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator+=(const tvec<4, U>& rhs) noexcept
    {
        this->x += static_cast<T>(rhs.x);
        this->y += static_cast<T>(rhs.y);
        this->z += static_cast<T>(rhs.z);
        this->w += static_cast<T>(rhs.w);
        return *this;
    }

    template <typename U>
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator-=(U scalar) noexcept
    {
        this->x -= static_cast<T>(scalar);
        this->y -= static_cast<T>(scalar);
        this->z -= static_cast<T>(scalar);
        this->w -= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator-=(const tvec<4, U>& rhs) noexcept
    {
        this->x -= static_cast<T>(rhs.x);
        this->y -= static_cast<T>(rhs.y);
        this->z -= static_cast<T>(rhs.z);
        this->w -= static_cast<T>(rhs.w);
        return *this;
    }

    template <typename U>
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator*=(U scalar) noexcept
    {
        this->x *= static_cast<T>(scalar);
        this->y *= static_cast<T>(scalar);
        this->z *= static_cast<T>(scalar);
        this->w *= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator*=(const tvec<4, U>& rhs) noexcept
    {
        this->x *= static_cast<T>(rhs.x);
        this->y *= static_cast<T>(rhs.y);
        this->z *= static_cast<T>(rhs.z);
        this->w *= static_cast<T>(rhs.w);
        return *this;
    }

    template <typename U>
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator/=(U scalar) noexcept
    {
        this->x /= static_cast<T>(scalar);
        this->y /= static_cast<T>(scalar);
        this->z /= static_cast<T>(scalar);
        this->w /= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator/=(const tvec<4, U>& rhs) noexcept
    {
        this->x /= static_cast<T>(rhs.x);
        this->y /= static_cast<T>(rhs.y);
        this->z /= static_cast<T>(rhs.z);
        this->w /= static_cast<T>(rhs.w);
        return *this;
    }

    constexpr inline tvec& operator++() noexcept
    {
        ++this->x;
        ++this->y;
        ++this->z;
        ++this->w;
        return *this;
    }

    constexpr inline tvec& operator--() noexcept
    {
        --this->x;
        --this->y;
        --this->z;
        --this->w;
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
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator%=(U scalar) noexcept
    {
        this->x %= static_cast<T>(scalar);
        this->y %= static_cast<T>(scalar);
        this->z %= static_cast<T>(scalar);
        this->w %= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    constexpr inline tvec& operator%=(const tvec<4, U>& rhs) noexcept
    {
        this->x %= static_cast<T>(rhs.x);
        this->y %= static_cast<T>(rhs.y);
        this->z %= static_cast<T>(rhs.z);
        this->w %= static_cast<T>(rhs.w);
        return *this;
    }

    template <typename U>
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator^=(U scalar) noexcept
    {
        this->x ^= static_cast<T>(scalar);
        this->y ^= static_cast<T>(scalar);
        this->z ^= static_cast<T>(scalar);
        this->w ^= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_xor<U> && detail::has_operator_bit_xor<T>
    constexpr inline tvec& operator^=(const tvec<4, U>& rhs) noexcept
    {
        this->x ^= static_cast<T>(rhs.x);
        this->y ^= static_cast<T>(rhs.y);
        this->z ^= static_cast<T>(rhs.z);
        this->w ^= static_cast<T>(rhs.w);
        return *this;
    }

    template <typename U>
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator&=(U scalar) noexcept
    {
        this->x &= static_cast<T>(scalar);
        this->y &= static_cast<T>(scalar);
        this->z &= static_cast<T>(scalar);
        this->w &= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_and<U> && detail::has_operator_bit_and<T>
    constexpr inline tvec& operator&=(const tvec<4, U>& rhs) noexcept
    {
        this->x &= static_cast<T>(rhs.x);
        this->y &= static_cast<T>(rhs.y);
        this->z &= static_cast<T>(rhs.z);
        this->w &= static_cast<T>(rhs.w);
        return *this;
    }

    template <typename U>
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator|=(U scalar) noexcept
    {
        this->x |= static_cast<T>(scalar);
        this->y |= static_cast<T>(scalar);
        this->z |= static_cast<T>(scalar);
        this->w |= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    requires detail::has_operator_bit_or<U> && detail::has_operator_bit_or<T>
    constexpr inline tvec& operator|=(const tvec<4, U>& rhs) noexcept
    {
        this->x |= static_cast<T>(rhs.x);
        this->y |= static_cast<T>(rhs.y);
        this->z |= static_cast<T>(rhs.z);
        this->w |= static_cast<T>(rhs.w);
        return *this;
    }

    template <typename U>
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator<<=(U scalar) noexcept
    {
        this->x <<= static_cast<T>(scalar);
        this->y <<= static_cast<T>(scalar);
        this->z <<= static_cast<T>(scalar);
        this->w <<= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    requires detail::has_operator_left_shift<U> && detail::has_operator_left_shift<T>
    constexpr inline tvec& operator<<=(const tvec<4, U>& rhs) noexcept
    {
        this->x <<= static_cast<T>(rhs.x);
        this->y <<= static_cast<T>(rhs.y);
        this->z <<= static_cast<T>(rhs.z);
        this->w <<= static_cast<T>(rhs.w);
        return *this;
    }

    template <typename U>
    EIRIN_REQUIRES_NOT_SWIZZLE_PROXY constexpr inline tvec& operator>>=(U scalar) noexcept
    {
        this->x >>= static_cast<T>(scalar);
        this->y >>= static_cast<T>(scalar);
        this->z >>= static_cast<T>(scalar);
        this->w >>= static_cast<T>(scalar);
        return *this;
    }

    template <typename U>
    requires detail::has_operator_right_shift<U> && detail::has_operator_right_shift<T>
    constexpr inline tvec& operator>>=(const tvec<4, U>& rhs) noexcept
    {
        this->x >>= static_cast<T>(rhs.x);
        this->y >>= static_cast<T>(rhs.y);
        this->z >>= static_cast<T>(rhs.z);
        this->w >>= static_cast<T>(rhs.w);
        return *this;
    }

    template <typename U = T>
    requires detail::has_operator_bit_not<T> && std::same_as<U, T>
    constexpr inline tvec operator~() const noexcept
    {
        tvec res;
        res.x = ~this->x;
        res.y = ~this->y;
        res.z = ~this->z;
        res.w = ~this->w;
        return res;
    }

    constexpr inline bool_type operator&&(const tvec& rhs) noexcept
    {
        bool_type res;
        res.x = this->x && rhs.x;
        res.y = this->y && rhs.y;
        res.z = this->z && rhs.z;
        res.w = this->w && rhs.w;
        return res;
    }

    constexpr inline bool_type operator||(const tvec& rhs) noexcept
    {
        bool_type res;
        res.x = this->x || rhs.x;
        res.y = this->y || rhs.y;
        res.z = this->z || rhs.z;
        res.w = this->w || rhs.w;
        return res;
    }

    /* These functions are defined for conversion from swizzle_proxy to tvec.
       Should contains: +, -, *, /, %, ^, &, |, <<, >>
    */
#if EIRIN_VEC_SWIZZLE_ENABLE == EIRIN_ENABLE
#    define EIRIN_VEC_SWIZZLE_CONVERSION_OP_FUNC(op)                      \
        constexpr inline tvec& operator op## = (const tvec& rhs) noexcept \
        {                                                                 \
            this->x op## = static_cast<T>(rhs.x);                         \
            this->y op## = static_cast<T>(rhs.y);                         \
            this->z op## = static_cast<T>(rhs.z);                         \
            this->w op## = static_cast<T>(rhs.w);                         \
            return *this;                                                 \
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

// unary operators for vec4
template <typename T>
constexpr inline tvec<4, T> operator+(const tvec<4, T>& v)
{
    return v;
}

template <typename T>
constexpr inline tvec<4, T> operator-(const tvec<4, T>& v)
{
    tvec<4, T> res;
    res.x = -v.x;
    res.y = -v.y;
    res.z = -v.z;
    res.w = -v.w;
    return res;
}

// binary operators for vec4
template <typename T>
constexpr inline tvec<4, T> operator+(const tvec<4, T>& v, T scalar)
{
    tvec<4, T> res;
    res.x = v.x + scalar;
    res.y = v.y + scalar;
    res.z = v.z + scalar;
    res.w = v.w + scalar;
    return res;
}

template <typename T>
constexpr inline tvec<4, T> operator+(T scalar, const tvec<4, T>& v)
{
    tvec<4, T> res;
    res.x = scalar + v.x;
    res.y = scalar + v.y;
    res.z = scalar + v.z;
    res.w = scalar + v.w;
    return res;
}

template <typename T>
constexpr inline tvec<4, T> operator-(const tvec<4, T>& v, T scalar)
{
    tvec<4, T> res;
    res.x = v.x - scalar;
    res.y = v.y - scalar;
    res.z = v.z - scalar;
    res.w = v.w - scalar;
    return res;
}

template <typename T>
constexpr inline tvec<4, T> operator-(T scalar, const tvec<4, T>& v)
{
    tvec<4, T> res;
    res.x = scalar - v.x;
    res.y = scalar - v.y;
    res.z = scalar - v.z;
    res.w = scalar - v.w;
    return res;
}

template <typename T>
constexpr inline tvec<4, T> operator*(const tvec<4, T>& v, T scalar)
{
    tvec<4, T> res;
    res.x = v.x * scalar;
    res.y = v.y * scalar;
    res.z = v.z * scalar;
    res.w = v.w * scalar;
    return res;
}

template <typename T>
constexpr inline tvec<4, T> operator*(T scalar, const tvec<4, T>& v)
{
    tvec<4, T> res;
    res.x = scalar * v.x;
    res.y = scalar * v.y;
    res.z = scalar * v.z;
    res.w = scalar * v.w;
    return res;
}

template <typename T>
constexpr inline tvec<4, T> operator/(const tvec<4, T>& v, T scalar)
{
    tvec<4, T> res;
    res.x = v.x / scalar;
    res.y = v.y / scalar;
    res.z = v.z / scalar;
    res.w = v.w / scalar;
    return res;
}

template <typename T>
constexpr inline tvec<4, T> operator/(T scalar, const tvec<4, T>& v)
{
    tvec<4, T> res;
    res.x = scalar / v.x;
    res.y = scalar / v.y;
    res.z = scalar / v.z;
    res.w = scalar / v.w;
    return res;
}

template <typename T>
constexpr inline tvec<4, T> operator%(const tvec<4, T>& v, T scalar)
{
    tvec<4, T> res;
    res.x = v.x % scalar;
    res.y = v.y % scalar;
    res.z = v.z % scalar;
    res.w = v.w % scalar;
    return res;
}

template <typename T>
constexpr inline tvec<4, T> operator%(T scalar, const tvec<4, T>& v)
{
    tvec<4, T> res;
    res.x = scalar % v.x;
    res.y = scalar % v.y;
    res.z = scalar % v.z;
    res.w = scalar % v.w;
    return res;
}

template <typename T>
requires detail::has_operator_bit_xor<T>
constexpr inline tvec<4, T> operator^(const tvec<4, T>& v, T scalar)
{
    tvec<4, T> res;
    res.x = v.x ^ scalar;
    res.y = v.y ^ scalar;
    res.z = v.z ^ scalar;
    res.w = v.w ^ scalar;
    return res;
}

template <typename T>
requires detail::has_operator_bit_xor<T>
constexpr inline tvec<4, T> operator^(T scalar, const tvec<4, T>& v)
{
    tvec<4, T> res;
    res.x = scalar ^ v.x;
    res.y = scalar ^ v.y;
    res.z = scalar ^ v.z;
    res.w = scalar ^ v.w;
    return res;
}

template <typename T>
requires detail::has_operator_bit_and<T>
constexpr inline tvec<4, T> operator&(const tvec<4, T>& v, T scalar)
{
    tvec<4, T> res;
    res.x = v.x & scalar;
    res.y = v.y & scalar;
    res.z = v.z & scalar;
    res.w = v.w & scalar;
    return res;
}

template <typename T>
requires detail::has_operator_bit_and<T>
constexpr inline tvec<4, T> operator&(T scalar, const tvec<4, T>& v)
{
    tvec<4, T> res;
    res.x = scalar & v.x;
    res.y = scalar & v.y;
    res.z = scalar & v.z;
    res.w = scalar & v.w;
    return res;
}

template <typename T>
requires detail::has_operator_bit_or<T>
constexpr inline tvec<4, T> operator|(const tvec<4, T>& v, T scalar)
{
    tvec<4, T> res;
    res.x = v.x | scalar;
    res.y = v.y | scalar;
    res.z = v.z | scalar;
    res.w = v.w | scalar;
    return res;
}

template <typename T>
requires detail::has_operator_bit_or<T>
constexpr inline tvec<4, T> operator|(T scalar, const tvec<4, T>& v)
{
    tvec<4, T> res;
    res.x = scalar | v.x;
    res.y = scalar | v.y;
    res.z = scalar | v.z;
    res.w = scalar | v.w;
    return res;
}

template <typename T>
requires detail::has_operator_left_shift<T>
constexpr inline tvec<4, T> operator<<(const tvec<4, T>& v, T scalar)
{
    tvec<4, T> res;
    res.x = v.x << scalar;
    res.y = v.y << scalar;
    res.z = v.z << scalar;
    res.w = v.w << scalar;
    return res;
}

template <typename T>
requires detail::has_operator_left_shift<T>
constexpr inline tvec<4, T> operator<<(T scalar, const tvec<4, T>& v)
{
    tvec<4, T> res;
    res.x = scalar << v.x;
    res.y = scalar << v.y;
    res.z = scalar << v.z;
    res.w = scalar << v.w;
    return res;
}

template <typename T>
requires detail::has_operator_right_shift<T>
constexpr inline tvec<4, T> operator>>(const tvec<4, T>& v, T scalar)
{
    tvec<4, T> res;
    res.x = v.x >> scalar;
    res.y = v.y >> scalar;
    res.z = v.z >> scalar;
    res.w = v.w >> scalar;
    return res;
}

template <typename T>
requires detail::has_operator_right_shift<T>
constexpr inline tvec<4, T> operator>>(T scalar, const tvec<4, T>& v)
{
    tvec<4, T> res;
    res.x = scalar >> v.x;
    res.y = scalar >> v.y;
    res.z = scalar >> v.z;
    res.w = scalar >> v.w;
    return res;
}

template <typename T>
EIRIN_ALWAYS_INLINE constexpr bool operator==(const tvec<4, T>& v1, const tvec<4, T>& v2)
{
    return detail::compute_equal<T, std::numeric_limits<T>::is_iec559>::eval(v1.x, v2.x) &&
           detail::compute_equal<T, std::numeric_limits<T>::is_iec559>::eval(v1.y, v2.y) &&
           detail::compute_equal<T, std::numeric_limits<T>::is_iec559>::eval(v1.z, v2.z) &&
           detail::compute_equal<T, std::numeric_limits<T>::is_iec559>::eval(v1.w, v2.w);
}
} // namespace eirin

#endif
