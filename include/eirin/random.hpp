#ifndef EIRIN_MATH_RANDOM_HPP
#define EIRIN_MATH_RANDOM_HPP

#pragma once

#include <cstdint>
#include <cstdio>
#include <vector>
#include <random>
#include <istream>
#include <ostream>
#include "macro.hpp"
#include "fixed.hpp"
#include "detail/distribution.hpp"

namespace eirin
{
namespace detail
{
    template <typename T, T state, T multiplier, T increment, T mix_multiplier, T xor_multiplier>
    concept pcg_params_check = is_unsigned_v<T> && multiplier != T{0} && increment != T{0} && mix_multiplier != T{0} && xor_multiplier != T{0};

    /**
     * @brief The mixing kernel of the Stellaris CNoiseRandom-style generator, operating
     *        on one 32-bit lane.
     *
     * Reproduces the decompiled sequence:
     *   x = (x >> 8) ^ x;  x += increment;
     *   x = (x * 0x100) ^ x;  x *= xor_multiplier;
     *   x = (x >> 8) ^ x;  x *= mix_multiplier;
     *
     * @tparam Word lane type (uint32_t).
     * @return the mixed raw lane value; the caller applies the final
     *         full-width `x ^ (x >> 8)` output mixing.
     */
    template <typename Word>
    EIRIN_ALWAYS_INLINE constexpr Word __pcg_mix(Word x, Word increment, Word xor_multiplier, Word mix_multiplier) noexcept
    {
        x = (x ^ (x >> 8)) + increment;
        x = (x * static_cast<Word>(0x100)) ^ x;
        x *= xor_multiplier;
        x = (x ^ (x >> 8)) * mix_multiplier;
        return x;
    }

    template <typename FixedType, typename _RandomNumberEngine>
    concept fixed_random_engine_type_check = requires {
        requires is_fixed_point_v<FixedType>;
        requires std::uniform_random_bit_generator<_RandomNumberEngine>;
        requires sizeof(typename FixedType::value_type) <= sizeof(typename _RandomNumberEngine::result_type);
        { FixedType::template from_fixed_num_value<FixedType::precision>(static_cast<typename FixedType::value_type>(_RandomNumberEngine()())) } -> std::same_as<FixedType>;
    };
} // namespace detail

/**
 * @brief Permuted Congruential Random Generator Variant.
 * PCG is a family of simple fast space-efficient statistically good algorithms for random number generation.
 * Unlike many general-purpose RNGs, they are also hard to predict.
 * And in this instance, we use a state counter.
 *
 * @tparam UIntType unsigned int type, use for parameters and return type.
 * @tparam state initial state.
 * @tparam multiplier
 * @tparam increment
 * @tparam mix_multiplier
 * @tparam xor_multiplier
 */
template <typename UIntType, UIntType state, UIntType multiplier, UIntType increment, UIntType mix_multiplier, UIntType xor_multiplier>
requires detail::pcg_params_check<UIntType, state, multiplier, increment, mix_multiplier, xor_multiplier>
class permuted_congruential_engine
{
protected:
    UIntType m_seed;
    UIntType m_state;

    EIRIN_ALWAYS_INLINE UIntType generate()
    {
        if constexpr(sizeof(UIntType) == 8)
        {
            const uint32_t c = static_cast<uint32_t>(m_state);
            const uint32_t s = static_cast<uint32_t>(m_seed);
            const uint32_t mul = static_cast<uint32_t>(multiplier);
            const uint32_t inc = static_cast<uint32_t>(increment);
            const uint32_t xm = static_cast<uint32_t>(xor_multiplier);
            const uint32_t mm = static_cast<uint32_t>(mix_multiplier);

            const uint32_t raw0 = detail::__pcg_mix<uint32_t>(c * mul + s, inc, xm, mm);
            const uint32_t raw1 = detail::__pcg_mix<uint32_t>((c + 1u) * mul + s, inc, xm, mm);
            m_state = static_cast<UIntType>(c) + UIntType{2};

            const UIntType out0 = static_cast<UIntType>(raw0 ^ (raw0 >> 8));
            const UIntType out1 = static_cast<UIntType>(raw1 ^ (raw1 >> 8));
            return (out0 << 32) | out1;
        }
        else
        {
            const UIntType raw = detail::__pcg_mix<UIntType>(
                m_state * multiplier + m_seed, increment, xor_multiplier, mix_multiplier
            );
            ++m_state;
            return raw ^ (raw >> 8);
        }
    }

public:
    using result_type = UIntType;
    static constexpr result_type init_state = state;
    static constexpr result_type increment_size = increment;
    static constexpr result_type multiplier_size = multiplier;
    static constexpr result_type mix_multiplier_size = mix_multiplier;
    static constexpr result_type xor_multiplier_size = xor_multiplier;
    // default seed for PCG, shoule be a small number.
    static constexpr result_type default_seed = 0x89ABCD;

    permuted_congruential_engine()
        : m_seed(default_seed), m_state(state) {}

    permuted_congruential_engine(UIntType seed)
        : m_seed(seed), m_state(state) {}

    result_type operator()()
    {
        return generate();
    }

    void seed(result_type seed = default_seed)
    {
        m_seed = seed;
        m_state = state;
    }

