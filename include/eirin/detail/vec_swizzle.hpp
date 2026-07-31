#ifndef EIRIN_MATH_DETAIL_VEC_SWIZZLE_HPP
#define EIRIN_MATH_DETAIL_VEC_SWIZZLE_HPP

#pragma once

#if EIRIN_VEC_SWIZZLE_ENABLE == EIRIN_ENABLE

#    include <eirin/detail/type_tvec.hpp>

namespace eirin
{
namespace detail
{
    template <std::size_t N, int... Is>
    constexpr bool are_swizzle_indices_unique() noexcept
    {
        constexpr int arr[] = {Is...};
        static_assert(sizeof...(Is) == 4, "Expected exactly 4 indices");
        for(std::size_t i = 0; i < N; ++i)
            for(std::size_t j = i + 1; j < N; ++j)
                if(arr[i] == arr[j])
                    return false;
        return true;
    }

    template <typename>
    struct is_swizzle_proxy_type : std::false_type
    {};

    template <typename T>
    concept is_swizzle_proxy = is_swizzle_proxy_type<T>::value;

#    define EIRIN_REQUIRES_NOT_SWIZZLE_PROXY requires(!detail::is_swizzle_proxy<U>)
} // namespace detail

template <std::size_t N, int E0, int E1, int E2, int E3>
concept swizzle_unique = !(E0 == E1 || E0 == E2 || E0 == E3 || E1 == E2 || E1 == E3 || E2 == E3);

template <std::size_t N, std::size_t C, typename T, int E0, int E1, int E2, int E3>
struct swizzle_proxy
{
    using value_type = tvec<C, T>;
    using ref_type = value_type&;
    using data_type = value_type*;

    EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL explicit swizzle_proxy(data_type ptr) noexcept
        : m_data(ptr){};

