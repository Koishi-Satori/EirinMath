#include <gtest/gtest.h>
#include <eirin/eirin.hpp>
#include <eirin/io/format.hpp>
#include <eirin/ext/cordic.hpp>
#include <eirin/detail/util.hpp>
#include <eirin/detail/perf.hpp>

#ifdef EIRIN_DEV_TEST_MODE
#    include <eirin/ext/simd_math.hpp>
#endif

using namespace eirin;

TEST(FixedNum, Construct)
{
    auto fp1 = 0_f32;
    EXPECT_EQ((int)fp1, 0);
    io::parse("-114.514a", "a", fp1);
    EXPECT_EQ(fp1, -114.514_f32);

#ifdef EIRIN_MATH_HAS_INT128
    auto fp2 = 0_f64;
    EXPECT_EQ((int)fp2, 0);
    EXPECT_EQ("-114.514"_f64, -114.514_f64);
#endif
}

TEST(Fixed32, Operator)
{
    auto fp1 = 0_f32;
    EXPECT_EQ(++fp1, 1_f32);
    fp1 = 1.14_f32;
    EXPECT_EQ(fp1 + 5.14_f32, 6.28_f32);
    EXPECT_EQ(fp1 * 2, 2.28_f32);
    EXPECT_EQ(514_f32 / 2, 257_f32);
    auto fp2 = 514_f32;
    EXPECT_EQ(fp2.divide(2), 257_f32);
    EXPECT_EQ(515_f32 % 2_f32, 1_f32);
    EXPECT_EQ(--fp1, 0.14_f32);

#ifndef EIRIN_NO_EXCEPTIONS

    EXPECT_THROW(
        (void)(fp2.divide(0)),
        divide_by_zero
    );

#endif
}

TEST(Fixed32, Rounding)
{
    EXPECT_EQ(round(114.414_f32), 114_f32);
    EXPECT_EQ(round(114.514_f32), 115_f32);
    EXPECT_EQ(round(-114.414_f32), -114_f32);
    EXPECT_EQ(round(-114.514_f32), -115_f32);
}

TEST(Fixed32, Decompression)
{
    EXPECT_FALSE(signbit(0_f32));
    EXPECT_FALSE(signbit(1_f32));
    EXPECT_TRUE(signbit(-1_f32));

    EXPECT_EQ((1_f32).raw_integral_part(), 1);
    EXPECT_EQ((-1_f32).raw_integral_part(), 32767);

    EXPECT_EQ((1_f32).integral_part(), 1);
    EXPECT_EQ((-1_f32).integral_part(), 1);

    EXPECT_EQ((1_f32).fractional_part(), 0);
    EXPECT_EQ((-1_f32).fractional_part(), 0);

    {
        auto val = 1_f32 / 2;
        EXPECT_EQ(val.integral_part(), 0);
        EXPECT_EQ(val.fractional_part(), 1ULL << (val.precision - 1));
    }
}

namespace test_math
{
static constexpr eirin::fixed32 arc_triangle_max_error = 0.0003_f32;
static constexpr eirin::fixed64 arc_triangle_max_error_64 = 0.0003_f64;

static testing::AssertionResult expect_fixed_eq(
    const eirin::fixed32& lhs,
    const eirin::fixed32& rhs,
    const eirin::fixed32& eps = fixed32::nearly_compare_epsilon()
)
{
    auto diff = abs(lhs - rhs);
    if(diff > eps)
        return testing::AssertionFailure()
               << lhs << " != " << rhs << ", diff: " << diff
               << ", Internal values: " << lhs.internal_value() << ", " << rhs.internal_value();
    else
        return testing::AssertionSuccess();
}

#ifdef EIRIN_MATH_HAS_INT128
static testing::AssertionResult expect_fixed_eq(
    const eirin::fixed64& lhs,
    const eirin::fixed64& rhs,
    const eirin::fixed64& eps = fixed64::nearly_compare_epsilon()
)
{
    auto diff = abs(lhs - rhs);
    if(diff > eps)
        return testing::AssertionFailure()
               << lhs << " != " << rhs << ", diff: " << diff
               << ", Internal values: " << lhs.internal_value() << ", " << rhs.internal_value();
    else
        return testing::AssertionSuccess();
}
#endif
} // namespace test_math

