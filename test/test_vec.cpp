#include <gtest/gtest.h>
#include <eirin/eirin.hpp>
#include <eirin/io/format.hpp>

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
}

TEST(Vec, SwizzleWriteDuplicateCompileCheck)
{
    // These should compile failed.
    /*
    tvec<2, int> v(1,2);
    v.xx() = tvec<2, int>(3,4);
    v.xx() += 1;
    ++v.xx();
    */
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
