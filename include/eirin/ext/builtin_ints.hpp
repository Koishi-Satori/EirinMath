#ifndef EIRIN_MATH_EXT_BUILTIN_INTS_HPP
#define EIRIN_MATH_EXT_BUILTIN_INTS_HPP

#pragma once

#include <cstdint>
#include <concepts>
#include <limits>
#include <type_traits>
#include "../macro.hpp"
#include "../detail/type_traits_impl.hpp"

#if defined(_MSC_VER)
#    include <intrin.h>
#endif

#define EIRIN_MATH_HAS_INT128_EXT

namespace eirin::ext
{
namespace detail
{
    EIRIN_ALWAYS_INLINE constexpr uint64_t umul128(uint64_t a, uint64_t b, uint64_t& hi) noexcept
    {
#if defined(_MSC_VER) && defined(_M_X64)
        if(!std::is_constant_evaluated())
            return _umul128(a, b, &hi);
#endif
#if defined(__SIZEOF_INT128__)
        if(!std::is_constant_evaluated())
        {
            const unsigned __int128 p = static_cast<unsigned __int128>(a) * b;
            hi = static_cast<uint64_t>(p >> 64);
            return static_cast<uint64_t>(p);
        }
#endif
        const uint64_t a0 = static_cast<uint32_t>(a);
        const uint64_t a1 = a >> 32;
        const uint64_t b0 = static_cast<uint32_t>(b);
        const uint64_t b1 = b >> 32;
        const uint64_t p00 = a0 * b0;
        const uint64_t p01 = a0 * b1;
        const uint64_t p10 = a1 * b0;
        const uint64_t p11 = a1 * b1;
        const uint64_t mid = (p00 >> 32) + static_cast<uint32_t>(p01) + static_cast<uint32_t>(p10);
        hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
        return (mid << 32) | static_cast<uint32_t>(p00);
    }

    EIRIN_ALWAYS_INLINE constexpr void udivmod128(
        uint64_t alo, uint64_t ahi,
        uint64_t blo, uint64_t bhi,
        uint64_t& qlo, uint64_t& qhi,
        uint64_t& rlo, uint64_t& rhi
    ) noexcept
    {
        qlo = qhi = rlo = rhi = 0;
        if(blo == 0 && bhi == 0)
            return;

        if(bhi == 0 && ahi < blo)
        {
#if defined(_MSC_VER) && defined(_M_X64)
            if(!std::is_constant_evaluated())
            {
                qlo = _udiv128(ahi, alo, blo, &rlo);
                return;
            }
#endif
#if defined(__SIZEOF_INT128__)
            if(!std::is_constant_evaluated())
            {
                const unsigned __int128 v = (static_cast<unsigned __int128>(ahi) << 64) | alo;
                qlo = static_cast<uint64_t>(v / blo);
                rlo = static_cast<uint64_t>(v % blo);
                return;
            }
#endif
        }

        for(int i = 127; i >= 0; --i)
        {
            // r = r * 2 + bit i of a
            rhi = (rhi << 1) | (rlo >> 63);
            rlo = (rlo << 1) | ((i >= 64 ? (ahi >> (i - 64)) : (alo >> i)) & 1u);
            // if r >= b: r -= b and set bit i of the quotient
            if(rhi > bhi || (rhi == bhi && rlo >= blo))
            {
                const uint64_t new_rlo = rlo - blo;
                rhi = rhi - bhi - (rlo < blo ? 1u : 0u);
                rlo = new_rlo;
                if(i >= 64)
                    qhi |= uint64_t(1) << (i - 64);
                else
                    qlo |= uint64_t(1) << i;
            }
        }
    }
} // namespace detail

class int128
{
public:
    constexpr int128() noexcept = default;
    int128(const int128&) noexcept = default;
    int128& operator=(const int128&) noexcept = default;

