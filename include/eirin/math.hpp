#ifndef EIRIN_MATH_FPMATH_HPP
#define EIRIN_MATH_FPMATH_HPP

#pragma once

#include "fixed.hpp"
#include <stdexcept>
#include "numbers.hpp"

namespace eirin
{
constexpr inline fixed32 f32_max = fixed32::from_internal_value(0x7FFFFFFF);
constexpr inline fixed32 f32_min = fixed32::from_internal_value(0x80000000);
#ifdef EIRIN_MATH_HAS_INT128
constexpr inline fixed64 f64_max = fixed64::from_internal_value(0x7FFFFFFFFFFFFFFF);
constexpr inline fixed64 f64_min = fixed64::from_internal_value(0x8000000000000000);
#endif

template <fixed_point T>
EIRIN_ALWAYS_INLINE constexpr T max_value() noexcept
{
    return std::numeric_limits<T>::max();
}

template <fixed_point T>
EIRIN_ALWAYS_INLINE constexpr T min_value() noexcept
{
    return std::numeric_limits<T>::min();
}

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> ceil(fixed_num<T, I, f, r> fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    constexpr auto frac_mult = T(1) << f;
    auto value = fp.internal_value();
    if(value > 0)
        value += frac_mult - 1;
    // overflow check.
    if(value < 0)
        return fixed::from_internal_value(fp.internal_value() / frac_mult * frac_mult + frac_mult - 1);
    return fixed::from_internal_value(value / frac_mult * frac_mult);
}

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> floor(fixed_num<T, I, f, r> fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    constexpr auto frac_mult = T(1) << f;
    auto value = fp.internal_value();
    auto neg = value < 0;
    if(value < 0)
        value -= frac_mult - 1;
    // underflow check.
    if(neg && value > 0)
        return fixed::from_internal_value(fp.internal_value() / frac_mult * frac_mult - frac_mult + 1);
    return fixed::from_internal_value(value / frac_mult * frac_mult);
}

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> trunc(fixed_num<T, I, f, r> fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    constexpr auto frac_mult = T(1) << f;
    return fixed::from_internal_value(fp.internal_value() / frac_mult * frac_mult);
}

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> round(fixed_num<T, I, f, r> fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    auto frac_mult = T(1) << f;
    auto value = fp.internal_value() / (frac_mult / 2);
    return fixed::from_internal_value((value / 2 + (value % 2)) << f);
}

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> abs(fixed_num<T, I, f, r> fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    auto value = fp.internal_value();
    return fixed::from_internal_value(value < 0 ? -value : value);
}

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> min(fixed_num<T, I, f, r> a, fixed_num<T, I, f, r> b) noexcept
{
    auto a_i = a.internal_value();
    auto b_i = b.internal_value();
    return a_i < b_i ? a : b;
}

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> max(fixed_num<T, I, f, r> a, fixed_num<T, I, f, r> b) noexcept
{
    auto a_i = a.internal_value();
    auto b_i = b.internal_value();
    return a_i > b_i ? a : b;
}

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> sqrt(fixed_num<T, I, f, r> fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    // test if T is int32_t, if so, we can use the fast sqrt algorithm.
    // if not, using newton method.
    if constexpr(std::is_same_v<T, int32_t>)
    {
        uint64_t t, q = 0, b = 0x40000000UL, v;
        if constexpr(f > 16)
        {
            constexpr auto move = f - 16;
            v = fp.internal_value() >> move;
        }
        else
        {
            constexpr auto move = 16 - f;
            v = fp.internal_value() << move;
        }

        // fix number sqrt using bit hack.
        if(v < 0x40000200)
        {
            while(b != 0x40)
            {
                t = q + b;
                if(v >= t)
                {
                    v -= t;
                    q = t + b;
                }
                v <<= 1;
                b >>= 1;
            }
            return fixed::from_internal_value(static_cast<T>(q >> 8));
        }
        while(b > 0x40)
        {
            t = q + b;
            if(v >= t)
            {
                v -= t;
                q = t + b;
            }
            if((v & 0x80000000) != 0)
            {
                q >>= 1;
                b >>= 1;
                v >>= 1;
                while(b > 0x20)
                {
                    t = q + b;
                    if(v >= t)
                    {
                        v -= t;
                        q = t + b;
                    }
                    v <<= 1;
                    b >>= 1;
                }
                return fixed::from_internal_value(static_cast<T>(q >> 7));
            }
            v <<= 1;
            b >>= 1;
        }
        return fixed::from_internal_value(static_cast<T>(q >> 8));
    }
    else
    {
        if(fp < fixed(0))
            return fixed(-1);
        if(fp == fixed(0))
            return fixed(0);
        const T val = fp.internal_value();
        const auto exponent = detail::find_msb(val);

        const auto init_value = fixed::get_sqrt_init_value(exponent);
        auto x = fixed::from_internal_value(init_value);

        auto eps = fixed::epsilon();
        for(int i = 0; i < 5; ++i)
        {
            x = (x + fp / x) / 2;
            if(abs(fp - x * x) < eps)
                break;
        }
        return x;
    }
}

/**
 * @brief sine function for fixed point number.
 *
 * @tparam T @see fixed_num
 * @tparam I @see fixed_num
 * @tparam f @see fixed_num
 * @tparam r @see fixed_num
 * @tparam pi the pi value, default is pi_v<fixed_num<T, I, f, r>>(). if you want more precision for fixed types like fixed128, you can pass the value you want.
 * @param fp
 * @return sin(fp)
 */
template <typename T, typename I, unsigned int f, bool r, fixed_num<T, I, f, r> pi = numbers::pi_v<fixed_num<T, I, f, r>>()>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> sin(fixed_num<T, I, f, r> fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    auto x = fixed(fp);
    x %= fixed::double_pi();
    x /= fixed::pi_2();
    constexpr auto fp1 = fixed(1);
    constexpr auto fp2 = fixed(2);

    if(x < fixed(0))
        x += fixed(4);

    auto negative = false;
    if(x > fp2)
    {
        negative = true;
        x -= fp2;
    }

    // we reconginzed the sin(x) = x when x is small enough.
    // sin(0.00015) = 0.000149999999437, so we use 0.0001 as the threshold.
    // TODO: replace it with fixed::template from_fixed_num_value<61>(0x13A92A0000000)
    if(abs(fp) < fixed(0.00015))
        return negative ? -fp : fp;

    // reduce the range to [0, 1] due to sin is
    // symmetrical around PI / 2 in the domain [0, PI].
    if(x > fp1)
        x = fp2 - x;

    // we use tyler series to calculate sin(x).
    // n = 4 has enough precision.
    const auto x2 = x * x;
    constexpr auto a = pi * pi / 24;
    constexpr auto b = pi * pi / 80;
    constexpr auto c = pi * pi / 168;
    constexpr auto d = pi * pi / 288;
    auto res = fixed::pi() * x * (fp1 - a * x2 * (fp1 - b * x2 * (fp1 - c * x2 * (fp1 - d * x2)))) / 2;
    return negative ? -res : res;
}

/**
 * @brief cosine function for fixed point number.
 *
 * @tparam T
 * @tparam I
 * @tparam f
 * @tparam r
 * @tparam pi the pi value, default is pi_v<fixed_num<T, I, f, r>>(). if you want more precision for fixed types like fixed128, you can pass the value you want.
 * @param fp
 * @return cos(fp)
 */
template <typename T, typename I, unsigned int f, bool r, fixed_num<T, I, f, r> pi = numbers::pi_v<fixed_num<T, I, f, r>>()>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> cos(fixed_num<T, I, f, r> fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    constexpr auto pi_2 = pi / fixed(2);
    constexpr auto double_pi = pi * fixed(2);
    return sin(fp.internal_value() > 0 ? fp - (double_pi - pi_2) : fp + pi_2);
}

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> tan(fixed_num<T, I, f, r> fp)
{
    auto cosx = cos(fp);
    if(abs(cosx).internal_value() > 1)
        return sin(fp) / cosx;
    else
        EIRIN_THROW_EXCEPTION(std::domain_error, "tan() domain error");
}

/**
     * @brief Arctangent function for fixed point number, using the fitting
     *        method from the paper "Efficient Approximations for the Arctangent Function".
     * @note reference paper: https://ieeexplore.ieee.org/document/1628884
     *
     * @tparam T @see fixed_num
     * @tparam I @see fixed_num
     * @tparam f @see fixed_num
     * @tparam r @see fixed_num
     * @tparam pi the pi value, default is pi_v<fixed_num<T, I, f, r>>(). if you want more precision for fixed types like fixed128, you can pass the value you want.
     * @param fp the x of atan(x)
     * @return atan(x).
     */
template <typename T, typename I, unsigned int f, bool r, fixed_num<T, I, f, r> pi = numbers::pi_v<fixed_num<T, I, f, r>>()>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> atan(fixed_num<T, I, f, r> fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    constexpr auto a = -1 * fixed::template from_fixed_num_value<61>(0X17CE62CE264A340); // 0.0464964749
    constexpr auto b = fixed::template from_fixed_num_value<61>(0X5191A2296020A80); // 0.15931422
    constexpr auto c = fixed::template from_fixed_num_value<61>(0XA7BE2BC19C39800); // 0.327622764
    // TODO: because fixed point cannot represent infinity, so we need to handle the case when fp is very large.
    auto abs_fp = abs(fp);
    auto x = abs_fp;
    auto x2 = x * x;
    fixed result = ((a * x2 + b) * x2 - c) * x2 * x + x;
    // if the input is negative, return negative result.
    if(fp.signbit_mask() & fp.internal_value())
        result = -result;
    return result;
    // another calc algorithm: x / (1 + 0.28125 * x * x)
    // constexpr auto factor = fixed::template from_fixed_num_value<61>(0X900000000000000); // 0.28125
    // constexpr auto fp_1 = fixed(1);
    // if(abs(fp) > fixed(1))
    // {
    //     constexpr auto pi_2 = pi / fixed(2);
    //     constexpr auto neg_pi_2 = -pi_2;
    //     auto x = fp_1 / fp;
    //     if(fp > fixed(0))
    //         return pi_2 - (x / (fp_1 + factor * x * x));
    //     else
    //         return neg_pi_2 - (x / (fp_1 + factor * x * x));
    // }
    // return fp / (fp_1 + factor * fp * fp);
    // constexpr auto a = fixed::template from_fixed_num_value<61>(0X730BE0DED288D00); // 0.2247
    // constexpr auto b = fixed::template from_fixed_num_value<61>(0X21F212D77318FC0); // 0.0663
    // constexpr auto pi_4 = pi / fixed(4);
    // return pi_4 * fp - fp * (abs(fp) - fixed(1)) * (a - b * abs(fp));
}

template <typename T, typename I, unsigned int f, bool r, fixed_num<T, I, f, r> pi = numbers::pi_v<fixed_num<T, I, f, r>>()>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> asin(fixed_num<T, I, f, r> fp)
{
    using fixed = fixed_num<T, I, f, r>;
    if(abs(fp) > fixed(1))
        EIRIN_THROW_EXCEPTION(std::domain_error, "asin() domain error");
    if(fp == fixed(1))
        return pi / fixed(2);
    else if(fp == fixed(-1))
        return -pi / fixed(2);
    return atan(fp / sqrt(fixed(1) - fp * fp));
}

template <typename T, typename I, unsigned int f, bool r, fixed_num<T, I, f, r> pi = numbers::pi_v<fixed_num<T, I, f, r>>()>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> acos(fixed_num<T, I, f, r> fp)
{
    using fixed = fixed_num<T, I, f, r>;
    if(abs(fp) > fixed(1))
        EIRIN_THROW_EXCEPTION(std::domain_error, "acos() domain error");
    if(fp == fixed(1))
        return fixed(0);
    else if(fp == fixed(-1))
        return pi;
    else if(fp == fixed(0))
        return pi / fixed(2);
    return pi / 2 - asin(fp);
}

//TODO: Implement asin and acos functions with CORDIC, and optimize other functions with CORDIC.

/**
 * @brief cbrt function for fixed point number, which used newton method to calculate the cbrt.
 *
 * @tparam T @see fixed_num
 * @tparam I @see fixed_num
 * @tparam f @see fixed_num
 * @tparam r @see fixed_num
 * @tparam iter_max max iteration times, default is 200.
 * @param fp
 * @return cbrt(fp)
 */
template <typename T, typename I, unsigned int f, bool r, int iter_max = 200>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> cbrt(fixed_num<T, I, f, r> fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    if(abs(fp) < fixed::epsilon())
        return fixed(0);
    auto x = (fixed(fp) + 2) / 3;
    auto iter_count = 0;
    constexpr auto precision = fixed::nearly_compare_epsilon() * 2;

    while(abs(fp - (x * x * x)) >= precision && iter_count <= iter_max)
    {
        x = (fp / (x * x) + x * 2) / 3;
        ++iter_count;
    }
    return x;
}

/**
 * @brief log2 function for fixed point number, which used some bit hacks.
 *
 * @tparam T @see fixed_num
 * @tparam I @see fixed_num
 * @tparam f @see fixed_num
 * @tparam r @see fixed_num
 * @param fp
 * @return log2(fp)
 */
template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> log2(fixed_num<T, I, f, r> fp)
{
    using fixed = fixed_num<T, I, f, r>;

    // This implementation is based on Clay. S. Turner's fast binary logarithm[1].
    // For some unknown reason, the GCC/Clang optimize this code to a very fast implementation,
    // when using '-O2' optimization and the fixed point number is a constant or literal,
    // it is even faster than the standard library's floating-point log2 function.
    // However, '-O3' optimization will not optimize this code to a fast implementation in the
    // same situation.
    // To be specific, the third loop(the for loop) will be optimized to the next assembly code:
    // ```
    // .L4:
    //         imul    rax, rax
    //         mov     rdi, rax
    //         shr     rax, 16
    //         cmp     rax, 131071
    //         jbe     .L3
    //         mov     rax, rdi
    //         add     rsi, rcx
    //         shr     rax, 17
    // ```
    // The .L3 code segments is the main function code, and the .L4 code segments is the for loop code.
    // The assembly code of '-O3' optimization is to expand the for loop code into tons of code segments,
    // which makes the performance of the code much slower than '-O2' optimization.
    // I prefer this situation is abnormal, but I don't know why.
    T b = 1u << (f - 1), y = 0, x = fp.internal_value();
    if(fp <= fixed(0))
        EIRIN_THROW_EXCEPTION(std::domain_error, "log2() domain error");

    while(x < (static_cast<T>(1u) << f))
    {
        x <<= 1;
        y -= (static_cast<T>(1u) << f);
    }

    while(x >= (static_cast<T>(2u) << f))
    {
        x >>= 1;
        y += (static_cast<T>(1u) << f);
    }

    I z = x;
    for(size_t i = 0; i < f; ++i)
    {
        z = (z * z) >> f;
        if(z >= (static_cast<T>(2u) << f))
        {
            z >>= 1;
            y += b;
        }
        b >>= 1;
    }

    return fixed::from_internal_value(y);
}

/**
 * @brief ln function for fixed point number, which used the log2 function to calculate the ln.
 *
 * @tparam T @see fixed_num
 * @tparam I @see fixed_num
 * @tparam f @see fixed_num
 * @tparam r @see fixed_num
 * @tparam log2_e the log2(e) value, and its default value is designed for 32bit and 64bit fixed number. if you want more precision for fixed types like fixed128, you can pass the value you want.
 * @param fp
 * @return log(fp)
 */
template <typename T, typename I, unsigned int f, bool r, fixed_num<T, I, f, r> log2_e = fixed_num<T, I, f, r>::template from_fixed_num_value<60>(0x171547652B82FE00ll)>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> log(fixed_num<T, I, f, r> fp)
{
    return log2(fp) / log2_e;
}

/**
 * @brief the log10 function for fixed point number, which used the log2 function to calculate the log10.
 *
 * @tparam T @see fixed_num
 * @tparam I @see fixed_num
 * @tparam f @see fixed_num
 * @tparam r @see fixed_num
 * @tparam log2_10 the log2(10) value, and its default value is designed for 32bit and 64bit fixed number. if you want more precision for fixed types like fixed128, you can pass the value you want.
 * @param fp
 * @return log10(fp)
 */
template <typename T, typename I, unsigned int f, bool r, fixed_num<T, I, f, r> log2_10 = fixed_num<T, I, f, r>::template from_fixed_num_value<60>(0x35269E12F346E200ll)>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> log10(fixed_num<T, I, f, r> fp)
{
    return log2(fp) / log2_10;
}

template <typename T, typename I, unsigned int f, bool r, std::integral E>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> pow(fixed_num<T, I, f, r> b, E e) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    if(b == fixed(0))
    {
        if(e == 0)
            return fixed(1);
        return fixed(0);
    }

