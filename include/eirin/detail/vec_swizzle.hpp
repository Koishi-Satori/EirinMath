#ifndef EIRIN_MATH_DETAIL_VEC_SWIZZLE_HPP
#define EIRIN_MATH_DETAIL_VEC_SWIZZLE_HPP

#pragma once

#if EIRIN_VEC_SWIZZLE_ENABLE == EIRIN_ENABLE

#    include <array>
#    include <cstddef>
#    include <type_traits>
#    include <utility>
#    include <eirin/detail/type_tvec.hpp>

namespace eirin
{
namespace detail
{
    template <std::size_t Pos, std::size_t Head, std::size_t... Tail>
    struct idx_at : idx_at<Pos - 1, Tail...>
    {};

    template <std::size_t Head, std::size_t... Tail>
    struct idx_at<0, Head, Tail...>
    {
        static constexpr std::size_t value = Head;
    };

    template <std::size_t... Indices>
    constexpr bool all_distinct() noexcept
    {
        constexpr std::size_t arr[] = {Indices...};
        for(std::size_t i = 0; i < sizeof...(Indices); ++i)
            for(std::size_t j = i + 1; j < sizeof...(Indices); ++j)
                if(arr[i] == arr[j])
                    return false;
        return true;
    }

    template <typename>
    struct is_swizzle_proxy_type : std::false_type
    {};

    template <typename T>
    concept is_swizzle_proxy = is_swizzle_proxy_type<T>::value;
} // namespace detail

template <std::size_t N, typename T, bool IsConst, std::size_t... Indices>
class swizzle_proxy;

namespace detail
{
    template <std::size_t N, typename T, bool IsConst, std::size_t... Indices>
    struct is_swizzle_proxy_type<swizzle_proxy<N, T, IsConst, Indices...>> : std::true_type
    {};

    template <typename P>
    struct proxy_traits;

    template <std::size_t N, typename T, bool IsConst, std::size_t... Indices>
    struct proxy_traits<swizzle_proxy<N, T, IsConst, Indices...>>
    {
        static constexpr std::size_t source_size = N;
        static constexpr std::size_t size = sizeof...(Indices);
        using value_type = T;
    };
} // namespace detail

/**
 * @brief Shared implementation of a swizzle view over `tvec<N, T>`.
 *
 * `Indices...` are absolute component indices in the original vector. The
 * proxy keeps only a pointer to that vector, so no data is copied; reads go
 * through `operator[]` and mutations write through it when the proxy is not
 * const and the selection has no duplicated component.
 */
template <std::size_t N, typename T, bool IsConst, std::size_t... Indices>
class swizzle_proxy_base
{
    static_assert(sizeof...(Indices) >= 2 && sizeof...(Indices) <= 4, "a swizzle proxy must select 2..4 components");
    static_assert(((Indices < N) && ...), "swizzle index out of range");

protected:
    using vec_type = tvec<N, T>;
    using pointer_type = std::conditional_t<IsConst, const vec_type*, vec_type*>;

    pointer_type m_data;

    static constexpr std::size_t k_size = sizeof...(Indices);
    static constexpr bool k_distinct = detail::all_distinct<Indices...>();

    static constexpr std::array<std::size_t, k_size> index_table() noexcept
    {
        return {Indices...};
    }

    template <std::size_t Pos>
    static constexpr std::size_t mapped() noexcept
    {
        return detail::idx_at<Pos, Indices...>::value;
    }

public:
    constexpr swizzle_proxy_base(pointer_type ptr) noexcept
        : m_data(ptr)
    {}

    static constexpr std::size_t size() noexcept
    {
        return k_size;
    }

    // ---- element access -------------------------------------------------
    constexpr const T& operator[](std::size_t i) const noexcept
    {
        return (*m_data)[index_table()[i]];
    }

    template <typename Dummy = void>
    requires(!IsConst && k_distinct)
    constexpr T& operator[](std::size_t i) noexcept
    {
        return (*m_data)[index_table()[i]];
    }

#    define EIRIN_SWIZZLE_PROXY_COMPONENT_ACCESS(letter, position) \
        template <typename Dummy = void>                           \
        requires(!IsConst && k_size >= (position + 1))             \
        constexpr T& letter() noexcept                             \
        {                                                          \
            return (*m_data)[mapped<position>()];                  \
        }                                                          \
        template <typename Dummy = void>                           \
        requires(k_size >= (position + 1))                         \
        constexpr const T& letter() const noexcept                 \
        {                                                          \
            return (*m_data)[mapped<position>()];                  \
        }

