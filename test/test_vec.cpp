#include <gtest/gtest.h>
#include <type_traits>
#include <utility>
#include <eirin/eirin.hpp>
#include <eirin/io/format.hpp>
#include "compile_check.hpp"

using namespace eirin;
using namespace eirin::literals;

template <std::size_t N, typename T>
::testing::AssertionResult vec_nearly_eq(const tvec<N, T>& a, const tvec<N, T>& b, T eps = T{1e-5})
{
    for(std::size_t i = 0; i < N; ++i)
    {
        if(abs(a[i] - b[i]) > eps)
            return ::testing::AssertionFailure() << "a[" << i << "] = " << a[i] << ", b[" << i << "] = " << b[i];
    }
    return ::testing::AssertionSuccess();
}

// ==================== Construction and Basic Attr ====================
TEST(Vec, Constructors)
{
    // Default Construct
    tvec<2, int> v2i_default{};
    EXPECT_EQ(v2i_default.x, 0);
    EXPECT_EQ(v2i_default.y, 0);

    // Construct from scalar
    tvec<3, int> v3i_scalar(5);
    EXPECT_EQ(v3i_scalar.x, 5);
    EXPECT_EQ(v3i_scalar.y, 5);
    EXPECT_EQ(v3i_scalar.z, 5);

    // Construct from xyzw
    tvec<4, int> v4i(1, 2, 3, 4);
    EXPECT_EQ(v4i.x, 1);
    EXPECT_EQ(v4i.y, 2);
    EXPECT_EQ(v4i.z, 3);
    EXPECT_EQ(v4i.w, 4);

    // Copy Construct
    tvec<2, int> v2i_copy(v4i); // This should copy the first two elements.
    EXPECT_EQ(v2i_copy.x, 1);
    EXPECT_EQ(v2i_copy.y, 2);

    // Construct from other types.
    tvec<2, fixed32> v2f = tvec<2, int>(3, 4);
    EXPECT_EQ(v2f.x, 3_f32);
    EXPECT_EQ(v2f.y, 4_f32);

    // Construct from mix types.
    tvec<3, fixed32> v3f_mixed(1_f32, 2, 3.0f);
    EXPECT_EQ(v3f_mixed.x, 1_f32);
    EXPECT_EQ(v3f_mixed.y, 2_f32);
    EXPECT_EQ(v3f_mixed.z, 3_f32);
}

// ==================== Access Elements ====================
TEST(Vec, ElementAccess)
{
    tvec<3, int> v(10, 20, 30);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);
    EXPECT_EQ(v[2], 30);
    v[1] = 99;
    EXPECT_EQ(v.y, 99);

    const tvec<3, int> cv(1, 2, 3);
    EXPECT_EQ(cv[0], 1);
    EXPECT_EQ(cv[2], 3);
}

// ==================== Arithmetic Compute ====================
TEST(Vec, Arithmetic)
{
    tvec<2, int> a(1, 2), b(3, 4);

    // vec + vec
    auto c = a + b;
    EXPECT_EQ(c.x, 4);
    EXPECT_EQ(c.y, 6);

    // vec - vec
    c = a - b;
    EXPECT_EQ(c.x, -2);
    EXPECT_EQ(c.y, -2);

    // vec * vec
    c = a * b;
    EXPECT_EQ(c.x, 3);
    EXPECT_EQ(c.y, 8);

    // vec / vec
    c = a / b;
    EXPECT_EQ(c.x, 0);
    EXPECT_EQ(c.y, 0);

    // vec % vec
    c = a % b;
    EXPECT_EQ(c.x, 1);
    EXPECT_EQ(c.y, 2);

    // vec operate with scalar
    c = a + 10;
    EXPECT_EQ(c.x, 11);
    EXPECT_EQ(c.y, 12);

    c = 10 - a;
    EXPECT_EQ(c.x, 9);
    EXPECT_EQ(c.y, 8);

    c = a * 5;
    EXPECT_EQ(c.x, 5);
    EXPECT_EQ(c.y, 10);

    c = 3 * a;
    EXPECT_EQ(c.x, 3);
    EXPECT_EQ(c.y, 6);

    c = a / 2;
    EXPECT_EQ(c.x, 0);
    EXPECT_EQ(c.y, 1);

    c = 10 % a;
    EXPECT_EQ(c.x, 0);
    EXPECT_EQ(c.y, 0);
}