    // construct from integral types with bit_width <= 64.
    // only set lo bits, and hi bits are -1 or 0 (if val >= 0).
    template <std::integral T>
    requires(sizeof(T) <= 8)
    constexpr int128(T val) noexcept
        : high(val < 0 ? -1 : 0), low(static_cast<uint64_t>(val)){};

    template <std::floating_point T>
    requires(sizeof(T) <= 8)
    constexpr explicit int128(T val) noexcept
        : high(0), low(0)
    {
        const int64_t ipart = static_cast<int64_t>(val);
        high = val < 0 ? -1 : 0;
        low = static_cast<uint64_t>(ipart);
    }

    // construct from lo+hi
    template <std::integral T>
    requires(sizeof(T) <= 8)
    constexpr explicit int128(T high, T low) noexcept
        : high(static_cast<int64_t>(high)), low(static_cast<uint64_t>(low)){};

    EIRIN_ALWAYS_INLINE constexpr int64_t high_bits() const noexcept
    {
        return high;
    }

    EIRIN_ALWAYS_INLINE constexpr int64_t low_bits() const noexcept
    {
        return static_cast<int64_t>(low);
    }

    template <std::integral T>
    requires(sizeof(T) <= 8)
    constexpr explicit operator T() const noexcept
    {
        // low 64 bits of the two's-complement value
        return static_cast<T>(low);
    }

    constexpr explicit operator bool() const noexcept
    {
        return (low | static_cast<uint64_t>(high)) != 0;
    }

    template <std::floating_point T>
    constexpr explicit operator T() const noexcept
    {
        return static_cast<T>(
            static_cast<long double>(high) * 18446744073709551616.0L + static_cast<long double>(low)
        );
    }

    /* arithmetic */

    friend constexpr int128 operator+(int128 a, int128 b) noexcept
    {
        const uint64_t lo = a.low + b.low;
        const uint64_t carry = lo < a.low ? 1u : 0u;
        return from_words(lo, a.high + b.high + static_cast<int64_t>(carry));
    }

    friend constexpr int128 operator-(int128 a, int128 b) noexcept
    {
        const uint64_t lo = a.low - b.low;
        const uint64_t borrow = a.low < b.low ? 1u : 0u;
        return from_words(lo, a.high - b.high - static_cast<int64_t>(borrow));
    }

    friend constexpr int128 operator-(int128 a) noexcept
    {
        const uint64_t lo = uint64_t(0) - a.low;
        const int64_t hi = int64_t(0) - a.high - (a.low != 0 ? 1 : 0);
        return from_words(lo, hi);
    }

    friend constexpr int128 operator*(int128 a, int128 b) noexcept
    {
        uint64_t hi = 0;
        const uint64_t lo = detail::umul128(a.low, b.low, hi);
        hi += static_cast<uint64_t>(a.high) * b.low;
        hi += a.low * static_cast<uint64_t>(b.high);
        return from_words(lo, static_cast<int64_t>(hi));
    }

    friend constexpr int128 operator/(int128 a, int128 b) noexcept
    {
        const bool aneg = a.high < 0;
        const bool bneg = b.high < 0;
        const int128 ma = aneg ? -a : a;
        const int128 mb = bneg ? -b : b;
        uint64_t qlo = 0, qhi = 0, rlo = 0, rhi = 0;
        detail::udivmod128(
            ma.low, static_cast<uint64_t>(ma.high),
            mb.low, static_cast<uint64_t>(mb.high),
            qlo, qhi, rlo, rhi
        );
        const int128 q = from_words(qlo, static_cast<int64_t>(qhi));
        return (aneg != bneg) ? -q : q;
    }

    friend constexpr int128 operator%(int128 a, int128 b) noexcept
    {
        const bool aneg = a.high < 0;
        const bool bneg = b.high < 0;
        const int128 ma = aneg ? -a : a;
        const int128 mb = bneg ? -b : b;
        uint64_t qlo = 0, qhi = 0, rlo = 0, rhi = 0;
        detail::udivmod128(
            ma.low, static_cast<uint64_t>(ma.high),
            mb.low, static_cast<uint64_t>(mb.high),
            qlo, qhi, rlo, rhi
        );
        const int128 r = from_words(rlo, static_cast<int64_t>(rhi));
        return aneg ? -r : r;
    }