    auto res = fixed(1);
    if(e < 0)
    {
        for(auto i = b; e != 0; e /= 2, i *= i)
        {
            if(e % 2 != 0)
                res /= i;
        }
    }
    else
    {
        for(auto i = b; e != 0; e /= 2, i *= i)
        {
            if(e % 2 != 0)
                res *= i;
        }
    }
    return res;
}

namespace detail
{
    template <typename T, typename I, unsigned int f, bool r>
    EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> exp_expand(fixed_num<T, I, f, r> fp) noexcept
    {
        using fixed = fixed_num<T, I, f, r>;
        // the integer part of the input fixed point number.
        const T x_int = fp.integral_part();
        fp -= x_int;

        constexpr auto a = fixed::template from_fixed_num_value<63>(0x01C798ECC0CBC856ll); // 1.3903728105644451e-2
        constexpr auto b = fixed::template from_fixed_num_value<63>(0x04745859810836DAll); // 3.4800571158543038e-2
        constexpr auto c = fixed::template from_fixed_num_value<63>(0x15CFBB5C306F85F3ll); // 1.7040197373796334e-1
        constexpr auto d = fixed::template from_fixed_num_value<63>(0x3FE26186C531F98Ell); // 4.9909609871464493e-1
        constexpr auto e = fixed::template from_fixed_num_value<63>(0x40014D4407008BB0ll); // 1.0000794567422495
        constexpr auto _f = fixed::template from_fixed_num_value<63>(0x7FFFF686446F1B43ll); // 9.9999887043019773e-1
        return pow(numbers::e_v<fixed>(), x_int) * (_f + fp * (e + fp * (d + fp * (c + fp * (b + fp * a)))));
    }
} // namespace detail

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> exp(fixed_num<T, I, f, r> fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;

    if(fp == fixed(0))
        return fixed(1);
    if(fp < fixed(0))
        return fixed(1) / detail::exp_expand(-fp);
    return detail::exp_expand(fp);
}