// ==================== Assignment ====================
TEST(Vec, CompoundAssignment)
{
    tvec<2, int> a(1, 2);

    a += tvec<2, int>(3, 4);
    EXPECT_EQ(a.x, 4);
    EXPECT_EQ(a.y, 6);

    a -= 2;
    EXPECT_EQ(a.x, 2);
    EXPECT_EQ(a.y, 4);

    a *= tvec<2, int>(2, 3);
    EXPECT_EQ(a.x, 4);
    EXPECT_EQ(a.y, 12);

    a /= 2;
    EXPECT_EQ(a.x, 2);
    EXPECT_EQ(a.y, 6);

    a %= 4;
    EXPECT_EQ(a.x, 2);
    EXPECT_EQ(a.y, 2);
}

// ==================== BitWise ====================
TEST(Vec, BitwiseOps)
{
    tvec<2, int> a(0b1010, 0b1100), b(0b0110, 0b0011);

    auto c = a & b;
    EXPECT_EQ(c.x, 0b0010);
    EXPECT_EQ(c.y, 0b0000);

    c = a | b;
    EXPECT_EQ(c.x, 0b1110);
    EXPECT_EQ(c.y, 0b1111);

    c = a ^ b;
    EXPECT_EQ(c.x, 0b1100);
    EXPECT_EQ(c.y, 0b1111);

    c = a << 1;
    EXPECT_EQ(c.x, 0b10100);
    EXPECT_EQ(c.y, 0b11000);

    c = a >> 1;
    EXPECT_EQ(c.x, 0b0101);
    EXPECT_EQ(c.y, 0b0110);

    // assignment
    a &= b;
    EXPECT_EQ(a.x, 0b0010);
    a |= tvec<2, int>(0b1111, 0b1111);
    EXPECT_EQ(a.x, 0b1111);
    a ^= 0b0101;
    EXPECT_EQ(a.x, 0b1010);
    a <<= 2;
    EXPECT_EQ(a.x, 0b101000);
    a >>= 3;
    EXPECT_EQ(a.x, 0b000101);

    a = ~a;
}

// ==================== Logical Test ====================
TEST(Vec, LogicalOps)
{
    tvec<2, int> a(1, 0), b(0, 2);
    auto r = a && b;
    EXPECT_EQ(r.x, false);
    EXPECT_EQ(r.y, false); // 0 && 2 => false

    r = a || b;
    EXPECT_EQ(r.x, true);
    EXPECT_EQ(r.y, true); // 0 || 2 => true

    a = tvec<2, int>(2, 0);
    b = tvec<2, int>(3, 4);
    r = a && b;
    EXPECT_EQ(r.x, true);
    EXPECT_EQ(r.y, false);
}

// ==================== Unary Test ====================
TEST(Vec, UnaryOps)
{
    tvec<2, int> a(1, -2);
    auto b = -a;
    EXPECT_EQ(b.x, -1);
    EXPECT_EQ(b.y, 2);

    b = +a;
    EXPECT_EQ(b.x, 1);
    EXPECT_EQ(b.y, -2);

    tvec<2, int> c(0b1010, 0b0101);
    auto d = ~c;
    (void)d;
}

// ==================== Self Incr/Desc ====================
TEST(Vec, IncDec)
{
    tvec<2, int> a(1, 2);
    auto b = ++a;
    EXPECT_EQ(b.x, 2);
    EXPECT_EQ(b.y, 3);
    EXPECT_EQ(a.x, 2);
    EXPECT_EQ(a.y, 3);

    auto c = a++;
    EXPECT_EQ(c.x, 2);
    EXPECT_EQ(c.y, 3);
    EXPECT_EQ(a.x, 3);
    EXPECT_EQ(a.y, 4);

    auto d = --a;
    EXPECT_EQ(d.x, 2);
    EXPECT_EQ(d.y, 3);
    EXPECT_EQ(a.x, 2);
    EXPECT_EQ(a.y, 3);

    auto e = a--;
    EXPECT_EQ(e.x, 2);
    EXPECT_EQ(e.y, 3);
    EXPECT_EQ(a.x, 1);
    EXPECT_EQ(a.y, 2);
}

