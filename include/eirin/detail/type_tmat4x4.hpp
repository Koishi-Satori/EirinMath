#ifndef EIRIN_MATH_DETAIL_TYPE_TMAT4X4_HPP
#define EIRIN_MATH_DETAIL_TYPE_TMAT4X4_HPP

#pragma once

#include <cstddef>
#include <array>
#include <bit>
#include <type_traits>
#include "../macro.hpp"
#include "type_tmat.hpp"
#include "type_tvec4.hpp"
#include "vector_func.hpp"
#include "matrix_func.hpp"
#include "scalar_traits_impl.hpp"
#include "compute_vec_rel.hpp"

namespace eirin
{
/**
 * @brief 4x4 shape matrix, with column major order.
 * @note the internal storage is like:
 *                                     [0,0] [0,1] [0,2] [0,3]
 *                                     [1,0] [1,1] [1,2] [1,3]
 *                                     [2,0] [2,1] [2,2] [2,3]
 *                                     [3,0] [3,1] [3,2] [3,3]
 * 
 * @tparam T 
 */
template <typename T>
struct tmat<4, 4, T>
{
    using col_type = tvec<4, T>;
    using row_type = tvec<4, T>;
    using type = tmat<4, 4, T>;
    using transpose_type = tmat<4, 4, T>;
    using value_type = T;
    using is_tmat_type = std::true_type;

private:
    col_type m_value[4];

public:
    using size_type = std::size_t;

    [[nodiscard]]
    EIRIN_MATH_SMALL_FUNC_API static size_type size() noexcept
    {
        return 4;
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
              col_type{scalar,      0,      0,      0},
              col_type{     0, scalar,      0,      0},
              col_type{     0,      0, scalar,      0},
              col_type{     0,      0,      0, scalar}
    } {};

    constexpr tmat(const col_type& v1, const col_type& v2, const col_type& v3, const col_type& v4)
        : m_value{v1, v2, v3, v4} {};

    /* Convert Constructors */

    // clang-format off

    template <
        typename X1, typename Y1, typename Z1, typename W1,
        typename X2, typename Y2, typename Z2, typename W2,
        typename X3, typename Y3, typename Z3, typename W3,
        typename X4, typename Y4, typename Z4, typename W4>
    constexpr tmat(
        const X1& m00, const Y1& m10, const Z1& m20, const W1& m30,
        const X2& m01, const Y2& m11, const Z2& m21, const W2& m31,
        const X3& m02, const Y3& m12, const Z3& m22, const W3& m32,
        const X4& m03, const Y4& m13, const Z4& m23, const W4& m33
    ) : m_value{
        col_type{m00, m10, m20, m30},
        col_type{m01, m11, m21, m31},
        col_type{m02, m12, m22, m32},
        col_type{m03, m13, m23, m33}} {};

    // clang-format on

    template <typename V1T, typename V2T, typename V3T, typename V4T>
    constexpr tmat(const tvec<4, V1T>& v1, const tvec<4, V2T>& v2, const tvec<4, V3T>& v3, const tvec<4, V4T>& v4)
        : m_value{col_type{v1}, col_type{v2}, col_type{v3}, col_type{v4}} {};

    /* TODO: Matrix Constructors */

    template <typename U>
    constexpr tmat(const tmat<4, 4, U>& mat)
        : m_value{col_type(mat[0]), col_type(mat[1]), col_type(mat[2]), col_type(mat[3])} {};

    /* The smaller matrices are embedded into the top left corner, the remaining
       diagonal elements are 1 and every other element is 0. */
    template <typename U>
    constexpr tmat(const tmat<3, 3, U>& mat)
        : m_value{
              col_type(mat[0][0], mat[0][1], mat[0][2], T{0}),
              col_type(mat[1][0], mat[1][1], mat[1][2], T{0}),
              col_type(mat[2][0], mat[2][1], mat[2][2], T{0}),
              col_type(T{0}, T{0}, T{0}, T{1})
          } {};

