#ifndef EIRIN_MATH_DETAIL_TYPE_TMAT2X2_HPP
#define EIRIN_MATH_DETAIL_TYPE_TMAT2X2_HPP

#pragma once

#include <cstddef>
#include <type_traits>
#include "../macro.hpp"
#include "type_tmat.hpp"
#include "type_tvec2.hpp"
#include "matrix_func.hpp"
#include "scalar_traits_impl.hpp"
#include "compute_vec_rel.hpp"

namespace eirin
{
/**
 * @brief 2x2 shape matrix, with column major order.
 * @note the internal storage is like:
 *                                     [0,0] [0,1]
 *                                     [1,0] [1,1]
 * 
 * @tparam T 
 */
template <typename T>
struct tmat<2, 2, T>
{
    using col_type = tvec<2, T>;
    using row_type = tvec<2, T>;
    using type = tmat<2, 2, T>;
    using transpose_type = tmat<2, 2, T>;
    using value_type = T;
    using is_tmat_type = std::true_type;

private:
    col_type m_value[2];

public:
    using size_type = std::size_t;

    [[nodiscard]]
    EIRIN_MATH_SMALL_FUNC_API static size_type size() noexcept
    {
        return 2;
    }

    [[nodiscard]]
    EIRIN_MATH_SMALL_FUNC_API static size_type rows() noexcept
    {
        return size();
    }

    [[nodiscard]]
    EIRIN_MATH_SMALL_FUNC_API static size_type cols() noexcept
    {
        return size();
    }

    /* Common Constructors */

    constexpr tmat() = default;

    explicit constexpr tmat(T scalar) noexcept
        : m_value{
              col_type{scalar,      0},
              col_type{     0, scalar}
    } {};

    constexpr tmat(T const& x1, T const& y1, T const& x2, T const& y2)
        : m_value{
              col_type{x1, y1},
              col_type{x2, y2}
    } {};

    constexpr tmat(const col_type& v1, const col_type& v2)
        : m_value{v1, v2} {};

    /* Convert Constructors */

    template <typename X1, typename Y1, typename X2, typename Y2>
    constexpr tmat(const X1& m00, const Y1& m10, const X2& m01, const Y2& m11)
        : m_value{
              col_type{static_cast<T>(m00), static_cast<T>(m10)},
              col_type{static_cast<T>(m01), static_cast<T>(m11)}
    } {};

    template <typename V1T, typename V2T>
    constexpr tmat(const tvec<2, V1T>& v1, const tvec<2, V2T>& v2)
        : m_value{static_cast<col_type>(v1), static_cast<col_type>(v2)} {};

    /* TODO: Matrix Constructors */

    template <typename U>
    constexpr tmat(const tmat<2, 2, U>& mat)
        : m_value{col_type(mat[0]), col_type(mat[1])} {};

    /* Accesses */

    /* The accessors come in two flavours.  operator[](i), at(i) and
       element(i) take a column index and give a reference to a col_type;
       operator()(r, c), at(r, c) and element(r, c) take a row and a column
       index and give a reference to a value_type.  The plain operators check
       the index with an assertion in debug builds only, at() throws
       std::out_of_range on an out of range index and element() wraps it
       around. */

    /**
     * @brief Get i-th col matrix component.
     * 
     * @param i the col index
     * @note any index out of matrix indexes range is UB.
     * @return i-th col matrix component.
     */
    EIRIN_MATH_SMALL_FUNC_API col_type& operator[](size_type i) noexcept
    {
        EIRIN_INDEX_LENGTH_ASSERT(i, this->size());
        return this->m_value[i];
    }

    /**
     * @brief Get i-th col matrix component.
     * 
     * @param i the col index
     * @note any index out of matrix indexes range is UB.
     * @return i-th col matrix component.
     */
    EIRIN_MATH_SMALL_FUNC_API const col_type& operator[](size_type i) const noexcept
    {
        EIRIN_INDEX_LENGTH_ASSERT(i, this->size());
        return this->m_value[i];
    }

    /**
     * @brief Get r-th row, c-col matrix element.
     * 
     * @param r the row index
     * @param c the col index
     * @note any index out of matrix indexes range is UB.
     * @return r-th row, c-col matrix element.
     */
    EIRIN_MATH_SMALL_FUNC_API value_type& operator()(size_type r, size_type c) noexcept
    {
        EIRIN_INDEX_LENGTH_ASSERT(r, this->rows());
        EIRIN_INDEX_LENGTH_ASSERT(c, this->cols());
        return this->m_value[c][r];
    }

