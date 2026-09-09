#ifndef EIRIN_MATH_EXT_UNIFORM_DIST_HPP
#define EIRIN_MATH_EXT_UNIFORM_DIST_HPP

#pragma once
#ifdef _MSC_VER
// C4244: conversion from 'type1' to 'type2', possible loss of data
// This is excepted, so disable it.
#    pragma warning(push)
#    pragma warning(disable : 4244)
#endif

#include "../fixed.hpp"

namespace eirin
{
/**
 * @brief A adapter to distribute fixed point numbers using integral distribution.
 * @note Different from `fixed_distribution_adapter`, this adapter works directly with standard random engines.
 * @note `DistType` must be a standard uniform integer distribution type (for
 *       example `std::uniform_int_distribution`): the adapter samples the raw
 *       internal values of `FixedType` uniformly in `[a, b]`.
 * @code {.cpp}
 * using namespace eirin;
 * random_device rd;
 * mt19937 mt_32(rd());
 * fixed_int_distribution_adapter<fixed32, std::uniform_int_distribution<>> dist_32;
 * std::array<fixed32, 10> values_32;
 * std::generate(values_32.begin(), values_32.end(), &[]() { return dist_32(mt_32) });
 * @endcode
 *
 * @tparam FixedType Fixed point type, must satisfy concept `is_fixed_point_v`.
 * @tparam DistType Distribution type, must be an integral distribution, and support UniformRandomBitGenerator.
 */
template <typename FixedType, typename DistType>
requires is_fixed_point_v<FixedType> && std::is_integral_v<typename DistType::result_type>
class fixed_int_distribution_adapter
{
public:
    typedef FixedType result_type;
    typedef typename result_type::value_type value_type;
    typedef std::make_unsigned_t<value_type> unsigned_type;
    typedef DistType underlying_type;

    struct param_type
    {
        using target_type = typename DistType::param_type;

        explicit param_type(const result_type& a = std::numeric_limits<result_type>::min(), const result_type& b = std::numeric_limits<result_type>::max())
            : m_a(a), m_b(b)
        {
            assert(a <= b);
        }

        result_type a() const
        {
            return m_a;
        }

        result_type b() const
        {
            return m_b;
        }

        friend bool operator==(const param_type& lhs, const param_type& rhs) noexcept
        {
            return lhs.m_a == rhs.m_a && lhs.m_b == rhs.m_b;
        }

        friend bool operator!=(const param_type& lhs, const param_type& rhs) noexcept
        {
            return !(lhs == rhs);
        }

        target_type to_underlying() const
        {
            return target_type(static_cast<value_type>(m_a.internal_value()), static_cast<value_type>(m_b.internal_value()));
        }

    private:
        result_type m_a;
        result_type m_b;
    };

    /**
     * @brief Construct a new uniform fixed distribution object, with given range [a, b].
     *
     * @param a
     * @param b
     */
    explicit fixed_int_distribution_adapter(const FixedType& a = std::numeric_limits<FixedType>::min(), const FixedType& b = std::numeric_limits<FixedType>::max())
        : m_dist(static_cast<value_type>(a.internal_value()), static_cast<value_type>(b.internal_value())), m_param(a, b)
    {}

    explicit fixed_int_distribution_adapter(const param_type& param)
        : m_dist(static_cast<value_type>(param.a().internal_value()), static_cast<value_type>(param.b().internal_value())), m_param(param)
    {}

    template <typename _UniformRandomBitGenerator>
    FixedType operator()(_UniformRandomBitGenerator& g)
    {
        return operator()(g, m_param);
    }

    template <typename _UniformRandomBitGenerator>
    FixedType operator()(_UniformRandomBitGenerator& g, const param_type& param)
    {
        // the m_param and the underlying distribution should be consistent, so just call m_dist.operator()
        auto raw_val = m_dist(g, param.to_underlying());
        return FixedType::template from_fixed_num_value<result_type::precision>(static_cast<value_type>(raw_val));
    }

    param_type param() const noexcept
    {
        return m_param;
    }

    void param(const param_type& param)
    {
        m_param = param;
        m_dist = DistType(static_cast<value_type>(param.a().internal_value()), static_cast<value_type>(param.b().internal_value()));
    }

    result_type a() const
    {
        return m_param.a();
    }

    result_type b() const
    {
        return m_param.b();
    }

    result_type min() const
    {
        return this->a();
    }

    result_type max() const
    {
        return this->b();
    }