TEST(Fixed32, Math)
{
    using test_math::expect_fixed_eq;

    // 1 bit for 1, 16 bits for fraction.
    EXPECT_EQ((1_f32).bit_width(), 17);
    EXPECT_EQ((2_f32).bit_width(), 18);
    EXPECT_EQ((4_f32).bit_width(), 19);
    // 1 bit for 1, 16 bits for fraction.
    EXPECT_EQ((-1_f32).bit_width(), 17);
    EXPECT_EQ((-2_f32).bit_width(), 18);
    EXPECT_EQ((-4_f32).bit_width(), 19);

    EXPECT_EQ(abs(-114.514_f32), 114.514_f32);
    EXPECT_EQ(sin(0_f32), 0_f32);
    EXPECT_TRUE(expect_fixed_eq(sin(1_f32), 0.841471_f32));
    EXPECT_TRUE(expect_fixed_eq(sin(fixed32::pi() / 6), 0.5_f32));
    EXPECT_EQ(detail::eval_integral_part_max_digits10<fixed32>(), 5); // max value of fixed32 is 32767.
    EXPECT_EQ(detail::eval_integral_part_max_digits10<fixed64>(), 10); // max value of fixed64 is 0X7FFFFFFF.
    EXPECT_EQ(cos(0_f32), 1_f32);
    EXPECT_TRUE(expect_fixed_eq(cos(fixed32::pi() / 3), 0.5_f32));
    EXPECT_TRUE(expect_fixed_eq(cos(1_f32), 0.540302_f32));
    EXPECT_EQ(tan(0_f32), 0_f32);
    EXPECT_TRUE(expect_fixed_eq(tan(1_f32), 1.557407_f32));
    EXPECT_TRUE(expect_fixed_eq(atan(0_f32), 0_f32, test_math::arc_triangle_max_error));
    EXPECT_TRUE(expect_fixed_eq(atan(fixed32::pi() / 6), 0.482348_f32, test_math::arc_triangle_max_error));
    EXPECT_TRUE(expect_fixed_eq(atan(1_f32), 0.785398_f32, test_math::arc_triangle_max_error));
    EXPECT_TRUE(expect_fixed_eq(asin(0_f32), 0_f32, test_math::arc_triangle_max_error));
    EXPECT_TRUE(expect_fixed_eq(asin(0.5_f32), 0.523598_f32, test_math::arc_triangle_max_error));
    EXPECT_TRUE(expect_fixed_eq(acos(0_f32), 1.570796_f32, test_math::arc_triangle_max_error));
    EXPECT_TRUE(expect_fixed_eq(acos(0.5_f32), 1.047197_f32, test_math::arc_triangle_max_error));
    EXPECT_EQ(sqrt(0_f32), 0_f32);
    EXPECT_EQ(sqrt(4_f32), 2_f32);
    EXPECT_EQ(sqrt(114.514_f32), 10.701121_f32);
    EXPECT_TRUE(expect_fixed_eq(cbrt(0_f32), 0_f32));
    EXPECT_EQ(cbrt(1_f32), 1_f32);
    EXPECT_EQ(cbrt(8_f32), 2_f32);
    EXPECT_EQ(cbrt(27_f32), 3_f32);
    EXPECT_EQ(log2(2_f32), 1_f32);
    EXPECT_TRUE(expect_fixed_eq(log2(10_f32), 3.321928_f32));
    EXPECT_TRUE(expect_fixed_eq(log(fixed32::e()), 1_f32));
    EXPECT_TRUE(expect_fixed_eq(log(114.514_f32), 4.740697_f32));
    EXPECT_TRUE(expect_fixed_eq(log10(10_f32), 1_f32));
    EXPECT_TRUE(expect_fixed_eq(log10(114.514_f32), 2.05885858494_f32));
    EXPECT_TRUE(expect_fixed_eq(log10(114.514_f64), 2.05885858494_f64));
    EXPECT_TRUE(expect_fixed_eq(exp(1_f32), fixed32::e()));
    EXPECT_TRUE(expect_fixed_eq(radians(180_f32), numbers::pi));
    EXPECT_TRUE(expect_fixed_eq(degrees(numbers::pi), 180_f32));
}

