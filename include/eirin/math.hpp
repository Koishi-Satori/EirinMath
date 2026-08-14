#ifndef EIRIN_MATH_FPMATH_HPP
#define EIRIN_MATH_FPMATH_HPP

#pragma once

#include "fixed.hpp"
#include <stdexcept>
#include "numbers.hpp"
#include "detail/math_impl.hpp"

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
    if constexpr(std::is_same_v<T, int32_t> && f == 16)
    {
        if(fp < fixed(0))
            return fixed(-1);
        if(fp == fixed(0))
            return fixed(0);
        const uint64_t N = static_cast<uint64_t>(static_cast<uint32_t>(fp.internal_value())) << f;
        uint64_t m = uint64_t(1) << (2 * ((std::bit_width(N) - 1) / 2));
        uint64_t y = 0, R = N;
        while(m != 0)
        {
            const uint64_t b = y | m;
            y >>= 1;
            const uint64_t ge = uint64_t(0) - static_cast<uint64_t>(R >= b);
            R -= b & ge;
            y |= m & ge;
            m >>= 2;
        }
        return fixed::from_internal_value(static_cast<T>(y));
    }
    else
    {
        if(fp < fixed(0))
            return fixed(-1);
        if(fp == fixed(0))
            return fixed(0);

        const T val = fp.internal_value();
        const int e = static_cast<int>(fp.bit_width()) - 1;
        int64_t seed;
        if(e >= 12)
        {
            const int t = static_cast<int>(static_cast<uint64_t>(val) >> (e - 11));
            seed = detail::sqrt_mantissa_table[t];
            int sh = (e - 11) / 2 + static_cast<int>(f) / 2 - 16;
            if((e & 1) == 0)
            {
                // round(sqrt(2)·2^15) = 46341
                seed = (seed * 46341) >> 15; // *sqrt(2)
            }
            seed = sh >= 0 ? seed << sh : seed >> (-sh);
        }
        else
        {
            seed = detail::sqrt_mantissa_table[static_cast<int>(static_cast<uint64_t>(val))];
            const int sh = static_cast<int>(f) / 2 - 16;
            seed = sh >= 0 ? seed << sh : seed >> (-sh);
        }
        auto x = fixed::from_internal_value(static_cast<T>(seed));
        x = (x + fp / x) / 2;
        x = (x + fp / x) / 2;
        return x;
    }
}

/**
 * @brief sine function with widened internal precision.
 *
 * Computes sin(x) on a fixed point type with more fraction bits when the
 * fraction of the input type is too small to express the minimax parameters,
 * so that the minimax polynomial evaluation is carried out with higher
 * precision before the result is narrowed back to the original type.
 *
 * The wide_fraction is chosen as the smallest fraction that can exactly
 * express the minimax coefficients (currently 32). When the fixed point type
 * cannot provide that many fraction bits (wide_fraction larger than the
 * type's feasible fraction), this function falls back to the Taylor sin().
 *
 * @tparam T @see fixed_num
 * @tparam I @see fixed_num
 * @tparam f @see fixed_num
 * @tparam r @see fixed_num
 * @param fp the input angle.
 * @return sin(fp), computed with widened precision when possible.
 */