    template <typename U>
    constexpr tmat(const tmat<2, 2, U>& mat)
        : m_value{
              col_type(mat[0][0], mat[0][1], T{0}, T{0}),
              col_type(mat[1][0], mat[1][1], T{0}, T{0}),
              col_type(T{0}, T{0}, T{1}, T{0}),
              col_type(T{0}, T{0}, T{0}, T{1})
          } {};

    /// Contiguous view of the column major elements, for SIMD loads and API
    /// interop (the layout `glUniformMatrix*fv(..., GL_FALSE, ...)` expects).
    /// The element layout is verified by the assertions in the unit tests; this
    /// is a runtime facility, do not use it on a temporary.
    constexpr inline T* data() noexcept
    {
        return this->m_value[0].data();
    }

    /// Contiguous view of the column major elements, read only overload.
    constexpr inline const T* data() const noexcept
    {
        return this->m_value[0].data();
    }

    /// Strictly conforming copy of the column major elements, usable in
    /// constant expressions as well.
    constexpr inline std::array<T, 16> to_array() const noexcept
    {
        return std::bit_cast<std::array<T, 16>>(*this);
    }

    /* Accesses */

    /*
        The accessors come in two behaviour: operator[](i), at(i) and
        element(i) take a column index and give a reference to a col_type;
        operator()(r, c), at(r, c) and element(r, c) take a row and a column
        index and give a reference to a value_type.

        For C++23 with multi-dim subscript, operator[](r, c) is also available.

        The operators[] check the index with an assertion in debug builds only and UB in runtime;
        at() throws std::out_of_range on an out of range index;
        element() wraps it around.
    */

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

#define EIRIN_MATRIX_FUNC_ACCESSES_IMPL(name, ...)                    \
    {                                                                 \
        return detail::__matrix_get_##name /**/ (*this, __VA_ARGS__); \
    }

    /**
     * @brief Get the i-th column of the matrix, bounds checked.
     * 
     * @param i the column index, a negative value is out of range.
     * @note If `i` is not in `[0, cols())` the function throws
     *       `std::out_of_range` (or calls `std::terminate()` with
     *       `EIRIN_NO_EXCEPTIONS`).
     * @return reference to the i-th column, i.e. a `col_type`.
     */
    EIRIN_MATH_SMALL_FUNC_API col_type& at(std::integral auto i) EIRIN_MATRIX_FUNC_ACCESSES_IMPL(at, i);

    /**
     * @brief Get the i-th column of the matrix, bounds checked.
     * 
     * @param i the column index, a negative value is out of range.
     * @note If `i` is not in `[0, cols())` the function throws
     *       `std::out_of_range` (or calls `std::terminate()` with
     *       `EIRIN_NO_EXCEPTIONS`).
     * @return const reference to the i-th column, i.e. a `col_type`.
     */
    EIRIN_MATH_SMALL_FUNC_API const col_type& at(std::integral auto i) const EIRIN_MATRIX_FUNC_ACCESSES_IMPL(at, i);

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
    EIRIN_MATH_SMALL_FUNC_API value_type& at(std::integral auto r, std::integral auto c) EIRIN_MATRIX_FUNC_ACCESSES_IMPL(at, r, c);

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
    EIRIN_MATH_SMALL_FUNC_API const value_type& at(std::integral auto r, std::integral auto c) const EIRIN_MATRIX_FUNC_ACCESSES_IMPL(at, r, c);

    /**
     * @brief Get the i-th column of the matrix, wrapping out of range indices.
     * 
     * @param i the column index, negative values count from the end, so
     *          `element(-1)` is the last column.
     * @note This function never throws, `i` is reduced modulo `cols()`.
     * @return reference to the wrapped column, i.e. a `col_type`.
     */
    EIRIN_MATH_SMALL_FUNC_API col_type& element(std::integral auto i) noexcept EIRIN_MATRIX_FUNC_ACCESSES_IMPL(element, i);