    swizzle_proxy(const swizzle_proxy&) = default;
    swizzle_proxy(swizzle_proxy&&) = default;
    swizzle_proxy& operator=(const swizzle_proxy&) = default;
    swizzle_proxy& operator=(swizzle_proxy&&) = default;

#    define EIRIN_SWIZZLE_PROXY_DELETE_FUNC_DUPLICATE_NO_TP \
        template <typename _Empty = T>                      \
        requires std::same_as<_Empty, T> && swizzle_unique<N, E0, E1, E2, E3>

#    define EIRIN_SWIZZLE_PROXY_DELETE_FUNC_DUPLICATE_TP \
        requires swizzle_unique<N, E0, E1, E2, E3>

#    define EIRIN_TVEC_SWIZZLE_PROXY_APPLY_SCALAR_IMPL(op, scalar)                    \
        do {                                                                          \
            if constexpr(N >= 1 && E0 != -1) (*m_data)[E0] op static_cast<T>(scalar); \
            if constexpr(N >= 2) (*m_data)[E1] op static_cast<T>(scalar);             \
            if constexpr(N >= 3) (*m_data)[E2] op static_cast<T>(scalar);             \
            if constexpr(N >= 4) (*m_data)[E3] op static_cast<T>(scalar);             \
        } while(0)

#    define EIRIN_TVEC_SWIZZLE_PROXY_APPLY_VECTOR_IMPL(op, vec)             \
        do {                                                                \
            if constexpr(N >= 1) (*m_data)[E0] op static_cast<T>((vec)[0]); \
            if constexpr(N >= 2) (*m_data)[E1] op static_cast<T>((vec)[1]); \
            if constexpr(N >= 3) (*m_data)[E2] op static_cast<T>((vec)[2]); \
            if constexpr(N >= 4) (*m_data)[E3] op static_cast<T>((vec)[3]); \
        } while(0)
#    define EIRIN_TVEC_SWIZZLE_PROXY_COMPOUND_ASSIGN_FUNC(op)                                               \
        template <typename U>                                                                               \
        EIRIN_SWIZZLE_PROXY_DELETE_FUNC_DUPLICATE_TP                                                        \
            EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL swizzle_proxy& operator op##=(const U scalar) noexcept       \
        {                                                                                                   \
            EIRIN_TVEC_SWIZZLE_PROXY_APPLY_SCALAR_IMPL(op## =, scalar);                                     \
            return *this;                                                                                   \
        }                                                                                                   \
        template <std::size_t VecN, typename U>                                                             \
        requires(VecN == N) && swizzle_unique<N, E0, E1, E2, E3>                                            \
        EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL swizzle_proxy& operator op##=(const tvec<VecN, U>& vec) noexcept \
        {                                                                                                   \
            EIRIN_TVEC_SWIZZLE_PROXY_APPLY_VECTOR_IMPL(op## =, vec);                                        \
            return *this;                                                                                   \
        }

    template <typename U>
    requires swizzle_unique<N, E0, E1, E2, E3> && (!detail::is_swizzle_proxy<U>)
    EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL swizzle_proxy& operator=(const U scalar)
    {
        EIRIN_TVEC_SWIZZLE_PROXY_APPLY_SCALAR_IMPL(=, scalar);
        return *this;
    }

    template <std::size_t VecN, typename U>
    requires(VecN >= N) && swizzle_unique<N, E0, E1, E2, E3>
    EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL swizzle_proxy& operator=(const tvec<VecN, U>& vec)
    {
        EIRIN_TVEC_SWIZZLE_PROXY_APPLY_VECTOR_IMPL(=, vec);
        return *this;
    }

    template <typename Proxy>
    requires swizzle_unique<N, E0, E1, E2, E3> && (detail::is_swizzle_proxy<Proxy>)
    EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL swizzle_proxy& operator=(const Proxy& that)
    {
        EIRIN_TVEC_SWIZZLE_PROXY_APPLY_VECTOR_IMPL(=, that);
        return *this;
    }

    EIRIN_TVEC_SWIZZLE_PROXY_COMPOUND_ASSIGN_FUNC(+)
    EIRIN_TVEC_SWIZZLE_PROXY_COMPOUND_ASSIGN_FUNC(-)
    EIRIN_TVEC_SWIZZLE_PROXY_COMPOUND_ASSIGN_FUNC(*)
    EIRIN_TVEC_SWIZZLE_PROXY_COMPOUND_ASSIGN_FUNC(/)
    EIRIN_TVEC_SWIZZLE_PROXY_COMPOUND_ASSIGN_FUNC(%)
    EIRIN_TVEC_SWIZZLE_PROXY_COMPOUND_ASSIGN_FUNC(^)
    EIRIN_TVEC_SWIZZLE_PROXY_COMPOUND_ASSIGN_FUNC(&)
    EIRIN_TVEC_SWIZZLE_PROXY_COMPOUND_ASSIGN_FUNC(|)
    EIRIN_TVEC_SWIZZLE_PROXY_COMPOUND_ASSIGN_FUNC(<<)
    EIRIN_TVEC_SWIZZLE_PROXY_COMPOUND_ASSIGN_FUNC(>>)

    template <typename _Empty = T>
    requires std::same_as<_Empty, T> && swizzle_unique<N, E0, E1, E2, E3>
    EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL swizzle_proxy& operator++() noexcept
    {
        if constexpr(C >= 1 && E0 >= 0) ++(*m_data)[E0];
        if constexpr(C >= 2 && E1 >= 0) ++(*m_data)[E1];
        if constexpr(C >= 3 && E2 >= 0) ++(*m_data)[E2];
        if constexpr(C >= 4 && E3 >= 0) ++(*m_data)[E3];
        return *this;
    }

    EIRIN_SWIZZLE_PROXY_DELETE_FUNC_DUPLICATE_NO_TP
    EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL tvec<N, T> operator++(int) noexcept
    {
        tvec<N, T> old;
        if constexpr(N >= 1) old[0] = (*m_data)[E0];
        if constexpr(N >= 2) old[1] = (*m_data)[E1];
        if constexpr(N >= 3) old[2] = (*m_data)[E2];
        if constexpr(N >= 4) old[3] = (*m_data)[E3];
        if constexpr(N >= 1) ++(*m_data)[E0];
        if constexpr(N >= 2) ++(*m_data)[E1];
        if constexpr(N >= 3) ++(*m_data)[E2];
        if constexpr(N >= 4) ++(*m_data)[E3];
        return old;
    }

    EIRIN_SWIZZLE_PROXY_DELETE_FUNC_DUPLICATE_NO_TP
    EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL swizzle_proxy& operator--() noexcept
    {
        if constexpr(C >= 1 && E0 >= 0) --(*m_data)[E0];
        if constexpr(C >= 2 && E1 >= 0) --(*m_data)[E1];
        if constexpr(C >= 3 && E2 >= 0) --(*m_data)[E2];
        if constexpr(C >= 4 && E3 >= 0) --(*m_data)[E3];
        return *this;
    }

    EIRIN_SWIZZLE_PROXY_DELETE_FUNC_DUPLICATE_NO_TP
    EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL tvec<N, T> operator--(int) noexcept
    {
        tvec<N, T> old;
        if constexpr(N >= 1) old[0] = (*m_data)[E0];
        if constexpr(N >= 2) old[1] = (*m_data)[E1];
        if constexpr(N >= 3) old[2] = (*m_data)[E2];
        if constexpr(N >= 4) old[3] = (*m_data)[E3];
        if constexpr(N >= 1) --(*m_data)[E0];
        if constexpr(N >= 2) --(*m_data)[E1];
        if constexpr(N >= 3) --(*m_data)[E2];
        if constexpr(N >= 4) --(*m_data)[E3];
        return old;
    }

    template <typename U>
    EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL operator tvec<N, U>() const noexcept
    {
        tvec<N, U> result;
        if constexpr(N >= 1) result[0] = static_cast<U>((*m_data)[E0]);
        if constexpr(N >= 2) result[1] = static_cast<U>((*m_data)[E1]);
        if constexpr(N >= 3) result[2] = static_cast<U>((*m_data)[E2]);
        if constexpr(N >= 4) result[3] = static_cast<U>((*m_data)[E3]);
        return result;
    }

    EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL const T& operator[](std::size_t i) const noexcept
    {
        switch(i)
        {
        case 0:
            if constexpr(E0 >= 0)
                return (*m_data)[E0];
            break;
        case 1:
            if constexpr(E1 >= 0)
                return (*m_data)[E1];
            break;
        case 2:
            if constexpr(E2 >= 0)
                return (*m_data)[E2];
            break;
        case 3:
            if constexpr(E3 >= 0)
                return (*m_data)[E3];
            break;
        default:
            EIRIN_UNREACHABLE;
            return (*m_data)[0];
        }
        EIRIN_UNREACHABLE;
        return (*m_data)[0];
    }

    EIRIN_SWIZZLE_PROXY_DELETE_FUNC_DUPLICATE_NO_TP
    EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL T& operator[](std::size_t i) noexcept
    {
        switch(i)
        {
        case 0:
            if constexpr(E0 >= 0)
                return (*m_data)[E0];
            break;
        case 1:
            if constexpr(E1 >= 0)
                return (*m_data)[E1];
            break;
        case 2:
            if constexpr(E2 >= 0)
                return (*m_data)[E2];
            break;
        case 3:
            if constexpr(E3 >= 0)
                return (*m_data)[E3];
            break;
        default:
            EIRIN_UNREACHABLE;
            return (*m_data)[0];
        }
        EIRIN_UNREACHABLE;
        return (*m_data)[0];
    }

    EIRIN_TVEC_SWIZZLE_PROXY_FUNC_DECL tvec<C, T> operator()() const noexcept
    {
        tvec<C, T> result{};
        if constexpr(C >= 1 && E0 >= 0) result[0] = (*m_data)[E0];
        if constexpr(C >= 2 && E1 >= 0) result[1] = (*m_data)[E1];
        if constexpr(C >= 3 && E2 >= 0) result[2] = (*m_data)[E2];
        if constexpr(C >= 4 && E3 >= 0) result[3] = (*m_data)[E3];
        return result;
    }

#    undef EIRIN_TVEC_SWIZZLE_PROXY_APPLY_SCALAR_IMPL
#    undef EIRIN_TVEC_SWIZZLE_PROXY_APPLY_VECTOR_IMPL
#    undef EIRIN_TVEC_SWIZZLE_PROXY_COMPOUND_ASSIGN_FUNC
#    undef EIRIN_SWIZZLE_PROXY_DELETE_FUNC_DUPLICATE_NO_TP
#    undef EIRIN_SWIZZLE_PROXY_DELETE_FUNC_DUPLICATE_TP

private:
    data_type m_data;
};

namespace detail
{
    template <std::size_t N, std::size_t C, typename T, int E0, int E1, int E2, int E3>
    struct is_swizzle_proxy_type<swizzle_proxy<N, C, T, E0, E1, E2, E3>> : std::true_type
    {};
} // namespace detail

#    define EIRIN_TVEC_SWIZZLE_PROXY_UNARY_OPERATOR_IMPL(op)                                                                                                   \
        template <std::size_t N, std::size_t C1, typename T1, int E0, int E1, int E2, int E3, std::size_t C2, typename T2, int E02, int E12, int E22, int E32> \
        constexpr auto operator op(const swizzle_proxy<N, C1, T1, E0, E1, E2, E3>& lhs, const swizzle_proxy<N, C2, T2, E02, E12, E22, E32>& rhs) noexcept      \
            -> decltype(static_cast<tvec<N, T1>>(lhs) op static_cast<tvec<N, T2>>(rhs))                                                                        \
        {                                                                                                                                                      \
            return static_cast<tvec<N, T1>>(lhs) op static_cast<tvec<N, T2>>(rhs);                                                                             \
        }                                                                                                                                                      \
                                                                                                                                                               \
        template <typename U, std::size_t N, std::size_t C, typename T, int E0, int E1, int E2, int E3>                                                        \
        requires(!detail::is_swizzle_proxy<U>)                                                                                                                 \
        constexpr auto operator op(const swizzle_proxy<N, C, T, E0, E1, E2, E3>& lhs, U rhs) noexcept                                                          \
            -> decltype(static_cast<tvec<N, T>>(lhs) op rhs)                                                                                                   \
        {                                                                                                                                                      \
            return static_cast<tvec<N, T>>(lhs) op rhs;                                                                                                        \
        }                                                                                                                                                      \
                                                                                                                                                               \
        template <typename U, std::size_t N, std::size_t C, typename T, int E0, int E1, int E2, int E3>                                                        \
        requires(!detail::is_swizzle_proxy<U>)                                                                                                                 \
        constexpr auto operator op(U lhs, const swizzle_proxy<N, C, T, E0, E1, E2, E3>& rhs) noexcept                                                          \
            -> decltype(lhs op static_cast<tvec<N, T>>(rhs))                                                                                                   \
        {                                                                                                                                                      \
            return lhs op static_cast<tvec<N, T>>(rhs);                                                                                                        \
        }

EIRIN_TVEC_SWIZZLE_PROXY_UNARY_OPERATOR_IMPL(+)
EIRIN_TVEC_SWIZZLE_PROXY_UNARY_OPERATOR_IMPL(-)
EIRIN_TVEC_SWIZZLE_PROXY_UNARY_OPERATOR_IMPL(*)
EIRIN_TVEC_SWIZZLE_PROXY_UNARY_OPERATOR_IMPL(/)
EIRIN_TVEC_SWIZZLE_PROXY_UNARY_OPERATOR_IMPL(%)
EIRIN_TVEC_SWIZZLE_PROXY_UNARY_OPERATOR_IMPL(^)
EIRIN_TVEC_SWIZZLE_PROXY_UNARY_OPERATOR_IMPL(&)
EIRIN_TVEC_SWIZZLE_PROXY_UNARY_OPERATOR_IMPL(|)
EIRIN_TVEC_SWIZZLE_PROXY_UNARY_OPERATOR_IMPL(<<)
EIRIN_TVEC_SWIZZLE_PROXY_UNARY_OPERATOR_IMPL(>>)

#    undef EIRIN_TVEC_SWIZZLE_PROXY_UNARY_OPERATOR_IMPL

template <std::size_t N, std::size_t C, typename T, int E0, int E1, int E2, int E3>
struct const_swizzle_proxy
{
    using value_type = tvec<C, T>;
    using data_type = const value_type*;

    explicit constexpr const_swizzle_proxy(data_type ptr) noexcept
        : m_data(ptr) {}

    operator tvec<C, T>() const noexcept
    {
        tvec<C, T> result{};
        if constexpr(N >= 1) result[0] = (*m_data)[E0];
        if constexpr(N >= 2) result[1] = (*m_data)[E1];
        if constexpr(N >= 3) result[2] = (*m_data)[E2];
        if constexpr(N >= 4) result[3] = (*m_data)[E3];
        return result;
    }

    const T& operator[](std::size_t i) const noexcept
    {
        switch(i)
        {
        case 0:
            if constexpr(E0 >= 0)
                return (*m_data)[E0];
            break;
        case 1:
            if constexpr(E1 >= 0)
                return (*m_data)[E1];
            break;
        case 2:
            if constexpr(E2 >= 0)
                return (*m_data)[E2];
            break;
        case 3:
            if constexpr(E3 >= 0)
                return (*m_data)[E3];
            break;
        default:
            EIRIN_UNREACHABLE;
            return (*m_data)[0];
        }
        EIRIN_UNREACHABLE;
        return (*m_data)[0];
    }

private:
    data_type m_data;
};

#    define EIRIN_TVEC_SWIZZLE_DECL_IMPL(N, C, T, NAME, E0_IDX, E1_IDX, E2_IDX, E3_IDX)                                  \
        EIRIN_ALWAYS_INLINE constexpr swizzle_proxy<N, C, T, E0_IDX, E1_IDX, E2_IDX, E3_IDX> NAME() noexcept             \
        {                                                                                                                \
            return swizzle_proxy<N, C, T, E0_IDX, E1_IDX, E2_IDX, E3_IDX>{this};                                         \
        }                                                                                                                \
        EIRIN_ALWAYS_INLINE constexpr const_swizzle_proxy<N, C, T, E0_IDX, E1_IDX, E2_IDX, E3_IDX> NAME() const noexcept \
        {                                                                                                                \
            return const_swizzle_proxy<N, C, T, E0_IDX, E1_IDX, E2_IDX, E3_IDX>{this};                                   \
        }

#    define EIRIN_TVEC_SWIZZLE_4_MEMBERS_DECL_IMPL(N, T, E0, E1, E2, E3, E0_IDX, E1_IDX, E2_IDX, E3_IDX) \
        EIRIN_TVEC_SWIZZLE_DECL_IMPL(N, T, E0##E1##E2##E3, E0_IDX, E1_IDX, E2_IDX, E3_IDX)
#    define EIRIN_TVEC_SWIZZLE_3_MEMBERS_DECL_IMPL(N, T, E0, E1, E2, E0_IDX, E1_IDX, E2_IDX) \
        EIRIN_TVEC_SWIZZLE_DECL_IMPL(N, T, E0##E1##E2, E0_IDX, E1_IDX, E2_IDX, 0)
#    define EIRIN_TVEC_SWIZZLE_2_MEMBERS_DECL_IMPL(N, T, E0, E1, E0_IDX, E1_IDX) \
        EIRIN_TVEC_SWIZZLE_DECL_IMPL(N, T, E0##E1, E0_IDX, E1_IDX, 0, 0)

#    include <eirin/detail/vec_swizzle_decl.hpp>

} // namespace eirin

#else

#    define EIRIN_REQUIRES_NOT_SWIZZLE_PROXY

#endif

#endif