TEST(FixedNum, Constants)
{
    GTEST_LOG_(INFO) << "fixed32 max value: " << max_value<fixed32>() << ", min value: " << min_value<fixed32>();
    GTEST_LOG_(INFO) << "fixed32 e value: " << fixed32::e() << ", pi value: " << fixed32::pi();
    GTEST_LOG_(INFO) << "fixed32 log2_e value: " << numbers::log2e_v<fixed32>();
    GTEST_LOG_(INFO) << "fixed32 log10_e value: " << numbers::log10e_v<fixed32>();
    GTEST_LOG_(INFO) << "fixed32 inv_pi value: " << numbers::inv_pi_v<fixed32>();
    GTEST_LOG_(INFO) << "fixed32 inv_sqrtpi value: " << numbers::inv_sqrtpi_v<fixed32>();
    GTEST_LOG_(INFO) << "fixed32 ln2 value: " << numbers::ln2_v<fixed32>();
    GTEST_LOG_(INFO) << "fixed32 ln10 value: " << numbers::ln10_v<fixed32>();
    GTEST_LOG_(INFO) << "fixed32 sqrt2 value: " << numbers::sqrt2_v<fixed32>();
    GTEST_LOG_(INFO) << "fixed32 sqrt3 value: " << numbers::sqrt3_v<fixed32>();
    GTEST_LOG_(INFO) << "fixed32 inv_sqrt3 value: " << numbers::inv_sqrt3_v<fixed32>();
    GTEST_LOG_(INFO) << "fixed32 egamma value: " << numbers::egamma_v<fixed32>();
    GTEST_LOG_(INFO) << "fixed32 phi value: " << numbers::phi_v<fixed32>();

#ifdef EIRIN_MATH_HAS_INT128
    // fixed64
    GTEST_LOG_(INFO) << "fixed64 max value: " << max_value<fixed64>() << ", min value: " << min_value<fixed64>();
    GTEST_LOG_(INFO) << "fixed64 e value: " << fixed64::e() << ", pi value: " << fixed64::pi();
    GTEST_LOG_(INFO) << "fixed64 log2_e value: " << numbers::log2e_v<fixed64>();
    GTEST_LOG_(INFO) << "fixed64 log10_e value: " << numbers::log10e_v<fixed64>();
    GTEST_LOG_(INFO) << "fixed64 inv_pi value: " << numbers::inv_pi_v<fixed64>();
    GTEST_LOG_(INFO) << "fixed64 inv_sqrtpi value: " << numbers::inv_sqrtpi_v<fixed64>();
    GTEST_LOG_(INFO) << "fixed64 ln2 value: " << numbers::ln2_v<fixed64>();
    GTEST_LOG_(INFO) << "fixed64 ln10 value: " << numbers::ln10_v<fixed64>();
    GTEST_LOG_(INFO) << "fixed64 sqrt2 value: " << numbers::sqrt2_v<fixed64>();
    GTEST_LOG_(INFO) << "fixed64 sqrt3 value: " << numbers::sqrt3_v<fixed64>();
    GTEST_LOG_(INFO) << "fixed64 inv_sqrt3 value: " << numbers::inv_sqrt3_v<fixed64>();
    GTEST_LOG_(INFO) << "fixed64 egamma value: " << numbers::egamma_v<fixed64>();
    GTEST_LOG_(INFO) << "fixed64 phi value: " << numbers::phi_v<fixed64>();
#endif
}

