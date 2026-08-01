#ifndef EIRIN_MATH_RANDOM_HPP
#define EIRIN_MATH_RANDOM_HPP

#pragma once

#include <cstdint>
#include <cstdio>
#include <vector>
#include <random>
#include "macro.hpp"
#include "fixed.hpp"
#include "detail/distribution.hpp"

namespace eirin
{
namespace detail
{
    template <typename T, T state, T multiplier, T increment, T mix_multiplier, T xor_multiplier>
    concept pcg_params_check = std::is_unsigned_v<T>;

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

    EIRIN_ALWAYS_INLINE UIntType mix_first(UIntType x)
    {
        x >>= 8 ^ x;
        x += increment;
        return x;
    }

    EIRIN_ALWAYS_INLINE UIntType _xor(UIntType x)
    {
        x = x * 0x100 ^ x;
        x *= xor_multiplier;
        return x;
    }

    EIRIN_ALWAYS_INLINE UIntType mix_second(UIntType x)
    {
        x >>= 8 ^ x;
        x *= mix_multiplier;
        return x;
    }

    EIRIN_ALWAYS_INLINE UIntType generate()
    {
        result_type val = m_state * multiplier + m_seed;
        val = mix_second(_xor(mix_first(val)));
        ++m_state;
        return (val & 0x7FFFFFFE) ^ (val >> 8);
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
    }

    void seed(std::seed_seq& seq)
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

    template <typename charT, typename charTraits>
    inline friend std::basic_ostream<charT, charTraits>& operator<<(std::basic_ostream<charT, charTraits>& os, const permuted_congruential_engine& obj)
    {
        // set fmtflags to dec and left, padding with space.
        os.setf(std::ios::fmtflags(os.flags() | std::ios::dec | std::ios_base::left), std::ios::basefield);
        // write the current state into the stream.
        os << obj.m_seed << ' ' << obj.m_state;
        return os;
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

    fixed_random_engine_adapter()
        : m_engine(default_engine()) {}

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

    void seed(std::seed_seq& seq)
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

    underlying_type& __underlying_engine()
    {
        return m_engine;
    }

private:
    static underlying_type default_engine()
    {
        static underlying_type instance;
        return instance;
    }

    underlying_type m_engine;
};

using random_device = std::random_device;
using mt19937 = std::mt19937;
using mt19937_64 = std::mt19937_64;
using minstd_rand0 = std::minstd_rand0;
using minstd_rand = std::minstd_rand;
using ranlux24_base = std::ranlux24_base;
using ranlux48_base = std::ranlux48_base;

typedef permuted_congruential_engine<uint32_t, 0x0, 0xB5297A4D, 0x68E31DA4, 0x1B56C4E9, 0x92D68CA2> pcg2014;
typedef permuted_congruential_engine<uint64_t, 0x0, 0x5851F42D4C957F2D, 0x9E3779B97F4A7C15, 0x94D049BB133111EB, 0xBF58476D1CE4E5B9> pcg2014_64;

// /**
//  * @brief Temporary test function for random number generator, remove it in the future.
//  *
//  */
// inline void test()
// {
//     auto seed = 114514u;
//     std::unordered_map<int32_t, uint32_t> rands;
//     // open a file stream to write the random results
//     auto fs = std::ofstream("random_res.txt", std::ios::out);
//     pcg2014 rng(seed);
//     std::random_device rd;
//     std::mt19937 mt(rd());
//     pcg2014 pcg(rd());
//     std::uniform_int_distribution<uint32_t> dist(0, 114514);
//     printf("test mt: %ud\n", dist(mt));
//     printf("test pcg: %ud\n", dist(pcg));
//     for(int i = 0; i < 114514; ++i)
//     {
//         auto val = rng();
//         rng.seed(val);
//         rands[val]++;
//         if(fs.is_open())
//         {
//             fs << "random " << i << " for seed " << seed << " " << val << '\n';
//         }
//         else
//             printf("random %d for seed %u %u\n", i, seed, val);
//     }
//     fs.close();
//     std::vector<std::pair<int32_t, int32_t>> pairs;
//     pairs.reserve(rands.size());
//     fs.open("random_res_summary.txt", std::ios::out);
//     for(const auto& [key, value] : rands)
//     {
//         pairs.push_back(std::make_pair(key, value));
//     }
//     std::sort(pairs.begin(), pairs.end(), [](const auto& a, const auto& b)
//               { return a.second > b.second; });
//     for(const auto& [key, value] : pairs)
//     {
//         if(fs.is_open())
//         {
//             fs << "value " << key << " appears " << value << " times\n";
//         }
//     }
//     fs.close();
// }
} // namespace eirin

#endif