    template <typename Sseq>
    void seed(Sseq& seq)
    {
        std::vector<result_type> seeds(2);
        seq.generate(seeds.begin(), seeds.end());
        m_seed = seeds[0];
        m_state = seeds[1];
    }

    void discard(unsigned long long z)
    {
        m_state += z;
    }

    static constexpr result_type min() noexcept
    {
        return std::numeric_limits<result_type>::min();
    }

    static constexpr result_type max() noexcept
    {
        return std::numeric_limits<result_type>::max();
    }

    friend bool operator==(const permuted_congruential_engine& lhs, const permuted_congruential_engine& rhs)
    {
        return lhs.m_seed == rhs.m_seed && lhs.m_state == rhs.m_state;
    }

    friend bool operator!=(const permuted_congruential_engine& lhs, const permuted_congruential_engine& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename U_, U_ s_, U_ m_, U_ i_, U_ mm_, U_ xm_, typename charT_, typename charTraits_>
    friend std::basic_ostream<charT_, charTraits_>& operator<<(std::basic_ostream<charT_, charTraits_>& os, const permuted_congruential_engine<U_, s_, m_, i_, mm_, xm_>& obj);

    template <typename charT, typename charTraits>
    inline friend std::basic_istream<charT, charTraits>& operator>>(std::basic_istream<charT, charTraits>& is, permuted_congruential_engine& obj)
    {
        is >> obj.m_seed >> obj.m_state;
        return is;
    }
};

template <typename U, U s, U m, U i, U mm, U xm, typename charT, typename charTraits>
inline std::basic_ostream<charT, charTraits>& operator<<(std::basic_ostream<charT, charTraits>& os, const permuted_congruential_engine<U, s, m, i, mm, xm>& obj)
{
    // set fmtflags to dec and left, padding with space.
    using _ios_base = typename std::basic_ostream<charT, charTraits>::ios_base;
    const typename _ios_base::fmtflags flags = os.flags();
    const charT fill = os.fill(), space = os.widen(' ');
    os.flags(_ios_base::dec | _ios_base::fixed | _ios_base::left);
    os.fill(space);
    // write the current state into the stream.
    os << obj.m_seed << space << obj.m_state;
    os.flags(flags);
    os.fill(fill);
    return os;
}

/**
 * @brief A adapter for standard random engines to support fixed point types, which fits RandomNumberEngine requirements.
 *
 * @tparam FixedType Fixed point type, must satisfy concept `is_fixed_point_v`.
 * @tparam _RandomNumberEngine The underlying random number engine type.
 */
template <typename FixedType, typename _RandomNumberEngine>
requires detail::fixed_random_engine_type_check<FixedType, _RandomNumberEngine>
class fixed_random_engine_adapter
{
public:
    typedef FixedType result_type;
    typedef _RandomNumberEngine underlying_type;

    fixed_random_engine_adapter() = default;

    result_type operator()()
    {
        return FixedType::template from_fixed_num_value<FixedType::precision>(static_cast<typename FixedType::value_type>(m_engine()));
    }

    static constexpr result_type min() noexcept
    {
        return std::numeric_limits<result_type>::min();
    }

    static constexpr result_type max() noexcept
    {
        return std::numeric_limits<result_type>::max();
    }

    void seed(typename underlying_type::result_type seed = underlying_type::default_seed)
    {
        m_engine.seed(seed);
    }

    template <typename Sseq>
    void seed(Sseq& seq)
    {
        m_engine.seed(seq);
    }

    void discard(unsigned long long z)
    {
        m_engine.discard(z);
    }

    friend bool operator==(const fixed_random_engine_adapter& lhs, const fixed_random_engine_adapter& rhs)
    {
        return lhs.m_engine == rhs.m_engine;
    }

    friend bool operator!=(const fixed_random_engine_adapter& lhs, const fixed_random_engine_adapter& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename charT, typename charTraits>
    inline friend std::basic_ostream<charT, charTraits>& operator<<(std::basic_ostream<charT, charTraits>& os, const fixed_random_engine_adapter& obj)
    {
        os << obj.m_engine;
        return os;
    }

    template <typename charT, typename charTraits>
    inline friend std::basic_istream<charT, charTraits>& operator>>(std::basic_istream<charT, charTraits>& is, fixed_random_engine_adapter& obj)
    {
        is >> obj.m_engine;
        return is;
    }

    underlying_type& __underlying_engine()
    {
        return m_engine;
    }

private:
    underlying_type m_engine;
};

using random_device = std::random_device;
using mt19937 = std::mt19937;
using mt19937_64 = std::mt19937_64;
using minstd_rand0 = std::minstd_rand0;
using minstd_rand = std::minstd_rand;
using ranlux24_base = std::ranlux24_base;
using ranlux48_base = std::ranlux48_base;

typedef permuted_congruential_engine<uint32_t, 0x0, 0xB5297A4D, 0x68E31DA4, 0x92D68CA2, 0x1B56C4E9> pcg2014;
typedef permuted_congruential_engine<uint64_t, 0x0, 0xB5297A4D, 0x68E31DA4, 0x92D68CA2, 0x1B56C4E9> pcg2014_64;

} // namespace eirin

#endif