    /* shifts */

    template <std::integral T>
    friend constexpr int128 operator<<(int128 a, T n) noexcept
    {
        if(n < 0)
            return a >> static_cast<T>(-n);
        const uint64_t s = static_cast<uint64_t>(n);
        if(s >= 128)
            return from_words(0, 0);
        if(s == 0)
            return a;
        if(s >= 64)
            return from_words(0, static_cast<int64_t>(a.low << (s - 64)));
        const uint64_t hi = (static_cast<uint64_t>(a.high) << s) | (a.low >> (64 - s));
        return from_words(a.low << s, static_cast<int64_t>(hi));
    }

    template <std::integral T>
    friend constexpr int128 operator>>(int128 a, T n) noexcept
    {
        if(n < 0)
            return a << static_cast<T>(-n);
        const uint64_t s = static_cast<uint64_t>(n);
        if(s >= 128)
            return a.high < 0 ? from_words(uint64_t(-1), -1) : from_words(0, 0);
        if(s == 0)
            return a;
        if(s >= 64)
        {
            const int64_t shifted = a.high >> (s - 64); // arithmetic shift
            return from_words(static_cast<uint64_t>(shifted), shifted < 0 ? -1 : 0);
        }
        const uint64_t lo = (a.low >> s) | (static_cast<uint64_t>(a.high) << (64 - s));
        return from_words(lo, a.high >> s);
    }

    /* bitwise */

    friend constexpr int128 operator&(int128 a, int128 b) noexcept
    {
        return from_words(a.low & b.low, a.high & b.high);
    }
    friend constexpr int128 operator|(int128 a, int128 b) noexcept
    {
        return from_words(a.low | b.low, a.high | b.high);
    }
    friend constexpr int128 operator^(int128 a, int128 b) noexcept
    {
        return from_words(a.low ^ b.low, a.high ^ b.high);
    }
    friend constexpr int128 operator~(int128 a) noexcept
    {
        return from_words(~a.low, ~a.high);
    }

    /* comparisons */

    friend constexpr bool operator==(int128 a, int128 b) noexcept
    {
        return a.low == b.low && a.high == b.high;
    }
    friend constexpr bool operator!=(int128 a, int128 b) noexcept
    {
        return !(a == b);
    }
    friend constexpr bool operator<(int128 a, int128 b) noexcept
    {
        return a.high < b.high || (a.high == b.high && a.low < b.low);
    }
    friend constexpr bool operator>(int128 a, int128 b) noexcept
    {
        return b < a;
    }
    friend constexpr bool operator<=(int128 a, int128 b) noexcept
    {
        return !(b < a);
    }
    friend constexpr bool operator>=(int128 a, int128 b) noexcept
    {
        return !(a < b);
    }

    /* compound assignment / increment */

    constexpr int128& operator+=(int128 b) noexcept
    {
        *this = *this + b;
        return *this;
    }
    constexpr int128& operator-=(int128 b) noexcept
    {
        *this = *this - b;
        return *this;
    }
    constexpr int128& operator*=(int128 b) noexcept
    {
        *this = *this * b;
        return *this;
    }
    constexpr int128& operator/=(int128 b) noexcept
    {
        *this = *this / b;
        return *this;
    }
    constexpr int128& operator%=(int128 b) noexcept
    {
        *this = *this % b;
        return *this;
    }
    template <std::integral T>
    constexpr int128& operator<<=(T n) noexcept
    {
        *this = *this << n;
        return *this;
    }
    template <std::integral T>
    constexpr int128& operator>>=(T n) noexcept
    {
        *this = *this >> n;
        return *this;
    }
    constexpr int128& operator&=(int128 b) noexcept
    {
        *this = *this & b;
        return *this;
    }
    constexpr int128& operator|=(int128 b) noexcept
    {
        *this = *this | b;
        return *this;
    }
    constexpr int128& operator^=(int128 b) noexcept
    {
        *this = *this ^ b;
        return *this;
    }