// ==================== 9. Dot and Cross ====================
TEST(Vec, DotCross)
{
    tvec<2, int> a(1, 2), b(3, 4);
    int d = a.dot(b);
    EXPECT_EQ(d, 1 * 3 + 2 * 4);

    d = tvec<2, int>(1, 0).dot(tvec<2, int>(0, 1));
    EXPECT_EQ(d, 0);

    tvec<3, int> c(1, 2, 3), e(4, 5, 6);
    auto cr = c.cross(e);
    EXPECT_EQ(cr.x, 2 * 6 - 3 * 5);
    EXPECT_EQ(cr.y, 3 * 4 - 1 * 6);
    EXPECT_EQ(cr.z, 1 * 5 - 2 * 4);

    auto dot1 = c.dot(cr);
    auto dot2 = e.dot(cr);
    EXPECT_EQ(dot1, 0);
    EXPECT_EQ(dot2, 0);
}

// ==================== 10. Compare ====================
TEST(Vec, Compare)
{
    tvec<2, int> a(1, 2), b(1, 2), c(1, 3);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);

    // nearly_eq for fixed point
    tvec<2, fixed32> fa(1.0_f32, 2.0_f32);
    tvec<2, fixed32> fb(1.00001_f32, 2.00001_f32);
    EXPECT_TRUE(fa.nearly_eq(fb));

    tvec<2, int> ia(1, 2), ib(1, 3);
    EXPECT_FALSE(ia.nearly_eq(ib));
}

// ==================== 11. Swizzle Test ====================
TEST(Vec, SwizzleRead)
{
    tvec<4, int> v(1, 2, 3, 4);

    auto xy = v.xy();
    EXPECT_EQ(xy.x(), 1);
    EXPECT_EQ(xy.y(), 2);

    auto xyz = v.xyz();
    EXPECT_EQ(xyz.x(), 1);
    EXPECT_EQ(xyz.y(), 2);
    EXPECT_EQ(xyz.z(), 3);

    auto xyzw = v.xyzw();
    EXPECT_EQ(xyzw.x(), 1);
    EXPECT_EQ(xyzw.y(), 2);
    EXPECT_EQ(xyzw.z(), 3);
    EXPECT_EQ(xyzw.w(), 4);

    auto xx = v.xx();
    EXPECT_EQ(xx.x(), 1);
    EXPECT_EQ(xx.y(), 1);

    auto yxy = v.yxy();
    EXPECT_EQ(yxy.x(), 2);
    EXPECT_EQ(yxy.y(), 1);
    EXPECT_EQ(yxy.z(), 2);

    const tvec<4, int> cv(5, 6, 7, 8);
    auto cxy = cv.xy();
    EXPECT_EQ(cxy.x(), 5);
    EXPECT_EQ(cxy.y(), 6);

    auto xyxy = cv.xyzw().xyxy();
    EXPECT_EQ(xyxy.x(), 5);
    EXPECT_EQ(xyxy.y(), 6);
    EXPECT_EQ(xyxy.xxxx().y(), 5);

    // --- deeper nesting ------------------------------------------------
    // v = (1,2,3,4):
    //   wzyx -> (4,3,2,1); yxz -> (3,4,2); zz -> (2,2); z() -> 2
    auto deep1 = v.wzyx().yxz().zz();
    EXPECT_EQ(deep1.x(), 2);
    EXPECT_EQ(deep1.y(), 2);
    EXPECT_EQ(v.wzyx().yxz().z(), 2);

    //   wzyx -> (4,3,2,1); yxz -> (3,4,2); xy -> (3,4)
    tvec<2, int> deep2 = v.wzyx().yxz().xy();
    EXPECT_EQ(deep2.x, 3);
    EXPECT_EQ(deep2.y, 4);

    //   yzxy -> (2,3,1,2); xwzy -> (2,2,1,3); yxx -> (2,2,2); x() -> 2
    auto deep3 = v.yzxy().xwzy().yxx();
    EXPECT_EQ(deep3.x(), 2);
    EXPECT_EQ(deep3.y(), 2);
    EXPECT_EQ(deep3.z(), 2);
    EXPECT_EQ(v.yzxy().xwzy().yxx().x(), 2);

    // const chain: cv = (5,6,7,8)
    //   wzyx -> (8,7,6,5); xxyy -> (8,8,7,7); yz -> (8,7)
    auto deep4 = cv.wzyx().xxyy().yz();
    EXPECT_EQ(deep4.x(), 8);
    EXPECT_EQ(deep4.y(), 7);
    EXPECT_EQ(cv.wzyx().xxyy().yz().x(), 8);

    // nested chain materialized through operator()
    tvec<3, int> deep5 = v.xyzw().wzyx().yxz()();
    EXPECT_EQ(deep5.x, 3);
    EXPECT_EQ(deep5.y, 4);
    EXPECT_EQ(deep5.z, 2);
}