    void reset() noexcept
    {
        // Nothing to do for uniform distribution.
    }

private:
    // The underlying int distribution.
    // To be noticed that we use the internal representation of the fixed number.
    underlying_type m_dist;
    param_type m_param;
};

/**
 * @brief A adapter to distribute fixed point numbers using integral distribution with the adapter `fixed_random_engine_adapter`.
 * @note This is similar to `fixed_int_distribution_adapter`, but designed to work with `fixed_random_engine_adapter`.
 * @note `DistType` must be a standard uniform integer distribution type (for
 *       example `std::uniform_int_distribution`): the adapter samples the raw
 *       internal values of `FixedType` uniformly in `[a, b]`.
 * @code {.cpp}
 * using namespace eirin;
 * random_device rd;
 * fixed_random_engine_adapter<fixed32, mt19937> test_adapter;
 * fixed_distribution_adapter<fixed32, std::uniform_int_distribution<>> test_dist_adapter;
 * std::array<fixed32, 10> values_32;
 * std::generate(values_32.begin(), values_32.end(), &[]() { return test_dist_adapter(test_adapter) });
 * @endcode
 *
 * @tparam FixedType Fixed point type, must satisfy concept `is_fixed_point_v`.
 * @tparam DistType Distribution type, must be an integral distribution, and support UniformRandomBitGenerator.
 */
template <typename FixedType, typename DistType>
requires is_fixed_point_v<FixedType> && std::is_integral_v<typename DistType::result_type>
class fixed_distribution_adapter
{
public:
    typedef FixedType result_type;
    typedef typename result_type::value_type value_type;
    typedef std::make_unsigned_t<value_type> unsigned_type;
    typedef DistType underlying_type;

    struct param_type
    {
        using target_type = typename DistType::param_type;

        explicit param_type(const result_type& a = std::numeric_limits<result_type>::min(), const result_type& b = std::numeric_limits<result_type>::max())
            : m_a(a), m_b(b)
        {
            assert(a <= b);
        }

        result_type a() const
        {
            return m_a;
        }

        result_type b() const
        {
            return m_b;
        }

        friend bool operator==(const param_type& lhs, const param_type& rhs) noexcept
        {
            return lhs.m_a == rhs.m_a && lhs.m_b == rhs.m_b;
        }

        friend bool operator!=(const param_type& lhs, const param_type& rhs) noexcept
        {
            return !(lhs == rhs);
        }

        target_type to_underlying() const
        {
            return target_type(static_cast<value_type>(m_a.internal_value()), static_cast<value_type>(m_b.internal_value()));
        }

    private:
        result_type m_a;
        result_type m_b;
    };

    /**
     * @brief Construct a new uniform fixed distribution object, with given range [a, b].
     *
     * @param a
     * @param b
     */
    explicit fixed_distribution_adapter(const FixedType& a = std::numeric_limits<FixedType>::min(), const FixedType& b = std::numeric_limits<FixedType>::max())
        : m_dist(static_cast<value_type>(a.internal_value()), static_cast<value_type>(b.internal_value())), m_param(a, b)
    {}

    explicit fixed_distribution_adapter(const param_type& param)
        : m_dist(static_cast<value_type>(param.a().internal_value()), static_cast<value_type>(param.b().internal_value())), m_param(param)
    {}

    template <typename _UniformRandomBitGeneratorAdapter>
    FixedType operator()(_UniformRandomBitGeneratorAdapter& a)
    {
        return operator()(a, m_param);
    }

    template <typename _UniformRandomBitGeneratorAdapter>
    FixedType operator()(_UniformRandomBitGeneratorAdapter& a, const param_type& param)
    {
        // the m_param and the underlying distribution should be consistent, so just call m_dist.operator()
        auto raw_val = m_dist(a.__underlying_engine(), param.to_underlying());
        return FixedType::template from_fixed_num_value<result_type::precision>(static_cast<value_type>(raw_val));
    }

    param_type param() const noexcept
    {
        return m_param;
    }

    void param(const param_type& param)
    {
        m_param = param;
        m_dist = underlying_type(static_cast<value_type>(param.a().internal_value()), static_cast<value_type>(param.b().internal_value()));
    }

    result_type a() const
    {
        return m_param.a();
    }

    result_type b() const
    {
        return m_param.b();
    }

    result_type min() const
    {
        return this->a();
    }

    result_type max() const
    {
        return this->b();
    }

    void reset() noexcept
    {
        // Nothing to do for uniform distribution.
    }

private:
    // The underlying distribution.
    // To be noticed that we use the internal representation of the fixed number.
    underlying_type m_dist;
    param_type m_param;
};
} // namespace eirin

#ifdef _MSC_VER
#    pragma warning(pop)
#endif

#endif
