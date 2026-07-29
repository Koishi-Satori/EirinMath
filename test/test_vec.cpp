#include <gtest/gtest.h>
#include <eirin/eirin.hpp>

TEST(Fixed32, vec2)
{
    using namespace eirin::literals;
    eirin::tvec<2, eirin::fixed32> v1{11, 11}, v2{"11"_f32, "11"_f32};
    auto v = v1 + v2 + 1_f32;
    v += eirin::vec2<int>{1};
    eirin::vec2<eirin::fixed32> v_cmp{24, 24};
    // eirin::vec2<bool> true_vec2{true, true};
    EXPECT_EQ(v1, v2);
    EXPECT_EQ(v, v_cmp);
    EXPECT_NE(v, v1);
    EXPECT_TRUE((v1 && v2).x);
}
