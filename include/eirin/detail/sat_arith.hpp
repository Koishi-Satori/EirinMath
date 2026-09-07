#ifndef EIRIN_MATH_DETAIL_SAT_ARITH_HPP
#define EIRIN_MATH_DETAIL_SAT_ARITH_HPP

#pragma once

#include <type_traits>
#include "type_traits_impl.hpp"
#include "numeric_traits.hpp"

namespace eirin::detail
{
    template <typename T, typename I>
    concept __has_enough_bits_imul = sizeof(I) >= sizeof(T) * 2;

    template <typename T>
    concept __saturating_arithmetic_type = detail::is_integral_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool>;

    /**
     * @brief Detect overflow of `x + y` in the storage type `T`.
     *
     * @tparam T the storage type.
     * @tparam I the wider intermediate type.
     * @param x the first addend.
     * @param y the second addend.
     * @param z receives the sum truncated back to `T`. This is the exact sum
     *          when there is no overflow and the mod-2^N wrapped sum when
     *          there is, so `z` is always usable.
     * @return `true` iff `x + y` overflows `T`.
     */
    template <typename T, typename I>
    EIRIN_MATH_SMALL_FUNC_API bool __add_overflow(T x, T y, T* z) noexcept
    {
        if constexpr(detail::is_signed_v<T> && has_make_unsigned_v<T>)
        {
            // use unsigned version to check.
            using u_type = detail::make_unsigned_t<T>;
            u_type res = static_cast<u_type>(x) + static_cast<u_type>(y);
            *z = static_cast<T>(res);
            constexpr u_type sign_bit = static_cast<u_type>(1) << (std::numeric_limits<u_type>::is_specialized ? std::numeric_limits<u_type>::digits - 1 : sizeof(T) * 8 - 1);
            const bool pos_overflow = (x > 0 && y > 0 && (res & sign_bit));
            const bool neg_overflow = (x < 0 && y < 0 && !(res & sign_bit));
            return pos_overflow || neg_overflow;
        }
        I res = static_cast<I>(x) + static_cast<I>(y);
        *z = static_cast<T>(res);
        return res > static_cast<I>(detail::__any_int_traits<T>::max) ||
               res < static_cast<I>(detail::__any_int_traits<T>::min);
    }

    template <typename T, typename I>
    EIRIN_MATH_SMALL_FUNC_API bool __sub_overflow(T x, T y, T* z) noexcept
    {
        if constexpr(detail::is_signed_v<T> && has_make_unsigned_v<T>)
        {
            // use unsigned version to check.
            // neg - pos can neg overflow, pos - neg can pos overflow
            using u_type = detail::make_unsigned_t<T>;
            u_type res = static_cast<u_type>(x) - static_cast<u_type>(y);
            *z = static_cast<T>(res);
            constexpr u_type sign_bit = static_cast<u_type>(1) << (std::numeric_limits<u_type>::is_specialized ? std::numeric_limits<u_type>::digits - 1 : sizeof(T) * 8 - 1);
            const bool pos_overflow = (x >= 0 && y < 0 && (res & sign_bit));
            const bool neg_overflow = (x < 0 && y >= 0 && !(res & sign_bit));
            return pos_overflow || neg_overflow;
        }
        I res = static_cast<I>(x) - static_cast<I>(y);
        *z = static_cast<T>(res);
        return res > static_cast<I>(detail::__any_int_traits<T>::max) ||
               res < static_cast<I>(detail::__any_int_traits<T>::min);
    }

    template <typename T, typename I, bool r, unsigned int f>
    requires __has_enough_bits_imul<T, I>
    EIRIN_MATH_SMALL_FUNC_API I __mul_scaled(T x, T y) noexcept
    {
        const I p = static_cast<I>(x) * static_cast<I>(y);
        if constexpr(r)
        {
            constexpr I fraction_multiplier = I(1) << f;
            I v = p / (fraction_multiplier / 2);
            return (v + (v % 2)) >> 1;
        }
        else
        {
            return p >> f;
        }
    }

    template <typename T, typename I, bool r, unsigned int f>
    requires __has_enough_bits_imul<T, I>
    EIRIN_MATH_SMALL_FUNC_API bool __mul_overflow(T x, T y, T* z) noexcept
    {
        const I res = __mul_scaled<T, I, r, f>(x, y);
        *z = static_cast<T>(res);
        return res > static_cast<I>(detail::__any_int_traits<T>::max) ||
               res < static_cast<I>(detail::__any_int_traits<T>::min);
    }