    EIRIN_SWIZZLE_PROXY_COMPONENT_ACCESS(x, 0)
    EIRIN_SWIZZLE_PROXY_COMPONENT_ACCESS(y, 1)
    EIRIN_SWIZZLE_PROXY_COMPONENT_ACCESS(z, 2)
    EIRIN_SWIZZLE_PROXY_COMPONENT_ACCESS(w, 3)

#    undef EIRIN_SWIZZLE_PROXY_COMPONENT_ACCESS

    // ---- materialization -------------------------------------------------
    template <typename U>
    constexpr operator tvec<k_size, U>() const noexcept
    {
        tvec<k_size, U> result{};
        for(std::size_t i = 0; i < k_size; ++i)
            result[i] = static_cast<U>((*m_data)[index_table()[i]]);
        return result;
    }

    constexpr tvec<k_size, T> operator()() const noexcept
    {
        return static_cast<tvec<k_size, T>>(*this);
    }

    // ---- mutation helpers used by the concrete proxy classes -------------

protected:
    template <typename U>
    constexpr void assign_scalar(U scalar) noexcept
    {
        [&]<std::size_t... Ps>(std::index_sequence<Ps...>)
        {
            (static_cast<void>((*m_data)[index_table()[Ps]] = static_cast<T>(scalar)), ...);
        }(std::make_index_sequence<k_size>{});
    }

    template <std::size_t RN, typename U>
    constexpr void assign_vec(const tvec<RN, U>& vec) noexcept
    {
        static_assert(RN == k_size);
        [&]<std::size_t... Ps>(std::index_sequence<Ps...>)
        {
            (static_cast<void>((*m_data)[index_table()[Ps]] = static_cast<T>(vec[Ps])), ...);
        }(std::make_index_sequence<k_size>{});
    }