TEST(Vec, SwizzleWriteValid)
{
    eirin::vec4fixed64 vf{495, 514_f64, 19, 0};
    eirin::vec4i vi{114, 514.0, 0, 0}, vii{1919810};
    vf.xyz() += 1_f32;
    vi.xyzw() = eirin::vec4i{1, 2, 3, 4} + vf.xyxy();
    vi.xwzy() = vii.yxxy();
    EXPECT_EQ(vi.x, 1919810);
    EXPECT_EQ(vi.y, 1919810);
    EXPECT_EQ(vi.z, 1919810);
    EXPECT_EQ(vi.w, 1919810);
    vi.wxyz()[0] = 1;
    EXPECT_EQ(vi.w, 1);
    // vii.xxxx()[0] = 1; // This should compile failed.
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ASSIGN(vii.xxxx()[0], 1));

    tvec<4, int> v(1, 2, 3, 4);

    v.xy() = tvec<2, int>(10, 20);
    EXPECT_EQ(v.x, 10);
    EXPECT_EQ(v.y, 20);
    EXPECT_EQ(v.z, 3);
    EXPECT_EQ(v.w, 4);

    v.xyz() = tvec<3, int>(30, 40, 50);
    EXPECT_EQ(v.x, 30);
    EXPECT_EQ(v.y, 40);
    EXPECT_EQ(v.z, 50);
    EXPECT_EQ(v.w, 4);

    v.x = 100;
    EXPECT_EQ(v.x, 100);

    v.xy() += tvec<2, int>(1, 2);
    EXPECT_EQ(v.x, 101);
    EXPECT_EQ(v.y, 42);

    v.xy() *= 2;
    EXPECT_EQ(v.x, 202);
    EXPECT_EQ(v.y, 84);

    ++v.xyz();
    EXPECT_EQ(v.x, 203);
    EXPECT_EQ(v.y, 85);
    EXPECT_EQ(v.z, 51);

    auto old = v.xy()++;
    EXPECT_EQ(old.x, 203);
    EXPECT_EQ(old.y, 85);
    EXPECT_EQ(v.x, 204);
    EXPECT_EQ(v.y, 86);

    // overlapping selections must snapshot the right-hand side first
    tvec<2, int> ov(1, 2);
    ov.xy() = ov.yx(); // swap
    EXPECT_EQ(ov.x, 2);
    EXPECT_EQ(ov.y, 1);

    tvec<2, int> cv2(1, 2);
    cv2.xy() += cv2.yx(); // (1+2, 2+1)
    EXPECT_EQ(cv2.x, 3);
    EXPECT_EQ(cv2.y, 3);
}

TEST(Vec, SwizzleWriteDuplicateCompileCheck)
{
    tvec<2, int> v(1, 2);
    // These should compile failed.
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ASSIGN(v.xx(), (tvec<2, int>(3, 4)))); // v.xx() = tvec<2, int>(3,4)
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ADD_ASSIGN(v.xx(), 1)); // v.xx() += 1
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_PRE_INCR(v.xx())); // ++v.xx()
    SUCCEED();
}