    constexpr int128& operator++() noexcept
    {
        *this += 1;
        return *this;
    }
    constexpr int128 operator++(int) noexcept
    {
        const int128 t = *this;
        ++*this;
        return t;
    }
    constexpr int128& operator--() noexcept
    {
        *this -= 1;
        return *this;
    }
    constexpr int128 operator--(int) noexcept
    {
        const int128 t = *this;
        --*this;
        return t;
    }

private:
    static constexpr int128 from_words(uint64_t lo, int64_t hi) noexcept
    {
        int128 r;
        r.low = lo;
        r.high = hi;
        return r;
    }

    int64_t high;
    uint64_t low;
};

class uint128
{
public:
    constexpr uint128() noexcept = default;
    uint128(const uint128&) noexcept = default;
    uint128& operator=(const uint128&) noexcept = default;

    template <std::integral T>
    requires(sizeof(T) <= 8)
    constexpr uint128(T val) noexcept
        : high(0), low(static_cast<uint64_t>(val)){};

    template <std::floating_point T>
    requires(sizeof(T) <= 8)
    constexpr explicit uint128(T val) noexcept
        : high(0), low(static_cast<uint64_t>(static_cast<int64_t>(val))){};

    template <std::integral T>
    requires(sizeof(T) <= 8)
    constexpr explicit uint128(T high, T low) noexcept
        : high(static_cast<uint64_t>(high)), low(static_cast<uint64_t>(low)){};

    EIRIN_ALWAYS_INLINE constexpr uint64_t high_bits() const noexcept
    {
        return high;
    }

    EIRIN_ALWAYS_INLINE constexpr uint64_t low_bits() const noexcept
    {
        return low;
    }

    template <std::integral T>
    requires(sizeof(T) <= 8)
    constexpr explicit operator T() const noexcept
    {
        return static_cast<T>(low);
    }

    constexpr explicit operator bool() const noexcept
    {
        return (low | high) != 0;
    }

    template <std::floating_point T>
    constexpr explicit operator T() const noexcept
    {
        return static_cast<T>(
            static_cast<long double>(high) * 18446744073709551616.0L + static_cast<long double>(low)
        );
    }

    friend constexpr uint128 operator+(uint128 a, uint128 b) noexcept
    {
        const uint64_t lo = a.low + b.low;
        const uint64_t carry = lo < a.low ? 1u : 0u;
        return from_words(lo, a.high + b.high + carry);
    }
    friend constexpr uint128 operator-(uint128 a, uint128 b) noexcept
    {
        const uint64_t lo = a.low - b.low;
        const uint64_t borrow = a.low < b.low ? 1u : 0u;
        return from_words(lo, a.high - b.high - borrow);
    }
    friend constexpr uint128 operator-(uint128 a) noexcept
    {
        const uint64_t lo = uint64_t(0) - a.low;
        const uint64_t hi = uint64_t(0) - a.high - (a.low != 0 ? 1u : 0u);
        return from_words(lo, hi);
    }
    friend constexpr uint128 operator*(uint128 a, uint128 b) noexcept
    {
        uint64_t hi = 0;
        const uint64_t lo = detail::umul128(a.low, b.low, hi);
        hi += a.high * b.low;
        hi += a.low * b.high;
        return from_words(lo, hi);
    }
    friend constexpr uint128 operator/(uint128 a, uint128 b) noexcept
    {
        uint64_t qlo = 0, qhi = 0, rlo = 0, rhi = 0;
        detail::udivmod128(a.low, a.high, b.low, b.high, qlo, qhi, rlo, rhi);
        return from_words(qlo, qhi);
    }
    friend constexpr uint128 operator%(uint128 a, uint128 b) noexcept
    {
        uint64_t qlo = 0, qhi = 0, rlo = 0, rhi = 0;
        detail::udivmod128(a.low, a.high, b.low, b.high, qlo, qhi, rlo, rhi);
        return from_words(rlo, rhi);
    }