    /**
     * @brief Get r-th row, c-col matrix element.
     * 
     * @param r the row index
     * @param c the col index
     * @note any index out of matrix indexes range is UB.
     * @return r-th row, c-col matrix element.
     */
    EIRIN_MATH_SMALL_FUNC_API const value_type& operator()(size_type r, size_type c) const noexcept
    {
        EIRIN_INDEX_LENGTH_ASSERT(r, this->rows());
        EIRIN_INDEX_LENGTH_ASSERT(c, this->cols());
        return this->m_value[c][r];
    }

#ifdef EIRIN_HAS_CXX_FEATURE_MULTIDIM_SUBSCRIPT

    /**
     * @brief Get r-th row, c-col matrix element.
     * 
     * @param r the row index
     * @param c the col index
     * @note any index out of matrix indexes range is UB.
     * @return r-th row, c-col matrix element.
     */
    EIRIN_MATH_SMALL_FUNC_API value_type& operator[](size_type r, size_type c) noexcept
    {
        EIRIN_INDEX_LENGTH_ASSERT(r, this->rows());
        EIRIN_INDEX_LENGTH_ASSERT(c, this->cols());
        return this->m_value[c][r];
    }

    /**
     * @brief Get r-th row, c-col matrix element.
     * 
     * @param r the row index
     * @param c the col index
     * @note any index out of matrix indexes range is UB.
     * @return r-th row, c-col matrix element.
     */
    EIRIN_MATH_SMALL_FUNC_API const value_type& operator[](size_type r, size_type c) const noexcept
    {
        EIRIN_INDEX_LENGTH_ASSERT(r, this->rows());
        EIRIN_INDEX_LENGTH_ASSERT(c, this->cols());
        return this->m_value[c][r];
    }

#endif

    /**
     * @brief Get the i-th column of the matrix, bounds checked.
     * 
     * @param i the column index, a negative value is out of range.
     * @note If `i` is not in `[0, cols())` the function throws
     *       `std::out_of_range` (or calls `std::terminate()` with
     *       `EIRIN_NO_EXCEPTIONS`).
     * @return reference to the i-th column, i.e. a `col_type`.
     */
    EIRIN_MATH_SMALL_FUNC_API col_type& at(std::integral auto i)
    {
        return detail::__matrix_get_at(*this, i);
    }

    /**
     * @brief Get the i-th column of the matrix, bounds checked.
     * 
     * @param i the column index, a negative value is out of range.
     * @note If `i` is not in `[0, cols())` the function throws
     *       `std::out_of_range` (or calls `std::terminate()` with
     *       `EIRIN_NO_EXCEPTIONS`).
     * @return const reference to the i-th column, i.e. a `col_type`.
     */
    EIRIN_MATH_SMALL_FUNC_API const col_type& at(std::integral auto i) const
    {
        return detail::__matrix_get_at(*this, i);
    }

    /**
     * @brief Get the element at row `r` and column `c`, bounds checked.
     * 
     * @param r the row index, a negative value is out of range.
     * @param c the column index, a negative value is out of range.
     * @note If `r` is not in `[0, rows())` or `c` is not in `[0, cols())` the
     *       function throws `std::out_of_range` (or calls `std::terminate()`
     *       with `EIRIN_NO_EXCEPTIONS`).
     * @return reference to the element, i.e. a `value_type`.
     */
    EIRIN_MATH_SMALL_FUNC_API value_type& at(std::integral auto r, std::integral auto c)
    {
        return detail::__matrix_get_at(*this, r, c);
    }

    /**
     * @brief Get the element at row `r` and column `c`, bounds checked.
     * 
     * @param r the row index, a negative value is out of range.
     * @param c the column index, a negative value is out of range.
     * @note If `r` is not in `[0, rows())` or `c` is not in `[0, cols())` the
     *       function throws `std::out_of_range` (or calls `std::terminate()`
     *       with `EIRIN_NO_EXCEPTIONS`).
     * @return const reference to the element, i.e. a `value_type`.
     */
    EIRIN_MATH_SMALL_FUNC_API const value_type& at(std::integral auto r, std::integral auto c) const
    {
        return detail::__matrix_get_at(*this, r, c);
    }

    /**
     * @brief Get the i-th column of the matrix, wrapping out of range indices.
     * 
     * @param i the column index, negative values count from the end, so
     *          `element(-1)` is the last column.
     * @note This function never throws, `i` is reduced modulo `cols()`.
     * @return reference to the wrapped column, i.e. a `col_type`.
     */
    EIRIN_MATH_SMALL_FUNC_API col_type& element(std::integral auto i) noexcept
    {
        return detail::__matrix_get_element(*this, i);
    }

