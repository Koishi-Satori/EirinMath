#ifndef EIRIN_MATH_VEC_HPP
#define EIRIN_MATH_VEC_HPP

#include <cstddef>

namespace eirin
{
template <std::size_t N, typename T>
struct tvec;

template <std::size_t N, typename T, typename Derived>
struct tvec_base
{
    using size_type = std::size_t;
    using value_type = T;

    [[nodiscard]]
    static size_type size() noexcept
    {
        return N;
    }

    Derived operator+(const Derived& rhs) const noexcept
    {
        Derived tmp(derived());
        tmp += rhs;
        return tmp;
    }

private:
    Derived& derived() noexcept
    {
        return static_cast<Derived&>(*this);
    }

    const Derived& derived() const noexcept
    {
        return static_cast<const Derived&>(*this);
    }
};

template <typename T>
struct tvec<2, T> : public tvec_base<2, T, tvec<2, T>>
{
    using value_type = T;

    union data_type
    {
        T x, y;
    };

    data_type data;

    tvec& operator=(const tvec& other) noexcept = default;

    constexpr T& operator[](std::size_t i) noexcept
    {
        if(i == 0) return data.x;
        return data.y;
    }

    constexpr const T& operator[](std::size_t i) const noexcept
    {
        if(i == 0) return data.x;
        return data.y;
    }

    tvec& operator+=(const tvec& rhs) noexcept
    {
        data.x += rhs.data.x;
        data.y += rhs.data.y;
        return *this;
    }
};
} // namespace eirin

#endif