TEST(Vec, SwizzleProxyChain)
{
    tvec<4, int> v(1, 2, 3, 4);

    // vector -> proxy keeps absolute component indices
    auto xy = v.xy();
    EXPECT_EQ(xy.size(), 2u);
    EXPECT_EQ(xy[0], 1);
    EXPECT_EQ(xy[1], 2);
    EXPECT_EQ(xy.x(), 1);
    EXPECT_EQ(xy.y(), 2);

    // proxy -> proxy composes through the current selection
    tvec<2, int> yx = v.xy().yx(); // (2, 1)
    EXPECT_EQ(yx.x, 2);
    EXPECT_EQ(yx.y, 1);

    tvec<4, int> xxyy = v.xxyy(); // (1, 1, 2, 2)
    EXPECT_EQ(xxyy.x, 1);
    EXPECT_EQ(xxyy.y, 1);
    EXPECT_EQ(xxyy.z, 2);
    EXPECT_EQ(xxyy.w, 2);

    tvec<2, int> from_chain = v.xxyy().yx(); // view (1,1,2,2).yx() = view[1],view[0]
    EXPECT_EQ(from_chain.x, 1);
    EXPECT_EQ(from_chain.y, 1);

    // three-level chain: wzyx -> (4,3,2,1); yxz -> (3,4,2); xz -> (3,2)
    tvec<2, int> deep = v.wzyx().yxz().xz();
    EXPECT_EQ(deep.x, 3);
    EXPECT_EQ(deep.y, 2);

    // single-component access on a temporary proxy
    EXPECT_EQ(v.yzxy().z(), 1); // yzxy = (2,3,1,2); z() reads its third lane

    // reads through a const vector produce a const proxy
    const tvec<4, int> cv(5, 6, 7, 8);
    auto cxy = cv.xy();
    EXPECT_EQ(cxy[0], 5);
    EXPECT_EQ(cxy[1], 6);
    EXPECT_EQ(cxy.x(), 5);

    // writing through a distinct proxy maps lanes back to the original vector
    v.yx() = tvec<2, int>(7, 8); // y=7, x=8
    EXPECT_EQ(v.x, 8);
    EXPECT_EQ(v.y, 7);
    v.xy() += tvec<2, int>(1, 2);
    EXPECT_EQ(v.x, 9);
    EXPECT_EQ(v.y, 9);

    constexpr auto constexpr_chain = []() constexpr
    {
        tvec<4, int> w(1, 2, 3, 4);
        const auto p = w.yzxy().xw();
        return p[0] == 2 && p[1] == 2 && w.xyz().z() == 3;
    };
    static_assert(constexpr_chain());
}

// ==================== 11.5 Chained Swizzle as Lvalue ====================
TEST(Vec, SwizzleChainLvaluePolicy)
{
    tvec<4, int> v(1, 2, 3, 4);

    // The first hop from a vector is always a writable proxy.
    static_assert(std::is_same_v<decltype(v.xy()), swizzle_proxy<4, int, false, 0, 1>>);

    // A chained hop is a read-only view by default, and a writable proxy only
    // when EIRIN_VEC_ENABLE_SWIZZLE_CHAIN_AS_LVALUE is defined.
#if EIRIN_VEC_SWIZZLE_CHAIN_AS_LVALUE == EIRIN_ENABLE
    static_assert(std::is_same_v<decltype(v.xy().yx()), swizzle_proxy<4, int, false, 1, 0>>);
#else
    static_assert(std::is_same_v<decltype(v.xy().yx()), swizzle_proxy<4, int, true, 1, 0>>);
    // these should compile failed.
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ASSIGN(v.xy().yx(), (tvec<2, int>(9, 9)))); // v.xy().yx() = tvec<2, int>(9, 9)
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ADD_ASSIGN(v.xy().yx(), (tvec<2, int>(1, 1)))); // v.xy().yx() += tvec<2, int>(1, 1)
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ASSIGN(v.xy().yx(), v.yx())); // v.xy().yx() = v.yx()
#endif

    // reads through chained selectors keep working in both configurations
    EXPECT_EQ(v.xy().yx()[0], 2);
    EXPECT_EQ(v.xy().yx()[1], 1);
    v.xy() = tvec<2, int>(10, 20);
    EXPECT_EQ(v.x, 10);
    EXPECT_EQ(v.y, 20);
}

#if EIRIN_VEC_SWIZZLE_CHAIN_AS_LVALUE == EIRIN_ENABLE
TEST(Vec, SwizzleChainWrite)
{
    tvec<4, int> v(1, 2, 3, 4);
    v.xy().yx() = tvec<2, int>(7, 8); // y = 7, x = 8
    EXPECT_EQ(v.x, 8);
    EXPECT_EQ(v.y, 7);
    EXPECT_EQ(v.z, 3);
    EXPECT_EQ(v.w, 4);

    // overlapping selections keep GLSL/GLM snapshot semantics through chains
    tvec<2, int> ov(1, 2);
    ov.xy().xy() = ov.yx(); // swap
    EXPECT_EQ(ov.x, 2);
    EXPECT_EQ(ov.y, 1);

    tvec<2, int> cv2(1, 2);
    cv2.xy().xy() += cv2.yx(); // (1+2, 2+1)
    EXPECT_EQ(cv2.x, 3);
    EXPECT_EQ(cv2.y, 3);

    // deeper chains stay writable: wzyx -> (4,3,2,1); yx -> (3,4) = (z, w)
    tvec<4, int> deep(1, 2, 3, 4);
    deep.xyzw().wzyx().yx() = tvec<2, int>(30, 40);
    EXPECT_EQ(deep.x, 1);
    EXPECT_EQ(deep.y, 2);
    EXPECT_EQ(deep.z, 30);
    EXPECT_EQ(deep.w, 40);

    // duplicated selections stay non-writable through chains
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ASSIGN(deep.xx().yx(), (tvec<2, int>(1, 2))));
}