    template <std::integral T>
    friend constexpr uint128 operator<<(uint128 a, T n) noexcept
    {
        if(n < 0)
            return a >> static_cast<T>(-n);
        const uint64_t s = static_cast<uint64_t>(n);
        if(s >= 128)
            return from_words(0, 0);
        if(s == 0)
            return a;
        if(s >= 64)
            return from_words(0, a.low << (s - 64));
        return from_words(a.low << s, (a.high << s) | (a.low >> (64 - s)));
    }
    template <std::integral T>
    friend constexpr uint128 operator>>(uint128 a, T n) noexcept
    {
        if(n < 0)
            return a << static_cast<T>(-n);
        const uint64_t s = static_cast<uint64_t>(n);
        if(s >= 128)
            return from_words(0, 0);
        if(s == 0)
            return a;
        if(s >= 64)
            return from_words(a.high >> (s - 64), 0);
        return from_words((a.low >> s) | (a.high << (64 - s)), a.high >> s);
    }

    friend constexpr uint128 operator&(uint128 a, uint128 b) noexcept
    {
        return from_words(a.low & b.low, a.high & b.high);
    }
    friend constexpr uint128 operator|(uint128 a, uint128 b) noexcept
    {
        return from_words(a.low | b.low, a.high | b.high);
    }
    friend constexpr uint128 operator^(uint128 a, uint128 b) noexcept
    {
        return from_words(a.low ^ b.low, a.high ^ b.high);
    }
    friend constexpr uint128 operator~(uint128 a) noexcept
    {
        return from_words(~a.low, ~a.high);
    }

    friend constexpr bool operator==(uint128 a, uint128 b) noexcept
    {
        return a.low == b.low && a.high == b.high;
    }
    friend constexpr bool operator!=(uint128 a, uint128 b) noexcept
    {
        return !(a == b);
    }
    friend constexpr bool operator<(uint128 a, uint128 b) noexcept
    {
        return a.high < b.high || (a.high == b.high && a.low < b.low);
    }
    friend constexpr bool operator>(uint128 a, uint128 b) noexcept
    {
        return b < a;
    }
    friend constexpr bool operator<=(uint128 a, uint128 b) noexcept
    {
        return !(b < a);
    }
    friend constexpr bool operator>=(uint128 a, uint128 b) noexcept
    {
        return !(a < b);
    }

    constexpr uint128& operator+=(uint128 b) noexcept
    {
        *this = *this + b;
        return *this;
    }
    constexpr uint128& operator-=(uint128 b) noexcept
    {
        *this = *this - b;
        return *this;
    }
    constexpr uint128& operator*=(uint128 b) noexcept
    {
        *this = *this * b;
        return *this;
    }
    constexpr uint128& operator/=(uint128 b) noexcept
    {
        *this = *this / b;
        return *this;
    }
    constexpr uint128& operator%=(uint128 b) noexcept
    {
        *this = *this % b;
        return *this;
    }
    template <std::integral T>
    constexpr uint128& operator<<=(T n) noexcept
    {
        *this = *this << n;
        return *this;
    }
    template <std::integral T>
    constexpr uint128& operator>>=(T n) noexcept
    {
        *this = *this >> n;
        return *this;
    }
    constexpr uint128& operator&=(uint128 b) noexcept
    {
        *this = *this & b;
        return *this;
    }
    constexpr uint128& operator|=(uint128 b) noexcept
    {
        *this = *this | b;
        return *this;
    }
    constexpr uint128& operator^=(uint128 b) noexcept
    {
        *this = *this ^ b;
        return *this;
    }

    constexpr uint128& operator++() noexcept
    {
        *this += 1;
        return *this;
    }
    constexpr uint128 operator++(int) noexcept
    {
        const uint128 t = *this;
        ++*this;
        return t;
    }
    constexpr uint128& operator--() noexcept
    {
        *this -= 1;
        return *this;
    }
    constexpr uint128 operator--(int) noexcept
    {
        const uint128 t = *this;
        --*this;
        return t;
    }

private:
    static constexpr uint128 from_words(uint64_t lo, uint64_t hi) noexcept
    {
        uint128 r;
        r.low = lo;
        r.high = hi;
        return r;
    }