TEST(FixedNum, Random)
{
    eirin::random_device rd;
    eirin::pcg2014 pcg_32(rd());
    eirin::mt19937 mt_32(rd());
    auto tmp_mt_int = std::uniform_int_distribution<>()(mt_32);
    // avoid C4834 on MSVC and unused variable.
    (void)tmp_mt_int;
    eirin::fixed_int_distribution_adapter<fixed32, std::uniform_int_distribution<>> dist_32;
    std::array<fixed32, 10> values_32;
    auto test_random = [&](auto& engine, auto& dist, auto& values, const char* label)
    {
        std::generate(values.begin(), values.end(), [&]()
                      { return dist(engine); });
        GTEST_LOG_(INFO) << label << " Random distribution in [" << dist.min() << ", " << dist.max() << "]: ";
        std::cout << "  { ";
        for(size_t i = 0; i < values.size() - 1; ++i)
        {
            std::cout << values[i] << ", ";
        }
        std::cout << values[values.size() - 1] << " }" << std::endl;
    };

    test_random(pcg_32, dist_32, values_32, "Fixed32 PCG2014");
    test_random(mt_32, dist_32, values_32, "Fixed32 MT19937");

    fixed_random_engine_adapter<fixed32, eirin::mt19937> test_wrapper;
    fixed_distribution_adapter<fixed32, std::uniform_int_distribution<>> test_dist_wrapper;
    test_random(test_wrapper, test_dist_wrapper, values_32, "Fixed32 Wrapped MT19937");

#ifdef EIRIN_MATH_HAS_INT128
    eirin::pcg2014_64 pcg_64(rd());
    eirin::mt19937_64 mt_64(rd());
    eirin::fixed_int_distribution_adapter<fixed64, std::uniform_int_distribution<>> dist_64;
    std::array<fixed64, 10> values_64;
    test_random(pcg_64, dist_64, values_64, "Fixed64 PCG2014");
    test_random(mt_64, dist_64, values_64, "Fixed64 MT19937");
#endif
}

#ifdef EIRIN_MATH_HAS_INT128
TEST(Fixed64, Operator)
{
    auto fp1 = 0_f64;
    EXPECT_EQ(++fp1, 1_f64);
    fp1 = 1.14_f64;
    EXPECT_EQ(fp1 + 5.14_f64, 6.28_f64);
    EXPECT_EQ(fp1 * 2, 2.28_f64);
    EXPECT_EQ(514_f64 / 2, 257_f64);
    auto fp2 = 514_f64;
    EXPECT_EQ(fp2.divide(2), 257_f64);
    EXPECT_EQ(515_f64 % 2_f64, 1_f64);
    EXPECT_EQ(--fp1, 0.14_f64);

#    ifndef EIRIN_NO_EXCEPTIONS
    EXPECT_THROW(
        (void)(fp2.divide(0)),
        divide_by_zero
    );
#    endif
}

TEST(Fixed64, Rounding)
{
    EXPECT_EQ(round(114.414_f64), 114_f64);
    EXPECT_EQ(round(114.514_f64), 115_f64);
    EXPECT_EQ(round(-114.414_f64), -114_f64);
    EXPECT_EQ(round(-114.514_f64), -115_f64);
}

TEST(Fixed64, Decompression)
{
    EXPECT_FALSE(signbit(0_f64));
    EXPECT_FALSE(signbit(1_f64));
    EXPECT_TRUE(signbit(-1_f64));

    EXPECT_EQ((1_f64).raw_integral_part(), 1);
    EXPECT_EQ((-1_f64).raw_integral_part(), 2147483647);

    EXPECT_EQ((1_f64).integral_part(), 1);
    EXPECT_EQ((-1_f64).integral_part(), 1);

    EXPECT_EQ((1_f64).fractional_part(), 0);
    EXPECT_EQ((-1_f64).fractional_part(), 0);

    {
        auto val = 1_f64 / 2;
        EXPECT_EQ(val.integral_part(), 0);
        EXPECT_EQ(val.fractional_part(), 1ULL << (val.precision - 1));
    }
}

