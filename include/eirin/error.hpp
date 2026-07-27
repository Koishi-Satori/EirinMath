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
    do {                                                          \
        ::eirin::detail::throw_exception<Exception>(__VA_ARGS__); \
    } while(0)
} // namespace eirin

#endif