    /**
     * @brief Get the i-th column of the matrix, wrapping out of range indices.
     * 
     * @param i the column index, negative values count from the end, so
     *          `element(-1)` is the last column.
     * @note This function never throws, `i` is reduced modulo `cols()`.
     * @return const reference to the wrapped column, i.e. a `col_type`.
     */
    EIRIN_MATH_SMALL_FUNC_API const col_type& element(std::integral auto i) const noexcept
    {
        return detail::__matrix_get_element(*this, i);
    }

    /**
     * @brief Get the element at row `r` and column `c`, wrapping out of range
     *        indices.
     * 
     * @param r the row index, reduced modulo `rows()`, negative values count
     *          from the end.
     * @param c the column index, reduced modulo `cols()`, negative values
     *          count from the end.
     * @note This function never throws.
     * @return reference to the wrapped element, i.e. a `value_type`.
     */
    EIRIN_MATH_SMALL_FUNC_API value_type& element(std::integral auto r, std::integral auto c) noexcept
    {
        return detail::__matrix_get_element(*this, r, c);
    }

    /**
     * @brief Get the element at row `r` and column `c`, wrapping out of range
     *        indices.
     * 
     * @param r the row index, reduced modulo `rows()`, negative values count
     *          from the end.
     * @param c the column index, reduced modulo `cols()`, negative values
     *          count from the end.
     * @note This function never throws.
     * @return const reference to the wrapped element, i.e. a `value_type`.
     */
    EIRIN_MATH_SMALL_FUNC_API const value_type& element(std::integral auto r, std::integral auto c) const noexcept
    {
        return detail::__matrix_get_element(*this, r, c);
    }

    /* Unary Arithmetic Operators */

    EIRIN_MATH_FUNC_API type& operator=(const type& mat) noexcept = default;

    template <typename U>
    EIRIN_MATH_FUNC_API type& operator=(const tmat<2, 2, U>& mat) noexcept
    {
        this->m_value[0] = mat[0];
        this->m_value[1] = mat[1];
        return *this;
    }

    template <typename U>
    requires scalar_type<U>
    EIRIN_MATH_FUNC_API type& operator+=(U scalar) noexcept
    {
        this->m_value[0] += scalar;
        this->m_value[1] += scalar;
        return *this;
    }

    template <typename U>
    EIRIN_MATH_FUNC_API type& operator+=(const tmat<2, 2, U>& mat) noexcept
    {
        this->m_value[0] += mat[0];
        this->m_value[1] += mat[1];
        return *this;
    }

    template <typename U>
    requires scalar_type<U>
    EIRIN_MATH_FUNC_API type& operator-=(U scalar) noexcept
    {
        this->m_value[0] -= scalar;
        this->m_value[1] -= scalar;
        return *this;
    }

    template <typename U>
    EIRIN_MATH_FUNC_API type& operator-=(const tmat<2, 2, U>& mat) noexcept
    {
        this->m_value[0] -= mat[0];
        this->m_value[1] -= mat[1];
        return *this;
    }

    template <typename U>
    requires scalar_type<U> && std::same_as<U, T>
    EIRIN_MATH_FUNC_API type& operator*=(U scalar) noexcept
    {
        this->m_value[0] *= scalar;
        this->m_value[1] *= scalar;
        return *this;
    }

    template <typename U>
    requires std::same_as<U, T>
    EIRIN_MATH_FUNC_API type& operator*=(const tmat<2, 2, U>& mat) noexcept
    {
        return (*this = *this * mat);
    }

    template <typename U>
    requires scalar_type<U> && std::same_as<U, T>
    EIRIN_MATH_FUNC_API type& operator/=(U scalar) noexcept
    {
        this->m_value[0] /= scalar;
        this->m_value[1] /= scalar;
        return *this;
    }

    template <typename U>
    requires fractional_scalar<U> && std::same_as<U, T>
    EIRIN_MATH_FUNC_API type& operator/=(const tmat<2, 2, U>& mat) noexcept
    {
        return *this *= inverse(mat);
    }

    /* Increment and Decrement Operators */

    EIRIN_MATH_FUNC_API type& operator++() noexcept
    {
        ++this->m_value[0];
        ++this->m_value[1];
        return *this;
    }

    EIRIN_MATH_FUNC_API type& operator--() noexcept
    {
        --this->m_value[0];
        --this->m_value[1];
        return *this;
    }

    EIRIN_MATH_FUNC_API type operator++(int) noexcept
    {
        type result(*this);
        ++*this;
        return result;
    }

    EIRIN_MATH_FUNC_API type operator--(int) noexcept
    {
        type result(*this);
        --*this;
        return result;
    }