    /**
     * @brief Get the i-th column of the matrix, wrapping out of range indices.
     * 
     * @param i the column index, negative values count from the end, so
     *          `element(-1)` is the last column.
     * @note This function never throws, `i` is reduced modulo `cols()`.
     * @return const reference to the wrapped column, i.e. a `col_type`.
     */
    EIRIN_MATH_SMALL_FUNC_API const col_type& element(std::integral auto i) const noexcept EIRIN_MATRIX_FUNC_ACCESSES_IMPL(element, i);

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
    EIRIN_MATH_SMALL_FUNC_API value_type& element(std::integral auto r, std::integral auto c) noexcept EIRIN_MATRIX_FUNC_ACCESSES_IMPL(element, r, c);

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
    EIRIN_MATH_SMALL_FUNC_API const value_type& element(std::integral auto r, std::integral auto c) const noexcept EIRIN_MATRIX_FUNC_ACCESSES_IMPL(element, r, c);

#undef EIRIN_MATRIX_FUNC_ACCESSES_IMPL

    /* Unary Arithmetic Operators */

    EIRIN_MATH_FUNC_API type& operator=(const type& mat) noexcept = default;

    template <typename U>
    EIRIN_MATH_FUNC_API type& operator=(const tmat<4, 4, U>& mat) noexcept
    {
        this->m_value[0] = mat[0];
        this->m_value[1] = mat[1];
        this->m_value[2] = mat[2];
        this->m_value[3] = mat[3];
        return *this;
    }

    template <typename U>
    requires scalar_type<U>
    EIRIN_MATH_FUNC_API type& operator+=(U scalar) noexcept
    {
        this->m_value[0] += scalar;
        this->m_value[1] += scalar;
        this->m_value[2] += scalar;
        this->m_value[3] += scalar;
        return *this;
    }

    template <typename U>
    EIRIN_MATH_FUNC_API type& operator+=(const tmat<4, 4, U>& mat) noexcept
    {
        this->m_value[0] += mat[0];
        this->m_value[1] += mat[1];
        this->m_value[2] += mat[2];
        this->m_value[3] += mat[3];
        return *this;
    }

    template <typename U>
    requires scalar_type<U>
    EIRIN_MATH_FUNC_API type& operator-=(U scalar) noexcept
    {
        this->m_value[0] -= scalar;
        this->m_value[1] -= scalar;
        this->m_value[2] -= scalar;
        this->m_value[3] -= scalar;
        return *this;
    }

    template <typename U>
    EIRIN_MATH_FUNC_API type& operator-=(const tmat<4, 4, U>& mat) noexcept
    {
        this->m_value[0] -= mat[0];
        this->m_value[1] -= mat[1];
        this->m_value[2] -= mat[2];
        this->m_value[3] -= mat[3];
        return *this;
    }

    template <typename U>
    requires scalar_type<U> && std::same_as<U, T>
    EIRIN_MATH_FUNC_API type& operator*=(U scalar) noexcept
    {
        this->m_value[0] *= scalar;
        this->m_value[1] *= scalar;
        this->m_value[2] *= scalar;
        this->m_value[3] *= scalar;
        return *this;
    }

    template <typename U>
    requires std::same_as<U, T>
    EIRIN_MATH_FUNC_API type& operator*=(const tmat<4, 4, U>& mat) noexcept
    {
        return (*this = *this * mat);
    }

    template <typename U>
    requires scalar_type<U> && std::same_as<U, T>
    EIRIN_MATH_FUNC_API type& operator/=(U scalar) noexcept
    {
        this->m_value[0] /= scalar;
        this->m_value[1] /= scalar;
        this->m_value[2] /= scalar;
        this->m_value[3] /= scalar;
        return *this;
    }

    template <typename U>
    requires fractional_scalar<U> && std::same_as<U, T>
    EIRIN_MATH_FUNC_API type& operator/=(const tmat<4, 4, U>& mat) noexcept
    {
        return *this *= inverse(mat);
    }

    /* Increment and Decrement Operators */