    template <typename T, typename I, bool r, unsigned int f>
    requires __has_enough_bits_imul<T, I>
    EIRIN_MATH_SMALL_FUNC_API I __div_scaled(T x, T y) noexcept
    {
        const I num = static_cast<I>(x) << f; // x * 2^f
        if constexpr(r)
        {
            const I v = (num * 2) / y;
            return (v + (v % 2)) >> 1;
        }
        else
        {
            return num / y;
        }
    }

    template <typename T, typename I, bool r, unsigned int f, bool modwarp = true>
    requires __has_enough_bits_imul<T, I>
    EIRIN_MATH_SMALL_FUNC_API bool __div_overflow(T x, T y, T* z) noexcept
    {
        // division by zero has no fixed-point value, so always report overflow.
        if(y == 0)
        {
            *z = T(0);
            return true;
        }

        const I res = __div_scaled<T, I, r, f>(x, y);

        const bool overflow = res > static_cast<I>(detail::__any_int_traits<T>::max) ||
                              res < static_cast<I>(detail::__any_int_traits<T>::min);
        if(overflow)
        {
            if constexpr(modwarp)
                *z = static_cast<T>(res); // mod-2^N wrapped quotient
            else
                *z = T(0); // caller saturates and ignores z
        }
        else
        {
            *z = static_cast<T>(res);
        }
        return overflow;
    }

    /* Internal Function of Integral Saturating Arithmetic (C++26 like) */

    // fallback option for add_overflow
    template <typename _Tp>
    requires detail::__saturating_arithmetic_type<_Tp>
    EIRIN_MATH_SMALL_FUNC_API bool __builtin_add_overflow_fallback(_Tp x, _Tp y, _Tp* z) noexcept
    {
        if constexpr(detail::is_signed_v<_Tp>)
        {
            using u_type = detail::make_unsigned_t<_Tp>;
            u_type ux = static_cast<u_type>(x);
            u_type uy = static_cast<u_type>(y);
            u_type sum = static_cast<u_type>(ux + uy);
            *z = static_cast<_Tp>(sum);
            constexpr u_type sign_bit = static_cast<u_type>(1) << (detail::__any_int_traits<u_type>::digits - 1);
            return (((~(ux ^ uy)) & (ux ^ sum)) & sign_bit) != 0;
        }
        else
        {
            _Tp sum = static_cast<_Tp>(x + y);
            *z = sum;
            return sum < x;
        }
    }

    // fallback option for sub_overflow
    template <typename _Tp>
    requires detail::__saturating_arithmetic_type<_Tp>
    EIRIN_MATH_SMALL_FUNC_API bool __builtin_sub_overflow_fallback(_Tp x, _Tp y, _Tp* z) noexcept
    {
        if constexpr(detail::is_signed_v<_Tp>)
        {
            using u_type = detail::make_unsigned_t<_Tp>;
            u_type ux = static_cast<u_type>(x);
            u_type uy = static_cast<u_type>(y);
            u_type sub = static_cast<u_type>(ux - uy);
            *z = static_cast<_Tp>(sub);
            constexpr u_type sign_bit = static_cast<u_type>(1) << (detail::__any_int_traits<u_type>::digits - 1);
            return (((ux ^ uy) & (ux ^ sub)) & sign_bit) != 0;
        }
        else
        {
            _Tp sum = static_cast<_Tp>(x - y);
            *z = sum;
            return sum > x;
        }
    }

    // fallback option for mul_overflow
    template <typename _Tp>
    requires detail::__saturating_arithmetic_type<_Tp>
    EIRIN_MATH_SMALL_FUNC_API bool __builtin_mul_overflow_fallback(_Tp x, _Tp y, _Tp* z) noexcept
    {
        if constexpr(detail::is_signed_v<_Tp>)
        {
            using u_type = detail::make_unsigned_t<_Tp>;
            u_type ux = x < 0 ? u_type(0) - static_cast<u_type>(x) : static_cast<u_type>(x);
            u_type uy = y < 0 ? u_type(0) - static_cast<u_type>(y) : static_cast<u_type>(y);
            bool neg = (x < 0) != (y < 0);
            u_type bound = neg ? static_cast<u_type>(detail::__any_int_traits<_Tp>::min) : static_cast<u_type>(detail::__any_int_traits<_Tp>::max);
            bool overflowed = uy != 0 && ux > bound / uy;
            u_type product = ux * uy;
            *z = static_cast<_Tp>(neg ? u_type(0) - product : product);
            return overflowed;
        }
        else
        {
            bool overflowed = y != 0 && x > detail::__any_int_traits<_Tp>::max / y;
            *z = static_cast<_Tp>(x * y);
            return overflowed;
        }
    }