    EIRIN_ALWAYS_INLINE constexpr bool nearly_eq(const tmat<2, 2, T>& rhs) const noexcept
    {
        return this->m_value[0].nearly_eq(rhs[0]) && this->m_value[1].nearly_eq(rhs[1]);
    }
};

/* Unary Constant Operators */

template <typename T>
EIRIN_MATH_FUNC_API tmat<2, 2, T> operator+(const tmat<2, 2, T>& mat) noexcept
{
    return mat;
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<2, 2, T> operator-(const tmat<2, 2, T>& mat) noexcept
{
    return tmat<2, 2, T>(-mat[0], -mat[1]);
}

/* Binary Arithmetic Operators */

template <typename T>
EIRIN_MATH_FUNC_API tmat<2, 2, T> operator+(const tmat<2, 2, T>& mat, T scalar) noexcept
{
    return tmat<2, 2, T>(mat[0] + scalar, mat[1] + scalar);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<2, 2, T> operator+(T scalar, const tmat<2, 2, T>& mat) noexcept
{
    return tmat<2, 2, T>(mat[0] + scalar, mat[1] + scalar);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<2, 2, T> operator+(const tmat<2, 2, T>& mat1, const tmat<2, 2, T>& mat2) noexcept
{
    return tmat<2, 2, T>(mat1[0] + mat2[0], mat1[1] + mat2[1]);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<2, 2, T> operator-(const tmat<2, 2, T>& mat, T scalar) noexcept
{
    return tmat<2, 2, T>(mat[0] - scalar, mat[1] - scalar);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<2, 2, T> operator-(T scalar, const tmat<2, 2, T>& mat) noexcept
{
    return tmat<2, 2, T>(scalar - mat[0], scalar - mat[1]);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<2, 2, T> operator-(const tmat<2, 2, T>& mat1, const tmat<2, 2, T>& mat2) noexcept
{
    return tmat<2, 2, T>(mat1[0] - mat2[0], mat1[1] - mat2[1]);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<2, 2, T> operator*(const tmat<2, 2, T>& mat, T scalar) noexcept
{
    return tmat<2, 2, T>(mat[0] * scalar, mat[1] * scalar);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<2, 2, T> operator*(T scalar, const tmat<2, 2, T>& mat) noexcept
{
    return tmat<2, 2, T>(mat[0] * scalar, mat[1] * scalar);
}

template <typename T>
EIRIN_MATH_FUNC_API typename tmat<2, 2, T>::col_type operator*(const tmat<2, 2, T>& mat, typename tmat<2, 2, T>::row_type const vec) noexcept
{
    return tvec<2, T>(mat[0][0] * vec.x + mat[1][0] * vec.y, mat[0][1] * vec.x + mat[1][1] * vec.y);
}

template <typename T>
EIRIN_MATH_FUNC_API typename tmat<2, 2, T>::row_type operator*(typename tmat<2, 2, T>::col_type const vec, const tmat<2, 2, T>& mat) noexcept
{
    return tvec<2, T>(vec.x * mat[0][0] + vec.y * mat[0][1], vec.x * mat[1][0] + vec.y * mat[1][1]);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<2, 2, T> operator*(const tmat<2, 2, T>& mat1, const tmat<2, 2, T>& mat2) noexcept
{
    return tmat<2, 2, T>(
        mat1[0][0] * mat2[0][0] + mat1[1][0] * mat2[0][1],
        mat1[0][1] * mat2[0][0] + mat1[1][1] * mat2[0][1],
        mat1[0][0] * mat2[1][0] + mat1[1][0] * mat2[1][1],
        mat1[0][1] * mat2[1][0] + mat1[1][1] * mat2[1][1]
    );
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<2, 2, T> operator/(const tmat<2, 2, T>& mat, T scalar) noexcept
{
    return tmat<2, 2, T>(mat[0] / scalar, mat[1] / scalar);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<2, 2, T> operator/(T scalar, const tmat<2, 2, T>& mat) noexcept
{
    return tmat<2, 2, T>(scalar / mat[0], scalar / mat[1]);
}

// TODO: add multiply with other shape of matrix later.(2x2 * 3x2, 2x2 * 4x2)

template <typename T>
EIRIN_MATH_FUNC_API tmat<2, 2, T> operator/(const tmat<2, 2, T>& mat1, const tmat<2, 2, T>& mat2) noexcept
{
    tmat<2, 2, T> mat(mat1);
    return mat /= mat2;
}

/* Compare Operators */

template <typename T>
EIRIN_MATH_FUNC_API bool operator==(const tmat<2, 2, T>& lhs, const tmat<2, 2, T>& rhs) noexcept
{
    return lhs[0] == rhs[0] && lhs[1] == rhs[1];
}

template <typename T>
EIRIN_MATH_FUNC_API bool operator!=(const tmat<2, 2, T>& lhs, const tmat<2, 2, T>& rhs) noexcept
{
    return lhs[0] != rhs[0] || lhs[1] != rhs[1];
}

} // namespace eirin

#endif