    EIRIN_MATH_FUNC_API type& operator++() noexcept
    {
        ++this->m_value[0];
        ++this->m_value[1];
        ++this->m_value[2];
        ++this->m_value[3];
        return *this;
    }

    EIRIN_MATH_FUNC_API type& operator--() noexcept
    {
        --this->m_value[0];
        --this->m_value[1];
        --this->m_value[2];
        --this->m_value[3];
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

    EIRIN_ALWAYS_INLINE constexpr bool nearly_eq(const tmat<4, 4, T>& rhs) const noexcept
    {
        return this->m_value[0].nearly_eq(rhs[0]) && this->m_value[1].nearly_eq(rhs[1]) &&
               this->m_value[2].nearly_eq(rhs[2]) && this->m_value[3].nearly_eq(rhs[3]);
    }
};

/* Unary Constant Operators */

template <typename T>
EIRIN_MATH_FUNC_API tmat<4, 4, T> operator+(const tmat<4, 4, T>& mat) noexcept
{
    return mat;
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<4, 4, T> operator-(const tmat<4, 4, T>& mat) noexcept
{
    return tmat<4, 4, T>(-mat[0], -mat[1], -mat[2], -mat[3]);
}

/* Binary Arithmetic Operators */

template <typename T>
EIRIN_MATH_FUNC_API tmat<4, 4, T> operator+(const tmat<4, 4, T>& mat, T scalar) noexcept
{
    return tmat<4, 4, T>(mat[0] + scalar, mat[1] + scalar, mat[2] + scalar, mat[3] + scalar);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<4, 4, T> operator+(T scalar, const tmat<4, 4, T>& mat) noexcept
{
    return tmat<4, 4, T>(mat[0] + scalar, mat[1] + scalar, mat[2] + scalar, mat[3] + scalar);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<4, 4, T> operator+(const tmat<4, 4, T>& mat1, const tmat<4, 4, T>& mat2) noexcept
{
    return tmat<4, 4, T>(mat1[0] + mat2[0], mat1[1] + mat2[1], mat1[2] + mat2[2], mat1[3] + mat2[3]);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<4, 4, T> operator-(const tmat<4, 4, T>& mat, T scalar) noexcept
{
    return tmat<4, 4, T>(mat[0] - scalar, mat[1] - scalar, mat[2] - scalar, mat[3] - scalar);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<4, 4, T> operator-(T scalar, const tmat<4, 4, T>& mat) noexcept
{
    return tmat<4, 4, T>(scalar - mat[0], scalar - mat[1], scalar - mat[2], scalar - mat[3]);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<4, 4, T> operator-(const tmat<4, 4, T>& mat1, const tmat<4, 4, T>& mat2) noexcept
{
    return tmat<4, 4, T>(mat1[0] - mat2[0], mat1[1] - mat2[1], mat1[2] - mat2[2], mat1[3] - mat2[3]);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<4, 4, T> operator*(const tmat<4, 4, T>& mat, T scalar) noexcept
{
    return tmat<4, 4, T>(mat[0] * scalar, mat[1] * scalar, mat[2] * scalar, mat[3] * scalar);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<4, 4, T> operator*(T scalar, const tmat<4, 4, T>& mat) noexcept
{
    return tmat<4, 4, T>(mat[0] * scalar, mat[1] * scalar, mat[2] * scalar, mat[3] * scalar);
}

template <typename T>
EIRIN_MATH_FUNC_API typename tmat<4, 4, T>::col_type operator*(const tmat<4, 4, T>& mat, typename tmat<4, 4, T>::row_type const& vec) noexcept
{
    using col_type = tmat<4, 4, T>::col_type;
    const col_type m0(vec[0]);
    const col_type m1(vec[1]);
    const col_type mul0 = mat[0] * m0;
    const col_type mul1 = mat[1] * m1;
    const col_type add0 = mul0 + mul1;
    const col_type m2(vec[2]);
    const col_type m3(vec[3]);
    const col_type mul2 = mat[2] * m2;
    const col_type mul3 = mat[3] * m3;
    const col_type add1 = mul2 + mul3;
    return add0 + add1;
}

template <typename T>
EIRIN_MATH_FUNC_API typename tmat<4, 4, T>::row_type operator*(typename tmat<4, 4, T>::col_type const& vec, const tmat<4, 4, T>& mat) noexcept
{
    return typename tmat<4, 4, T>::row_type(dot(mat[0], vec), dot(mat[1], vec), dot(mat[2], vec), dot(mat[3], vec));
}

namespace detail
{
    // this might be optimized to SIMD by compiler.
    // TODO: implement SIMD version further.
    template <typename T>
    EIRIN_MATH_FUNC_API tmat<4, 4, T> __mat4x4mul4x4(const tmat<4, 4, T>& mat1, const tmat<4, 4, T>& mat2) noexcept
    {
        using col_type = typename tmat<4, 4, T>::col_type;
        const col_type& a0 = mat1[0];
        const col_type& a1 = mat1[1];
        const col_type& a2 = mat1[2];
        const col_type& a3 = mat1[3];
        const col_type& b0 = mat2[0];
        const col_type& b1 = mat2[1];
        const col_type& b2 = mat2[2];
        const col_type& b3 = mat2[3];

        col_type tmp0 = a0 * b0.x;
        tmp0 += a1 * b0.y;
        tmp0 += a2 * b0.z;
        tmp0 += a3 * b0.w;
        col_type tmp1 = a0 * b1.x;
        tmp1 += a1 * b1.y;
        tmp1 += a2 * b1.z;
        tmp1 += a3 * b1.w;
        col_type tmp2 = a0 * b2.x;
        tmp2 += a1 * b2.y;
        tmp2 += a2 * b2.z;
        tmp2 += a3 * b2.w;
        col_type tmp3 = a0 * b3.x;
        tmp3 += a1 * b3.y;
        tmp3 += a2 * b3.z;
        tmp3 += a3 * b3.w;

        return tmat<4, 4, T>(tmp0, tmp1, tmp2, tmp3);
    }
} // namespace detail

template <typename T>
EIRIN_MATH_FUNC_API tmat<4, 4, T> operator*(const tmat<4, 4, T>& mat1, const tmat<4, 4, T>& mat2) noexcept
{
    return detail::__mat4x4mul4x4(mat1, mat2);
}

// TODO: add multiply with other shape of matrix later. (4x4 * 2x4/3x4)

template <typename T>
EIRIN_MATH_FUNC_API tmat<4, 4, T> operator/(const tmat<4, 4, T>& mat, T scalar) noexcept
{
    return tmat<4, 4, T>(mat[0] / scalar, mat[1] / scalar, mat[2] / scalar, mat[3] / scalar);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<4, 4, T> operator/(T scalar, const tmat<4, 4, T>& mat) noexcept
{
    return tmat<4, 4, T>(scalar / mat[0], scalar / mat[1], scalar / mat[2], scalar / mat[3]);
}

template <typename T>
EIRIN_MATH_FUNC_API tmat<4, 4, T> operator/(const tmat<4, 4, T>& mat1, const tmat<4, 4, T>& mat2) noexcept
{
    tmat<4, 4, T> mat(mat1);
    return mat /= mat2;
}

/* Compare Operators */

template <typename T>
EIRIN_MATH_FUNC_API bool operator==(const tmat<4, 4, T>& lhs, const tmat<4, 4, T>& rhs) noexcept
{
    return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3];
}

template <typename T>
EIRIN_MATH_FUNC_API bool operator!=(const tmat<4, 4, T>& lhs, const tmat<4, 4, T>& rhs) noexcept
{
    return lhs[0] != rhs[0] || lhs[1] != rhs[1] || lhs[2] != rhs[2] || lhs[3] != rhs[3];
}

} // namespace eirin

#endif // EIRIN_MATH_DETAIL_TYPE_TMAT4X4_HPP
