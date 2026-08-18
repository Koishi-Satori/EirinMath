#ifndef EIRIN_MATH_EIRIN_HPP
#define EIRIN_MATH_EIRIN_HPP

// IWYU pragma: begin_exports

#include "macro.hpp"
#include "fixed.hpp"
#include "math.hpp"
#include "io/parse.hpp"
#include "numbers.hpp"
#include "random.hpp"
#include "vec.hpp"

// IWYU pragma: end_exports

#define EIRIN_MATH_MAJOR_VERSION 1
#define EIRIN_MATH_MINOR_VERSION 2
#define EIRIN_MATH_PATCH_VERSION 0

#include <tuple>

namespace eirin
{
[[nodiscard]]
inline constexpr std::tuple<int, int, int> get_version() noexcept
{
    return {EIRIN_MATH_MAJOR_VERSION, EIRIN_MATH_MINOR_VERSION, EIRIN_MATH_PATCH_VERSION};
}
} // namespace eirin

#endif
