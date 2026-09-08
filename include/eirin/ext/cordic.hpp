#ifndef EIRIN_MATH_EXT_CORDIC_HPP
#define EIRIN_MATH_EXT_CORDIC_HPP

#include "../fixed.hpp"
#include "../numbers.hpp"
#include "../math.hpp"

namespace eirin
{
namespace detail
{
    // CORDIC constants for fixed-point calculations.
    template <fixed_point T>
    EIRIN_ALWAYS_INLINE constexpr T cordic_k()
    {
        T val;
        fixed_from_cstring("0.6072529350088812561694", 22, val);
        return val;
    }

    template <fixed_point T>
    const T K = cordic_k<T>();

    template <typename T, typename I, unsigned int f, bool r>
    struct cordic_angels
    {
        using fixed = fixed_num<T, I, f, r>;

        static constexpr std::array<fixed, 41> generate_values()
        {
            std::array<fixed, 41> arr;
            const char* angle_strs[41] = {
                "0.785398163397448", "0.463647609000806", "0.244978663126864", "0.124354994546761", "0.062418809995957", "0.031239833430268", "0.015623728620477", "0.007812341060101", "0.003906230131967", "0.001953122516479", "0.000976562189559", "0.000488281211195", "0.000244140620149", "0.000122070311894", "0.000061035156174", "0.000030517578116", "0.000015258789061", "0.000007629394531", "0.000003814697266", "0.000001907348633", "0.000000953674316", "0.000000476837158", "0.000000238418579", "0.000000119209290", "0.000000059604645", "0.000000029802322", "0.000000014901161", "0.000000007450581", "0.000000003725290", "0.000000001862645", "0.000000000931323", "0.000000000465661", "0.000000000232831", "0.000000000116415", "0.000000000058208", "0.000000000029104", "0.000000000014552", "0.000000000007276", "0.000000000003638", "0.000000000001819", "0.000000000000909"
            };
            for(size_t i = 0; i < 41; ++i)
            {
                fixed_from_cstring(angle_strs[i], 22, arr[i]);
            }
            return arr;
        }

        static constexpr std::array<fixed, 41> values = generate_values();
    };

    // atan1, atan(1/2), atan(1/4), etc.
    template <typename T, typename I, unsigned int f, bool r, size_t count>
    EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> angel(size_t index)
    {
        static_assert(count > 0 && count <= 41, "count must be in (0, 41]");
        return cordic_angels<T, I, f, r>::values[index];
    }

    /**
     * @brief Normalize the angle Q into the range of [0, pi/2].
     *
     * @tparam FP Fixed Point Type.
     * @tparam sine if the angel is for sine, default is true.
     * @param Q angel.
     * @param sin_sign
     * @param cos_sign
     * @return EIRIN_ALWAYS_INLINE constexpr
     */
    template <fixed_point FP, bool sine = true>
    EIRIN_ALWAYS_INLINE constexpr FP cordic_optimize_angel(FP Q, int8_t& sin_sign, int8_t& cos_sign)
    {
        constexpr auto pi = numbers::pi_v<FP>();
        constexpr auto double_pi = 2 * pi;
        constexpr auto half_pi = pi / 2;
        constexpr auto const_0 = FP(0);

        sin_sign = Q < const_0 ? -1 : 1;

        // abs Q with signbit mask.
        auto sign_mask = Q.signbit_mask();
        auto abs_value = Q.m_value & ~sign_mask;
        Q = FP::from_internal_value(abs_value);

        // scale angel into [-2*pi, 2*pi].
        Q %= double_pi;

        const bool in_second_half = (Q > pi);
        Q = in_second_half ? (Q - pi) : Q;

        sin_sign *= (1 - 2 * in_second_half);
        cos_sign *= (1 - 2 * in_second_half);

        const bool in_second_quad = (Q > half_pi);
        Q = in_second_quad ? (pi - Q) : Q;

        cos_sign *= (1 - 2 * in_second_quad);

        return Q;
    }

    template <typename T, typename I, unsigned int f, bool r, size_t count, bool sine = true>
    EIRIN_ALWAYS_INLINE constexpr fixed_num<T, I, f, r> cordic(fixed_num<T, I, f, r> Q)
    {
        using fixed = fixed_num<T, I, f, r>;
        int8_t sin_sign = 1, cos_sign = 1;
        Q = cordic_optimize_angel<fixed, sine>(Q, sin_sign, cos_sign);

        I x = static_cast<I>(K<fixed>.m_value), y = I(0), temp = I(0);
        fixed q = fixed(0);
        for(size_t i = 0; i < count; ++i)
        {
            if(q < Q)
            {
                temp = x - (y >> i);
                y = y + (x >> i);
                x = temp;
                q += angel<T, I, f, r, count>(i);
            }
            else
            {
                temp = x + (y >> i);
                y = y - (x >> i);
                x = temp;
                q -= angel<T, I, f, r, count>(i);
            }
        }
        if constexpr(sine)
        {
            return fixed::from_internal_value(static_cast<T>(y) * sin_sign);
        }
        else
        {
            return fixed::from_internal_value(static_cast<T>(x) * cos_sign);
        }
    }
} // namespace detail

#ifdef EIRIN_MATH_HAS_INT128
EIRIN_ALWAYS_INLINE constexpr auto cordic_sine(fixed64 x) noexcept
{
    return detail::cordic<int64_t, detail::int128_t, 32, false, 41, true>(x);
}
#endif
EIRIN_ALWAYS_INLINE constexpr auto cordic_sine(fixed32 x) noexcept
{
    return detail::cordic<int32_t, int64_t, 16, false, 41, true>(x);
}
} // namespace eirin

#endif // EIRIN_MATH_EXT_CORDIC_HPP