template <typename T, typename I, unsigned int f, bool r, fixed_num<T, I, f, r> pi = numbers::pi_v<fixed_num<T, I, f, r>>()>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> sin(fixed_num<T, I, f, r> fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    constexpr unsigned int best_wide_fraction = detail::sin_minimax_required_fraction();

    if constexpr(f >= best_wide_fraction)
    {
        return detail::sin_minimax<T, I, f, r, pi>(fp);
    }
    else
    {
        constexpr int max_fraction = static_cast<int>(fixed::digits) - 3;
        if constexpr(max_fraction >= static_cast<int>(best_wide_fraction))
        {
            using wide = fixed_num<T, I, best_wide_fraction, r>;
            constexpr I scale = static_cast<I>(1) << (best_wide_fraction - f);
            const I scaled = static_cast<I>(fp.internal_value()) * scale;
            if(scaled >= static_cast<I>(std::numeric_limits<T>::min()) &&
               scaled <= static_cast<I>(std::numeric_limits<T>::max()))
            {
                const wide wx = wide::from_internal_value(static_cast<T>(scaled));
                const wide wres = detail::sin_minimax<T, I, f, r, pi>(wx);
                return fixed::from_internal_value(static_cast<T>(wres.internal_value() / scale));
            }
        }
        return detail::sin_taylor<T, I, f, r, pi>(fp);
    }
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
template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> atan(fixed_num<T, I, f, r> fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    constexpr auto a = detail::eval_const<char, T, I, f, r>("-0.0464964749");
    constexpr auto b = detail::eval_const<char, T, I, f, r>("0.15931422");
    constexpr auto c = detail::eval_const<char, T, I, f, r>("0.327622764");
    // TODO: because fixed point cannot represent infinity, so we need to handle the case when fp is very large.
    auto abs_fp = abs(fp);
    auto x = abs_fp;
    auto x2 = x * x;
    fixed result = ((a * x2 + b) * x2 - c) * x2 * x + x;
    // if the input is negative, return negative result.
    if(fp.signbit_mask() & fp.internal_value())
        result = -result;
    return result;
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

/**
 * @brief Exponential function for fixed point number.
 *
 * On Q-formats (digits_int == 0, e.g. f == digits) exp(r) >= 1 does not fit
 * the storage type, so the raw-intermediate detail::exp_inline of pow is used
 * (with saturation semantics on overflow).
 */
template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> exp(fixed_num<T, I, f, r> fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    if constexpr(f == static_cast<unsigned int>(fixed::digits))
        return detail::exp_inline<T, I, f, r>(static_cast<I>(fp.internal_value()), I(0));
    else
        return detail::exp_impl<T, I, f, r, overflow_strategy::DEFAULT>(fp);
}

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> exp_sat(fixed_num<T, I, f, r> fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    if constexpr(f == static_cast<unsigned int>(fixed::digits))
        return detail::exp_inline<T, I, f, r>(static_cast<I>(fp.internal_value()), I(0));
    else
        return detail::exp_impl<T, I, f, r, overflow_strategy::SATURATION>(fp);
}

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> exp_modwarp(fixed_num<T, I, f, r> fp) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    if constexpr(f == static_cast<unsigned int>(fixed::digits))
        return detail::exp_inline<T, I, f, r>(static_cast<I>(fp.internal_value()), I(0));
    else
        return detail::exp_impl<T, I, f, r, overflow_strategy::MODWRAP>(fp);
}

/**
 * @brief Fast log2 based on polynomial fitting.
 *
 * This function perform exponent-digit decomposition first: x = m*2^e, 1 <= m < 2.
 * Then exponent e = floor(log2 x), we can use bitwise to get this.
 * And we use minimax poly to calc log(m).
 * Finally, according to logarithmic formula, result = e + log(m).
 *
 * For Q-formats (digits_int == 0, e.g. f == digits) the polynomial coefficients
 * would exceed the storage range, so log2 falls back to the raw-intermediate
 * ln machinery used by pow (detail::log_inline).
 *
 * @tparam T @see fixed_num
 * @tparam I @see fixed_num
 * @tparam f @see fixed_num
 * @tparam r @see fixed_num
 * @param fp the input x, must be positive.
 * @return log2(x) as a fixed-point approximation.
 */
template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> log2(fixed_num<T, I, f, r> fp)
{
    using fixed = fixed_num<T, I, f, r>;
    if(fp <= fixed(0))
        EIRIN_THROW_EXCEPTION(std::domain_error, "log2() domain error");

    if constexpr(f == static_cast<unsigned int>(fixed::digits))
    {
        // Q-formats (digits_int == 0): the log2 polynomial coefficients exceed
        // the storage range, so reuse the raw-intermediate ln machinery of pow.
        const auto ln = detail::log_inline(fp);
        constexpr auto sc = detail::pow_scales<T, I, f>{};
        using J = typename detail::make_signed<I>::type;
        const J hi = static_cast<J>(ln.hi);
        const J lo = static_cast<J>(ln.lo);
        // ln(b) at f bits: hi at 2^L, lo at 2^S
        const J ln_f = (sc.L >= f ? detail::pow_rshift(hi, sc.L - f) : hi << (f - sc.L)) +
                       detail::pow_rshift(lo, sc.S - f);
        // log2(b) = ln(b) * log2(e); log2(e)*2^61 = 0x2E2A8ECA5705FC00
        constexpr J log2e_f = detail::pow_scale_61<J, f>(0x2E2A8ECA5705FC00ll);
        const J log2_f = detail::pow_rshift(ln_f * log2e_f, f);
        return fixed::from_internal_value(static_cast<T>(log2_f));
    }

    constexpr unsigned int max_wide_fraction = static_cast<unsigned int>(fixed::digits) - 2 < 60u ?
                                                   static_cast<unsigned int>(fixed::digits) - 2 :
                                                   60u;
    constexpr unsigned int best_fraction = max_wide_fraction > f ? (2 * f < max_wide_fraction ? 2 * f : max_wide_fraction) : f;
    using work = fixed_num<T, I, best_fraction, false>;
    constexpr I isqrt2 = static_cast<I>(numbers::sqrt2_v<work>().internal_value());

    // e = floor(log2(fp)); m = fp / 2^e in [1, 2).
    const I x_i = static_cast<I>(fp.internal_value());
    const int e = static_cast<int>(fp.bit_width()) - 1 - static_cast<int>(f);
    I m_work;
    if(e >= 0)
        m_work = static_cast<I>(x_i >> e) << (best_fraction - f);
    else
        m_work = static_cast<I>(x_i << (-e)) << (best_fraction - f);

    // Reduce m to [sqrt(2)/2, sqrt(2)) so that t stays in the fit interval.
    int e2 = e;
    if(m_work >= isqrt2)
    {
        m_work >>= 1;
        ++e2;
    }

    const work t = work::from_internal_value(static_cast<T>(m_work - (static_cast<I>(1) << best_fraction)));
    const auto p = detail::log2_minimax<T, I, best_fraction, false>(t);

    // result = e2 + p, narrowed back to f bits with round-to-nearest.
    I p_f;
    if constexpr(best_fraction > f)
    {
        constexpr unsigned int shift = best_fraction - f;
        const I p_work = static_cast<I>(p.internal_value());
        if(p_work >= 0)
            p_f = (p_work + (static_cast<I>(1) << (shift - 1))) >> shift;
        else
            p_f = -(((-p_work) + (static_cast<I>(1) << (shift - 1))) >> shift);
    }
    else
    {
        p_f = static_cast<I>(p.internal_value());
    }

    return fixed::from_internal_value(static_cast<T>(static_cast<I>(e2) << f) + static_cast<T>(p_f));
}

/**
 * @brief ln function for fixed point number.
 *
 * Computed as log2(fp) * ln(2): the multiply form works for any fraction
 * width (the old log2_e divisor cannot be represented on formats whose
 * integer part is empty, e.g. Q127), and the constant is a raw 61-bit dyadic
 * so no parse limit applies.
 *
 * @tparam T @see fixed_num
 * @tparam I @see fixed_num
 * @tparam f @see fixed_num
 * @tparam r @see fixed_num
 * @param fp
 * @return log(fp)
 */
template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> log(fixed_num<T, I, f, r> fp)
{
    using fixed = fixed_num<T, I, f, r>;
    // ln(2)*2^61 = 0x162E42FEFA39EF00
    constexpr fixed ln2 = f < 20 ? fixed::from_internal_value(
                                       static_cast<T>(detail::pow_scale_61<I, f>(0x162E42FEFA39EF00ll))
                                   ) :
                                   numbers::ln2_v<fixed>();
    return log2(fp) * ln2;
}

/**
 * @brief the log10 function for fixed point number.
 *
 * Computed as log2(fp) * log10(2), see log for the motivation.
 *
 * @tparam T @see fixed_num
 * @tparam I @see fixed_num
 * @tparam f @see fixed_num
 * @tparam r @see fixed_num
 * @param fp
 * @return log10(fp)
 */
template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> log10(fixed_num<T, I, f, r> fp)
{
    using fixed = fixed_num<T, I, f, r>;
    // log10(2)*2^61 = 0x9A209A84FBCFF7A
    constexpr fixed log10_2 = f < 20 ? fixed::from_internal_value(
                                           static_cast<T>(detail::pow_scale_61<I, f>(0x9A209A84FBCFF7All))
                                       ) :
                                       detail::eval_const<char, T, I, f, r>("0.301029995663981195213738894724493027");
    return log2(fp) * log10_2;
}

/**
 * @brief High-precision fixed-point pow, computed as exp(e * ln(b)).
 *
 * Follows the same algorithm shape as glibc's double-precision pow
 * (sysdeps/ieee754/dbl-64/e_pow.c): detail::log_inline evaluates ln(b) as a
 * hi/lo split, e * ln(b) is formed as one wide product (e*hi + e*lo), and
 * detail::exp_inline finishes the exponentiation with the residual tail folded
 * into the argument reduction.
 *
 * Implementation notes:
 *  - The ln(1+t)/t minimax polynomial (degree 17, 60-bit coefficients, error
 *    ~= 7.4e-16) and the exp minimax polynomial (degree 9, 60-bit
 *    coefficients, error ~= 1.9e-14) are generated with sollya fpminimax
 *    following tools/sollya_fpminimax.py, and rescaled at compile time to fit
 *    the widest scale the intermediate type allows.
 *  - Out-of-range results saturate (positive overflow to max, negative
 *    overflow to 0), negative bases are supported for integer exponents.
 *
 * Works for any fixed_num<T, I, f, r>, including user-defined and unsigned
 * types; for unsigned types the base must be >= 1 (same restriction as exp).
 *
 * @tparam T @see fixed_num
 * @tparam I @see fixed_num
 * @tparam f @see fixed_num
 * @tparam r @see fixed_num
 * @param b the base.
 * @param e the exponent.
 * @return b^e as a fixed-point approximation.
 */
template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> pow(fixed_num<T, I, f, r> b, fixed_num<T, I, f, r> e) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    using UI = typename detail::make_signed<I>::type;

    // special cases
    if(b == fixed(0))
        return e == fixed(0) ? fixed(1) : fixed(0);
    if(e == fixed(0))
        return fixed(1);
    if(b == fixed(1))
        return fixed(1);
    if(e == fixed(1))
        return b;

    bool sign_neg = false;
    if constexpr(detail::is_signed_v<I>)
    {
        if(b < fixed(0))
        {
            // a negative base is only defined for integer exponents
            if(e.fractional_part() != 0)
                EIRIN_THROW_EXCEPTION(std::domain_error, "pow() domain error");
            sign_neg = ((static_cast<I>(e.internal_value()) >> f) & 1) != 0;
            b = -b;
        }
    }
    if(b == fixed(1)) // (-1)^e
        return sign_neg ? fixed(-1) : fixed(1);

    constexpr auto sc = detail::pow_scales<T, I, f>{};
    constexpr unsigned int L = sc.L;
    constexpr unsigned int S = sc.S;
    constexpr unsigned int Pp = sc.Pp;
    constexpr unsigned int f_plus_L = f + L;

    // ln(b) = hi/2^L + lo/2^S
    const auto ln = detail::log_inline(b);
    const UI ln_hi = static_cast<UI>(ln.hi);
    const UI ln_lo = static_cast<UI>(ln.lo);
    const UI e_j = static_cast<UI>(e.internal_value());

    // e*ln(b) = e*hi + e*lo, kept at f+L bits; pre-scale lo on very wide types
    // so that the raw e*lo product always fits in I
    constexpr unsigned int lo_bits = S - L;
    constexpr unsigned int elo_sh = (f + lo_bits > sc.I_bits - 1u) ? (f + lo_bits - (sc.I_bits - 1u)) : 0u;
    constexpr UI elo_bound = static_cast<UI>(1) << (f + sc.di_bits);
    const UI ln_abs = ln_hi < 0 ? -ln_hi : ln_hi;
    const UI e_abs = e_j < 0 ? -e_j : e_j;
    if(ln_abs != 0 && e_abs > (std::numeric_limits<UI>::max() - elo_bound) / ln_abs)
    {
        // |e * ln b| is beyond the representable range of the result
        const bool prod_neg = (ln_hi < 0) != (e_j < 0);
        fixed sat = prod_neg ? fixed(0) : fixed::from_internal_value(std::numeric_limits<T>::max());
        return sign_neg ? -sat : sat;
    }
    const UI ehi_raw = e_j * ln_hi; // at f+L bits
    const UI elo_part = detail::pow_rshift(e_j * (ln_lo >> elo_sh), lo_bits - elo_sh); // at f+L bits
    const UI A = ehi_raw + elo_part;

    // exponent at Pp bits, split into an f-bit head and a residual tail
    const UI prod = detail::pow_rshift(A, f_plus_L - Pp);
    const UI hi_exp = detail::pow_rshift(prod, Pp - f);
    const UI lo_exp = prod - (hi_exp << (Pp - f));

    const fixed res = detail::exp_inline<T, I, f, r>(static_cast<I>(hi_exp), static_cast<I>(lo_exp));
    return sign_neg ? -res : res;
}

template <typename T, typename I, unsigned int f, bool r>
EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> pow_fast(fixed_num<T, I, f, r> b, fixed_num<T, I, f, r> e) noexcept
{
    using fixed = fixed_num<T, I, f, r>;
    // special cases
    if(b == fixed(0))
        return e == fixed(0) ? fixed(1) : fixed(0);
    if(e == fixed(0))
        return fixed(1);
    if(b == fixed(1))
        return fixed(1);
    if(e == fixed(1))
        return b;

    constexpr fixed ln2 = f < 20 ? fixed::from_internal_value(
                                       static_cast<T>(detail::pow_scale_61<I, f>(0x162E42FEFA39EF00ll))
                                   ) :
                                   numbers::ln2_v<fixed>();

    return exp(e * log2(b * ln2));
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
