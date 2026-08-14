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
    // Rescale a 61-bit dyadic constant (value = k61/2^61) to s fraction bits.
    template <typename I, unsigned int s>
    constexpr I pow_scale_61(int64_t k61) noexcept
    {
        if constexpr(s >= 61u)
            return static_cast<I>(k61) << (s - 61u);
        else
            return static_cast<I>(static_cast<int64_t>(k61) >> (61u - s));
    }

    // Rescale a 60-bit dyadic polynomial coefficient (k60 = coeff*2^60) to
    // cbits fraction bits with round-half-away.
    template <typename J>
    constexpr J pow_coeff_60(int64_t k60, unsigned int cbits) noexcept
    {
        if(cbits >= 60u)
            return static_cast<J>(k60) << (cbits - 60u);
        const int64_t half = int64_t(1) << int(59u - cbits);
        const int shift = int(60u - cbits);
        const int64_t v = k60 >= 0 ? (k60 + half) >> shift : -(((-k60) + half) >> shift);
        return static_cast<J>(v);
    }

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
     * final round-to-nearest step. The 60-bit cap matches sin and is imposed by the
     * int64 parsing inside eval_const. Unlike sin's digits-3 cap, the wide type here
     * only needs to hold the reduced r (|r| <= ln2), not the original input, so one
     * extra fraction bit is available.
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
        // sqrt(2)*2^61 = 0x2D413CCCFE779A00
        constexpr I sqrt2_bf = pow_scale_61<I, bf>(0x2D413CCCFE779A00ll);
        if(m >= sqrt2_bf)
        {
            m >>= 1;
            ++e2_adj;
        }
        const J t = static_cast<J>(m) - (static_cast<J>(1) << bf);

        // ln(1+t)/t, degree 17, sollya fpminimax fixed 60 bits, interval
        // [sqrt(2)/2-1, sqrt(2)-1], error ~= 7.4e-16
        constexpr J c0 = pow_coeff_60<J>(1152921504606845043ll, C);
        constexpr J c1 = pow_coeff_60<J>(-576460752303385276ll, C);
        constexpr J c2 = pow_coeff_60<J>(384307168204853210ll, C);
        constexpr J c3 = pow_coeff_60<J>(-288230376170093992ll, C);
        constexpr J c4 = pow_coeff_60<J>(230584300368830707ll, C);
        constexpr J c5 = pow_coeff_60<J>(-192153581285657424ll, C);
        constexpr J c6 = pow_coeff_60<J>(164703116521311305ll, C);
        constexpr J c7 = pow_coeff_60<J>(-144115392171992599ll, C);
        constexpr J c8 = pow_coeff_60<J>(128100689551715941ll, C);
        constexpr J c9 = pow_coeff_60<J>(-115284134418239191ll, C);
        constexpr J c10 = pow_coeff_60<J>(104843898330878982ll, C);
        constexpr J c11 = pow_coeff_60<J>(-96255097752277119ll, C);
        constexpr J c12 = pow_coeff_60<J>(88403978006163229ll, C);
        constexpr J c13 = pow_coeff_60<J>(-80144740893915366ll, C);
        constexpr J c14 = pow_coeff_60<J>(76866489725209434ll, C);
        constexpr J c15 = pow_coeff_60<J>(-85601825389816036ll, C);
        constexpr J c16 = pow_coeff_60<J>(82859280330190359ll, C);
        constexpr J c17 = pow_coeff_60<J>(-39584298757743610ll, C);

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

        // ln(b) = e2*ln2 + ln(1+t) at S bits
        constexpr J ln2_S = pow_scale_61<J, S>(0x162E42FEFA39EF00ll);
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

        // k = floor(x * log2(e)); log2(e)*2^61 = 0x2E2A8ECA5705FC00.
        // On high-fraction formats the exponent range can be so large that the
        // full product overflows the intermediate type; pre-scaling x_best by
        // k_sh bits only perturbs k slightly, which the reduction loops below
        // correct.
        constexpr unsigned int kc_bits = f < (sc.I_bits - 2u) ? f : (sc.I_bits - 2u);
        constexpr J log2e_kc = pow_scale_61<J, kc_bits>(0x2E2A8ECA5705FC00ll);
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
        J r_work;
        if constexpr(2 * bf <= 61)
        {
            constexpr J ln2_2w = pow_scale_61<J, 2 * bf>(0x162E42FEFA39EF00ll);
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
            constexpr J ln2_best = pow_scale_61<J, bf>(0x162E42FEFA39EF00ll);
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
        // error ~= 1.9e-14
        constexpr J e0 = pow_coeff_60<J>(1152921504606824971ll, C);
        constexpr J e1 = pow_coeff_60<J>(1152921504613156941ll, C);
        constexpr J e2 = pow_coeff_60<J>(576460752004932245ll, C);
        constexpr J e3 = pow_coeff_60<J>(192153589574804099ll, C);
        constexpr J e4 = pow_coeff_60<J>(48038345109486436ll, C);
        constexpr J e5 = pow_coeff_60<J>(9607950726683145ll, C);
        constexpr J e6 = pow_coeff_60<J>(1600401176479225ll, C);
        constexpr J e7 = pow_coeff_60<J>(230504170452262ll, C);
        constexpr J e8 = pow_coeff_60<J>(26512680658222ll, C);
        constexpr J e9 = pow_coeff_60<J>(4507898505267ll, C);

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
} // namespace detail

} // namespace eirin

#endif