TEST(Fixed64, Math)
{
    using test_math::expect_fixed_eq;

    EXPECT_EQ(abs(-114.514_f64), 114.514_f64);
    EXPECT_EQ(sin(0_f64), 0_f64);
    EXPECT_TRUE(expect_fixed_eq(sin(1_f64), 0.841471_f64));
    EXPECT_TRUE(expect_fixed_eq(sin(fixed64::pi() / 6), 0.5_f64));
    EXPECT_TRUE(expect_fixed_eq(cos(0_f64), 1_f64));
    EXPECT_TRUE(expect_fixed_eq(cos(fixed64::pi() / 3), 0.5_f64));
    EXPECT_TRUE(expect_fixed_eq(cos(1_f64), 0.540302_f64));
    EXPECT_EQ(tan(0_f64), 0_f64);
    EXPECT_TRUE(expect_fixed_eq(tan(1_f64), 1.557407_f64));
    EXPECT_TRUE(expect_fixed_eq(atan(0_f64), 0_f64, test_math::arc_triangle_max_error_64));
    EXPECT_TRUE(expect_fixed_eq(atan(fixed64::pi() / 6), 0.482348_f64, test_math::arc_triangle_max_error_64));
    EXPECT_TRUE(expect_fixed_eq(atan(1_f64), 0.785398_f64, test_math::arc_triangle_max_error_64));
    EXPECT_TRUE(expect_fixed_eq(asin(0_f64), 0_f64, test_math::arc_triangle_max_error_64));
    EXPECT_TRUE(expect_fixed_eq(asin(0.5_f64), 0.523598_f64, test_math::arc_triangle_max_error_64));
    EXPECT_TRUE(expect_fixed_eq(acos(0_f64), 1.570796_f64, test_math::arc_triangle_max_error_64));
    EXPECT_TRUE(expect_fixed_eq(acos(0.5_f64), 1.047197_f64, test_math::arc_triangle_max_error_64));
    EXPECT_TRUE(expect_fixed_eq(sqrt(0_f64), 0_f64));
    EXPECT_TRUE(expect_fixed_eq(sqrt(4_f64), 2_f64));
    EXPECT_TRUE(expect_fixed_eq(sqrt(114.514_f64), 10.701121_f64));
    EXPECT_TRUE(expect_fixed_eq(cbrt(0_f64), 0_f64));
    EXPECT_TRUE(expect_fixed_eq(cbrt(1_f64), 1_f64));
    EXPECT_TRUE(expect_fixed_eq(cbrt(8_f64), 2_f64));
    EXPECT_TRUE(expect_fixed_eq(cbrt(27_f64), 3_f64));
    EXPECT_TRUE(expect_fixed_eq(log2(2_f64), 1_f64));
    EXPECT_TRUE(expect_fixed_eq(log2(10_f64), 3.321928_f64));
    EXPECT_TRUE(expect_fixed_eq(log(fixed64::e()), 1_f64));
    EXPECT_TRUE(expect_fixed_eq(log(114.514_f64), 4.740697_f64));
    EXPECT_TRUE(expect_fixed_eq(log10(10_f64), 1_f64));
    EXPECT_TRUE(expect_fixed_eq(log10(114.514_f64), 2.058859_f64));
    EXPECT_TRUE(expect_fixed_eq(exp(1_f64), fixed64::e()));
    EXPECT_EQ(radians(180_f64), numbers::pi_f64);
    EXPECT_EQ(degrees(numbers::pi_f64), 180_f64);
    EXPECT_EQ(radians(90_f64), numbers::pi_f64 / 2);
    EXPECT_EQ(degrees(numbers::pi_f64 / 2), 90_f64);
}
#endif