// TODO: This function has performance issue on MSVC,
// which is about 5x slower than the result of clang-cl.
template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> pow(fixed_num<T, I, f, r> b, fixed_num<T, I, f, r> e) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    if(b == fixed(0))
    {
        if(e == fixed(0))
            return fixed(1);
        return fixed(0);
    }

    // the integer part of the input exponent.
    const T e_int = static_cast<T>(e.internal_value() / (I(1) << f));
    e -= e_int;

    return pow(b, e_int) * exp(e * log(b));
}

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> fmod(fixed_num<T, I, f, r> a, fixed_num<T, I, f, r> b) noexcept
{
    return a - b * floor(a / b);
}

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> modf(fixed_num<T, I, f, r> fp, fixed_num<T, I, f, r>& int_part) noexcept
{
    int_part = floor(fp);
    return fp - int_part;
}

/**
 * @brief convert radian to degree.
 *
 * @tparam T @see fixed_num
 * @tparam I @see fixed_num
 * @tparam f @see fixed_num
 * @tparam r @see fixed_num
 * @tparam pi the pi value, default is pi_v<fixed_num<T, I, f, r>>(). if you want more precision for fixed types like fixed128, you can pass the value you want.
 * @param rad
 * @return deg(rad)
 */
template <typename T, typename I, unsigned int f, bool r, fixed_num<T, I, f, r> pi = eirin::numbers::pi_v<fixed_num<T, I, f, r>>()>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> degrees(fixed_num<T, I, f, r> rad) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    constexpr fixed factor = fixed(180);
    fixed deg = rad / pi * factor;
    return deg;
}

/**
 * @brief convert degree to radian.
 *
 * @tparam T @see fixed_num
 * @tparam I @see fixed_num
 * @tparam f @see fixed_num
 * @tparam r @see fixed_num
 * @tparam pi the pi value, default is pi_v<fixed_num<T, I, f, r>>(). if you want more precision for fixed types like fixed128, you can pass the value you want.
 * @param deg
 * @return rad(deg)
 */
template <typename T, typename I, unsigned int f, bool r, fixed_num<T, I, f, r> pi = eirin::numbers::pi_v<fixed_num<T, I, f, r>>()>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> radians(fixed_num<T, I, f, r> deg) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    constexpr fixed factor = fixed(180);
    fixed rad = deg / factor * pi;
    return rad;
}

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> hypot(fixed_num<T, I, f, r> x, fixed_num<T, I, f, r> y) noexcept
{
    return sqrt(x * x + y * y);
}

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> hypot(fixed_num<T, I, f, r> x, fixed_num<T, I, f, r> y, fixed_num<T, I, f, r> z) noexcept
{
    return sqrt(x * x + y * y + z * z);
}
} // namespace eirin

#endif // EIRIN_MATH_FPMATH_HPP
