#ifndef EIRIN_MATH_DETAIL_MATH_IMPL_HPP
#define EIRIN_MATH_DETAIL_MATH_IMPL_HPP

#pragma once

#include "../fixed.hpp"
#include "../numbers.hpp"

namespace eirin
{
enum class overflow_strategy
{
    // indicates never consider overflow, and might cause UB.
    DEFAULT = EIRIN_OVERFLOW_DEFAULT,
    // mod-warp when overflow.
    MODWRAP = EIRIN_OVERFLOW_MODWRAP,
    // using saturation arithmetic rule when overflow.
    SATURATION = EIRIN_OVERFLOW_SAT
};

namespace detail
{
    template <typename T, typename I>
    EIRIN_ALWAYS_INLINE consteval T __eval_max_value() noexcept
    {
        if constexpr(std::numeric_limits<T>::is_specialized)
        {
            return std::numeric_limits<T>::max();
        }
        else if constexpr(detail::is_signed_v<T>)
        {
            constexpr auto digits = sizeof(T) * 8 - 1;
            return static_cast<T>((static_cast<I>(1) << digits) - 1);
        }
        else
        {
            constexpr auto digits = sizeof(T) * 8;
            return static_cast<T>((static_cast<I>(1) << digits) - 1);
        }
    }

    template <typename T, typename I>
    EIRIN_ALWAYS_INLINE consteval T __eval_min_value() noexcept
    {
        if constexpr(std::numeric_limits<T>::is_specialized)
        {
            return std::numeric_limits<T>::min();
        }
        else if constexpr(detail::is_signed_v<T>)
        {
            constexpr auto digits = sizeof(T) * 8 - 1;
            return static_cast<T>(-(static_cast<I>(1) << digits));
        }
        else
        {
            return static_cast<T>(0);
        }
    }

    template <typename T, typename I>
    inline constexpr T __max_value = __eval_max_value<T, I>();

    template <typename T, typename I>
    inline constexpr T __min_value = __eval_min_value<T, I>();

    // Signed round-half-away right shift; a no-op for sh == 0.
    template <typename V>
    constexpr V pow_rshift(V v, unsigned int sh) noexcept
    {
        if(sh == 0)
            return v;
        const V half = static_cast<V>(1) << (sh - 1);
        if(v >= 0)
            return (v + half) >> sh;
        return -(((-v) + half) >> sh);
    }

    constexpr uint64_t constexpr_isqrt(uint64_t x) noexcept
    {
        uint64_t y = 0;
        uint64_t m = uint64_t(1) << 62;
        while(m != 0)
        {
            const uint64_t b = y | m;
            y >>= 1;
            if(x >= b)
            {
                x -= b;
                y |= m;
            }
            m >>= 2;
        }
        return y;
    }

    constexpr std::array<int64_t, 4096> sqrt_mantissa_table = []()
    {
        std::array<int64_t, 4096> t{};
        for(int i = 1; i < 4096; ++i)
            t[i] = static_cast<int64_t>(constexpr_isqrt(static_cast<uint64_t>(i) << 32));
        return t;
    }();
    constexpr int64_t sin_minimax_coeff_k[5] = {-715827882, 35791387, -852162, 11824, -103};
    constexpr unsigned int sin_minimax_coeff_fraction = 32;

    consteval unsigned int sin_minimax_required_fraction() noexcept
    {
        unsigned int min_trailing = sin_minimax_coeff_fraction;
        for(int64_t k : sin_minimax_coeff_k)
        {
            if(k == 0)
                continue;
            uint64_t v = k < 0 ? uint64_t(0) - uint64_t(k) : uint64_t(k);
            unsigned int t = 0;
            while((v & 1u) == 0u)
            {
                ++t;
                v >>= 1;
            }
            if(t < min_trailing)
                min_trailing = t;
        }
        return sin_minimax_coeff_fraction - min_trailing;
    }

    template <typename T, typename I, unsigned int f, bool r, fixed_num<T, I, f, r> pi = numbers::pi_v<fixed_num<T, I, f, r>>()>
    EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> sin_taylor(fixed_num<T, I, f, r> fp) noexcept
    {
        using fixed = fixed_num<T, I, f, r>;
        // we reconginzed the sin(x) = x when x is small enough.
        // sin(0.00015) = 0.000149999999437, so we use 0.00015 as the threshold.
        constexpr const fixed small_reg_threshold = detail::eval_const<char, T, I, f, r>("0.00015");
        if(abs(fp) < small_reg_threshold)
            return fp;

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

        // reduce the range to [0, 1] due to sin is
        // symmetrical around PI / 2 in the domain [0, PI].
        if(x > fp1)
            x = fp2 - x;

        // we use tyler series to calculate sin(x).
        // n = 4 has enough precision.
        const auto x2 = x * x;
        constexpr auto a = detail::eval_const<char, T, I, f, r>("0.41123351671205660911810379166150629730473747530170");
        constexpr auto b = detail::eval_const<char, T, I, f, r>("0.12337005501361698273543113749845188919142124259051");
        constexpr auto c = detail::eval_const<char, T, I, f, r>("0.05874764524457951558830054166592947104353392504310");
        constexpr auto d = detail::eval_const<char, T, I, f, r>("0.03426945972600471742650864930512552477539478960847");
        auto res = fixed::pi() * x * (fp1 - a * x2 * (fp1 - b * x2 * (fp1 - c * x2 * (fp1 - d * x2)))) / 2;
        return negative ? -res : res;
    }