TEST(Vec, SwizzleChainWriteSameType)
{
    // same-type chained proxy assignment must write lanes back instead of the
    // old implicit copy assignment (which silently re-bound the proxy pointer)
    tvec<4, int> a(1, 2, 3, 4), b(5, 6, 7, 8);
    b.xy().yx() = a.xy().yx();
    EXPECT_EQ(b.x, 1);
    EXPECT_EQ(b.y, 2);
    EXPECT_EQ(b.z, 7);
    EXPECT_EQ(b.w, 8);

    auto p = a.xy().yx();
    p = b.xy().yx();
    p.y() = 77; // lane y of the {1, 0} view maps back to original x
    EXPECT_EQ(a.x, 77);
    EXPECT_EQ(a.y, 2);
}
#endif

// ============ Same-type proxy copy assignment ============
TEST(Vec, SwizzleSameTypeCopyAssignValueSemantics)
{
    // same-type, same-selection assignment writes lanes back into the LHS
    // vector (GLSL l-value semantics) instead of re-binding the proxy.
    tvec<4, int> a(1, 2, 3, 4), b(5, 6, 7, 8);
    a.xy() = b.xy();
    EXPECT_EQ(a.x, 5);
    EXPECT_EQ(a.y, 6);
    EXPECT_EQ(a.z, 3);
    EXPECT_EQ(a.w, 4);

    // a stored proxy stays attached to its original vector after assignment
    auto p = a.xy();
    p = b.xy();
    p.y() = 42;
    EXPECT_EQ(a.x, 5);
    EXPECT_EQ(a.y, 42);
    EXPECT_EQ(b.y, 6);

    // only writable, duplicate-free selections are copy assignable; views
    // remain copy constructible so `auto q = p;` keeps view identity
    static_assert(std::is_copy_assignable_v<swizzle_proxy<4, int, false, 0, 1>>);
    static_assert(!std::is_copy_assignable_v<swizzle_proxy<4, int, true, 0, 1>>);
    static_assert(!std::is_copy_assignable_v<swizzle_proxy<4, int, false, 0, 0>>);
    static_assert(std::is_copy_constructible_v<swizzle_proxy<4, int, true, 0, 1>>);

    // read-only views and duplicated selections have no write path
    const tvec<4, int> cv(1, 2, 3, 4), cw(5, 6, 7, 8);
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ASSIGN(cv.xy(), cw.xy()));
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ASSIGN(a.xx(), b.xx()));
    (void)cv;
    (void)cw;
}

// ==================== 12. Mixed Type Math ====================
TEST(Vec, MixedTypes)
{
    tvec<2, int> a(1, 2);
    tvec<2, fixed32> b(3_f32, 4_f32);

    auto c = a + b;
    (void)c;
    tvec<2, fixed32> d = a;
    tvec<2, fixed32> e(a);
    EXPECT_EQ(e.x, 1_f32);
    EXPECT_EQ(e.y, 2_f32);

    d = e + 1_f32;
    EXPECT_EQ(d.x, 2_f32);
    EXPECT_EQ(d.y, 3_f32);

    tvec<2, double> f(e);
    EXPECT_EQ(f.x, 1.0);
    EXPECT_EQ(f.y, 2.0);
    // f = f + 1; // TODO: should we add this support? Not sure yet.
    // EXPECT_EQ(f.x, 2.0);
    // EXPECT_EQ(f.y, 3.0);
}

// ==================== 13. tvec_base Test ====================
TEST(Vec, BaseMembers)
{
    tvec<3, int> v(1, 2, 3);
    EXPECT_EQ(v.size(), 3);

    tvec<3, int> v2(1, 2, 3);
    EXPECT_TRUE(v == v2);
}

