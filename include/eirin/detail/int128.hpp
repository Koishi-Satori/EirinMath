#ifndef EIRIN_MATH_DETAIL_INT128_HPP
#define EIRIN_MATH_DETAIL_INT128_HPP

#pragma once
#ifdef __GNUC__
#    ifndef __clang__
#        pragma GCC diagnostic ignored "-Wpedantic"
#    endif
#endif

#include <version>
#include "../macro.hpp"
#if EIRIN_USE_EXT_BUILTIN_INT128 == EIRIN_ENABLE
#    include "eirin/ext/builtin_ints.hpp"
#endif

#ifdef _MSC_VER
#    define EIRIN_MATH_HAS_INT128
// int128 provided by MSVC STL
#    define EIRIN_MATH_DETAIL_INT128_MSVC_STL

#    include <__msvc_int128.hpp>
#endif

#ifdef __GNUC__
#    define EIRIN_MATH_HAS_INT128
// Built-in __int128
#    define EIRIN_MATH_DETAIL_BUILTIN__INT128
#endif

namespace eirin::detail
{
#if EIRIN_USE_EXT_BUILTIN_INT128 == EIRIN_ENABLE
using int128_t = ::eirin::ext::int128;
using uint128_t = ::eirin::ext::uint128;
#else
#    ifdef EIRIN_MATH_DETAIL_INT128_MSVC_STL

using int128_t = std::_Signed128;
using uint128_t = std::_Unsigned128;

#    endif

#    ifdef EIRIN_MATH_DETAIL_BUILTIN__INT128

using int128_t = __int128;
using uint128_t = unsigned __int128;

#    endif
#endif
} // namespace eirin::detail

#endif