    // this function is for keep the same style, not actually used.
    template <typename _Tp>
    requires detail::__saturating_arithmetic_type<_Tp>
    EIRIN_MATH_SMALL_FUNC_API bool __builtin_div_overflow_fallback(_Tp x, _Tp y, _Tp* z) noexcept
    {
        if constexpr(detail::is_signed_v<_Tp>)
        {
            constexpr _Tp min = detail::__any_int_traits<_Tp>::min;
            bool overflowed = x == min && y == -1;
            *z = overflowed ? 0 : static_cast<_Tp>(x / y);
        }
        else
        {
            *z = static_cast<_Tp>(x / y);
            return false;
        }
    }

    template <typename _Tp>
    requires detail::__saturating_arithmetic_type<_Tp>
    EIRIN_MATH_SMALL_FUNC_API bool __integral_add_overflow(_Tp x, _Tp y, _Tp* z) noexcept
    {
#if defined(__has_builtin)
#    if __has_builtin(__builtin_add_overflow)
        if constexpr(std::is_integral_v<_Tp>)
            return __builtin_add_overflow(x, y, z);
#    endif
#endif
        // we have _add_overflow_i8/16/32 on MSVC, and i64 ver is only defined for _M_X64
#if defined(_MSC_VER) && !defined(__clang__) && _MSC_VER >= 1937 && (defined(_M_IX86) || defined(_M_X64))
        EIRIN_IF_CONSTEVAL
        {
            return __builtin_add_overflow_fallback(x, y, z);
        }
        else
        {
            if constexpr(std::is_integral_v<_Tp> && std::is_signed_v<_Tp>)
            {
                constexpr auto size = sizeof(_Tp);
                if constexpr(size == sizeof(signed char))
                    return _add_overflow_i8(0, static_cast<signed char>(x), static_cast<signed char>(y), reinterpret_cast<signed char*>(z));
                else if constexpr(size == sizeof(signed short))
                    return _add_overflow_i16(0, static_cast<signed short>(x), static_cast<signed short>(y), reinterpret_cast<signed short*>(z));
                else if constexpr(size == sizeof(signed int))
                    return _add_overflow_i32(0, static_cast<signed int>(x), static_cast<signed int>(y), reinterpret_cast<signed int*>(z));
#    ifdef _M_X64
                else if constexpr(size == sizeof(signed __int64))
                    return _add_overflow_i64(0, static_cast<signed __int64>(x), static_cast<signed __int64>(y), reinterpret_cast<signed __int64*>(z));
#    endif
            }
        }
#endif
        return __builtin_add_overflow_fallback(x, y, z);
    }

    template <typename _Tp>
    requires detail::__saturating_arithmetic_type<_Tp>
    EIRIN_MATH_SMALL_FUNC_API bool __integral_sub_overflow(_Tp x, _Tp y, _Tp* z) noexcept
    {
#if defined(__has_builtin)
#    if __has_builtin(__builtin_sub_overflow)
        return __builtin_sub_overflow(x, y, z);
#    endif
#endif
        // we have _sub_overflow_i8/16/32 on MSVC, and i64 ver is only defined for _M_X64
#if defined(_MSC_VER) && !defined(__clang__) && _MSC_VER >= 1937 && (defined(_M_IX86) || defined(_M_X64))
        EIRIN_IF_CONSTEVAL
        {
            return __builtin_sub_overflow_fallback(x, y, z);
        }
        else
        {
            if constexpr(std::is_integral_v<_Tp> && std::is_signed_v<_Tp>)
            {
                constexpr auto size = sizeof(_Tp);
                if constexpr(size == sizeof(signed char))
                    return _sub_overflow_i8(0, static_cast<signed char>(x), static_cast<signed char>(y), reinterpret_cast<signed char*>(z));
                else if constexpr(size == sizeof(signed short))
                    return _sub_overflow_i16(0, static_cast<signed short>(x), static_cast<signed short>(y), reinterpret_cast<signed short*>(z));
                else if constexpr(size == sizeof(signed int))
                    return _sub_overflow_i32(0, static_cast<signed int>(x), static_cast<signed int>(y), reinterpret_cast<signed int*>(z));
#    ifdef _M_X64
                else if constexpr(size == sizeof(signed __int64))
                    return _sub_overflow_i64(0, static_cast<signed __int64>(x), static_cast<signed __int64>(y), reinterpret_cast<signed __int64*>(z));
#    endif
            }
        }
#endif
        return __builtin_sub_overflow_fallback(x, y, z);
    }