int main(int argc, char* argv[])
{
#ifdef EIRIN_NO_EXCEPTIONS
    std::cerr << "EIRIN_NO_EXCEPTIONS defined" << std::endl;
#endif
#ifdef EIRIN_MATH_DETAIL_INT128_MSVC_STL
    std::cerr << "EIRIN_MATH_DETAIL_INT128_MSVC_STL defined" << std::endl;
#endif
#ifdef EIRIN_MATH_DETAIL_BUILTIN__INT128
    std::cerr << "EIRIN_MATH_DETAIL_BUILTIN__INT128 defined" << std::endl;
#endif
#ifdef EIRIN_HAS_LIB_FORMAT
    std::cerr << "EIRIN_HAS_LIB_FORMAT defined" << std::endl;
#endif

    std::cout << 114.5625_f32 << std::endl;
    std::cout << -114.5625_f32 << std::endl;

    std::cout << cordic_sine(eirin::numbers::pi_f64 / 4) << std::endl;
    std::cout << sin(eirin::numbers::pi_f64 / 4) << std::endl;

    // measure the precision of atan, with steps of 0.01 from sin(-pi/4)/cos(-pi/4) to sin(pi/4)/cos(pi/4).
    fixed64 angle = eirin::numbers::pi_f64 / 4;
    constexpr fixed64 step = 0.01_f64;
    fixed64 max_esp, min_esp, max_angle, min_angle;
    max_esp = 0_f64, min_esp = 1_f64, max_angle = 0_f64, min_angle = 0_f64;
    for(fixed64 x = -angle; x <= angle; x += step)
    {
        auto sin_val = sin(x);
        auto cos_val = cos(x);
        if(cos_val == 0_f64)
            continue;
        auto atan_val = atan(sin_val / cos_val);
        auto esp = abs(atan_val - x);
        if(esp > max_esp)
        {
            max_esp = esp;
            max_angle = x;
        }
        if(esp < min_esp)
        {
            min_esp = esp;
            min_angle = x;
        }
    }

    {
        fixed64 angle = eirin::numbers::pi_f64 / 4;
        auto fp_sin = [](fixed64 x)
        {
            return sin(x);
        };
        auto std_sin = [](fixed64 x)
        {
            return fixed64(sin((double)x));
        };
        auto initializer = [](int64_t x)
        {
            return fixed64(x);
        };
        fixed64 max_esp, max_angle, min_esp, min_angle;
        auto ret = perf::measure_esp<fixed64>(-angle, angle, step, fp_sin, std_sin, initializer);
        max_esp = ret.max_esp;
        max_angle = ret.max_esp_input;
        min_esp = ret.min_esp;
        min_angle = ret.min_esp_input;
    }

    max_esp = 0_f64, min_esp = 1_f64, max_angle = 0_f64, min_angle = 0_f64;
    for(fixed64 x = -angle; x <= angle; x += step)
    {
        auto cos_val = cos(x);
        auto std_cos_val = fixed64(cos((double)x));
        auto esp = abs(std_cos_val - cos_val);
        if(esp > max_esp)
        {
            max_esp = esp;
            max_angle = x;
        }
        if(esp < min_esp)
        {
            min_esp = esp;
            min_angle = x;
        }
    }
    std::unordered_map<fixed64, fixed64, fixed_hash<fixed64>> test_map;

#ifdef EIRIN_DEV_TEST_MODE
    // SIM TESTS
    using namespace eirin::numbers;
    std::array input = {pi_f64, pi_f64 + pi_f64 / 2, pi_f64 / 2 - pi_f64, 2 * pi_f64 + pi_f64};
    auto simd_res = simd::simd_reduce_angle(input);
    std::cout << "simd_reduce_angle res: [";
    for(size_t i = 0; i < 3; ++i)
        std::cout << simd_res[i] << ',';
    std::cout << simd_res[3] << ']' << std::endl;
    // std::cout << fixed64::template from_fixed_num_value<61>(0x13A92A0000000) << std::endl;
#endif

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
