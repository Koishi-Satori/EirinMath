#include <gtest/gtest.h>
#include <eirin/eirin.hpp>

TEST(Fixed32, vec2)
{
    using namespace eirin::literals;
    eirin::tvec<2, eirin::fixed32> v1{11, 11}, v2{"1"_f32, "1"_f32};
    v1 + v2;
}