    template <typename T, typename I, unsigned int f, bool r, fixed_num<T, I, f, r> pi = numbers::pi_v<fixed_num<T, I, f, r>>()>
    EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> sin_minimax(fixed_num<T, I, f, r> fp) noexcept
    {
        using fixed = fixed_num<T, I, f, r>;
        // we reconginzed the sin(x) = x when x is small enough.
        // sin(0.00015) = 0.000149999999437, so we use 0.00015 as the threshold.
        constexpr const fixed small_reg_threshold = detail::eval_const<char, T, I, f, r>("0.00015");
        if(abs(fp) < small_reg_threshold)
            return fp;

        constexpr auto fp_double_pi = 2 * pi;
        auto x = fixed(fp);
        // mod x into [0, 2pi]
        x = fp % fp_double_pi;

        auto negative = false;
        // judge if x is negative, if so, we can use sin(-x) = -sin(x)
        if(x < fixed(0))
        {
            x = -x;
            negative = true;
        }
        if(x >= 2 * pi) x = fixed(0);
        // reduce the range to (0, pi] by sin(x) = -sin(x - pi) when x > pi.
        if(x > pi)
        {
            x = 2 * pi - x;
            negative = !negative;
        }
        // reduce the range to [0, pi/2] due to sin is
        // symmetrical around PI in the domain [0, PI].
        if(x > pi / 2)
        {
            x = pi - x;
        }

        const fixed x2 = x * x;
        constexpr fixed c1 = detail::eval_const<char, T, I, f, r>("-0.1666666665114462375640869140625");
        constexpr fixed c2 = detail::eval_const<char, T, I, f, r>("0.00833333167247474193572998046875");
        constexpr fixed c3 = detail::eval_const<char, T, I, f, r>("-0.0001984094269573688507080078125");
        constexpr fixed c4 = detail::eval_const<char, T, I, f, r>("0.0000027529895305633544921875");
        constexpr fixed c5 = detail::eval_const<char, T, I, f, r>("-0.00000002398155629634857177734375");

        auto res = x * (1 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * (c4 + x2 * c5)))));
        return negative ? -res : res;
    }

    /**
     * @brief Minimax polynomial for log2(1+t)/t, fitted on
     * t in [sqrt(2)/2 - 1, sqrt(2) - 1].
     *
     * The coefficients are generated with sollya (minimax on the reduced interval,
     * then rounded to 32 fraction bits); the quantized error of x*Q - log2(1+x)
     * is ~= 6.8e-11, about 0.29 ulp at 32 fraction bits. The result is t*Q(t)
     * ~= log2(1+t).
     */
    template <typename T, typename I, unsigned int f, bool r>
    EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> log2_minimax(fixed_num<T, I, f, r> t) noexcept
    {
        using fixed = fixed_num<T, I, f, r>;
        constexpr fixed c0 = detail::eval_const<char, T, I, f, r>("1.44269504095427691936492919921875");
        constexpr fixed c1 = detail::eval_const<char, T, I, f, r>("-0.72134752548299729824066162109375");
        constexpr fixed c2 = detail::eval_const<char, T, I, f, r>("0.480898332782089710235595703125");
        constexpr fixed c3 = detail::eval_const<char, T, I, f, r>("-0.360672764480113983154296875");
        constexpr fixed c4 = detail::eval_const<char, T, I, f, r>("0.28853964991867542266845703125");
        constexpr fixed c5 = detail::eval_const<char, T, I, f, r>("-0.2405051491223275661468505859375");
        constexpr fixed c6 = detail::eval_const<char, T, I, f, r>("0.20612564799375832080841064453125");
        constexpr fixed c7 = detail::eval_const<char, T, I, f, r>("-0.17905164719559252262115478515625");
        constexpr fixed c8 = detail::eval_const<char, T, I, f, r>("0.15863760374486446380615234375");
        constexpr fixed c9 = detail::eval_const<char, T, I, f, r>("-0.15664058853872120380401611328125");
        constexpr fixed c10 = detail::eval_const<char, T, I, f, r>("0.15646688290871679782867431640625");
        constexpr fixed c11 = detail::eval_const<char, T, I, f, r>("-0.0887590800411999225616455078125");

        if constexpr(sizeof(I) > sizeof(int64_t))
        {
            // int128*int128 is heavy-weight op.
            // use estrin's scheme to calc.
            const fixed t2 = t * t;
            const fixed t4 = t2 * t2;
            const fixed t6 = t2 * t4;
            const fixed q0 = c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * c5))));
            const fixed q1 = c6 + t * (c7 + t * (c8 + t * (c9 + t * (c10 + t * c11))));
            const fixed q = q0 + t6 * q1;
            return t * q;
        }
        else
        {
            return t * (c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * (c6 + t * (c7 + t * (c8 + t * (c9 + t * (c10 + t * c11)))))))))));
        }
    }

    /**
     * @brief Minimax polynomial for exp, fitted on [0, ln2].
     *
     * The coefficients are generated by tools/sollya_fpminimax.py: interval
     * [0, log(2)], degree 9, fixed mode, 32 fraction bits; sollya
     * dirtyinfnorm ~= 9.25e-14, about 0.0004 ulp at 32 fraction bits. Degree 9 is
     * the practical ceiling: degree 10 gains nothing because the 32-bit
     * coefficient format saturates around 8e-14.
     * The input is the reduced r with 0 <= r < ln2.
     *
     * @tparam T @see fixed_num
     * @tparam I @see fixed_num
     * @tparam f @see fixed_num
     * @tparam r @see fixed_num
     * @param fp the reduced r.
     * @return a fixed-point approximation of exp(r).
     */
    template <typename T, typename I, unsigned int f, bool r>
    EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> exp_minimax(fixed_num<T, I, f, r> fp) noexcept
    {
        using fixed = fixed_num<T, I, f, r>;
        constexpr fixed c2 = detail::eval_const<char, T, I, f, r>("0.5");
        constexpr fixed c3 = detail::eval_const<char, T, I, f, r>("0.16666666720993816852569580078125");
        constexpr fixed c4 = detail::eval_const<char, T, I, f, r>("0.0416666553355753421783447265625");
        constexpr fixed c5 = detail::eval_const<char, T, I, f, r>("0.00833342759869992733001708984375");
        constexpr fixed c6 = detail::eval_const<char, T, I, f, r>("0.00138848018832504749298095703125");
        constexpr fixed c7 = detail::eval_const<char, T, I, f, r>("0.00019941781647503376007080078125");
        constexpr fixed c8 = detail::eval_const<char, T, I, f, r>("0.000023395754396915435791015625");
        constexpr fixed c9 = detail::eval_const<char, T, I, f, r>("0.00000378047116100788116455078125");

        if constexpr(sizeof(I) > sizeof(int64_t))
        {
            // int128*int128 is heavy-weight op.
            // use estrin's scheme to calc.
            const fixed x2 = fp * fp;
            const fixed x4 = x2 * x2;
            const fixed x8 = x4 * x4;
            const fixed a0 = fixed(1) + fp; // c0 + c1*x
            const fixed a1 = c2 + fp * c3;
            const fixed a2 = c4 + fp * c5;
            const fixed a3 = c6 + fp * c7;
            const fixed a4 = c8 + fp * c9;
            const fixed s0 = a0 + x2 * a1; // degree <= 3
            const fixed s1 = a2 + x2 * a3; // degree 4..7, in parallel with s0
            const fixed low = s0 + x4 * s1; // degree <= 7
            return low + x8 * a4; // degree <= 9
        }
        else
        {
            // Horner keeps slightly better accuracy on narrow intermediates, where
            // the per-step truncation at low working fractions is not negligible.
            return fixed(1) + fp * (fixed(1) + fp * (c2 + fp * (c3 + fp * (c4 + fp * (c5 + fp * (c6 + fp * (c7 + fp * (c8 + fp * c9))))))));
        }
    }

    /**
     * @brief High-precision, high-performance fixed-point exp fitting function.
     *
     * Uses range reduction: exp(x) = 2^k * exp(r), where k = floor(x * log2(e))
     * and r = x - k * ln2 with 0 <= r < ln2. exp(r) is approximated by a minimax
     * polynomial, and the 2^k scaling is applied directly by shifting the internal
     * value (right shift for negative k), avoiding the integer power multiplication
     * and division used by the previous implementation.
     *
     * To avoid rounding loss on low-precision types, the range reduction and the
     * polynomial evaluation are performed at a wider fraction
     * (work_f = min(2f, digits-2, 60) bits) and narrowed back to f bits in one
     * final round-to-nearest step. The 60-bit cap matches sin and keeps the
     * work format well inside the intermediate type. Unlike sin's digits-3
     * cap, the wide type here only needs to hold the reduced r (|r| <= ln2),
     * not the original input, so one extra fraction bit is available.
     *
     * Works for any fixed_num<T, I, f, r> (including user-defined signed/unsigned
     * types): for unsigned types k is always non-negative and the reduction interval
     * [0, ln2) keeps r_work non-negative.
     *
     * @tparam T @see fixed_num
     * @tparam I @see fixed_num
     * @tparam f @see fixed_num
     * @tparam r @see fixed_num
     * @tparam os overflow handling strategy, see overflow_strategy.
     * @param fp the input x.
     * @return exp(x) as a fixed-point approximation.
     */
    template <typename T, typename I, unsigned int f, bool r, overflow_strategy os = overflow_strategy::DEFAULT>
    EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> exp_impl(fixed_num<T, I, f, r> fp) noexcept
    {
        using fixed = fixed_num<T, I, f, r>;
        if(fp == fixed(0))
            return fixed(1);

        // max wide fraction: min(2f, digits-2, 60), never below f.
        constexpr unsigned int max_wide_fraction = static_cast<unsigned int>(fixed::digits) - 2 < 60u ?
                                                       static_cast<unsigned int>(fixed::digits) - 2 :
                                                       60u;
        constexpr unsigned int best_fraction = max_wide_fraction > f ? (2 * f < max_wide_fraction ? 2 * f : max_wide_fraction) : f;
        using wide_fixed = fixed_num<T, I, best_fraction, false>;

        constexpr I log2e_f = static_cast<I>(numbers::log2e_v<fixed>().internal_value());

        const I x_i = static_cast<I>(fp.internal_value());
        const I x_best = x_i << (best_fraction - f);

        // k = floor(x * log2(e)), computed at best_fraction precision.
        const I prod = x_best * log2e_f;
        I k = prod >> (best_fraction + f);

        // process possible overflow
        if constexpr(os == overflow_strategy::SATURATION)
        {
            if(k > static_cast<I>(fixed::digits_int) + 2)
                return fixed::from_internal_value(std::numeric_limits<T>::max());
            if constexpr(detail::is_signed_v<I>)
            {
                if(k < -static_cast<I>(f) - 3)
                    return fixed(0);
            }
        }
        else if constexpr(os == overflow_strategy::MODWRAP)
        {
            if constexpr(detail::is_signed_v<I>)
            {
                if(k < -static_cast<I>(f) - 3)
                    return fixed(0);
            }
            if(k >= static_cast<I>(sizeof(T) * 8) - static_cast<I>(f) + static_cast<I>(best_fraction))
                return fixed(0);
        }

        // r = x - k * ln2, with correction loops converging r to [0, ln2).
        I r_work;
        if constexpr(2 * best_fraction <= 61)
        {
            // floor(ln2·2^55) << 6 = 0x162E42FEFA39EF00ll
            constexpr I ln2_2w = static_cast<I>(0x162E42FEFA39EF00ll >> (61 - 2 * best_fraction));
            constexpr I ln2_hi = ln2_2w >> best_fraction;
            constexpr I ln2_lo = ln2_2w - (ln2_hi << best_fraction);
            I r_2w = ((x_best - k * ln2_hi) << best_fraction) - k * ln2_lo;
            while(r_2w >= ln2_2w)
            {
                ++k;
                r_2w -= ln2_2w;
            }
            if constexpr(detail::is_signed_v<I>)
            {
                while(r_2w < 0)
                {
                    --k;
                    r_2w += ln2_2w;
                }
            }
            r_work = (r_2w + (static_cast<I>(1) << (best_fraction - 1))) >> best_fraction;
        }
        else
        {
            constexpr I ln2_best = static_cast<I>(
                detail::eval_const<char, T, I, best_fraction, false>("0.693147180559945309417232121458176568")
                    .internal_value()
            );
            r_work = x_best - k * ln2_best;
            while(r_work >= ln2_best)
            {
                ++k;
                r_work -= ln2_best;
            }
            if constexpr(detail::is_signed_v<I>)
            {
                while(r_work < 0)
                {
                    --k;
                    r_work += ln2_best;
                }
            }
        }

        // Polynomial evaluation: ~= exp(r) * 2^best_fraction.
        const wide_fixed wr = wide_fixed::from_internal_value(static_cast<T>(r_work));
        const auto e_r = exp_minimax<T, I, best_fraction, false>(wr);

        // result = exp(r) * 2^k, narrowed back to f bits (round-to-nearest on right
        // shifts); SATURATION clamps, DEFAULT and MODWRAP truncate via the cast.
        const I e_i = static_cast<I>(e_r.internal_value());
        const int k_int = static_cast<int>(k);
        const int sh = k_int - (static_cast<int>(best_fraction) - static_cast<int>(f));
        I v;
        if(sh >= 0)
            v = e_i << sh;
        else
            v = (e_i + (static_cast<I>(1) << (-sh - 1))) >> (-sh);

        if constexpr(os == overflow_strategy::SATURATION)
        {
            if(v > static_cast<I>(std::numeric_limits<T>::max()))
                return fixed::from_internal_value(std::numeric_limits<T>::max());
        }
        return fixed::from_internal_value(static_cast<T>(v));
    }

    // Shared precision scales of the pow implementation, derived only from the
    // fixed-point template parameters T, I and f so that user-defined storage
    // and intermediate types (including 128/256-bit integers) are supported.
    template <typename T, typename I, unsigned int f>
    struct pow_scales
    {
        static constexpr unsigned int digits = static_cast<unsigned int>(detail::__eval_max_bit_width<T>());
        static constexpr unsigned int I_bits = static_cast<unsigned int>(detail::__eval_max_bit_width<I>());
        static constexpr unsigned int digits_int = digits - f;
        // mantissa reduction scale, never below f so all shifts are non-negative
        static constexpr unsigned int bf = []() constexpr -> unsigned int
        {
            constexpr unsigned int wide = 2 * f < (digits - 2 < 60u ? digits - 2 : 60u) ?
                                              2 * f :
                                              (digits - 2 < 60u ? digits - 2 : 60u);
            return wide > f ? wide : f;
        }();
        // polynomial coefficient scale: keeps q*t inside I
        static constexpr unsigned int C = 60u < (I_bits - bf - 1u) ? 60u : (I_bits - bf - 1u);
        // ln(1+t) is evaluated at P = C + bf bits
        static constexpr unsigned int P = C + bf;
        // bits needed by the integer part of ln(b)
        static constexpr unsigned int e2_bits = std::bit_width(std::max(f, digits_int));
        // combined ln scale (the e2*ln2 term must fit)
        static constexpr unsigned int S = P < (I_bits - 1u - e2_bits) ? P : (I_bits - 1u - e2_bits);
        // bits needed for the largest representable |e*ln(b)|
        static constexpr unsigned int exp_bits = std::bit_width(std::max(f, digits_int) + 2u);
        static constexpr unsigned int di_bits = std::bit_width(digits_int);
        // hi/lo split scale: e*ln_hi fits in I for every representable result
        static constexpr unsigned int L_cap = (I_bits - 1u) > (f + exp_bits) ? (I_bits - 1u - f - exp_bits) : 0u;
        static constexpr unsigned int L = S < L_cap ? S : L_cap;
        // exponent product scale, capped so the product fits in I
        static constexpr unsigned int Pp = 2 * f < (f + L) ? 2 * f : (f + L);
    };

    /**
     * @brief The hi/lo split of ln(b) produced by log_inline.
     */
    template <typename T, typename I, unsigned int f, bool r>
    struct pow_ln_hi_lo
    {
        I hi; // ln(b)*2^L, the rounded head
        I lo; // (ln(b) - hi/2^L)*2^S, the residual tail, |lo| < 2^(S-L)
    };

    /**
     * @brief Natural logarithm of a positive fixed-point value as hi+lo.
     *
     * Mirrors glibc's log_inline (sysdeps/ieee754/dbl-64/e_pow.c): ln(b) is
     * reduced to a rounded head hi at 2^L scale plus a residual tail lo at 2^S
     * scale, ln(b) ~= hi/2^L + lo/2^S. The tail carries the extra precision
     * that pow_new needs when it forms e*ln(b) without an e-multiplied
     * rounding error.
     *
     * @tparam T @see fixed_num
     * @tparam I @see fixed_num
     * @tparam f @see fixed_num
     * @tparam r @see fixed_num
     * @param b the input, must be positive.
     * @return the {hi, lo} split of ln(b).
     */
    template <typename T, typename I, unsigned int f, bool r>
    EIRIN_ALWAYS_INLINE constexpr pow_ln_hi_lo<T, I, f, r> log_inline(fixed_num<T, I, f, r> b) noexcept
    {
        using J = typename make_signed<I>::type;
        constexpr auto sc = pow_scales<T, I, f>{};
        constexpr unsigned int bf = sc.bf;
        constexpr unsigned int C = sc.C;
        constexpr unsigned int P = sc.P;
        constexpr unsigned int S = sc.S;
        constexpr unsigned int L = sc.L;

        // decompose b = m * 2^e2 with m in [sqrt(2)/2, sqrt(2))
        const I b_i = static_cast<I>(b.internal_value());
        const int e2 = static_cast<int>(b.bit_width()) - 1 - static_cast<int>(f);
        const I b_best = b_i << (bf - f);
        I m = e2 >= 0 ? (b_best >> e2) : (b_best << (-e2));
        int e2_adj = e2;
        // sqrt(2), exact 61-bit dyadic
        constexpr I sqrt2_bf = eval_dyadic<I, bf>("0x1.6a09e667f3bcdp+0");
        if(m >= sqrt2_bf)
        {
            m >>= 1;
            ++e2_adj;
        }
        const J t = static_cast<J>(m) - (static_cast<J>(1) << bf);

        // ln(1+t)/t, degree 17, sollya fpminimax fixed 60 bits, interval
        // [sqrt(2)/2-1, sqrt(2)-1], error ~= 7.4e-16. The coefficients are
        // exact 60-bit dyadics written as hexfloat strings.
        constexpr J c0 = eval_dyadic<J, C>("0x1.ffffffffffff0e6p-1");
        constexpr J c1 = eval_dyadic<J, C>("-0x1.ffffffffffdaafp-2");
        constexpr J c2 = eval_dyadic<J, C>("0x1.555555555f23f68p-2");
        constexpr J c3 = eval_dyadic<J, C>("-0x1.00000000461f6ap-2");
        constexpr J c4 = eval_dyadic<J, C>("0x1.999999892210798p-3");
        constexpr J c5 = eval_dyadic<J, C>("-0x1.555555016ce5a8p-3");
        constexpr J c6 = eval_dyadic<J, C>("0x1.24924e50d386248p-3");
        constexpr J c7 = eval_dyadic<J, C>("-0x1.000017c289f30b8p-3");
        constexpr J c8 = eval_dyadic<J, C>("0x1.c71ae600366e65p-4");
        constexpr J c9 = eval_dyadic<J, C>("-0x1.99924f383cced7p-4");
        constexpr J c10 = eval_dyadic<J, C>("0x1.747af854e11406p-4");
        constexpr J c11 = eval_dyadic<J, C>("-0x1.55f7805d58507fp-4");
        constexpr J c12 = eval_dyadic<J, C>("0x1.3a12f319c9ef1dp-4");
        constexpr J c13 = eval_dyadic<J, C>("-0x1.1cbb379b98f8e6p-4");
        constexpr J c14 = eval_dyadic<J, C>("0x1.1115aa4733cb5ap-4");
        constexpr J c15 = eval_dyadic<J, C>("-0x1.301e681a7c28e4p-4");
        constexpr J c16 = eval_dyadic<J, C>("0x1.266013926cee17p-4");
        constexpr J c17 = eval_dyadic<J, C>("-0x1.19436b8446c7f4p-5");

        // with a wide mantissa the per-step truncation is far below the
        // polynomial error, so a plain shift keeps the Horner branch-free
        J q = c17;
        if constexpr(bf >= 45u)
        {
            q = (q * t >> bf) + c16;
            q = (q * t >> bf) + c15;
            q = (q * t >> bf) + c14;
            q = (q * t >> bf) + c13;
            q = (q * t >> bf) + c12;
            q = (q * t >> bf) + c11;
            q = (q * t >> bf) + c10;
            q = (q * t >> bf) + c9;
            q = (q * t >> bf) + c8;
            q = (q * t >> bf) + c7;
            q = (q * t >> bf) + c6;
            q = (q * t >> bf) + c5;
            q = (q * t >> bf) + c4;
            q = (q * t >> bf) + c3;
            q = (q * t >> bf) + c2;
            q = (q * t >> bf) + c1;
            q = (q * t >> bf) + c0;
        }
        else
        {
            q = pow_rshift(q * t, bf) + c16;
            q = pow_rshift(q * t, bf) + c15;
            q = pow_rshift(q * t, bf) + c14;
            q = pow_rshift(q * t, bf) + c13;
            q = pow_rshift(q * t, bf) + c12;
            q = pow_rshift(q * t, bf) + c11;
            q = pow_rshift(q * t, bf) + c10;
            q = pow_rshift(q * t, bf) + c9;
            q = pow_rshift(q * t, bf) + c8;
            q = pow_rshift(q * t, bf) + c7;
            q = pow_rshift(q * t, bf) + c6;
            q = pow_rshift(q * t, bf) + c5;
            q = pow_rshift(q * t, bf) + c4;
            q = pow_rshift(q * t, bf) + c3;
            q = pow_rshift(q * t, bf) + c2;
            q = pow_rshift(q * t, bf) + c1;
            q = pow_rshift(q * t, bf) + c0;
        }
        const J ln_m = q * t; // exact, at P = C + bf bits

        // ln(b) = e2*ln2 + ln(1+t) at S bits; ln(2), exact 61-bit dyadic
        constexpr J ln2_S = eval_dyadic<J, S>("0x1.62e42fefa39efp-1");
        const J ln_S = static_cast<J>(e2_adj) * ln2_S + (P > S ? pow_rshift(ln_m, P - S) : ln_m);

        const J ln_hi = pow_rshift(ln_S, S - L);
        const J ln_lo = ln_S - (ln_hi << (S - L));
        return {static_cast<I>(ln_hi), static_cast<I>(ln_lo)};
    }

    /**
     * @brief exp(x + lo) with the tail folded into the argument reduction.
     *
     * Mirrors glibc's exp_inline (sysdeps/ieee754/dbl-64/e_pow.c): the head
     * x_raw (the exponent as an I value at 2^f scale) plus the residual tail
     * lo_raw (at 2^Pp scale, |lo| <= 2^-f) is reduced to k*ln2 + r with r in
     * [0, ln2), and the tail is added to r before the minimax polynomial
     * evaluation. Out-of-range inputs saturate (positive overflow to max,
     * negative overflow to 0). The head is passed as an I value because the
     * exponent range can exceed the storage type T on high-fraction formats.
     *
     * @tparam T @see fixed_num
     * @tparam I @see fixed_num
     * @tparam f @see fixed_num
     * @tparam r @see fixed_num
     * @param x_raw the rounded exponent head at 2^f scale.
     * @param lo_raw the exponent residual tail at 2^Pp scale.
     * @return exp(x_raw/2^f + lo_raw/2^Pp).
     */
    template <typename T, typename I, unsigned int f, bool r>
    EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> exp_inline(I x_raw, I lo_raw) noexcept
    {
        using fixed = fixed_num<T, I, f, r>;
        using J = typename make_signed<I>::type;
        constexpr auto sc = pow_scales<T, I, f>{};
        constexpr unsigned int bf = sc.bf;
        constexpr unsigned int C = sc.C;
        constexpr unsigned int Pp = sc.Pp;
        constexpr int digits_int = static_cast<int>(sc.digits_int);

        if(x_raw == 0 && lo_raw == 0)
            return fixed(1);

        const J x_i = static_cast<J>(x_raw);
        const J x_best = x_i << (bf - f);

        // k = floor(x * log2(e)); log2(e), exact 61-bit dyadic. On
        // high-fraction formats the exponent range can be so large that the
        // full product overflows the intermediate type; pre-scaling x_best by
        // k_sh bits only perturbs k slightly, which the reduction loops below
        // correct.
        constexpr unsigned int kc_bits = f < (sc.I_bits - 2u) ? f : (sc.I_bits - 2u);
        constexpr J log2e_kc = eval_dyadic<J, kc_bits>("0x1.71547652b82fep+0");
        constexpr unsigned int k_sh = (bf + kc_bits + sc.exp_bits + 2u > sc.I_bits) ?
                                          (bf + kc_bits + sc.exp_bits + 2u - sc.I_bits) :
                                          0u;
        J k = ((x_best >> k_sh) * log2e_kc) >> (bf + kc_bits - k_sh);

        // saturation for out-of-range exponents
        if(k > static_cast<J>(digits_int) + 2)
            return fixed::from_internal_value(std::numeric_limits<T>::max());
        if constexpr(detail::is_signed_v<J>)
        {
            if(k < -static_cast<J>(f) - 3)
                return fixed(0);
        }

        // r = x - k*ln2 with the tail folded in, reduced to [0, ln2)
        // ln(2), exact 61-bit dyadic
        J r_work;
        if constexpr(2 * bf <= 61)
        {
            constexpr J ln2_2w = eval_dyadic<J, 2 * bf>("0x1.62e42fefa39efp-1");
            constexpr J ln2_hi = ln2_2w >> bf;
            constexpr J ln2_lo = ln2_2w - (ln2_hi << bf);
            J r_2w = ((x_best - k * ln2_hi) << bf) - k * ln2_lo;
            r_2w += static_cast<J>(lo_raw) << (2 * bf - Pp);
            while(r_2w >= ln2_2w)
            {
                ++k;
                r_2w -= ln2_2w;
            }
            if constexpr(detail::is_signed_v<J>)
            {
                while(r_2w < 0)
                {
                    --k;
                    r_2w += ln2_2w;
                }
            }
            r_work = (r_2w + (static_cast<J>(1) << (bf - 1))) >> bf;
        }
        else
        {
            constexpr J ln2_best = eval_dyadic<J, bf>("0x1.62e42fefa39efp-1");
            r_work = x_best - k * ln2_best;
            r_work += static_cast<J>(lo_raw) >> (Pp - bf);
            while(r_work >= ln2_best)
            {
                ++k;
                r_work -= ln2_best;
            }
            if constexpr(detail::is_signed_v<J>)
            {
                while(r_work < 0)
                {
                    --k;
                    r_work += ln2_best;
                }
            }
        }

        // exp(r) on [0, ln2], degree 9, sollya fpminimax fixed 60 bits,
        // error ~= 1.9e-14. The coefficients are exact 60-bit dyadics written
        // as hexfloat strings.
        constexpr J e0 = eval_dyadic<J, C>("0x1.fffffffffff5416p-1");
        constexpr J e1 = eval_dyadic<J, C>("0x1.00000000060484dp+0");
        constexpr J e2 = eval_dyadic<J, C>("0x1.fffffffb8d58a54p-2");
        constexpr J e3 = eval_dyadic<J, C>("0x1.555555f87611418p-3");
        constexpr J e4 = eval_dyadic<J, C>("0x1.55553d9fb276c8p-5");
        constexpr J e5 = eval_dyadic<J, C>("0x1.11130ad0aad048p-7");
        constexpr J e6 = eval_dyadic<J, C>("0x1.6be39be7d17e4p-10");
        constexpr J e7 = eval_dyadic<J, C>("0x1.a348de65ea4cp-13");
        constexpr J e8 = eval_dyadic<J, C>("0x1.81cf6e9a52ep-16");
        constexpr J e9 = eval_dyadic<J, C>("0x1.0664ec1a0ccp-18");

        J q = e9;
        if constexpr(bf >= 45u)
        {
            q = (q * r_work >> bf) + e8;
            q = (q * r_work >> bf) + e7;
            q = (q * r_work >> bf) + e6;
            q = (q * r_work >> bf) + e5;
            q = (q * r_work >> bf) + e4;
            q = (q * r_work >> bf) + e3;
            q = (q * r_work >> bf) + e2;
            q = (q * r_work >> bf) + e1;
            q = (q * r_work >> bf) + e0;
        }
        else
        {
            q = pow_rshift(q * r_work, bf) + e8;
            q = pow_rshift(q * r_work, bf) + e7;
            q = pow_rshift(q * r_work, bf) + e6;
            q = pow_rshift(q * r_work, bf) + e5;
            q = pow_rshift(q * r_work, bf) + e4;
            q = pow_rshift(q * r_work, bf) + e3;
            q = pow_rshift(q * r_work, bf) + e2;
            q = pow_rshift(q * r_work, bf) + e1;
            q = pow_rshift(q * r_work, bf) + e0;
        }
        // exp(r) at bf scale
        const J e_r = C > bf ? pow_rshift(q, C - bf) : (C < bf ? q << (bf - C) : q);

        // result = exp(r) * 2^k, narrowed back to f bits
        const int k_int = static_cast<int>(k);
        const int sh = k_int - (static_cast<int>(bf) - static_cast<int>(f));
        J v;
        if(sh >= 0)
            v = e_r << sh;
        else
            v = (e_r + (static_cast<J>(1) << (-sh - 1))) >> (-sh);
        if(v > static_cast<J>(std::numeric_limits<T>::max()))
            v = static_cast<J>(std::numeric_limits<T>::max());
        return fixed::from_internal_value(static_cast<T>(v));
    }

    template <typename T, typename I, unsigned int f, bool r>
    EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> atan_impl(fixed_num<T, I, f, r> x) noexcept
    {
        const auto x2 = x * x;
        // atan(x) = x * (c0 + c1 * t + c2 * t^2 + ... + cn * t^n)
        //         = x * (c0 + t * (c1 + c2 * t * (... + cn * t)))
        if constexpr(f <= 14)
        {
            constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.ffe728916fc60f4ep-1"); // x
            constexpr auto c1 = detail::eval_const<char, T, I, f, r>("-0x1.4e0be4cb5e8651ecp-2"); // x^3
            constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.40dc98abe713eaap-3"); // x^5
            constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.7117e8107b04d48p-5"); // x^7
            return x * (c0 + x2 * (c1 + x2 * (c2 + x2 * c3)));
        }
        else if constexpr(f <= 20)
        {
            // [0, 1/32]                   [1/32, 1/8]                 [1/8, 1/4]                  [1/4, 1/2]                   [1/2, 1]
            // "0x1.ffffffffffff8e74p-1"  "0x1.fffffffeb1a32d0ep-1"  "0x1.fffffb0105f5303cp-1"  "0x1.fffc848be606a9c8p-1"  "0x1.ffab0513821c0ca8p-1"
            // "-0x1.55555554e3c2c398p-2" "-0x1.55554b35e0788014p-2" "-0x1.55504c9d7f0c5128p-2" "-0x1.546fab8c4e0c38f8p-2" "-0x1.4e11c4a6dc7c687cp-2"
            // "0x1.999987d9962ce7p-3"    "0x1.99852a887a2d7a18p-3"  "0x1.97d8445da66468cp-3"  "0x1.84c62b86efc30208p-3"  "0x1.5aaca539e2989cap-3"
            // "-0x1.2420a37c73d54c3p-3"  "-0x1.1d2820673633c27p-3"  "-0x1.03ce4d2d23852fep-3"  "-0x1.7696f1432233f08p-4"  "-0x1.2bb2d69f894f16cp-4"
            //                                                                                                              "0x1.0928ae4258eb8bcp-6"
            // find msb, because x is in [0, 1], the msb must <= fraction.
            // this might be ugly, but we can use msb to directly switch.
            const auto bw = detail::bit_width(static_cast<detail::make_unsigned_t<T>>(x.internal_value()));
            switch(bw)
            {
            case f - 4:
            case f - 3:
                // this should fall in [1 << (f - 5), 1 << (f - 3)], means [1/32, 1/8)
                {
                    constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.fffffffeb1a32d0ep-1"); // x
                    constexpr auto c1 = detail::eval_const<char, T, I, f, r>("-0x1.55554b35e0788014p-2"); // x^3
                    constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.99852a887a2d7a18p-3"); // x^5
                    constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.1d2820673633c27p-3"); // x^7
                    return x * (c0 + x2 * (c1 + x2 * (c2 + x2 * c3)));
                }
                break;
            case f - 2:
                // this should fall in [1 << (f - 3), 1 << (f - 2)], means [1/8, 1/4)
                {
                    constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.fffffb0105f5303cp-1"); // x
                    constexpr auto c1 = detail::eval_const<char, T, I, f, r>("-0x1.55504c9d7f0c5128p-2"); // x^3
                    constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.97d8445da66468cp-3"); // x^5
                    constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.03ce4d2d23852fep-3"); // x^7
                    return x * (c0 + x2 * (c1 + x2 * (c2 + x2 * c3)));
                }
                break;
            case f - 1:
                // this should fall in [1 << (f - 2), 1 << (f - 1)), means [1/4, 1/2)
                {
                    constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.fffc848be606a9c8p-1"); // x
                    constexpr auto c1 = detail::eval_const<char, T, I, f, r>("-0x1.546fab8c4e0c38f8p-2"); // x^3
                    constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.84c62b86efc30208p-3"); // x^5
                    constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.7696f1432233f08p-4"); // x^7
                    return x * (c0 + x2 * (c1 + x2 * (c2 + x2 * c3)));
                }
                break;
            case f:
            case f + 1:
                // this should fall in [1 << (f - 1), 1 << f], means [1/2, 1]
                {
                    constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.ffab0513821c0ca8p-1"); // x
                    constexpr auto c1 = detail::eval_const<char, T, I, f, r>("-0x1.4e11c4a6dc7c687cp-2"); // x^3
                    constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.5aaca539e2989cap-3"); // x^5
                    constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.2bb2d69f894f16cp-4"); // x^7
                    constexpr auto c4 = detail::eval_const<char, T, I, f, r>("0x1.0928ae4258eb8bcp-6"); // x^9
                    return x * (c0 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * c4))));
                }
                break;
            default:
                // other ranges, should be [0, 1/32]
                {
                    constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.ffffffffffff8e74p-1"); // x
                    constexpr auto c1 = detail::eval_const<char, T, I, f, r>("-0x1.55555554e3c2c398p-2"); // x^3
                    constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.999987d9962ce7p-3"); // x^5
                    constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.2420a37c73d54c3p-3"); // x^7
                    return x * (c0 + x2 * (c1 + x2 * (c2 + x2 * c3)));
                }
                break;
            }
        }
        else
        {
            using UI = detail::make_unsigned_t<T>;
            // we noticed that the range border value can be represented in a * 2^(f-k) pattern.
            // so we can also use some trick to optimize performance.
            const UI ux = static_cast<UI>(x.internal_value());
            if(ux >= (UI(1) << (f - 5))) // 1 / 32 = 1 << (f - 5).
            {
                // extract factor a, due to the rest border value can be represented as a * 2^(f - 3).
                // and when x = 1, a = 8, we clamp it to 7.
                const UI factor = (ux >> (f - 3)) > UI(7) ? UI(7) : (ux >> (f - 3));
                switch(factor)
                {
                case 0:
                    {
                        constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.fffffffeb1a32d0ep-1"); // x
                        constexpr auto c1 = detail::eval_const<char, T, I, f, r>("-0x1.55554b35e0788014p-2"); // x^3
                        constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.99852a887a2d7a18p-3"); // x^5
                        constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.1d2820673633c27p-3"); // x^7
                        return x * (c0 + x2 * (c1 + x2 * (c2 + x2 * c3)));
                    }
                    break;
                case 1:
                    {
                        constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.ffffffdc8b727784p-1"); // x
                        constexpr auto c1 = detail::eval_const<char, T, I, f, r>("-0x1.555528b9bffbea64p-2"); // x^3
                        constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.998479ff66759dp-3"); // x^5
                        constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.2232f7522f8055f8p-3"); // x^7
                        constexpr auto c4 = detail::eval_const<char, T, I, f, r>("0x1.854d86ab77e7fcdp-4"); // x^9
                        return x * (c0 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * c4))));
                    }
                    break;
                case 2:
                    {
                        constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.ffffee189f6bdbc6p-1"); // x
                        constexpr auto c1 = detail::eval_const<char, T, I, f, r>("-0x1.554dbf4b72f9b33p-2"); // x^3
                        constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.9854775ae48b7c1p-3"); // x^5
                        constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.17012448538f94d8p-3"); // x^7
                        constexpr auto c4 = detail::eval_const<char, T, I, f, r>("0x1.32c430e846079a9p-4"); // x^9
                        return x * (c0 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * c4))));
                    }
                    break;
                case 3:
                    {
                        constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.ffffce07d95dd5e6p-1"); // x
                        constexpr auto c1 = detail::eval_const<char, T, I, f, r>("-0x1.55487f2347ca9afcp-2"); // x^3
                        constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.9838bccce20199ap-3"); // x^5
                        constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.1a58620785d0b83p-3"); // x^7
                        constexpr auto c4 = detail::eval_const<char, T, I, f, r>("0x1.6ef3f98a2f0e369p-4"); // x^9
                        constexpr auto c5 = detail::eval_const<char, T, I, f, r>("-0x1.2db77880b84e1fap-5"); // x^11
                        return x * (c0 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * (c4 + x2 * c5)))));
                    }
                    break;
                case 4:
                    {
                        constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.fffda0d68f67850ep-1"); // x
                        constexpr auto c1 = detail::eval_const<char, T, I, f, r>("-0x1.54f62e8511fafb28p-2"); // x^3
                        constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.934c98558fcdf18p-3"); // x^5
                        constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.07427adcea1b308p-3"); // x^7
                        constexpr auto c4 = detail::eval_const<char, T, I, f, r>("0x1.23ed8cff398f50cp-4"); // x^9
                        constexpr auto c5 = detail::eval_const<char, T, I, f, r>("-0x1.6c2430b3b0880bp-6"); // x^11
                        return x * (c0 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * (c4 + x2 * c5)))));
                    }
                    break;
                case 5:
                    {
                        constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.fff1514e3da0f304p-1"); // x
                        constexpr auto c1 = detail::eval_const<char, T, I, f, r>("-0x1.53c4b6d4d435a7f8p-2"); // x^3
                        constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.875afe16326edfbp-3"); // x^5
                        constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.d2360b44c784a9ap-4"); // x^7
                        constexpr auto c4 = detail::eval_const<char, T, I, f, r>("0x1.ae346d93cc4dfe6p-5"); // x^9
                        constexpr auto c5 = detail::eval_const<char, T, I, f, r>("-0x1.9c3af99ac2b2088p-7"); // x^11
                        return x * (c0 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * (c4 + x2 * c5)))));
                    }
                    break;
                case 6:
                    {
                        constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.ffc5cfeaa7dada7ep-1"); // x
                        constexpr auto c1 = detail::eval_const<char, T, I, f, r>("-0x1.50ccf3e25acba19cp-2"); // x^3
                        constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.727ede291047ca28p-3"); // x^5
                        constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.886fce7c8d59fdfp-4"); // x^7
                        constexpr auto c4 = detail::eval_const<char, T, I, f, r>("0x1.2ae8c57b3d6619ep-5"); // x^9
                        constexpr auto c5 = detail::eval_const<char, T, I, f, r>("-0x1.c0342d26ebc0bfp-8"); // x^11
                        return x * (c0 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * (c4 + x2 * c5)))));
                    }
                    break;
                default:
                    {
                        if(ux < (UI(15) << (f - 4))) // 15/16 = 15 * (1 << (f - 4))
                        {
                            constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.fea8def47638e9d2p-1"); // x
                            constexpr auto c1 = detail::eval_const<char, T, I, f, r>("-0x1.42df60310cc3d50cp-2"); // x^3
                            constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.2c9fe78c7c9d85bp-3"); // x^5
                            constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.b17b9ec5b80f08p-5"); // x^7
                            constexpr auto c4 = detail::eval_const<char, T, I, f, r>("0x1.3596c9ae62b7f7p-7"); // x^9
                            return x * (c0 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * c4))));
                        }
                        else
                        {
                            constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.fe013fadc1b4faf4p-1"); // x
                            constexpr auto c1 = detail::eval_const<char, T, I, f, r>("-0x1.3ceb8135bb1193fcp-2"); // x^3
                            constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.184ff9ded6e66358p-3"); // x^5
                            constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.73d0c48fa374424p-5"); // x^7
                            constexpr auto c4 = detail::eval_const<char, T, I, f, r>("0x1.dea201ffac30c3p-8"); // x^9
                            return x * (c0 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * c4))));
                        }
                    }
                }
            }
            else
            {
                constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.ffffffffffff8e74p-1"); // x
                constexpr auto c1 = detail::eval_const<char, T, I, f, r>("-0x1.55555554e3c2c398p-2"); // x^3
                constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.999987d9962ce7p-3"); // x^5
                constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.2420a37c73d54c3p-3"); // x^7
                return x * (c0 + x2 * (c1 + x2 * (c2 + x2 * c3)));
            }
        }
    }

    // r(x) = 1/sqrt(1 - x^2)，x in [0, 1/sqrt(2)] (t = x^2 in [0, 1/2]).
    // use deg-12 64-bit minimax.
    template <typename T, typename I, unsigned int f, bool r>
    EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> rsqrt_1mx2(fixed_num<T, I, f, r> x) noexcept
    {
        using fixed = fixed_num<T, I, f, r>;
        constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.000000002e7f1741p0");
        constexpr auto c1 = detail::eval_const<char, T, I, f, r>("0x1.fffffe2a167e8b3cp-2");
        constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.8000c496e59f5064p-2");
        constexpr auto c3 = detail::eval_const<char, T, I, f, r>("0x1.3fdfc026d8211784p-2");
        constexpr auto c4 = detail::eval_const<char, T, I, f, r>("0x1.1abd34bdae38fe14p-2");
        constexpr auto c5 = detail::eval_const<char, T, I, f, r>("0x1.b17db49337d3362p-3");
        constexpr auto c6 = detail::eval_const<char, T, I, f, r>("0x1.03fdc0798f519c3p-1");
        constexpr auto c7 = detail::eval_const<char, T, I, f, r>("-0x1.52af9d452468acd1p0");
        constexpr auto c8 = detail::eval_const<char, T, I, f, r>("0x1.72f0c8a911148a2ap2");
        constexpr auto c9 = detail::eval_const<char, T, I, f, r>("-0x1.afc259ad0b5d2a6cp3");
        constexpr auto c10 = detail::eval_const<char, T, I, f, r>("0x1.5bcdd247118febe1p4");
        constexpr auto c11 = detail::eval_const<char, T, I, f, r>("-0x1.3e85661d469d3e26p4");
        constexpr auto c12 = detail::eval_const<char, T, I, f, r>("0x1.19cea1d47abf3b81p3");
        const auto t = x * x;
        fixed res = c12;
        res = res * t + c11;
        res = res * t + c10;
        res = res * t + c9;
        res = res * t + c8;
        res = res * t + c7;
        res = res * t + c6;
        res = res * t + c5;
        res = res * t + c4;
        res = res * t + c3;
        res = res * t + c2;
        res = res * t + c1;
        res = res * t + c0;
        return res;
    }

    // integral sqrt without division.
    template <typename I>
    constexpr I isqrt_bits(I N) noexcept
    {
        if(N <= 0)
            return 0;
        const int nb = static_cast<int>(detail::bit_width(static_cast<detail::make_unsigned_t<I>>(N)));
        I m = static_cast<I>(1) << (2 * ((nb - 1) / 2));
        I s = 0;
        I R = N;
        while(m != 0)
        {
            const I b = s | m;
            s >>= 1;
            const I ge = static_cast<I>(0) - static_cast<I>(R >= b);
            R -= b & ge;
            s |= m & ge;
            m >>= 2;
        }
        return s;
    }

    // fast integer sqrt for N in [1, 2^64), returns floor(sqrt(N)).
    // table-seeded Newton; the divisor fits 32 bits, so on x64 this is a
    // 64/32-bit division. Used by the glibc-style near-1 asin branch.
    constexpr uint64_t fast_isqrt_u64(uint64_t N) noexcept
    {
        if(N <= 1)
            return N;
        const int nb = static_cast<int>(detail::bit_width(N));
        uint64_t g0;
        if(nb < 12)
        {
            g0 = static_cast<uint64_t>(sqrt_mantissa_table[static_cast<unsigned int>(N)]) >> 16;
        }
        else
        {
            const unsigned int idx = static_cast<unsigned int>(N >> (nb - 12));
            g0 = static_cast<uint64_t>(sqrt_mantissa_table[idx]);
            if((nb & 1) == 0)
            {
                const int sh = (nb - 44) / 2;
                g0 = sh >= 0 ? g0 << sh : g0 >> static_cast<unsigned>(-sh);
            }
            else
            {
                // sqrt_mantissa_table[idx] ≈ 2^16·sqrt(idx), multiply by sqrt(2) ≈ 46341/2^15.
                const int sh = (nb - 45) / 2;
                const int total = 15 - sh;
                g0 = total >= 0 ? (g0 * 46341u) >> total : (g0 * 46341u) << static_cast<unsigned>(-total);
            }
        }
        uint64_t g1 = (g0 + N / g0) >> 1;
        uint64_t g2 = (g1 + N / g1) >> 1;
        while(N / g2 < g2) // g2 > sqrt(N)
            --g2;
        while(g2 + 1 <= N / (g2 + 1)) // g2 < sqrt(N) - 1
            ++g2;
        return g2;
    }

    // eval extended fractions for cbrt version of frexp.
    template <typename T, typename I, unsigned int f>
    struct cbrt_frexp_scales
    {
        static constexpr unsigned int digits = static_cast<unsigned int>(detail::__eval_max_bit_width<T>());
        // extend to f + e_max / 3 + 4
        static constexpr unsigned int fraction = f + (digits - f) / 3u + 4u;
    };

    // extended frexp: x = m·2^e, m ∈ [0.5, 1), and keep the sign.
    // return: m_raw = m·2^F (F = cbrt_scales::F), exponent is given by e.
    // this version should has enough precision for cbrt.
    template <typename T, typename I, unsigned int f, bool r>
    EIRIN_ALWAYS_INLINE constexpr I cbrt_frexp(fixed_num<T, I, f, r> x, int& e) noexcept
    {
        using U = detail::make_unsigned_t<T>;
        constexpr unsigned int F = cbrt_frexp_scales<T, I, f>::fraction;
        auto X = x.internal_value();
        if(X == 0)
        {
            e = 0;
            return 0;
        }
        const bool neg = X < 0;
        const U mag = neg ? U(0) - static_cast<U>(X) : static_cast<U>(X);
        const auto msb = detail::bit_width(mag);
        e = static_cast<int>(msb) - static_cast<int>(f);
        U m_raw;
        if(msb > F)
        {
            const unsigned int sh = static_cast<unsigned int>(msb) - F;
            m_raw = (mag + (U(1) << (sh - 1))) >> sh;
        }
        else
        {
            m_raw = mag << (F - msb);
        }
        if(m_raw == (U(1) << F)) // re-normalize to (0.5, 1].
        {
            m_raw = U(1) << (F - 1);
            ++e;
        }
        return neg ? -static_cast<I>(m_raw) : static_cast<I>(m_raw);
    }

    template <typename I, unsigned int f, bool r>
    EIRIN_ALWAYS_INLINE constexpr I multiply_fixed_internal(const I& a, const I& b) noexcept
    {
        if constexpr(r)
        {
            constexpr auto fraction_multiplier = I(1) << f;
            auto _value = a * b / (fraction_multiplier / 2);
            return (_value + (_value % 2)) >> 1;
        }
        else
        {
            return (a * b) >> f;
        }
    }

    template <typename T, typename I, unsigned int f, bool r>
    EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> asin_impl(fixed_num<T, I, f, r> x) noexcept
    {
        // glibc-style piecewise polynomial (e_asin.c): asin(x) = x·Q(t), t = x².
        //   Q1 on [0, 1/4]   (|x| <= 1/2),   deg 8, 32-bit coeff err ~2^-37
        //   Q2 on [1/4, 1/2] (|x| <= 1/√2), deg 8, 32-bit coeff err ~2^-35
        using fixed = fixed_num<T, I, f, r>;
        const auto t = x * x;
        if(x <= fixed(1) / 2) // this branch is small enough - no need to optimize.
        {
            constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1p0");
            constexpr auto c1 = detail::eval_const<char, T, I, f, r>("0x1.5555554p-3");
            constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.333343bp-4");
            constexpr auto c3 = detail::eval_const<char, T, I, f, r>("0x1.6db1b8ap-5");
            constexpr auto c4 = detail::eval_const<char, T, I, f, r>("0x1.f293224p-6");
            constexpr auto c5 = detail::eval_const<char, T, I, f, r>("0x1.65bd03p-6");
            constexpr auto c6 = detail::eval_const<char, T, I, f, r>("0x1.547bcb8p-6");
            constexpr auto c7 = detail::eval_const<char, T, I, f, r>("0x1.b5667cp-10");
            constexpr auto c8 = detail::eval_const<char, T, I, f, r>("0x1.10043d4p-5");
            fixed q = c8;
            q = q * t + c7;
            q = q * t + c6;
            q = q * t + c5;
            q = q * t + c4;
            q = q * t + c3;
            q = q * t + c2;
            q = q * t + c1;
            q = q * t + c0;
            return x * q;
        }
        else
        {
            constexpr auto c0 = detail::eval_const<char, T, I, f, r>("0x1.000303ffp0");
            constexpr auto c1 = detail::eval_const<char, T, I, f, r>("0x1.5306ed58p-3");
            constexpr auto c2 = detail::eval_const<char, T, I, f, r>("0x1.64912ebp-4");
            constexpr auto c3 = detail::eval_const<char, T, I, f, r>("-0x1.ddd3acp-6");
            constexpr auto c4 = detail::eval_const<char, T, I, f, r>("0x1.41730df8p-2");
            constexpr auto c5 = detail::eval_const<char, T, I, f, r>("-0x1.5c16ae92p-1");
            constexpr auto c6 = detail::eval_const<char, T, I, f, r>("0x1.1e796198p0");
            constexpr auto c7 = detail::eval_const<char, T, I, f, r>("-0x1.ff6b941ep-1");
            constexpr auto c8 = detail::eval_const<char, T, I, f, r>("0x1.c5e212bp-2");
            fixed q = c8;
            q = q * t + c7;
            q = q * t + c6;
            q = q * t + c5;
            q = q * t + c4;
            q = q * t + c3;
            q = q * t + c2;
            q = q * t + c1;
            q = q * t + c0;
            return x * q;
        }
    }
} // namespace detail

} // namespace eirin

#endif
