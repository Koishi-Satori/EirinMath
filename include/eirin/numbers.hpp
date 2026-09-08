#ifndef EIRIN_MATH_NUMBERS_HPP
#define EIRIN_MATH_NUMBERS_HPP

#pragma once

#include "fixed.hpp"

namespace eirin::numbers
{
template <typename T>
using enable_if_fixed = std::enable_if_t<is_fixed_point_v<T>, T>;

// e
template <typename T>
EIRIN_ALWAYS_INLINE constexpr enable_if_fixed<T> e_v()
{
    return detail::eval_const<char, typename T::value_type, typename T::intermediate_type, T::precision, T::is_round_enable>("2.718281828459045235360287471352662498");
}

// pi
template <typename T>
EIRIN_ALWAYS_INLINE constexpr enable_if_fixed<T> pi_v()
{
    return detail::eval_const<char, typename T::value_type, typename T::intermediate_type, T::precision, T::is_round_enable>("3.141592653589793238462643383279502884");
}

// log2e
template <typename T>
EIRIN_ALWAYS_INLINE constexpr enable_if_fixed<T> log2e_v()
{
    return detail::eval_const<char, typename T::value_type, typename T::intermediate_type, T::precision, T::is_round_enable>("1.442695040888963407359924681001892137");
}

// log10e
template <typename T>
EIRIN_ALWAYS_INLINE constexpr enable_if_fixed<T> log10e_v()
{
    // return T::template from_fixed_num_value<61>(0xde5bd8a93728700ll);
    return detail::eval_const<char, typename T::value_type, typename T::intermediate_type, T::precision, T::is_round_enable>("0.434294481903251827651128918916605082");
}

// inv_pi
template <typename T>
EIRIN_ALWAYS_INLINE constexpr enable_if_fixed<T> inv_pi_v()
{
    // return T::template from_fixed_num_value<61>(0xa2f9836e4e44180ll);
    return detail::eval_const<char, typename T::value_type, typename T::intermediate_type, T::precision, T::is_round_enable>("0.318309886183790671537767526745028");
}

// inv_sqrtpi
template <typename T>
EIRIN_ALWAYS_INLINE constexpr enable_if_fixed<T> inv_sqrtpi_v()
{
    // return T::template from_fixed_num_value<61>(0x120dd750429b6d00ll);
    return detail::eval_const<char, typename T::value_type, typename T::intermediate_type, T::precision, T::is_round_enable>("0.564189583547756286948079451560772");
}

// ln2
template <typename T>
EIRIN_ALWAYS_INLINE constexpr enable_if_fixed<T> ln2_v()
{
    return detail::eval_const<char, typename T::value_type, typename T::intermediate_type, T::precision, T::is_round_enable>("0.693147180559945309417232121458176568");
}

// ln10
template <typename T>
EIRIN_ALWAYS_INLINE constexpr enable_if_fixed<T> ln10_v()
{
    // return T::template from_fixed_num_value<61>(0x49aec6eed5545800ll);
    return detail::eval_const<char, typename T::value_type, typename T::intermediate_type, T::precision, T::is_round_enable>("2.302585092994045684017991454684364208");
}

// sqrt2
template <typename T>
EIRIN_ALWAYS_INLINE constexpr enable_if_fixed<T> sqrt2_v()
{
    // return T::template from_fixed_num_value<61>(0x2d413cccfe779a00ll);
    return detail::eval_const<char, typename T::value_type, typename T::intermediate_type, T::precision, T::is_round_enable>("1.414213562373095048801688724209698");
}

// sqrt3
template <typename T>
EIRIN_ALWAYS_INLINE constexpr enable_if_fixed<T> sqrt3_v()
{
    // return T::template from_fixed_num_value<61>(0x376cf5d0b0995400ll);
    return detail::eval_const<char, typename T::value_type, typename T::intermediate_type, T::precision, T::is_round_enable>("1.732050807568877293527446341505872");
}

// inv_sqrt3
template <typename T>
EIRIN_ALWAYS_INLINE constexpr enable_if_fixed<T> inv_sqrt3_v()
{
    // return T::template from_fixed_num_value<61>(0x1279a74590331c00ll);
    return detail::eval_const<char, typename T::value_type, typename T::intermediate_type, T::precision, T::is_round_enable>("0.577350269189625764509148780501957");
}

// egamma
template <typename T>
EIRIN_ALWAYS_INLINE constexpr enable_if_fixed<T> egamma_v()
{
    // return T::template from_fixed_num_value<61>(0x12788cfc6fb61900ll);
    return detail::eval_const<char, typename T::value_type, typename T::intermediate_type, T::precision, T::is_round_enable>("0.577215664901532860606512090082402");
}

// phi
template <typename T>
EIRIN_ALWAYS_INLINE constexpr enable_if_fixed<T> phi_v()
{
    // return T::template from_fixed_num_value<61>(0x33c6ef372fe95000ll);
    return detail::eval_const<char, typename T::value_type, typename T::intermediate_type, T::precision, T::is_round_enable>("1.618033988749894848204586834365638");
}

inline constexpr auto pi = pi_v<fixed32>();
inline constexpr auto e = e_v<fixed32>();
inline constexpr auto log2e = log2e_v<fixed32>();
inline constexpr auto log10e = log10e_v<fixed32>();
inline constexpr auto inv_pi = inv_pi_v<fixed32>();
inline constexpr auto inv_sqrtpi = inv_sqrtpi_v<fixed32>();
inline constexpr auto ln2 = ln2_v<fixed32>();
inline constexpr auto ln10 = ln10_v<fixed32>();
inline constexpr auto sqrt2 = sqrt2_v<fixed32>();
inline constexpr auto sqrt3 = sqrt3_v<fixed32>();
inline constexpr auto inv_sqrt3 = inv_sqrt3_v<fixed32>();
inline constexpr auto egamma = egamma_v<fixed32>();
inline constexpr auto phi = phi_v<fixed32>();

#ifdef EIRIN_MATH_HAS_INT128
inline constexpr auto pi_f64 = pi_v<fixed64>();
inline constexpr auto e_f64 = e_v<fixed64>();
inline constexpr auto log2e_f64 = log2e_v<fixed64>();
inline constexpr auto log10e_f64 = log10e_v<fixed64>();
inline constexpr auto inv_pi_f64 = inv_pi_v<fixed64>();
inline constexpr auto inv_sqrtpi_f64 = inv_sqrtpi_v<fixed64>();
inline constexpr auto ln2_f64 = ln2_v<fixed64>();
inline constexpr auto ln10_f64 = ln10_v<fixed64>();
inline constexpr auto sqrt2_f64 = sqrt2_v<fixed64>();
inline constexpr auto sqrt3_f64 = sqrt3_v<fixed64>();
inline constexpr auto inv_sqrt3_f64 = inv_sqrt3_v<fixed64>();
inline constexpr auto egamma_f64 = egamma_v<fixed64>();
inline constexpr auto phi_f64 = phi_v<fixed64>();
#endif

} // namespace eirin::numbers

#endif