    uint64_t high;
    uint64_t low;
};
} // namespace eirin::ext

namespace eirin::detail
{
template <>
struct is_integral<eirin::ext::int128> : public std::true_type
{};
template <>
struct is_integral<eirin::ext::uint128> : public std::true_type
{};
template <>
struct is_signed<eirin::ext::int128> : public std::true_type
{};
template <>
struct is_unsigned<eirin::ext::uint128> : public std::true_type
{};
template <>
struct make_signed<eirin::ext::uint128, true>
{
    using type = eirin::ext::int128;
};
template <>
struct make_unsigned<eirin::ext::int128>
{
    using type = eirin::ext::uint128;
};
template <>
struct make_unsigned<eirin::ext::uint128>
{
    using type = eirin::ext::uint128;
};
} // namespace eirin::detail

namespace std
{
template <>
class numeric_limits<eirin::ext::int128>
{
public:
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = true;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;
    static constexpr float_denorm_style has_denorm = denorm_absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr float_round_style round_style = round_toward_zero;
    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = false;
    static constexpr int digits = 127;
    static constexpr int digits10 = 38;
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr bool traps = false;
    static constexpr bool tinyness_before = false;

    static constexpr eirin::ext::int128(min)() noexcept
    {
        return eirin::ext::int128(static_cast<int64_t>(0x8000000000000000ull), static_cast<int64_t>(0));
    }
    static constexpr eirin::ext::int128(max)() noexcept
    {
        return eirin::ext::int128(static_cast<int64_t>(0x7FFFFFFFFFFFFFFFull), static_cast<int64_t>(uint64_t(-1)));
    }
    static constexpr eirin::ext::int128 lowest() noexcept
    {
        return (min)();
    }
    static constexpr eirin::ext::int128 epsilon() noexcept
    {
        return 0;
    }
    static constexpr eirin::ext::int128 round_error() noexcept
    {
        return 0;
    }
    static constexpr eirin::ext::int128 infinity() noexcept
    {
        return 0;
    }
    static constexpr eirin::ext::int128 quiet_NaN() noexcept
    {
        return 0;
    }
    static constexpr eirin::ext::int128 signaling_NaN() noexcept
    {
        return 0;
    }
    static constexpr eirin::ext::int128 denorm_min() noexcept
    {
        return 0;
    }
};

template <>
class numeric_limits<eirin::ext::uint128>
{
public:
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = false;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;
    static constexpr float_denorm_style has_denorm = denorm_absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr float_round_style round_style = round_toward_zero;
    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = true;
    static constexpr int digits = 128;
    static constexpr int digits10 = 38;
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr bool traps = false;
    static constexpr bool tinyness_before = false;

    static constexpr eirin::ext::uint128(min)() noexcept
    {
        return 0;
    }
    static constexpr eirin::ext::uint128(max)() noexcept
    {
        return eirin::ext::uint128(uint64_t(-1), uint64_t(-1));
    }
    static constexpr eirin::ext::uint128 lowest() noexcept
    {
        return (min)();
    }
    static constexpr eirin::ext::uint128 epsilon() noexcept
    {
        return 0;
    }
    static constexpr eirin::ext::uint128 round_error() noexcept
    {
        return 0;
    }
    static constexpr eirin::ext::uint128 infinity() noexcept
    {
        return 0;
    }
    static constexpr eirin::ext::uint128 quiet_NaN() noexcept
    {
        return 0;
    }
    static constexpr eirin::ext::uint128 signaling_NaN() noexcept
    {
        return 0;
    }
    static constexpr eirin::ext::uint128 denorm_min() noexcept
    {
        return 0;
    }
};
} // namespace std

#endif