// ==================== 15. Bitwise and Assignment ====================
TEST(Vec, BitwiseCompound)
{
    tvec<2, int> a(0b1010, 0b1100);
    a ^= tvec<2, int>(0b0110, 0b0011);
    EXPECT_EQ(a.x, 0b1100);
    EXPECT_EQ(a.y, 0b1111);

    a &= 0b1010;
    EXPECT_EQ(a.x, 0b1000);
    EXPECT_EQ(a.y, 0b1010);

    a |= 0b0001;
    EXPECT_EQ(a.x, 0b1001);
    EXPECT_EQ(a.y, 0b1011);

    a <<= 2;
    EXPECT_EQ(a.x, 0b100100);
    EXPECT_EQ(a.y, 0b101100);

    a >>= 3;
    EXPECT_EQ(a.x, 0b0100);
    EXPECT_EQ(a.y, 0b0101);
}

// ==================== 16. Behavior of GLSL ====================

namespace
{
    // SFINAE probes for members that must not exist on the given tvec shape;
    // they pin the compile-time side of the GLSL rules below.
    template <typename T, typename = void>
    struct glsl_has_field_r : std::false_type
    {};
    template <typename T>
    struct glsl_has_field_r<T, std::void_t<decltype(std::declval<T>().r)>> : std::true_type
    {};

    template <typename T, typename = void>
    struct glsl_has_field_s : std::false_type
    {};
    template <typename T>
    struct glsl_has_field_s<T, std::void_t<decltype(std::declval<T>().s)>> : std::true_type
    {};

    template <typename T, typename = void>
    struct glsl_has_field_z : std::false_type
    {};
    template <typename T>
    struct glsl_has_field_z<T, std::void_t<decltype(std::declval<T>().z)>> : std::true_type
    {};

    template <typename T, typename = void>
    struct glsl_has_call_rgba : std::false_type
    {};
    template <typename T>
    struct glsl_has_call_rgba<T, std::void_t<decltype(std::declval<T>().rgba())>> : std::true_type
    {};

    template <typename T, typename = void>
    struct glsl_has_call_stpq : std::false_type
    {};
    template <typename T>
    struct glsl_has_call_stpq<T, std::void_t<decltype(std::declval<T>().stpq())>> : std::true_type
    {};

    template <typename T, typename = void>
    struct glsl_has_call_xyzwx : std::false_type
    {};
    template <typename T>
    struct glsl_has_call_xyzwx<T, std::void_t<decltype(std::declval<T>().xyzwx())>> : std::true_type
    {};
} // namespace