    template <typename Proxy>
    constexpr void assign_proxy(const Proxy& that) noexcept
    {
        [&]<std::size_t... Ps>(std::index_sequence<Ps...>)
        {
            (static_cast<void>((*m_data)[index_table()[Ps]] = static_cast<T>(that[Ps])), ...);
        }(std::make_index_sequence<k_size>{});
    }

public:
// ---- compound assignments -------------------------------------------------
#    define EIRIN_SWIZZLE_PROXY_COMPOUND(op)                                                                               \
        template <typename U>                                                                                              \
        requires(!IsConst && k_distinct && !detail::is_swizzle_proxy<U>)                                                   \
        constexpr swizzle_proxy_base& operator op##=(U scalar) noexcept                                                    \
        {                                                                                                                  \
            [&]<std::size_t... Ps>(std::index_sequence<Ps...>)                                                             \
            {                                                                                                              \
                (static_cast<void>((*m_data)[index_table()[Ps]] op## = static_cast<T>(scalar)), ...);                      \
            }(std::make_index_sequence<k_size>{});                                                                         \
            return *this;                                                                                                  \
        }                                                                                                                  \
        template <std::size_t RN, typename U>                                                                              \
        requires(!IsConst && k_distinct && RN == k_size)                                                                   \
        constexpr swizzle_proxy_base& operator op##=(const tvec<RN, U>& vec) noexcept                                      \
        {                                                                                                                  \
            [&]<std::size_t... Ps>(std::index_sequence<Ps...>)                                                             \
            {                                                                                                              \
                (static_cast<void>((*m_data)[index_table()[Ps]] op## = static_cast<T>(vec[Ps])), ...);                     \
            }(std::make_index_sequence<k_size>{});                                                                         \
            return *this;                                                                                                  \
        }                                                                                                                  \
        template <typename Proxy>                                                                                          \
        requires(!IsConst && k_distinct && detail::is_swizzle_proxy<Proxy> && detail::proxy_traits<Proxy>::size == k_size) \
        constexpr swizzle_proxy_base& operator op##=(const Proxy & that) noexcept                                          \
        {                                                                                                                  \
            [&]<std::size_t... Ps>(std::index_sequence<Ps...>)                                                             \
            {                                                                                                              \
                (static_cast<void>((*m_data)[index_table()[Ps]] op## = static_cast<T>(that[Ps])), ...);                    \
            }(std::make_index_sequence<k_size>{});                                                                         \
            return *this;                                                                                                  \
        }

    EIRIN_SWIZZLE_PROXY_COMPOUND(+)
    EIRIN_SWIZZLE_PROXY_COMPOUND(-)
    EIRIN_SWIZZLE_PROXY_COMPOUND(*)
    EIRIN_SWIZZLE_PROXY_COMPOUND(/)
    EIRIN_SWIZZLE_PROXY_COMPOUND(%)
    EIRIN_SWIZZLE_PROXY_COMPOUND(^)
    EIRIN_SWIZZLE_PROXY_COMPOUND(&)
    EIRIN_SWIZZLE_PROXY_COMPOUND(|)
    EIRIN_SWIZZLE_PROXY_COMPOUND(<<)
    EIRIN_SWIZZLE_PROXY_COMPOUND(>>)

#    undef EIRIN_SWIZZLE_PROXY_COMPOUND

    template <typename Dummy = void>
    requires(!IsConst && k_distinct)
    constexpr swizzle_proxy_base& operator++() noexcept
    {
        [&]<std::size_t... Ps>(std::index_sequence<Ps...>)
        {
            (++(*m_data)[index_table()[Ps]], ...);
        }(std::make_index_sequence<k_size>{});
        return *this;
    }

    template <typename Dummy = void>
    requires(!IsConst && k_distinct)
    constexpr swizzle_proxy_base& operator--() noexcept
    {
        [&]<std::size_t... Ps>(std::index_sequence<Ps...>)
        {
            (--(*m_data)[index_table()[Ps]], ...);
        }(std::make_index_sequence<k_size>{});
        return *this;
    }

    template <typename Dummy = void>
    requires(!IsConst && k_distinct)
    constexpr tvec<k_size, T> operator++(int) noexcept
    {
        const auto old = static_cast<tvec<k_size, T>>(*this);
        ++*this;
        return old;
    }

    template <typename Dummy = void>
    requires(!IsConst && k_distinct)
    constexpr tvec<k_size, T> operator--(int) noexcept
    {
        const auto old = static_cast<tvec<k_size, T>>(*this);
        --*this;
        return old;
    }
};

// assignment operators are not inherited, so every concrete proxy repeats a
// small forwarding set.
#    define EIRIN_SWIZZLE_PROXY_ASSIGN_DECLS                                                                                     \
        template <typename U>                                                                                                    \
        requires(!C && base::k_distinct && !detail::is_swizzle_proxy<U>)                                                         \
        constexpr swizzle_proxy& operator=(U scalar) noexcept                                                                    \
        {                                                                                                                        \
            this->assign_scalar(scalar);                                                                                         \
            return *this;                                                                                                        \
        }                                                                                                                        \
        template <std::size_t RN, typename U>                                                                                    \
        requires(!C && base::k_distinct && RN == base::k_size)                                                                   \
        constexpr swizzle_proxy& operator=(const tvec<RN, U>& vec) noexcept                                                      \
        {                                                                                                                        \
            this->assign_vec(vec);                                                                                               \
            return *this;                                                                                                        \
        }                                                                                                                        \
        template <typename Proxy>                                                                                                \
        requires(!C && base::k_distinct && detail::is_swizzle_proxy<Proxy> && detail::proxy_traits<Proxy>::size == base::k_size) \
        constexpr swizzle_proxy& operator=(const Proxy& that) noexcept                                                           \
        {                                                                                                                        \
            this->assign_proxy(that);                                                                                            \
            return *this;                                                                                                        \
        }

#    include <eirin/detail/vec_swizzle_decl.hpp>

// selectors that produce 2 components out of a 2-component view
#    define EIRIN_TVEC_SWIZZLE_IMPL_2(SRC, NAME, P0, P1)                                                       \
        constexpr auto NAME() noexcept                                                                         \
            -> swizzle_proxy<N, T, C, detail::idx_at<P0, I0, I1>::value, detail::idx_at<P1, I0, I1>::value>    \
        {                                                                                                      \
            return {this->m_data};                                                                             \
        }                                                                                                      \
        constexpr auto NAME() const noexcept                                                                   \
            -> swizzle_proxy<N, T, true, detail::idx_at<P0, I0, I1>::value, detail::idx_at<P1, I0, I1>::value> \
        {                                                                                                      \
            return {this->m_data};                                                                             \
        }
#    define EIRIN_TVEC_SWIZZLE_IMPL_3(SRC, NAME, P0, P1, P2)                                                                                      \
        constexpr auto NAME() noexcept                                                                                                            \
            -> swizzle_proxy<N, T, C, detail::idx_at<P0, I0, I1>::value, detail::idx_at<P1, I0, I1>::value, detail::idx_at<P2, I0, I1>::value>    \
        {                                                                                                                                         \
            return {this->m_data};                                                                                                                \
        }                                                                                                                                         \
        constexpr auto NAME() const noexcept                                                                                                      \
            -> swizzle_proxy<N, T, true, detail::idx_at<P0, I0, I1>::value, detail::idx_at<P1, I0, I1>::value, detail::idx_at<P2, I0, I1>::value> \
        {                                                                                                                                         \
            return {this->m_data};                                                                                                                \
        }
#    define EIRIN_TVEC_SWIZZLE_IMPL_4(SRC, NAME, P0, P1, P2, P3)                                                                                                                     \
        constexpr auto NAME() noexcept                                                                                                                                               \
            -> swizzle_proxy<N, T, C, detail::idx_at<P0, I0, I1>::value, detail::idx_at<P1, I0, I1>::value, detail::idx_at<P2, I0, I1>::value, detail::idx_at<P3, I0, I1>::value>    \
        {                                                                                                                                                                            \
            return {this->m_data};                                                                                                                                                   \
        }                                                                                                                                                                            \
        constexpr auto NAME() const noexcept                                                                                                                                         \
            -> swizzle_proxy<N, T, true, detail::idx_at<P0, I0, I1>::value, detail::idx_at<P1, I0, I1>::value, detail::idx_at<P2, I0, I1>::value, detail::idx_at<P3, I0, I1>::value> \
        {                                                                                                                                                                            \
            return {this->m_data};                                                                                                                                                   \
        }

template <std::size_t N, typename T, bool C, std::size_t I0, std::size_t I1>
class swizzle_proxy<N, T, C, I0, I1> : public swizzle_proxy_base<N, T, C, I0, I1>
{
    using base = swizzle_proxy_base<N, T, C, I0, I1>;

public:
    using base::base;
    EIRIN_SWIZZLE_PROXY_ASSIGN_DECLS
    EIRIN_TVEC_SWIZZLE2_4_MEMBERS_DECL(T, x, y)
    EIRIN_TVEC_SWIZZLE2_3_MEMBERS_DECL(T, x, y)
    EIRIN_TVEC_SWIZZLE2_2_MEMBERS_DECL(T, x, y)
};

#    undef EIRIN_TVEC_SWIZZLE_IMPL_2
#    undef EIRIN_TVEC_SWIZZLE_IMPL_3
#    undef EIRIN_TVEC_SWIZZLE_IMPL_4

// selectors for a 3-component view
#    define EIRIN_TVEC_SWIZZLE_IMPL_2(SRC, NAME, P0, P1)                                                               \
        constexpr auto NAME() noexcept                                                                                 \
            -> swizzle_proxy<N, T, C, detail::idx_at<P0, I0, I1, I2>::value, detail::idx_at<P1, I0, I1, I2>::value>    \
        {                                                                                                              \
            return {this->m_data};                                                                                     \
        }                                                                                                              \
        constexpr auto NAME() const noexcept                                                                           \
            -> swizzle_proxy<N, T, true, detail::idx_at<P0, I0, I1, I2>::value, detail::idx_at<P1, I0, I1, I2>::value> \
        {                                                                                                              \
            return {this->m_data};                                                                                     \
        }
#    define EIRIN_TVEC_SWIZZLE_IMPL_3(SRC, NAME, P0, P1, P2)                                                                                                  \
        constexpr auto NAME() noexcept                                                                                                                        \
            -> swizzle_proxy<N, T, C, detail::idx_at<P0, I0, I1, I2>::value, detail::idx_at<P1, I0, I1, I2>::value, detail::idx_at<P2, I0, I1, I2>::value>    \
        {                                                                                                                                                     \
            return {this->m_data};                                                                                                                            \
        }                                                                                                                                                     \
        constexpr auto NAME() const noexcept                                                                                                                  \
            -> swizzle_proxy<N, T, true, detail::idx_at<P0, I0, I1, I2>::value, detail::idx_at<P1, I0, I1, I2>::value, detail::idx_at<P2, I0, I1, I2>::value> \
        {                                                                                                                                                     \
            return {this->m_data};                                                                                                                            \
        }
#    define EIRIN_TVEC_SWIZZLE_IMPL_4(SRC, NAME, P0, P1, P2, P3)                                                                                                                                     \
        constexpr auto NAME() noexcept                                                                                                                                                               \
            -> swizzle_proxy<N, T, C, detail::idx_at<P0, I0, I1, I2>::value, detail::idx_at<P1, I0, I1, I2>::value, detail::idx_at<P2, I0, I1, I2>::value, detail::idx_at<P3, I0, I1, I2>::value>    \
        {                                                                                                                                                                                            \
            return {this->m_data};                                                                                                                                                                   \
        }                                                                                                                                                                                            \
        constexpr auto NAME() const noexcept                                                                                                                                                         \
            -> swizzle_proxy<N, T, true, detail::idx_at<P0, I0, I1, I2>::value, detail::idx_at<P1, I0, I1, I2>::value, detail::idx_at<P2, I0, I1, I2>::value, detail::idx_at<P3, I0, I1, I2>::value> \
        {                                                                                                                                                                                            \
            return {this->m_data};                                                                                                                                                                   \
        }

template <std::size_t N, typename T, bool C, std::size_t I0, std::size_t I1, std::size_t I2>
class swizzle_proxy<N, T, C, I0, I1, I2> : public swizzle_proxy_base<N, T, C, I0, I1, I2>
{
    using base = swizzle_proxy_base<N, T, C, I0, I1, I2>;

public:
    using base::base;
    EIRIN_SWIZZLE_PROXY_ASSIGN_DECLS
    EIRIN_TVEC_SWIZZLE3_4_MEMBERS_DECL(T, x, y, z)
    EIRIN_TVEC_SWIZZLE3_3_MEMBERS_DECL(T, x, y, z)
    EIRIN_TVEC_SWIZZLE3_2_MEMBERS_DECL(T, x, y, z)
};

#    undef EIRIN_TVEC_SWIZZLE_IMPL_2
#    undef EIRIN_TVEC_SWIZZLE_IMPL_3
#    undef EIRIN_TVEC_SWIZZLE_IMPL_4

// selectors for a 4-component view
#    define EIRIN_TVEC_SWIZZLE_IMPL_2(SRC, NAME, P0, P1)                                                                       \
        constexpr auto NAME() noexcept                                                                                         \
            -> swizzle_proxy<N, T, C, detail::idx_at<P0, I0, I1, I2, I3>::value, detail::idx_at<P1, I0, I1, I2, I3>::value>    \
        {                                                                                                                      \
            return {this->m_data};                                                                                             \
        }                                                                                                                      \
        constexpr auto NAME() const noexcept                                                                                   \
            -> swizzle_proxy<N, T, true, detail::idx_at<P0, I0, I1, I2, I3>::value, detail::idx_at<P1, I0, I1, I2, I3>::value> \
        {                                                                                                                      \
            return {this->m_data};                                                                                             \
        }
#    define EIRIN_TVEC_SWIZZLE_IMPL_3(SRC, NAME, P0, P1, P2)                                                                                                              \
        constexpr auto NAME() noexcept                                                                                                                                    \
            -> swizzle_proxy<N, T, C, detail::idx_at<P0, I0, I1, I2, I3>::value, detail::idx_at<P1, I0, I1, I2, I3>::value, detail::idx_at<P2, I0, I1, I2, I3>::value>    \
        {                                                                                                                                                                 \
            return {this->m_data};                                                                                                                                        \
        }                                                                                                                                                                 \
        constexpr auto NAME() const noexcept                                                                                                                              \
            -> swizzle_proxy<N, T, true, detail::idx_at<P0, I0, I1, I2, I3>::value, detail::idx_at<P1, I0, I1, I2, I3>::value, detail::idx_at<P2, I0, I1, I2, I3>::value> \
        {                                                                                                                                                                 \
            return {this->m_data};                                                                                                                                        \
        }
#    define EIRIN_TVEC_SWIZZLE_IMPL_4(SRC, NAME, P0, P1, P2, P3)                                                                                                                                                     \
        constexpr auto NAME() noexcept                                                                                                                                                                               \
            -> swizzle_proxy<N, T, C, detail::idx_at<P0, I0, I1, I2, I3>::value, detail::idx_at<P1, I0, I1, I2, I3>::value, detail::idx_at<P2, I0, I1, I2, I3>::value, detail::idx_at<P3, I0, I1, I2, I3>::value>    \
        {                                                                                                                                                                                                            \
            return {this->m_data};                                                                                                                                                                                   \
        }                                                                                                                                                                                                            \
        constexpr auto NAME() const noexcept                                                                                                                                                                         \
            -> swizzle_proxy<N, T, true, detail::idx_at<P0, I0, I1, I2, I3>::value, detail::idx_at<P1, I0, I1, I2, I3>::value, detail::idx_at<P2, I0, I1, I2, I3>::value, detail::idx_at<P3, I0, I1, I2, I3>::value> \
        {                                                                                                                                                                                                            \
            return {this->m_data};                                                                                                                                                                                   \
        }

template <std::size_t N, typename T, bool C, std::size_t I0, std::size_t I1, std::size_t I2, std::size_t I3>
class swizzle_proxy<N, T, C, I0, I1, I2, I3> : public swizzle_proxy_base<N, T, C, I0, I1, I2, I3>
{
    using base = swizzle_proxy_base<N, T, C, I0, I1, I2, I3>;

public:
    using base::base;
    EIRIN_SWIZZLE_PROXY_ASSIGN_DECLS
    EIRIN_TVEC_SWIZZLE4_4_MEMBERS_DECL(T, x, y, z, w)
    EIRIN_TVEC_SWIZZLE4_3_MEMBERS_DECL(T, x, y, z, w)
    EIRIN_TVEC_SWIZZLE4_2_MEMBERS_DECL(T, x, y, z, w)
};

#    undef EIRIN_TVEC_SWIZZLE_IMPL_2
#    undef EIRIN_TVEC_SWIZZLE_IMPL_3
#    undef EIRIN_TVEC_SWIZZLE_IMPL_4
#    undef EIRIN_SWIZZLE_PROXY_ASSIGN_DECLS

// ---- vector member function context: indices are literal component indices --
#    define EIRIN_TVEC_SWIZZLE_IMPL_2(SRC, NAME, P0, P1)         \
        EIRIN_ALWAYS_INLINE constexpr auto NAME() noexcept       \
            -> swizzle_proxy<SRC, T, false, P0, P1>              \
        {                                                        \
            return {this};                                       \
        }                                                        \
        EIRIN_ALWAYS_INLINE constexpr auto NAME() const noexcept \
            -> swizzle_proxy<SRC, T, true, P0, P1>               \
        {                                                        \
            return {this};                                       \
        }
#    define EIRIN_TVEC_SWIZZLE_IMPL_3(SRC, NAME, P0, P1, P2)     \
        EIRIN_ALWAYS_INLINE constexpr auto NAME() noexcept       \
            -> swizzle_proxy<SRC, T, false, P0, P1, P2>          \
        {                                                        \
            return {this};                                       \
        }                                                        \
        EIRIN_ALWAYS_INLINE constexpr auto NAME() const noexcept \
            -> swizzle_proxy<SRC, T, true, P0, P1, P2>           \
        {                                                        \
            return {this};                                       \
        }
#    define EIRIN_TVEC_SWIZZLE_IMPL_4(SRC, NAME, P0, P1, P2, P3) \
        EIRIN_ALWAYS_INLINE constexpr auto NAME() noexcept       \
            -> swizzle_proxy<SRC, T, false, P0, P1, P2, P3>      \
        {                                                        \
            return {this};                                       \
        }                                                        \
        EIRIN_ALWAYS_INLINE constexpr auto NAME() const noexcept \
            -> swizzle_proxy<SRC, T, true, P0, P1, P2, P3>       \
        {                                                        \
            return {this};                                       \
        }

// ---- free binary operators (materialize both sides) -----------------------
#    define EIRIN_SWIZZLE_PROXY_BINARY_OP(op)                                                                                                      \
        template <typename P1, typename P2>                                                                                                        \
        requires(detail::is_swizzle_proxy<P1> && detail::is_swizzle_proxy<P2> && detail::proxy_traits<P1>::size == detail::proxy_traits<P2>::size) \
        constexpr auto operator op(const P1& lhs, const P2& rhs) noexcept                                                                          \
            -> decltype(static_cast<tvec<detail::proxy_traits<P1>::size, typename detail::proxy_traits<P1>::value_type>>(lhs)                      \
                            op static_cast<tvec<detail::proxy_traits<P2>::size, typename detail::proxy_traits<P2>::value_type>>(rhs))              \
        {                                                                                                                                          \
            using L = tvec<detail::proxy_traits<P1>::size, typename detail::proxy_traits<P1>::value_type>;                                         \
            using R = tvec<detail::proxy_traits<P2>::size, typename detail::proxy_traits<P2>::value_type>;                                         \
            return static_cast<L>(lhs) op static_cast<R>(rhs);                                                                                     \
        }                                                                                                                                          \
        template <typename U, typename P>                                                                                                          \
        requires(detail::is_swizzle_proxy<P> && !detail::is_swizzle_proxy<U>)                                                                      \
        constexpr auto operator op(const P& lhs, U rhs) noexcept                                                                                   \
            -> decltype(static_cast<tvec<detail::proxy_traits<P>::size, typename detail::proxy_traits<P>::value_type>>(lhs)                        \
                            op rhs)                                                                                                                \
        {                                                                                                                                          \
            using L = tvec<detail::proxy_traits<P>::size, typename detail::proxy_traits<P>::value_type>;                                           \
            return static_cast<L>(lhs) op rhs;                                                                                                     \
        }                                                                                                                                          \
        template <typename U, typename P>                                                                                                          \
        requires(detail::is_swizzle_proxy<P> && !detail::is_swizzle_proxy<U>)                                                                      \
        constexpr auto operator op(U lhs, const P& rhs) noexcept                                                                                   \
            -> decltype(lhs op static_cast<tvec<detail::proxy_traits<P>::size, typename detail::proxy_traits<P>::value_type>>(rhs))                \
        {                                                                                                                                          \
            using R = tvec<detail::proxy_traits<P>::size, typename detail::proxy_traits<P>::value_type>;                                           \
            return lhs op static_cast<R>(rhs);                                                                                                     \
        }

EIRIN_SWIZZLE_PROXY_BINARY_OP(+)
EIRIN_SWIZZLE_PROXY_BINARY_OP(-)
EIRIN_SWIZZLE_PROXY_BINARY_OP(*)
EIRIN_SWIZZLE_PROXY_BINARY_OP(/)
EIRIN_SWIZZLE_PROXY_BINARY_OP(%)
EIRIN_SWIZZLE_PROXY_BINARY_OP(^)
EIRIN_SWIZZLE_PROXY_BINARY_OP(&)
EIRIN_SWIZZLE_PROXY_BINARY_OP(|)
EIRIN_SWIZZLE_PROXY_BINARY_OP(<<)
EIRIN_SWIZZLE_PROXY_BINARY_OP(>>)

#    undef EIRIN_SWIZZLE_PROXY_BINARY_OP

#    define EIRIN_REQUIRES_NOT_SWIZZLE_PROXY requires(!detail::is_swizzle_proxy<U>)

} // namespace eirin

#else

#    define EIRIN_REQUIRES_NOT_SWIZZLE_PROXY

#endif

#endif
