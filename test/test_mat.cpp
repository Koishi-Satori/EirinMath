#include <gtest/gtest.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <eirin/eirin.hpp>
#include <eirin/io/format.hpp>
#include "compile_check.hpp"

using namespace eirin;
using namespace eirin::literals;

namespace
{
template <typename T>
::testing::AssertionResult mat_nearly_eq(
    const tmat<2, 2, T>& a, const tmat<2, 2, T>& b, T eps = T{1e-5}
)
{
    using std::abs;
    for(std::size_t c = 0; c < 2; ++c)
    {
        for(std::size_t r = 0; r < 2; ++r)
        {
            if(abs(a(r, c) - b(r, c)) > eps)
                return ::testing::AssertionFailure()
                       << "a(" << r << ", " << c << ") = " << a(r, c)
                       << ", b(" << r << ", " << c << ") = " << b(r, c);
        }
    }
    return ::testing::AssertionSuccess();
}

/// Compile-time probes: the operands are only inspected through decltype.
template <typename M>
inline constexpr bool has_inverse_v = requires(M m) { inverse(m); };

template <typename M>
inline constexpr bool has_transpose_v = requires(M m) { transpose(m); };

template <typename M>
inline constexpr bool has_determinant_v = requires(M m) { determinant(m); };
} // namespace