TEST(Vec, GLSLBehavior)
{
    static_assert(!glsl_has_field_r<tvec<4, int>>::value);
    static_assert(!glsl_has_field_s<tvec<4, int>>::value);
    static_assert(!glsl_has_call_rgba<tvec<4, int>>::value);
    static_assert(!glsl_has_call_stpq<tvec<4, int>>::value);

    vec4i v{114, 495, 1919810, 514};
    static_assert(std::is_same_v<decltype(v.yx()()), tvec<2, int>>);
    static_assert(std::is_same_v<decltype(v.zyx()()), tvec<3, int>>);
    static_assert(std::is_same_v<decltype(v.wzyx()()), tvec<4, int>>);
    tvec<2, int> m2 = v.yx();
    EXPECT_EQ(m2.x, 495);
    EXPECT_EQ(m2.y, 114);
    tvec<3, int> m3 = v.zyx();
    EXPECT_EQ(m3.x, 1919810);
    EXPECT_EQ(m3.y, 495);
    EXPECT_EQ(m3.z, 114);
    tvec<4, int> m4 = v.wzyx();
    EXPECT_EQ(m4.x, 514);
    EXPECT_EQ(m4.y, 1919810);
    EXPECT_EQ(m4.z, 495);
    EXPECT_EQ(m4.w, 114);

    tvec<2, int> rep2 = v.xx();
    EXPECT_EQ(rep2.x, 114);
    EXPECT_EQ(rep2.y, 114);
    tvec<4, int> rep4 = v.yxxw();
    EXPECT_EQ(rep4.x, 495);
    EXPECT_EQ(rep4.y, 114);
    EXPECT_EQ(rep4.z, 114);
    EXPECT_EQ(rep4.w, 514);
    tvec<2, int> chain2 = v.xy().yx();
    EXPECT_EQ(chain2.x, 495);
    EXPECT_EQ(chain2.y, 114);

    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ASSIGN(v.xx(), (tvec<2, int>(7, 8))));
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ADD_ASSIGN(v.xx(), 1));
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_PRE_INCR(v.xx()));
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ASSIGN(v.xx().x(), 9));
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ASSIGN(v.xx()[0], 9));
    vec4i lv{1, 2, 3, 4};
    lv.xy() = tvec<2, int>(10, 20);
    EXPECT_EQ(lv.x, 10);
    EXPECT_EQ(lv.y, 20);
    EXPECT_EQ(lv.z, 3);
    EXPECT_EQ(lv.w, 4);

    vec4i wb{1, 2, 3, 4};
    wb.wzyx() = tvec<4, int>(8, 7, 6, 5);
    EXPECT_EQ(wb.x, 5);
    EXPECT_EQ(wb.y, 6);
    EXPECT_EQ(wb.z, 7);
    EXPECT_EQ(wb.w, 8);

    const vec4i c{1, 2, 3, 4};
    tvec<2, int> cr = c.yx();
    EXPECT_EQ(cr.x, 2);
    EXPECT_EQ(cr.y, 1);
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ASSIGN(c.xy(), (tvec<2, int>(9, 9))));
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ADD_ASSIGN(c.xy(), 1));
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_PRE_INCR(c.xy()));
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ASSIGN((c.x), 5));
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ASSIGN(c.xy().x(), 5));

    static_assert(std::is_same_v<decltype(v.x), int>);
    static_assert(std::is_same_v<decltype(lv.xy().x()), int&>);
    lv.y = 77;
    EXPECT_EQ(lv.y, 77);
    lv.xy().x() = 88;
    EXPECT_EQ(lv.x, 88);
    EXPECT_EQ(lv.y, 77);

    tvec<2, int> snap = v.yx(); // {495, 114}
    v.x = 1000;
    EXPECT_EQ(snap.x, 495);
    EXPECT_EQ(snap.y, 114);
    tvec<2, int> sum = v.xy() + v.zw();
    EXPECT_EQ(sum.x, 1000 + 1919810);
    EXPECT_EQ(sum.y, 495 + 514);

    tvec<2, int> ov(1, 2);
    ov.xy() = ov.yx();
    EXPECT_EQ(ov.x, 2);
    EXPECT_EQ(ov.y, 1);
    tvec<2, int> co(1, 2);
    co.xy() += co.yx(); // (1+2, 2+1)
    EXPECT_EQ(co.x, 3);
    EXPECT_EQ(co.y, 3);
    tvec<4, int> o4(1, 2, 3, 4);
    o4.yxwz() = o4.wzyx();
    EXPECT_EQ(o4.x, 3);
    EXPECT_EQ(o4.y, 4);
    EXPECT_EQ(o4.z, 1);
    EXPECT_EQ(o4.w, 2);

    static_assert(!glsl_has_field_z<tvec<2, int>>::value);
    static_assert(glsl_has_field_z<tvec<3, int>>::value);
    static_assert(!glsl_has_call_xyzwx<tvec<4, int>>::value);

    vec4i ch{1, 2, 3, 4};
#if EIRIN_VEC_SWIZZLE_CHAIN_AS_LVALUE == EIRIN_ENABLE
    ch.xy().yx() = tvec<2, int>(7, 8); // y=7, x=8
    EXPECT_EQ(ch.x, 8);
    EXPECT_EQ(ch.y, 7);
    EXPECT_EQ(ch.z, 3);
    EXPECT_EQ(ch.w, 4);
    ch.xy().xy() = ch.yx(); // 链式快照：交换 x/y
    EXPECT_EQ(ch.x, 7);
    EXPECT_EQ(ch.y, 8);
#else
    EXPECT_EQ(ch.xy().yx().x(), 2);
    EXPECT_EQ(ch.xy().yx().y(), 1);
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ASSIGN(ch.xy().yx(), (tvec<2, int>(9, 9))));
    EXPECT_FALSE(EIRIN_TESTING_COMPILE_ADD_ASSIGN(ch.xy().yx(), 1));
#endif
}

// ==================== Common usage Test ====================
TEST(Vec, CommonUsage)
{
    tvec<2, int> v(1, 2);
    v[0] = 5;
    EXPECT_EQ(v.x, 5);

    v.x = 10;
    EXPECT_EQ(v.x, 10);
    v.y = 20;
    EXPECT_EQ(v.y, 20);

    v.xy() = tvec<2, int>(30, 40);
    EXPECT_EQ(v.x, 30);
    EXPECT_EQ(v.y, 40);

    v = v + 10;
    EXPECT_EQ(v.x, 40);
    EXPECT_EQ(v.y, 50);

    v += 5;
    EXPECT_EQ(v.x, 45);
    EXPECT_EQ(v.y, 55);
}
