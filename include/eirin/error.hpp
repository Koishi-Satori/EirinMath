#ifndef EIRIN_MATH_ERROR_HPP
#define EIRIN_MATH_ERROR_HPP

#include <stdexcept>

namespace eirin
{
class divide_by_zero : public std::domain_error
{
public:
    divide_by_zero()
        : std::domain_error("divide by zero") {};
};

namespace detail
{
    template <typename Exception, typename... Args>
    [[noreturn]]
    constexpr inline void throw_exception(
        [[maybe_unused]] Args&&... args
    )
    {
#ifdef EIRIN_NO_EXCEPTIONS
        std::terminate();
#else
        throw Exception(std::forward<Args>(args)...);
#endif
    }
} // namespace detail

#define EIRIN_THROW_EXCEPTION(Exception, ...)                     \
    do                                                            \
    {                                                             \
        ::eirin::detail::throw_exception<Exception>(__VA_ARGS__); \
    } while(0)
} // namespace eirin

// math function domain-error behavior. By default the functions throw
// std::domain_error when the input is out of the function domain (or call
// std::terminate() when EIRIN_NO_EXCEPTIONS is defined, see macros before).
// Define EIRIN_MATH_DOMAIN_SILENT to make them return a sentinel value
// instead; the sentinel is documented per function in math.hpp.
// Also, this will cause some function which not marked as `noexcept` before
// marked as `noexcept`.
#ifdef EIRIN_MATH_DOMAIN_SILENT
#    define EIRIN_MATH_DOMAIN_ERROR(msg, ret) return (ret)
#    define EIRIN_MATH_TRY_NOEXCEPT noexcept
#else
#    define EIRIN_MATH_DOMAIN_ERROR(msg, ret) EIRIN_THROW_EXCEPTION(std::domain_error, msg)
#    define EIRIN_MATH_TRY_NOEXCEPT
#endif

#endif