// ==================== Construction ====================
TEST(Mat, Constructors)
{
    // Default construct does not initialize, `{}` value initializes to zero.
    mat2<int> zero{};
    EXPECT_EQ(zero(0, 0), 0);
    EXPECT_EQ(zero(0, 1), 0);
    EXPECT_EQ(zero(1, 0), 0);
    EXPECT_EQ(zero(1, 1), 0);

    // Construct from a scalar: GLSL `mat(2.0)` semantics, diagonal matrix.
    mat2<int> diag(2);
    EXPECT_EQ(diag(0, 0), 2);
    EXPECT_EQ(diag(1, 1), 2);
    EXPECT_EQ(diag(0, 1), 0);
    EXPECT_EQ(diag(1, 0), 0);

    // Construct from 4 scalars, column major: first column (1, 2), second (3, 4).
    mat2<int> cols(1, 2, 3, 4);
    EXPECT_EQ(cols(0, 0), 1);
    EXPECT_EQ(cols(1, 0), 2);
    EXPECT_EQ(cols(0, 1), 3);
    EXPECT_EQ(cols(1, 1), 4);
    EXPECT_EQ(cols[0], (tvec<2, int>(1, 2)));
    EXPECT_EQ(cols[1], (tvec<2, int>(3, 4)));

    // Construct from two columns, including mixed component types.
    mat2<int> from_cols(tvec<2, int>(1, 2), tvec<2, int>(3, 4));
    EXPECT_EQ(from_cols, cols);
    mat2<int> mixed_cols(tvec<2, double>(1.0, 2.0), tvec<2, float>(3.0f, 4.0f));
    EXPECT_EQ(mixed_cols, cols);

    // Construct from mixed scalar types.
    mat2<float> mixed(1, 2.5, 3u, 4.0f);
    EXPECT_FLOAT_EQ(mixed(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(mixed(1, 0), 2.5f);
    EXPECT_FLOAT_EQ(mixed(0, 1), 3.0f);
    EXPECT_FLOAT_EQ(mixed(1, 1), 4.0f);

    // Conversion construct and conversion assignment, both element wise.
    mat2<double> conv(cols);
    EXPECT_DOUBLE_EQ(conv(1, 0), 2.0);
    mat2<int> assigned{};
    assigned = cols;
    EXPECT_EQ(assigned, cols);
    assigned = mat2<double>(1.9, 2.9, 3.9, 4.9);
    EXPECT_EQ(assigned, mat2<int>(1, 2, 3, 4));

    // Fixed point matrix, same rules.
    mat2<fixed32> fdiag(3_f32);
    EXPECT_EQ(fdiag(0, 0), 3_f32);
    EXPECT_EQ(fdiag(0, 1), 0_f32);
    mat2<fixed32> fcols(1_f32, 2_f32, 3_f32, 4_f32);
    EXPECT_EQ(fcols(1, 0), 2_f32);
    EXPECT_EQ(fcols[1], vec2<fixed32>(3_f32, 4_f32));
}

// ==================== Type Traits and Aliases ====================
TEST(Mat, TypeTraits)
{
    static_assert(std::is_same_v<mat2<int>, tmat<2, 2, int>>);
    static_assert(std::is_same_v<mat2x2<int>, tmat<2, 2, int>>);
    static_assert(std::is_same_v<mat2i, tmat<2, 2, int>>);
    static_assert(std::is_same_v<mat2u, tmat<2, 2, unsigned int>>);
    static_assert(std::is_same_v<mat2x2i, tmat<2, 2, int>>);
    static_assert(std::is_same_v<mat2i32, tmat<2, 2, std::int32_t>>);
    static_assert(std::is_same_v<mat2u64, tmat<2, 2, std::uint64_t>>);
    static_assert(std::is_same_v<mat2f, tmat<2, 2, float>>);
    static_assert(std::is_same_v<mat2x2d, tmat<2, 2, double>>);
    static_assert(std::is_same_v<mat2fixed32, tmat<2, 2, fixed32>>);
    // fixed: `long` is 4 bytes on Windows (and wasm32), replace with int64_t
    static_assert(std::is_same_v<mat2fixed<int, std::int64_t, 16>, tmat<2, 2, fixed_num<int, std::int64_t, 16, false>>>);
    static_assert(std::is_same_v<mat3<int>, tmat<3, 3, int>>);
    static_assert(std::is_same_v<mat3x3<int>, tmat<3, 3, int>>);
    static_assert(std::is_same_v<mat3i, tmat<3, 3, int>>);
    static_assert(std::is_same_v<mat3x3u, tmat<3, 3, unsigned int>>);
    static_assert(std::is_same_v<mat3i32, tmat<3, 3, std::int32_t>>);
    static_assert(std::is_same_v<mat3u64, tmat<3, 3, std::uint64_t>>);
    static_assert(std::is_same_v<mat3f, tmat<3, 3, float>>);
    static_assert(std::is_same_v<mat3x3d, tmat<3, 3, double>>);
    static_assert(std::is_same_v<mat3fixed32, tmat<3, 3, fixed32>>);
    static_assert(std::is_same_v<mat3fixed<int, std::int64_t, 16>, tmat<3, 3, fixed_num<int, std::int64_t, 16, false>>>);
    static_assert(std::is_same_v<mat3<int>::col_type, tvec<3, int>>);
    static_assert(std::is_same_v<mat3<int>::transpose_type, tmat<3, 3, int>>);
    static_assert(std::is_same_v<mat4<int>, tmat<4, 4, int>>);
    static_assert(std::is_same_v<mat4x4<int>, tmat<4, 4, int>>);
    static_assert(std::is_same_v<mat4i, tmat<4, 4, int>>);
    static_assert(std::is_same_v<mat4x4u, tmat<4, 4, unsigned int>>);
    static_assert(std::is_same_v<mat4i32, tmat<4, 4, std::int32_t>>);
    static_assert(std::is_same_v<mat4u64, tmat<4, 4, std::uint64_t>>);
    static_assert(std::is_same_v<mat4f, tmat<4, 4, float>>);
    static_assert(std::is_same_v<mat4x4d, tmat<4, 4, double>>);
    static_assert(std::is_same_v<mat4fixed32, tmat<4, 4, fixed32>>);
    static_assert(std::is_same_v<mat4fixed<int, std::int64_t, 16>, tmat<4, 4, fixed_num<int, std::int64_t, 16, false>>>);
    static_assert(std::is_same_v<mat4<int>::col_type, tvec<4, int>>);
    static_assert(std::is_same_v<mat4<int>::transpose_type, tmat<4, 4, int>>);

    static_assert(std::is_same_v<mat2<int>::value_type, int>);
    static_assert(std::is_same_v<mat2<int>::col_type, tvec<2, int>>);
    static_assert(std::is_same_v<mat2<int>::row_type, tvec<2, int>>);
    static_assert(std::is_same_v<mat2<int>::transpose_type, tmat<2, 2, int>>);
    static_assert(std::is_same_v<mat2<int>::type, tmat<2, 2, int>>);
    static_assert(std::is_same_v<mat2<int>::is_tmat_type, std::true_type>);
    static_assert(matrix_type<mat2<int>>);
    static_assert(!matrix_type<tvec<2, int>>);
    static_assert(vector_type<tvec<2, int>>);
    static_assert(scalar_type<int>);
    static_assert(scalar_type<float>);
    static_assert(scalar_type<fixed32>);
    static_assert(!scalar_type<mat2<int>>);
    static_assert(!scalar_type<tvec<2, int>>);
    static_assert(fractional_scalar<float>);
    static_assert(fractional_scalar<fixed32>);
    static_assert(!fractional_scalar<int>);

    // Layout: a plain column array, no padding, trivially copyable.
    static_assert(sizeof(mat2<int>) == 4 * sizeof(int));
    static_assert(sizeof(mat2<float>) == 16);
    static_assert(std::is_trivially_copyable_v<mat2<int>>);
    static_assert(std::is_trivially_copy_constructible_v<mat2<int>>);
    static_assert(std::is_nothrow_copy_constructible_v<mat2<int>>);
    static_assert(std::is_standard_layout_v<mat2<int>>);
    static_assert(std::is_trivially_destructible_v<mat2<fixed32>>);

    EXPECT_EQ(mat2<int>::size(), 2u);
    EXPECT_EQ(mat2<int>::rows(), 2u);
    EXPECT_EQ(mat2<int>::cols(), 2u);
    EXPECT_EQ(mat3<int>::size(), 3u);
    EXPECT_EQ(mat3<int>::rows(), 3u);
    EXPECT_EQ(mat3<int>::cols(), 3u);
    EXPECT_EQ(mat4<int>::size(), 4u);
    EXPECT_EQ(mat4<int>::rows(), 4u);
    EXPECT_EQ(mat4<int>::cols(), 4u);
}

// ==================== Element Access ====================
TEST(Mat, ElementAccess)
{
    mat2<int> m(1, 2, 3, 4); // column major: col0 = (1, 2), col1 = (3, 4)

    // operator[] gives a column, operator() gives an element.
    EXPECT_EQ(m[0][0], 1);
    EXPECT_EQ(m[0][1], 2);
    EXPECT_EQ(m[1][0], 3);
    EXPECT_EQ(m[1][1], 4);
    EXPECT_EQ(m(0, 0), 1);
    EXPECT_EQ(m(1, 0), 2);
    EXPECT_EQ(m(0, 1), 3);
    EXPECT_EQ(m(1, 1), 4);

    // m[col][row] and m(row, col) are the same element.
    EXPECT_EQ(m[1][0], m(0, 1));
    EXPECT_EQ(m[0][1], m(1, 0));

    m[1][0] = 30;
    EXPECT_EQ(m(0, 1), 30);
    m(1, 0) = 20;
    EXPECT_EQ(m[0][1], 20);

    const mat2<int> cm(1, 2, 3, 4);
    EXPECT_EQ(cm[1][1], 4);
    EXPECT_EQ(cm(1, 1), 4);

#ifdef EIRIN_HAS_CXX_FEATURE_MULTIDIM_SUBSCRIPT
    // Note: `m[r, c]` needs its own parentheses inside gtest macros, the
    // preprocessor splits the comma of the subscript otherwise.
    EXPECT_EQ((m[0, 1]), m(0, 1));
    EXPECT_EQ((m[1, 0]), m(1, 0));
    m[1, 1] = 40;
    EXPECT_EQ(m(1, 1), 40);
#endif
}

// ==================== Checked Access (at / element) ====================
TEST(Mat, CheckedAccess)
{
    mat2<int> m(1, 2, 3, 4); // col0 = (1, 2), col1 = (3, 4)
    const mat2<int> cm(1, 2, 3, 4);

    // at() is bounds checked: index in [0, rows()) / [0, cols()).
    EXPECT_EQ(m.at(0)[0], 1);
    EXPECT_EQ(m.at(1)[1], 4);
    EXPECT_EQ(m.at(0, 1), 3);
    EXPECT_EQ(m.at(1, 0), 2);
    EXPECT_EQ(cm.at(1)[0], 3);
    EXPECT_EQ(cm.at(0, 1), 3);
    m.at(1)[0] = 30;
    EXPECT_EQ(m(0, 1), 30);
    m.at(1, 0) = 20;
    EXPECT_EQ(m(1, 0), 20);
#ifndef EIRIN_NO_EXCEPTIONS
    EXPECT_THROW((void)m.at(2), std::out_of_range);
    EXPECT_THROW((void)cm.at(2), std::out_of_range);
    EXPECT_THROW((void)m.at(0, 2), std::out_of_range);
    EXPECT_THROW((void)m.at(2, 0), std::out_of_range);
    EXPECT_THROW((void)m.at(-1, -1), std::out_of_range);
#endif

    // element() wraps: negative counts from the end, out of range wraps around.
    EXPECT_EQ(m.element(1)[1], 4);
    EXPECT_EQ(m.element(-1)[0], 30);
    EXPECT_EQ(m.element(-1, -1), 4);
    EXPECT_EQ(m.element(2, 2), 1);
    EXPECT_EQ(cm.element(-1, 0), 2);
    m.element(-1, -1) = 44;
    EXPECT_EQ(m(1, 1), 44);
}

// ==================== Arithmetic ====================
TEST(Mat, Arithmetic)
{
    mat2<int> a(1, 2, 3, 4); // col0 = (1, 2), col1 = (3, 4)
    mat2<int> b(5, 6, 7, 8);

    EXPECT_EQ(a + b, mat2<int>(6, 8, 10, 12));
    EXPECT_EQ(a - b, mat2<int>(-4, -4, -4, -4));
    EXPECT_EQ(+a, a);
    EXPECT_EQ(-a, mat2<int>(-1, -2, -3, -4));

    // Scalar on both sides (an extension over GLSL, consistent with tvec).
    EXPECT_EQ(a + 1, mat2<int>(2, 3, 4, 5));
    EXPECT_EQ(1 + a, mat2<int>(2, 3, 4, 5));
    EXPECT_EQ(a - 1, mat2<int>(0, 1, 2, 3));
    EXPECT_EQ(10 - a, mat2<int>(9, 8, 7, 6));
    EXPECT_EQ(a * 2, mat2<int>(2, 4, 6, 8));
    EXPECT_EQ(2 * a, mat2<int>(2, 4, 6, 8));
    EXPECT_EQ(a / 2, mat2<int>(0, 1, 1, 2));
    EXPECT_EQ(100 / a, mat2<int>(100, 50, 33, 25));

    mat2<int> c(a);
    c += b;
    EXPECT_EQ(c, a + b);
    c = a;
    c -= b;
    EXPECT_EQ(c, a - b);
    c = a;
    c += 1;
    EXPECT_EQ(c, a + 1);
    c = a;
    c -= 1;
    EXPECT_EQ(c, a - 1);
    c = a;
    c *= 2;
    EXPECT_EQ(c, a * 2);
    c = a;
    c /= 2;
    EXPECT_EQ(c, a / 2);
    c = a;
    c *= b;
    EXPECT_EQ(c, a * b);

    c = a;
    EXPECT_EQ(++c, a + 1);
    c = a;
    EXPECT_EQ(c++, a);
    EXPECT_EQ(c, a + 1);
    c = a;
    EXPECT_EQ(--c, a - 1);
    c = a;
    EXPECT_EQ(c--, a);
    EXPECT_EQ(c, a - 1);
}

// ==================== Matrix and Vector Products ====================
TEST(Mat, Multiply)
{
    // a = [[1, 2], [3, 4]], b = [[5, 6], [7, 8]]
    mat2<int> a(1, 3, 2, 4);
    mat2<int> b(5, 7, 6, 8);

    // a * b = [[19, 22], [43, 50]], column major.
    EXPECT_EQ(a * b, mat2<int>(19, 43, 22, 50));
    EXPECT_NE(a * b, b * a);

    // Matrix * vector, vector * matrix.
    EXPECT_EQ((a * tvec<2, int>(1, 2)), (tvec<2, int>(5, 11)));
    EXPECT_EQ((tvec<2, int>(1, 2) * a), (tvec<2, int>(7, 10)));

    // Identity behaves as identity, on both sides.
    mat2<int> identity(1, 0, 0, 1);
    EXPECT_EQ(a * identity, a);
    EXPECT_EQ(identity * a, a);
    EXPECT_EQ((a * tvec<2, int>(1, 2)), ((a * identity) * tvec<2, int>(1, 2)));

    // Fixed point products are exact for these values.
    mat2<fixed32> fa(1_f32, 3_f32, 2_f32, 4_f32);
    mat2<fixed32> fb(5_f32, 7_f32, 6_f32, 8_f32);
    EXPECT_EQ(fa * fb, mat2<fixed32>(19_f32, 43_f32, 22_f32, 50_f32));
    EXPECT_EQ(fa * vec2<fixed32>(1_f32, 2_f32), vec2<fixed32>(5_f32, 11_f32));
    EXPECT_EQ(vec2<fixed32>(1_f32, 2_f32) * fa, vec2<fixed32>(7_f32, 10_f32));
}

// ==================== Comparison ====================
TEST(Mat, Compare)
{
    mat2<int> a(1, 2, 3, 4);
    mat2<int> b(1, 2, 3, 5);
    EXPECT_TRUE(a == a);
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a != a);

    mat2<float> f(1.f, 2.f, 3.f, 4.f);
    EXPECT_TRUE(f.nearly_eq(f));
    EXPECT_FALSE(f == mat2<float>(1.f + 1e-4f, 2.f, 3.f, 4.f));
    EXPECT_TRUE(f.nearly_eq(mat2<float>(1.f + 1e-7f, 2.f, 3.f, 4.f)));
    EXPECT_FALSE(f.nearly_eq(mat2<float>(1.f + 1e-4f, 2.f, 3.f, 4.f)));
    EXPECT_FALSE(f.nearly_eq(mat2<float>(1.f + 1e-1f, 2.f, 3.f, 4.f)));

    mat2<fixed32> fm(1_f32, 2_f32, 3_f32, 4_f32);
    EXPECT_TRUE(fm.nearly_eq(fm));
    EXPECT_FALSE(fm.nearly_eq(mat2<fixed32>(2_f32, 2_f32, 3_f32, 4_f32)));
}

// ==================== Transpose and Determinant ====================
TEST(Mat, TransposeDeterminant)
{
    // a = [[1, 2], [3, 4]], transpose = [[1, 3], [2, 4]].
    mat2<int> a(1, 3, 2, 4);
    EXPECT_EQ(transpose(a), mat2<int>(1, 2, 3, 4));
    EXPECT_EQ(transpose(transpose(a)), a);
    EXPECT_EQ(transpose(a)(0, 1), a(1, 0));
    EXPECT_EQ(transpose(a)(1, 0), a(0, 1));

    EXPECT_EQ(determinant(a), -2);
    EXPECT_EQ(determinant(transpose(a)), determinant(a));
    EXPECT_EQ(determinant(mat2<int>(1, 0, 0, 1)), 1);

    mat2<fixed32> fa(1_f32, 3_f32, 2_f32, 4_f32);
    EXPECT_EQ(determinant(fa), -2_f32);
    EXPECT_EQ(transpose(fa), mat2<fixed32>(1_f32, 2_f32, 3_f32, 4_f32));
}

// ==================== Inverse ====================
TEST(Mat, Inverse)
{
    // a = [[4, 2], [7, 6]], det = 10, inverse = [[0.6, -0.2], [-0.7, 0.4]].
    mat2<float> a(4.f, 7.f, 2.f, 6.f);
    EXPECT_FLOAT_EQ(determinant(a), 10.f);

    const mat2<float> identity(1.f, 0.f, 0.f, 1.f);
    mat2<float> inv = inverse(a);
    EXPECT_TRUE(mat_nearly_eq(a * inv, identity, 1e-5f));
    EXPECT_TRUE(mat_nearly_eq(inv * a, identity, 1e-5f));
    EXPECT_TRUE(mat_nearly_eq(inv, mat2<float>(0.6f, -0.7f, -0.2f, 0.4f), 1e-5f));

    // a / b is defined as a * inverse(b).
    mat2<float> b(1.f, 2.f, 3.f, 5.f);
    EXPECT_TRUE(mat_nearly_eq(a / b, a * inverse(b), 1e-5f));

    // Fixed point inverse is only accurate to the fixed point precision.
    mat2<fixed32> fa(4_f32, 7_f32, 2_f32, 6_f32);
    EXPECT_TRUE(mat_nearly_eq(fa * inverse(fa), mat2<fixed32>(1_f32, 0_f32, 0_f32, 1_f32), 0.01_f32));
}

// ==================== 3x3 ====================
TEST(Mat, ThreeByThree)
{
    using mat3 = tmat<3, 3, int>;

    // Columns (1, 2, 3), (0, 1, 4), (5, 6, 0): det = 1, so the inverse is the
    // cofactor matrix and stays exact for integers, floats and fixed point.
    mat3 a(tvec<3, int>(1, 2, 3), tvec<3, int>(0, 1, 4), tvec<3, int>(5, 6, 0));
    mat3 b(tvec<3, int>(2, 1, 0), tvec<3, int>(0, 3, 1), tvec<3, int>(1, 0, 4));
    mat3 identity(1, 0, 0, 0, 1, 0, 0, 0, 1);

    static_assert(mat3::size() == 3);
    static_assert(mat3::rows() == 3);
    static_assert(mat3::cols() == 3);
    static_assert(sizeof(mat3) == 9 * sizeof(int));
    static_assert(std::is_trivially_copyable_v<mat3>);

    EXPECT_EQ(a(0, 0), 1);
    EXPECT_EQ(a(1, 0), 2);
    EXPECT_EQ(a(0, 2), 5);
    EXPECT_EQ(a[2], (tvec<3, int>(5, 6, 0)));
    EXPECT_EQ(determinant(a), 1);

    // Comparison, increment and compound assignment.
    EXPECT_TRUE(a == a);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
    mat3 c(a);
    EXPECT_EQ(++c, a + 1);
    c = a;
    EXPECT_EQ(c++, a);
    EXPECT_EQ(c, a + 1);
    c = a;
    EXPECT_EQ(--c, a - 1);
    c = a;
    c *= b;
    EXPECT_EQ(c, a * b);
    c = a;
    c -= a;
    EXPECT_EQ(c, mat3(0)); // diagonal scalar construct
    EXPECT_TRUE(a.nearly_eq(a));
    EXPECT_FALSE(a.nearly_eq(b));

    // Products: the matrix/matrix product must agree with the row by column
    // definition, and the matrix/vector products follow it.
    mat3 by_definition;
    for(std::size_t col = 0; col < 3; ++col)
    {
        for(std::size_t row = 0; row < 3; ++row)
        {
            int sum = 0;
            for(std::size_t k = 0; k < 3; ++k)
                sum += a[k][row] * b[col][k];
            by_definition[col][row] = sum;
        }
    }
    EXPECT_EQ(a * b, by_definition);
    EXPECT_EQ(a * identity, a);
    EXPECT_EQ(identity * a, a);
    // a * v uses the rows of a, v * a uses its columns.
    EXPECT_EQ((a * tvec<3, int>(1, 2, 3)), (tvec<3, int>(16, 22, 11)));
    EXPECT_EQ((tvec<3, int>(1, 2, 3) * a), (tvec<3, int>(14, 14, 17)));
    EXPECT_EQ(a / 2, mat3(0, 1, 1, 0, 0, 2, 2, 3, 0));
    // Scalar on the left divides element wise; skip the zero entries (a[1][0]
    // and a[2][2] are zero and integer division by zero would trap).
    mat3 nz(1, 2, 3, 1, 1, 4, 5, 6, 7);
    EXPECT_EQ(100 / nz, mat3(100, 50, 33, 100, 100, 25, 20, 16, 14));

    // transpose
    mat3 t = transpose(a);
    EXPECT_EQ(t(0, 1), a(1, 0));
    EXPECT_EQ(t(2, 0), a(0, 2));
    EXPECT_EQ(transpose(t), a);
    EXPECT_EQ(determinant(t), determinant(a));

    // inverse() needs a fractional scalar type; the entries of this example are
    // plain integers, so float and fixed point stay exact.
    tmat<3, 3, float> fa(1.f, 2.f, 3.f, 0.f, 1.f, 4.f, 5.f, 6.f, 0.f);
    EXPECT_FLOAT_EQ(determinant(fa), 1.f);
    tmat<3, 3, float> fb = inverse(fa);
    EXPECT_FLOAT_EQ(fb[0][0], -24.f);
    EXPECT_FLOAT_EQ(fb[0][1], 18.f);
    EXPECT_FLOAT_EQ(fb[0][2], 5.f);
    EXPECT_FLOAT_EQ(fb[1][0], 20.f);
    EXPECT_FLOAT_EQ(fb[1][1], -15.f);
    EXPECT_FLOAT_EQ(fb[1][2], -4.f);
    EXPECT_FLOAT_EQ(fb[2][0], -5.f);
    EXPECT_FLOAT_EQ(fb[2][1], 4.f);
    EXPECT_FLOAT_EQ(fb[2][2], 1.f);
    // Row i of the inverse is the cross product of the other two columns.
    EXPECT_EQ((tvec<3, float>(fb[0][0], fb[1][0], fb[2][0])), fa[1].cross(fa[2]));
    tmat<3, 3, float> fIdentity(1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f);
    EXPECT_EQ(fa * fb, fIdentity);
    EXPECT_EQ(fb * fa, fIdentity);
    EXPECT_EQ(fa / fa, fIdentity);

    tmat<3, 3, fixed32> f(1_f32, 2_f32, 3_f32, 0_f32, 1_f32, 4_f32, 5_f32, 6_f32, 0_f32);
    EXPECT_EQ(determinant(f), 1_f32);
    EXPECT_EQ(inverse(f)[0][0], -24_f32);
    EXPECT_EQ(inverse(f)[1][1], -15_f32);
    EXPECT_EQ(transpose(f)(0, 2), f(2, 0));
    EXPECT_EQ(f * inverse(f), (tmat<3, 3, fixed32>(1_f32, 0_f32, 0_f32, 0_f32, 1_f32, 0_f32, 0_f32, 0_f32, 1_f32)));

    // A 2x2 matrix is embedded into the top left corner, the remaining diagonal
    // element is 1 and everything else is 0.
    mat2<int> m22(1, 2, 3, 4);
    EXPECT_EQ(mat3(m22), mat3(1, 2, 0, 3, 4, 0, 0, 0, 1));
    EXPECT_EQ((tmat<3, 3, float>(m22)), (tmat<3, 3, float>(1.f, 2.f, 0.f, 3.f, 4.f, 0.f, 0.f, 0.f, 1.f)));
    // and the reverse conversion takes the top left corner back.
    EXPECT_EQ(mat2<int>(mat3(m22)), m22);
}

// ==================== 4x4 ====================
TEST(Mat, FourByFour)
{
    using mat4 = tmat<4, 4, float>;

    // Column major: columns (4, 7, 2, 6), (0, 1, 4, 3), (5, 6, 0, 1), (2, 1, 3, 8).
    mat4 a(tvec<4, float>(4, 7, 2, 6), tvec<4, float>(0, 1, 4, 3), tvec<4, float>(5, 6, 0, 1), tvec<4, float>(2, 1, 3, 8));
    mat4 identity(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f);

    static_assert(mat4::size() == 4);
    static_assert(mat4::rows() == 4);
    static_assert(mat4::cols() == 4);
    static_assert(sizeof(mat4) == 16 * sizeof(float));
    static_assert(std::is_trivially_copyable_v<mat4>);

    // Construction and access: 16 scalars are column major, and m[c][r] is the
    // same element as m(r, c).
    mat4 byScalars(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f);
    EXPECT_FLOAT_EQ(byScalars(0, 0), 1.f);
    EXPECT_FLOAT_EQ(byScalars(3, 0), 4.f);
    EXPECT_FLOAT_EQ(byScalars(0, 3), 13.f);
    EXPECT_FLOAT_EQ(byScalars[2][3], 12.f);
    EXPECT_EQ(a[3], (tvec<4, float>(2, 1, 3, 8)));
    EXPECT_EQ(a(2, 1), a[1][2]);
    mat4 diagonal(2.f);
    EXPECT_FLOAT_EQ(diagonal(0, 0), 2.f);
    EXPECT_FLOAT_EQ(diagonal(1, 1), 2.f);
    EXPECT_FLOAT_EQ(diagonal(0, 1), 0.f);
    EXPECT_FLOAT_EQ(diagonal(3, 2), 0.f);
#ifdef EIRIN_HAS_CXX_FEATURE_MULTIDIM_SUBSCRIPT
    EXPECT_FLOAT_EQ((a[1, 2]), a[2][1]);
#endif

    // Arithmetic, comparison and increments.
    mat4 sum = a + identity;
    EXPECT_FLOAT_EQ(sum(0, 0), a(0, 0) + 1.f);
    EXPECT_FLOAT_EQ(sum(1, 1), a(1, 1) + 1.f);
    EXPECT_FLOAT_EQ(sum(0, 1), a(0, 1)); // the identity only touches the diagonal
    EXPECT_EQ(a - a, mat4(0.f));
    EXPECT_EQ(-(-a), a);
    // Note: the scalar type has to match the element type, `a * 2` would deduce
    // conflicting types for the element type (float and int).
    EXPECT_EQ(a * 2.f, a + a);
    EXPECT_EQ(2.f * a, a + a);
    EXPECT_EQ(a / 2.f, a * 0.5f);
    EXPECT_TRUE(a == a);
    EXPECT_FALSE(a == identity);
    EXPECT_TRUE(a != identity);
    mat4 c(a);
    EXPECT_EQ(++c, a + 1.f);
    c = a;
    EXPECT_EQ(c--, a);
    EXPECT_EQ(c, a - 1.f);
    c = a;
    c *= 2.f;
    EXPECT_EQ(c, a + a);
    EXPECT_TRUE(a.nearly_eq(a));
    EXPECT_FALSE(a.nearly_eq(identity));

    // Products.
    mat4 b(tvec<4, float>(1, 0, 2, 1), tvec<4, float>(3, 1, 0, 2), tvec<4, float>(0, 2, 1, 0), tvec<4, float>(4, 1, 1, 3));
    mat4 byDefinition;
    for(std::size_t col = 0; col < 4; ++col)
    {
        for(std::size_t row = 0; row < 4; ++row)
        {
            float sum = 0.f;
            for(std::size_t k = 0; k < 4; ++k)
                sum += a[k][row] * b[col][k];
            byDefinition[col][row] = sum;
        }
    }
    EXPECT_EQ(a * b, byDefinition);
    EXPECT_EQ(a * identity, a);
    EXPECT_EQ(a * b * identity, byDefinition);

    // transpose
    mat4 t = transpose(a);
    for(std::size_t col = 0; col < 4; ++col)
    {
        for(std::size_t row = 0; row < 4; ++row)
            EXPECT_FLOAT_EQ(t[col][row], a[row][col]);
    }
    EXPECT_EQ(transpose(t), a);
    EXPECT_EQ(determinant(t), determinant(a));
    EXPECT_NEAR(determinant(a), 341.f, 1e-3f);

    // inverse
    mat4 inv = inverse(a);
    mat4 product = a * inv;
    for(std::size_t col = 0; col < 4; ++col)
    {
        for(std::size_t row = 0; row < 4; ++row)
            EXPECT_NEAR(product[col][row], col == row ? 1.f : 0.f, 1e-5);
    }
    EXPECT_NEAR(determinant(inv), 1.f / 341.f, 1e-7f);

    // a 3x3 or 2x2 matrix is embedded into the top left corner, the remaining
    // diagonal elements are 1 and everything else is 0.
    tmat<3, 3, float> m33(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f);
    mat4 from33(m33);
    EXPECT_FLOAT_EQ(from33[2][2], 9.f);
    EXPECT_FLOAT_EQ(from33[0][3], 0.f);
    EXPECT_FLOAT_EQ(from33[3][3], 1.f);
    EXPECT_FLOAT_EQ(from33[3][0], 0.f);
    tmat<2, 2, float> m22(1.f, 2.f, 3.f, 4.f);
    mat4 from22(m22);
    EXPECT_FLOAT_EQ(from22[1][1], 4.f);
    EXPECT_FLOAT_EQ(from22[2][2], 1.f);
    EXPECT_FLOAT_EQ(from22[3][3], 1.f);
    EXPECT_FLOAT_EQ(from22[2][0], 0.f);

    // Fixed point: same kernels, only accurate to the fixed point precision.
    tmat<4, 4, fixed32> f(4_f32, 7_f32, 2_f32, 6_f32, 0_f32, 1_f32, 4_f32, 3_f32, 5_f32, 6_f32, 0_f32, 1_f32, 2_f32, 1_f32, 3_f32, 8_f32);
    tmat<4, 4, fixed32> fIdentity(1_f32, 0_f32, 0_f32, 0_f32, 0_f32, 1_f32, 0_f32, 0_f32, 0_f32, 0_f32, 1_f32, 0_f32, 0_f32, 0_f32, 0_f32, 1_f32);
    EXPECT_EQ(determinant(f), 341_f32);
    tmat<4, 4, fixed32> fProduct = f * inverse(f);
    for(std::size_t col = 0; col < 4; ++col)
    {
        for(std::size_t row = 0; row < 4; ++row)
            EXPECT_LT(abs(fProduct[col][row] - fIdentity[col][row]), 0.01_f32);
    }
    EXPECT_EQ(transpose(f)(1, 2), f(2, 1));
}

// ==================== Contiguous Element View ====================
TEST(Mat, DataView)
{
    // Column major: the elements of a column are contiguous (asserted for the
    // vectors) and the columns are an array, so the whole matrix is contiguous.
    static_assert(sizeof(mat2<int>) == 4 * sizeof(int));
    static_assert(sizeof(mat3<int>) == 9 * sizeof(int));
    static_assert(sizeof(mat4<int>) == 16 * sizeof(int));
    static_assert(sizeof(mat4<float>) == 64);
    static_assert(std::is_same_v<decltype(std::declval<mat4<int>&>().data()), int*>);
    static_assert(std::is_same_v<decltype(std::declval<const mat4<int>&>().data()), const int*>);

    mat4<int> m(tvec<4, int>(4, 7, 2, 6), tvec<4, int>(0, 1, 4, 3), tvec<4, int>(5, 6, 0, 1), tvec<4, int>(2, 1, 3, 8));

    // The view starts at the first column and advances column by column.
    EXPECT_EQ(m.data(), m[0].data());
    EXPECT_EQ(m.data() + 4, m[1].data());
    EXPECT_EQ(m.data() + 8, m[2].data());
    EXPECT_EQ(m.data() + 12, m[3].data());
    EXPECT_EQ(m.data()[0], 4);
    EXPECT_EQ(m.data()[1], 7);
    EXPECT_EQ(m.data()[4], 0);  // row 0 of the second column
    EXPECT_EQ(m.data()[5], 1);

    // Writing through the view writes into the matrix.
    m.data()[1] = 70;
    EXPECT_EQ(m(1, 0), 70);
    m(1, 1) = 10;
    EXPECT_EQ(m.data()[5], 10);

    const mat4<int> cm(m);
    EXPECT_EQ(cm.data()[1], 70);
    EXPECT_EQ(cm.data()[5], 10);

    // The strictly conforming copy is column major for every shape.
    EXPECT_EQ(mat2<int>(1, 2, 3, 4).to_array(), (std::array<int, 4>{1, 2, 3, 4}));
    EXPECT_EQ(mat3<int>(1, 2, 3, 0, 1, 4, 5, 6, 0).to_array(), (std::array<int, 9>{1, 2, 3, 0, 1, 4, 5, 6, 0}));
    auto a = m.to_array();
    EXPECT_EQ(a[0], 4);
    EXPECT_EQ(a[1], 70);
    EXPECT_EQ(a[4], 0);
    EXPECT_EQ(a[15], 8);
    EXPECT_EQ((tmat<2, 2, fixed32>(4_f32, 7_f32, 2_f32, 6_f32).to_array()[3]), 6_f32);

    // Constant expressions: the first element through the view, or any element
    // through the copy.
    constexpr mat3<int> kMat(1, 2, 3, 0, 1, 4, 5, 6, 0);
    static_assert(kMat.data()[0] == 1);
    static_assert(kMat.to_array()[2] == 3);
    static_assert(kMat.to_array()[3] == 0);
    static_assert(kMat.to_array()[8] == 0);
}

// ==================== Compile Time Constraints ====================
TEST(Mat, CompileConstraints)
{
    mat2<int> m(1, 2, 3, 4);
    mat2<int> n(5, 6, 7, 8);
    tvec<2, int> v(1, 2);

    // Matrix/scalar compound assignment is accepted, matrix/vector is not.
    EXPECT_TRUE((EIRIN_TESTING_COMPILE_ADD_ASSIGN(m, n)));
    EXPECT_TRUE((EIRIN_TESTING_COMPILE_ADD_ASSIGN(m, 1)));
    EXPECT_FALSE((EIRIN_TESTING_COMPILE_ADD_ASSIGN(m, v)));
    EXPECT_TRUE((EIRIN_TESTING_COMPILE_SUB_ASSIGN(m, n)));
    EXPECT_FALSE((EIRIN_TESTING_COMPILE_SUB_ASSIGN(m, v)));
    EXPECT_TRUE((EIRIN_TESTING_COMPILE_MUL_ASSIGN(m, n)));
    EXPECT_TRUE((EIRIN_TESTING_COMPILE_MUL_ASSIGN(m, 2)));
    EXPECT_FALSE((EIRIN_TESTING_COMPILE_MUL_ASSIGN(m, 2.5)));
    EXPECT_FALSE((EIRIN_TESTING_COMPILE_MUL_ASSIGN(m, v)));
    EXPECT_TRUE((EIRIN_TESTING_COMPILE_DIV_ASSIGN(m, 2)));
    EXPECT_FALSE((EIRIN_TESTING_COMPILE_DIV_ASSIGN(m, v)));

    // Products require the same element type, += / -= accept a converted one.
    EXPECT_FALSE((EIRIN_TESTING_COMPILE_MUL_ASSIGN(m, mat2<double>(1, 2, 3, 4))));
    EXPECT_TRUE((EIRIN_TESTING_COMPILE_ADD_ASSIGN(m, mat2<double>(1, 2, 3, 4))));

    // inverse() is only available for shapes and scalar types that support it.
    EXPECT_TRUE(has_inverse_v<mat2<float>>);
    EXPECT_TRUE(has_inverse_v<mat2<fixed32>>);
    EXPECT_FALSE(has_inverse_v<mat2<int>>);
    EXPECT_FALSE((EIRIN_TESTING_COMPILE_DIV_ASSIGN(m, n)));
    EXPECT_TRUE(has_transpose_v<mat2<int>>);
    EXPECT_TRUE(has_determinant_v<mat2<int>>);
}

// ==================== Constant Expressions ====================
TEST(Mat, ConstexprApi)
{
    constexpr mat2<int> a(1, 3, 2, 4); // [[1, 2], [3, 4]]
    constexpr mat2<int> b(5, 7, 6, 8); // [[5, 6], [7, 8]]

    static_assert(a * b == mat2<int>(19, 43, 22, 50));
    static_assert(a * tvec<2, int>(1, 2) == tvec<2, int>(5, 11));
    static_assert(tvec<2, int>(1, 2) * a == tvec<2, int>(7, 10));
    static_assert(transpose(a) == mat2<int>(1, 2, 3, 4));
    static_assert(determinant(a) == -2);
    static_assert(+a == a);
    static_assert(-a == mat2<int>(-1, -3, -2, -4));
    static_assert(a.at(0)[1] == 3);
    static_assert(a.at(1, 0) == 3);
    static_assert(a.element(-1)[0] == 2);
    static_assert(a.element(2, 2) == 1);
    // fixed MSVC C2131: `a.nearly_eq(a)` is deliberately not asserted here. It evaluates
    // `static_cast<const tvec&>(*this)` on a vector that is a subobject of this matrix,
    // which MSVC's constant evaluator rejects.
    EXPECT_TRUE(a.nearly_eq(a));

    constexpr mat2<float> f(4.f, 7.f, 2.f, 6.f);
    constexpr mat2<float> fInv = inverse(f);
    static_assert(fInv(0, 0) > 0.59f && fInv(0, 0) < 0.61f);
    static_assert(fInv(1, 1) > 0.39f && fInv(1, 1) < 0.41f);
}

// ==================== Common Usage ====================
TEST(Mat, CommonUsage)
{
    // A typical 2D rotation applied to a column vector:
    // column 0 = (cos, sin), column 1 = (-sin, cos), i.e. R(theta).
    const float c = std::cos(0.5f);
    const float s = std::sin(0.5f);
    mat2<float> rot(c, s, -s, c);

    vec2<float> v(1.f, 0.f);
    vec2<float> r = rot * v; // (c, s)
    EXPECT_NEAR(r.x, c, 1e-5f);
    EXPECT_NEAR(r.y, s, 1e-5f);
    EXPECT_TRUE(rot.nearly_eq(inverse(transpose(rot))));
    EXPECT_FLOAT_EQ(determinant(rot), 1.f);
}
