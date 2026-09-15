#ifndef EIRIN_MATH_DETAIL_TYPE_TMAT_HPP
#define EIRIN_MATH_DETAIL_TYPE_TMAT_HPP

#pragma once

#include <cstddef>
#include <concepts>
#include <stdexcept>
#include "../macro.hpp"
#include "../error.hpp"

namespace eirin
{
namespace detail
{
    template <typename T, typename = void>
    struct is_matrix_type : std::false_type
    {};

    template <typename T>
    struct is_matrix_type<T, std::void_t<typename std::remove_cvref_t<T>::is_tmat_type>> : std::true_type
    {};

    template <typename T>
    inline constexpr bool is_matrix_type_v = is_matrix_type<T>::value;

    template <typename Matrix>
    requires is_matrix_type_v<Matrix>
    struct matrix_trait
    {
        using col_type = typename Matrix::col_type;
        using row_type = typename Matrix::row_type;
        using type = typename Matrix::type;
        using transpose_type = typename Matrix::transpose_type;
        using value_type = typename Matrix::value_type;
        using size_type = typename Matrix::size_type;
        using is_tmat_type = typename Matrix::is_tmat_type;

        inline constexpr static size_type size = Matrix::size();
        inline constexpr static size_type cols = Matrix::cols();
        inline constexpr static size_type rows = Matrix::rows();
    };

    template <std::size_t C, std::size_t R>
    concept matrix_shape = C > 0 && C <= 4 && R > 0 && R <= 4;

    template <typename size_type, size_type N>
    [[nodiscard]]
    EIRIN_ALWAYS_INLINE constexpr size_type __wrap_matrix_element_index(std::integral auto i) noexcept
    {
        if constexpr(std::is_signed_v<decltype(i)>)
        {
            using signed_size = std::make_signed_t<size_type>;
            constexpr auto len = static_cast<signed_size>(N);
            const auto rem = static_cast<signed_size>(i) % len;
            return static_cast<size_type>(rem < 0 ? rem + len : rem);
        }
        else
        {
            return static_cast<size_type>(i) % N;
        }
    }
} // namespace detail

template <typename T>
concept matrix_type = detail::is_matrix_type_v<T>;

template <std::size_t C, std::size_t R, typename T>
requires detail::matrix_shape<C, R>
struct tmat;

namespace detail
{
    template <std::size_t C, std::size_t R, typename T>
    EIRIN_ALWAYS_INLINE constexpr decltype(auto) __matrix_get_at(tmat<C, R, T>& mat, std::integral auto i)
    {
        using size_type = typename tmat<C, R, T>::size_type;
        constexpr size_type size = tmat<C, R, T>::size();

        size_type index = static_cast<size_type>(i);
        if(index >= size)
        {
            EIRIN_THROW_EXCEPTION(std::out_of_range, "mat index out of range.");
        }
        return mat[index];
    }

    template <std::size_t C, std::size_t R, typename T>
    EIRIN_ALWAYS_INLINE constexpr decltype(auto) __matrix_get_at(const tmat<C, R, T>& mat, std::integral auto i)
    {
        using size_type = typename tmat<C, R, T>::size_type;
        constexpr size_type size = tmat<C, R, T>::size();

        size_type index = static_cast<size_type>(i);
        if(index >= size)
        {
            EIRIN_THROW_EXCEPTION(std::out_of_range, "mat index out of range.");
        }
        return mat[index];
    }

    template <std::size_t C, std::size_t R, typename T>
    EIRIN_ALWAYS_INLINE constexpr decltype(auto) __matrix_get_at(tmat<C, R, T>& mat, std::integral auto r, std::integral auto c)
    {
        using size_type = typename tmat<C, R, T>::size_type;
        constexpr size_type rows = tmat<C, R, T>::rows(), cols = tmat<C, R, T>::cols();

        size_type r_index = static_cast<size_type>(r), c_index = static_cast<size_type>(c);
        if(r_index >= rows || c_index >= cols)
        {
            EIRIN_THROW_EXCEPTION(std::out_of_range, "mat index out of range.");
        }
        return mat[c_index][r_index];
    }

    template <std::size_t C, std::size_t R, typename T>
    EIRIN_ALWAYS_INLINE constexpr decltype(auto) __matrix_get_at(const tmat<C, R, T>& mat, std::integral auto r, std::integral auto c)
    {
        using size_type = typename tmat<C, R, T>::size_type;
        constexpr size_type rows = tmat<C, R, T>::rows(), cols = tmat<C, R, T>::cols();

        size_type r_index = static_cast<size_type>(r), c_index = static_cast<size_type>(c);
        if(r_index >= rows || c_index >= cols)
        {
            EIRIN_THROW_EXCEPTION(std::out_of_range, "mat index out of range.");
        }
        return mat[c_index][r_index];
    }

    template <std::size_t C, std::size_t R, typename T>
    EIRIN_ALWAYS_INLINE constexpr decltype(auto) __matrix_get_element(tmat<C, R, T>& mat, std::integral auto i)
    {
        using size_type = typename tmat<C, R, T>::size_type;
        constexpr size_type size = tmat<C, R, T>::size();
        return mat[__wrap_matrix_element_index<size_type, size>(i)];
    }

    template <std::size_t C, std::size_t R, typename T>
    EIRIN_ALWAYS_INLINE constexpr decltype(auto) __matrix_get_element(const tmat<C, R, T>& mat, std::integral auto i)
    {
        using size_type = typename tmat<C, R, T>::size_type;
        constexpr size_type size = tmat<C, R, T>::size();
        return mat[__wrap_matrix_element_index<size_type, size>(i)];
    }

    template <std::size_t C, std::size_t R, typename T>
    EIRIN_ALWAYS_INLINE constexpr decltype(auto) __matrix_get_element(tmat<C, R, T>& mat, std::integral auto r, std::integral auto c)
    {
        using size_type = typename tmat<C, R, T>::size_type;
        constexpr size_type rows = tmat<C, R, T>::rows(), cols = tmat<C, R, T>::cols();

        size_type r_index = __wrap_matrix_element_index<size_type, rows>(r);
        size_type c_index = __wrap_matrix_element_index<size_type, cols>(c);
        return mat[c_index][r_index];
    }

    template <std::size_t C, std::size_t R, typename T>
    EIRIN_ALWAYS_INLINE constexpr decltype(auto) __matrix_get_element(const tmat<C, R, T>& mat, std::integral auto r, std::integral auto c)
    {
        using size_type = typename tmat<C, R, T>::size_type;
        constexpr size_type rows = tmat<C, R, T>::rows(), cols = tmat<C, R, T>::cols();

        size_type r_index = __wrap_matrix_element_index<size_type, rows>(r);
        size_type c_index = __wrap_matrix_element_index<size_type, cols>(c);
        return mat[c_index][r_index];
    }

} // namespace detail
} // namespace eirin

#endif