    template <typename _Tp>
    requires detail::__saturating_arithmetic_type<_Tp>
    EIRIN_MATH_SMALL_FUNC_API bool __integral_mul_overflow(_Tp x, _Tp y, _Tp* z) noexcept
    {
#if defined(__has_builtin)
#    if __has_builtin(__builtin_mul_overflow)
        return __builtin_mul_overflow(x, y, z);
#    endif
#endif
        // we have _mul[_full]_overflow_xxx on MSVC, and i64/u64 ver is only defined for _M_X64
#if defined(_MSC_VER) && !defined(__clang__) && _MSC_VER >= 1937 && (defined(_M_IX86) || defined(_M_X64))
        EIRIN_IF_CONSTEVAL
        {
            return __builtin_mul_overflow_fallback(x, y, z);
        }
        else
        {
            if constexpr(std::is_integral_v<_Tp>)
            {
                constexpr auto size = sizeof(_Tp);
                if constexpr(std::is_signed_v<_Tp>)
                {
                    if constexpr(size == sizeof(signed char))
                    {
                        // _mul_full_overflow_i8 can only pass 16bits, which gets hi+lo
                        signed short sz;
                        bool overflowed = _mul_full_overflow_i8(static_cast<signed char>(x), static_cast<signed char>(y), &sz);
                        *z = static_cast<_Tp>(sz);
                        return overflowed;
                    }
                    else if constexpr(size == sizeof(signed short))
                        return _mul_overflow_i16(static_cast<signed short>(x), static_cast<signed short>(y), reinterpret_cast<signed short*>(z));
                    else if constexpr(size == sizeof(signed int))
                        return _mul_overflow_i32(static_cast<signed int>(x), static_cast<signed int>(y), reinterpret_cast<signed int*>(z));
#    ifdef _M_X64
                    else if constexpr(size == sizeof(signed __int64))
                        return _mul_overflow_i64(static_cast<signed __int64>(x), static_cast<signed __int64>(y), reinterpret_cast<signed __int64*>(z));
#    endif
                }
                else if constexpr(std::is_unsigned_v<_Tp>)
                {
                    if constexpr(size == sizeof(unsigned char))
                    {
                        // _mul_full_overflow_u8 can only pass 16bits, which gets hi+lo
                        unsigned short sz;
                        bool overflowed = _mul_full_overflow_u8(static_cast<unsigned char>(x), static_cast<unsigned char>(y), &sz);
                        *z = static_cast<_Tp>(sz);
                        return overflowed;
                    }
                    else if constexpr(size == sizeof(unsigned short))
                    {
                        unsigned short hi;
                        return _mul_full_overflow_u16(static_cast<unsigned short>(x), static_cast<unsigned short>(y), reinterpret_cast<unsigned short*>(z), reinterpret_cast<unsigned short*>(&hi));
                    }
                    else if constexpr(size == sizeof(unsigned int))
                    {
                        unsigned int hi;
                        return _mul_full_overflow_u32(static_cast<unsigned int>(x), static_cast<unsigned int>(y), reinterpret_cast<unsigned int*>(z), reinterpret_cast<unsigned int*>(&hi));
                    }
#    ifdef _M_X64
                    else if constexpr(size == sizeof(unsigned __int64))
                    {
                        unsigned __int64 hi;
                        return _mul_full_overflow_u64(static_cast<unsigned __int64>(x), static_cast<unsigned __int64>(y), reinterpret_cast<unsigned __int64*>(z), reinterpret_cast<unsigned __int64*>(&hi));
                    }
#    endif
                }
            }
        }
#endif
        return __builtin_mul_overflow_fallback(x, y, z);
    }

    // this function is for keep the same style, not actually used.
    template <typename _Tp>
    requires detail::__saturating_arithmetic_type<_Tp>
    EIRIN_MATH_SMALL_FUNC_API bool __integral_div_overflow(_Tp x, _Tp y, _Tp* z) noexcept
    {
        if constexpr(detail::is_signed_v<_Tp>)
        {
            constexpr _Tp min = detail::__any_int_traits<_Tp>::min;
            bool overflowed = x == min && y == -1;
            *z = overflowed ? 0 : static_cast<_Tp>(x / y);
        }
        else
        {
            *z = static_cast<_Tp>(x / y);
            return false;
        }
    }
} // namespace eirin::detail

#endif
