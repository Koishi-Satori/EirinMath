#include <random>
#include <vector>
#include <utility>
#include <eirin/fixed.hpp>
#include <eirin/math.hpp>
#include <eirin/numeric.hpp>
#include <benchmark/benchmark.h>
#include <eirin/ext/cordic.hpp>
#include <eirin/ext/builtin_ints.hpp>
#include <bench.hpp>

// on windows/msvc, -Wmaybe-uninitialized is not available
// so we can use #pragma to ignore the warning there
#ifdef _MSC_VER
// save warning levels, and drop it to level 3
#    pragma warning(push, 3)
// turn two warnings off
#    pragma warning(disable : 4701 4703)
#    include <__msvc_int128.hpp>
#endif

using namespace eirin;

#if defined(EIRIN_OS_WINDOWS) || defined(EIRIN_OS_LINUX) || defined(EIRIN_OS_MACOS)
static void eirin_ext_int128_mul(benchmark::State& state)
{
    std::vector<std::pair<ext::int128, ext::int128>> vec(1'000'000);
    std::mt19937_64 mt64;
    for(auto& [val1, val2] : vec)
    {
        val1 = ext::int128(mt64(), mt64());
        val2 = ext::int128(mt64(), mt64());
    }
    auto it = vec.begin();
    for(auto _ : state)
    {
        auto result = it->first * it->second;
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
        if(++it == vec.end())
            it = vec.begin();
    }
}
#endif

template <typename Fixed>
static std::vector<Fixed> make_random_cbrt_set(std::size_t n)
{
    std::vector<Fixed> vec(n);
    std::mt19937_64 mt64(0x114514);
    using T = typename Fixed::value_type;
    for(auto& v : vec)
        v = Fixed::from_internal_value(static_cast<T>(mt64()));
    return vec;
}

template <typename Fixed>
static std::vector<Fixed> make_random_fixed_set(std::size_t n, double lo, double hi, uint64_t seed = 0x114514)
{
    std::vector<Fixed> vec(n);
    std::mt19937_64 mt64(seed);
    std::uniform_real_distribution<double> dist(lo, hi);
    for(auto& v : vec)
        v = Fixed(dist(mt64));
    return vec;
}

template <typename Fixed, typename MathFunc>
static void bench_random_1(benchmark::State& state, MathFunc func, double lo, double hi)
{
    const auto vec = make_random_fixed_set<Fixed>(state.range(0), lo, hi);
    std::size_t i = 0;
    for(auto _ : state)
    {
        auto input = vec[i++ % vec.size()];
        benchmark::DoNotOptimize(input);
        auto result = func(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

template <typename Fixed, typename MathFunc>
static void bench_random_2(benchmark::State& state, MathFunc func, double lo1, double hi1, double lo2, double hi2)
{
    const auto a = make_random_fixed_set<Fixed>(state.range(0), lo1, hi1, 0x114514);
    const auto b = make_random_fixed_set<Fixed>(state.range(0), lo2, hi2, 0x1919810);
    std::size_t i = 0;
    for(auto _ : state)
    {
        auto x = a[i % a.size()];
        auto y = b[i % b.size()];
        ++i;
        benchmark::DoNotOptimize(x);
        benchmark::DoNotOptimize(y);
        auto result = func(x, y);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

// Values near ±max, so saturating arithmetic saturates on almost every pair.
template <typename Fixed>
static std::vector<Fixed> make_saturating_set(std::size_t n)
{
    std::vector<Fixed> vec(n);
    std::mt19937_64 mt64(0xBEAD);
    const double m = static_cast<double>(eirin::max_value<Fixed>());
    std::uniform_real_distribution<double> dist(0.9 * m, m);
    for(auto& v : vec)
        v = (mt64() & 1) ? Fixed(dist(mt64)) : Fixed(-dist(mt64));
    return vec;
}

template <typename Fixed, typename Op>
static void bench_sat(benchmark::State& state, bool boundary, Op op)
{
    const auto a = boundary ? make_saturating_set<Fixed>(state.range(0))
                            : make_random_fixed_set<Fixed>(state.range(0), -15000, 15000, 0x114514);
    const auto b = boundary ? make_saturating_set<Fixed>(state.range(0))
                            : make_random_fixed_set<Fixed>(state.range(0), -15000, 15000, 0x1919810);
    std::size_t i = 0;
    for(auto _ : state)
    {
        auto x = a[i % a.size()];
        auto y = b[i % b.size()];
        ++i;
        benchmark::DoNotOptimize(x);
        benchmark::DoNotOptimize(y);
        auto result = op(x, y);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_cbrt(benchmark::State& state)
{
    const auto vec = make_random_cbrt_set<fixed32>(state.range(0));
    std::size_t i = 0;
    for(auto _ : state)
    {
        auto input = vec[i++ % vec.size()];
        benchmark::DoNotOptimize(input);
        auto result = cbrt(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_cbrt(benchmark::State& state)
{
    std::vector<double> vec(state.range(0));
    std::mt19937_64 mt64(0xC0FFEE);
    for(auto& v : vec)
        v = static_cast<double>(static_cast<std::int64_t>(mt64())) / 4294967296.0;
    std::size_t i = 0;
    for(auto _ : state)
    {
        auto input = vec[i++ % vec.size()];
        benchmark::DoNotOptimize(input);
        auto result = std::cbrt(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

#ifdef EIRIN_MATH_HAS_INT128
static void f64_cbrt(benchmark::State& state)
{
    const auto vec = make_random_cbrt_set<fixed64>(state.range(0));
    std::size_t i = 0;
    for(auto _ : state)
    {
        auto input = vec[i++ % vec.size()];
        benchmark::DoNotOptimize(input);
        auto result = cbrt(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}
#endif

#ifdef _MSC_VER
static void msvc_int128_mul(benchmark::State& state)
{
    using int128 = std::_Signed128;
    std::vector<std::pair<int128, int128>> vec(1'000'000);
    std::mt19937_64 mt64;
    for(auto& [val1, val2] : vec)
    {
        val1._Word[0] = mt64();
        val1._Word[1] = mt64();
        val2._Word[0] = mt64();
        val2._Word[1] = mt64();
    }
    auto it = vec.begin();
    for(auto _ : state)
    {
        auto result = it->first * it->second;
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
        if(++it == vec.end())
            it = vec.begin();
    }
}
#endif

static void f32_create(benchmark::State& state)
{
    std::vector<double> vec(state.range(0));
    std::mt19937_64 mt64(0xC0FFEE);
    std::uniform_real_distribution<double> dist(-30000, 30000);
    for(auto& v : vec)
        v = dist(mt64);
    std::size_t i = 0;
    for(auto _ : state)
    {
        auto input = vec[i++ % vec.size()];
        benchmark::DoNotOptimize(input);
        auto result = fixed32(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_divide(benchmark::State& state)
{
    bench_random_2<fixed32>(state, [](fixed32 x, fixed32 y)
                            { return x / y; },
                            -30000,
                            30000,
                            1,
                            30000);
}

static void f32_multiple(benchmark::State& state)
{
    bench_random_2<fixed32>(state, [](fixed32 x, fixed32 y)
                            { return x * y; },
                            -150,
                            150,
                            -150,
                            150);
}

static void f32_add(benchmark::State& state)
{
    bench_random_2<fixed32>(state, [](fixed32 x, fixed32 y)
                            { return x + y; },
                            -15000,
                            15000,
                            -15000,
                            15000);
}

static void f32_minus(benchmark::State& state)
{
    bench_random_2<fixed32>(state, [](fixed32 x, fixed32 y)
                            { return x - y; },
                            -15000,
                            15000,
                            -15000,
                            15000);
}

static void f32_add_sat(benchmark::State& state)
{
    bench_sat<fixed32>(state, false, [](fixed32 x, fixed32 y)
                       { return saturating_add(x, y); });
}

static void f32_add_sat_boundary(benchmark::State& state)
{
    bench_sat<fixed32>(state, true, [](fixed32 x, fixed32 y)
                       { return saturating_add(x, y); });
}

static void f32_sub_sat(benchmark::State& state)
{
    bench_sat<fixed32>(state, false, [](fixed32 x, fixed32 y)
                       { return saturating_sub(x, y); });
}

static void f32_mul_sat(benchmark::State& state)
{
    bench_sat<fixed32>(state, false, [](fixed32 x, fixed32 y)
                       { return saturating_mul(x, y); });
}

static void f32_mul_sat_boundary(benchmark::State& state)
{
    bench_sat<fixed32>(state, true, [](fixed32 x, fixed32 y)
                       { return saturating_mul(x, y); });
}

static void f32_div_sat(benchmark::State& state)
{
    // divisor range excludes 0 (saturating_div by zero is UB) and spans both
    // |y| >= 1 (fast path) and |y| < 1 (amplifying path).
    bench_random_2<fixed32>(state, [](fixed32 x, fixed32 y)
                            { return saturating_div(x, y); },
                            -30000,
                            30000,
                            0.001,
                            30000);
}

static void f32_modwarp_add(benchmark::State& state)
{
    bench_sat<fixed32>(state, false, [](fixed32 x, fixed32 y)
                       { return modwarp_add(x, y); });
}

static void f32_modwarp_sub(benchmark::State& state)
{
    bench_sat<fixed32>(state, false, [](fixed32 x, fixed32 y)
                       { return modwarp_sub(x, y); });
}

static void f32_modwarp_mul(benchmark::State& state)
{
    bench_sat<fixed32>(state, false, [](fixed32 x, fixed32 y)
                       { return modwarp_mul(x, y); });
}

static void f32_modwarp_div(benchmark::State& state)
{
    // divisor range excludes 0 (modwarp_div by zero is UB).
    bench_random_2<fixed32>(state, [](fixed32 x, fixed32 y)
                            { return modwarp_div(x, y); },
                            -30000,
                            30000,
                            0.001,
                            30000);
}

static void f32_satcast_s2u(benchmark::State& state)
{
    using uq16 = fixed_num<std::uint32_t, std::uint64_t, 16, false>;
    bench_random_1<fixed32>(state, [](fixed32 x)
                            { return saturating_cast<uq16>(x); },
                            -30000,
                            30000);
}

static void f32_satcast_u2s(benchmark::State& state)
{
    using uq16 = fixed_num<std::uint32_t, std::uint64_t, 16, false>;
    bench_random_1<uq16>(state, [](uq16 x)
                         { return saturating_cast<fixed32>(x); },
                         0,
                         30000);
}

#ifdef EIRIN_MATH_HAS_INT128
static void f32_satcast_wide(benchmark::State& state)
{
    using wide16 = fixed_num<std::int64_t, detail::int128_t, 16, false>;
    bench_random_1<fixed32>(state, [](fixed32 x)
                            { return saturating_cast<wide16>(x); },
                            -30000,
                            30000);
}

static void f32_satcast_narrow(benchmark::State& state)
{
    using wide16 = fixed_num<std::int64_t, detail::int128_t, 16, false>;
    bench_random_1<wide16>(state, [](wide16 x)
                           { return saturating_cast<fixed32>(x); },
                           -30000,
                           30000);
}

static void f32_satcast_up(benchmark::State& state)
{
    bench_random_1<fixed32>(state, [](fixed32 x)
                            { return saturating_cast<fixed64>(x); },
                            -30000,
                            30000);
}

static void f32_satcast_down(benchmark::State& state)
{
    bench_random_1<fixed64>(state, [](fixed64 x)
                            { return saturating_cast<fixed32>(x); },
                            -30000,
                            30000);
}
#endif

static void f32_sqrt(benchmark::State& state)
{
    bench_random_1<fixed32>(state, [](fixed32 x)
                            { return sqrt(x); },
                            0,
                            30000);
}

static void f32_log2(benchmark::State& state)
{
    bench_random_1<fixed32>(state, [](fixed32 x)
                            { return log2(x); },
                            0.001,
                            30000);
}

static void f32_log(benchmark::State& state)
{
    bench_random_1<fixed32>(state, [](fixed32 x)
                            { return log(x); },
                            0.001,
                            30000);
}

static void f32_log10(benchmark::State& state)
{
    bench_random_1<fixed32>(state, [](fixed32 x)
                            { return log10(x); },
                            0.001,
                            30000);
}

static void f32_exp(benchmark::State& state)
{
    bench_random_1<fixed32>(state, [](fixed32 x)
                            { return exp(x); },
                            -5,
                            5);
}

static void f32_pow(benchmark::State& state)
{
    bench_random_2<fixed32>(state, [](fixed32 x, fixed32 y)
                            { return pow(x, y); },
                            0.5,
                            50,
                            -2,
                            2);
}

static void f32_pow_fast(benchmark::State& state)
{
    bench_random_2<fixed32>(state, [](fixed32 x, fixed32 y)
                            { return pow(x, y); },
                            0.5,
                            50,
                            -2,
                            2);
}

static void f32_sin(benchmark::State& state)
{
    bench_random_1<fixed32>(state, [](fixed32 x)
                            { return sin(x); },
                            -10,
                            10);
}

static void f32_cos(benchmark::State& state)
{
    bench_random_1<fixed32>(state, [](fixed32 x)
                            { return cos(x); },
                            -10,
                            10);
}

static void f32_tan(benchmark::State& state)
{
    bench_random_1<fixed32>(state, [](fixed32 x)
                            { return tan(x); },
                            -1.5,
                            1.5);
}

static void f32_atan(benchmark::State& state)
{
    bench_random_1<fixed32>(state, [](fixed32 x)
                            { return atan(x); },
                            -30000,
                            30000);
}

static void f32_asin(benchmark::State& state)
{
    bench_random_1<fixed32>(state, [](fixed32 x)
                            { return asin(x); },
                            -1,
                            1);
}

static void f32_acos(benchmark::State& state)
{
    bench_random_1<fixed32>(state, [](fixed32 x)
                            { return acos(x); },
                            -1,
                            1);
}

static void f32_cordic_sin(benchmark::State& state)
{
    bench_random_1<fixed32>(state, [](fixed32 x)
                            { return cordic_sine(x); },
                            -10,
                            10);
}


#ifdef EIRIN_MATH_HAS_INT128
static void f64_create(benchmark::State& state)
{
    std::vector<double> vec(state.range(0));
    std::mt19937_64 mt64(0xC0FFEE);
    std::uniform_real_distribution<double> dist(-2000000000.0, 2000000000.0);
    for(auto& v : vec)
        v = dist(mt64);
    std::size_t i = 0;
    for(auto _ : state)
    {
        auto input = vec[i++ % vec.size()];
        benchmark::DoNotOptimize(input);
        auto result = fixed64(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_divide(benchmark::State& state)
{
    bench_random_2<fixed64>(state, [](fixed64 x, fixed64 y)
                            { return x / y; },
                            -2000000000.0,
                            2000000000.0,
                            1,
                            2000000000.0);
}

static void f64_multiple(benchmark::State& state)
{
    bench_random_2<fixed64>(state, [](fixed64 x, fixed64 y)
                            { return x * y; },
                            -10000.0,
                            10000.0,
                            -10000.0,
                            10000.0);
}

static void f64_add(benchmark::State& state)
{
    bench_random_2<fixed64>(state, [](fixed64 x, fixed64 y)
                            { return x + y; },
                            -1000000000.0,
                            1000000000.0,
                            -1000000000.0,
                            1000000000.0);
}

static void f64_minus(benchmark::State& state)
{
    bench_random_2<fixed64>(state, [](fixed64 x, fixed64 y)
                            { return x - y; },
                            -1000000000.0,
                            1000000000.0,
                            -1000000000.0,
                            1000000000.0);
}

static void f64_add_sat(benchmark::State& state)
{
    bench_sat<fixed64>(state, false, [](fixed64 x, fixed64 y)
                       { return saturating_add(x, y); });
}

static void f64_add_sat_boundary(benchmark::State& state)
{
    bench_sat<fixed64>(state, true, [](fixed64 x, fixed64 y)
                       { return saturating_add(x, y); });
}

static void f64_sub_sat(benchmark::State& state)
{
    bench_sat<fixed64>(state, false, [](fixed64 x, fixed64 y)
                       { return saturating_sub(x, y); });
}

static void f64_mul_sat(benchmark::State& state)
{
    bench_sat<fixed64>(state, false, [](fixed64 x, fixed64 y)
                       { return saturating_mul(x, y); });
}

static void f64_mul_sat_boundary(benchmark::State& state)
{
    bench_sat<fixed64>(state, true, [](fixed64 x, fixed64 y)
                       { return saturating_mul(x, y); });
}

static void f64_div_sat(benchmark::State& state)
{
    // divisor range excludes 0 (saturating_div by zero is UB) and spans both
    // |y| >= 1 (fast path) and |y| < 1 (amplifying path).
    bench_random_2<fixed64>(state, [](fixed64 x, fixed64 y)
                            { return saturating_div(x, y); },
                            -2000000000.0,
                            2000000000.0,
                            0.001,
                            2000000000.0);
}

static void f64_modwarp_add(benchmark::State& state)
{
    bench_sat<fixed64>(state, false, [](fixed64 x, fixed64 y)
                       { return modwarp_add(x, y); });
}

static void f64_modwarp_sub(benchmark::State& state)
{
    bench_sat<fixed64>(state, false, [](fixed64 x, fixed64 y)
                       { return modwarp_sub(x, y); });
}

static void f64_modwarp_mul(benchmark::State& state)
{
    bench_sat<fixed64>(state, false, [](fixed64 x, fixed64 y)
                       { return modwarp_mul(x, y); });
}

static void f64_modwarp_div(benchmark::State& state)
{
    // divisor range excludes 0 (modwarp_div by zero is UB).
    bench_random_2<fixed64>(state, [](fixed64 x, fixed64 y)
                            { return modwarp_div(x, y); },
                            -2000000000.0,
                            2000000000.0,
                            0.001,
                            2000000000.0);
}

static void f64_sqrt(benchmark::State& state)
{
    bench_random_1<fixed64>(state, [](fixed64 x)
                            { return sqrt(x); },
                            0,
                            2000000000.0);
}

static void f64_log2(benchmark::State& state)
{
    bench_random_1<fixed64>(state, [](fixed64 x)
                            { return log2(x); },
                            0.001,
                            2000000000.0);
}

static void f64_log(benchmark::State& state)
{
    bench_random_1<fixed64>(state, [](fixed64 x)
                            { return log(x); },
                            0.001,
                            2000000000.0);
}

static void f64_log10(benchmark::State& state)
{
    bench_random_1<fixed64>(state, [](fixed64 x)
                            { return log10(x); },
                            0.001,
                            2000000000.0);
}

static void f64_exp(benchmark::State& state)
{
    bench_random_1<fixed64>(state, [](fixed64 x)
                            { return exp(x); },
                            -20,
                            20);
}

static void f64_pow_fast(benchmark::State& state)
{
    bench_random_2<fixed64>(state, [](fixed64 x, fixed64 y)
                            { return pow(x, y); },
                            0.5,
                            10000.0,
                            -2,
                            2);
}

static void f64_pow(benchmark::State& state)
{
    bench_random_2<fixed64>(state, [](fixed64 x, fixed64 y)
                            { return pow(x, y); },
                            0.5,
                            10000.0,
                            -2,
                            2);
}

static void f64_sin(benchmark::State& state)
{
    bench_random_1<fixed64>(state, [](fixed64 x)
                            { return sin(x); },
                            -10,
                            10);
}

static void f64_cos(benchmark::State& state)
{
    bench_random_1<fixed64>(state, [](fixed64 x)
                            { return cos(x); },
                            -10,
                            10);
}

static void f64_tan(benchmark::State& state)
{
    bench_random_1<fixed64>(state, [](fixed64 x)
                            { return tan(x); },
                            -1.5,
                            1.5);
}

static void f64_atan(benchmark::State& state)
{
    bench_random_1<fixed64>(state, [](fixed64 x)
                            { return atan(x); },
                            -2000000000.0,
                            2000000000.0);
}

static void f64_asin(benchmark::State& state)
{
    bench_random_1<fixed64>(state, [](fixed64 x)
                            { return asin(x); },
                            -1,
                            1);
}

static void f64_acos(benchmark::State& state)
{
    bench_random_1<fixed64>(state, [](fixed64 x)
                            { return acos(x); },
                            -1,
                            1);
}

static void f64_cordic_sin(benchmark::State& state)
{
    bench_random_1<fixed64>(state, [](fixed64 x)
                            { return cordic_sine(x); },
                            -10,
                            10);
}

#endif

#if defined(EIRIN_OS_WINDOWS) || defined(EIRIN_OS_LINUX) || defined(EIRIN_OS_MACOS)
BENCHMARK(eirin_ext_int128_mul)->Args({0x114514, 0x7FFFFFFFFFFFFFFFll, 0x495, 0x1919810});
#endif
#ifdef _MSC_VER
BENCHMARK(msvc_int128_mul)->Args({0x114514, 0x7FFFFFFFFFFFFFFFll, 0x495, 0x1919810});
#endif

BENCHMARK(f32_create)->Args({4096});
BENCHMARK(f32_divide)->Args({4096});
BENCHMARK(f32_multiple)->Args({4096});
BENCHMARK(f32_add)->Args({4096});
BENCHMARK(f32_add_sat)->Args({4096});
BENCHMARK(f32_add_sat_boundary)->Args({4096});
BENCHMARK(f32_sub_sat)->Args({4096});
BENCHMARK(f32_mul_sat)->Args({4096});
BENCHMARK(f32_mul_sat_boundary)->Args({4096});
BENCHMARK(f32_div_sat)->Args({4096});
BENCHMARK(f32_modwarp_add)->Args({4096});
BENCHMARK(f32_modwarp_sub)->Args({4096});
BENCHMARK(f32_modwarp_mul)->Args({4096});
BENCHMARK(f32_modwarp_div)->Args({4096});
BENCHMARK(f32_satcast_s2u)->Args({4096});
BENCHMARK(f32_satcast_u2s)->Args({4096});
#ifdef EIRIN_MATH_HAS_INT128
BENCHMARK(f32_satcast_wide)->Args({4096});
BENCHMARK(f32_satcast_narrow)->Args({4096});
BENCHMARK(f32_satcast_up)->Args({4096});
BENCHMARK(f32_satcast_down)->Args({4096});
#endif
BENCHMARK(f32_minus)->Args({4096});
BENCHMARK(f32_sqrt)->Args({4096});
BENCHMARK(f32_log2)->Args({4096});
BENCHMARK(f32_log)->Args({4096});
BENCHMARK(f32_log10)->Args({4096});
BENCHMARK(f32_exp)->Args({4096});
BENCHMARK(f32_pow)->Args({4096});
BENCHMARK(f32_pow_fast)->Args({4096});
BENCHMARK(f32_sin)->Args({4096});
BENCHMARK(f32_cos)->Args({4096});
BENCHMARK(f32_tan)->Args({4096});
BENCHMARK(f32_atan)->Args({4096});
BENCHMARK(f32_acos)->Args({4096});
BENCHMARK(f32_asin)->Args({4096});
BENCHMARK(f32_cordic_sin)->Args({4096});
BENCHMARK(f32_cbrt)->Args({4096});
BENCHMARK(double_cbrt)->Args({4096});
#ifdef EIRIN_MATH_HAS_INT128
BENCHMARK(f64_create)->Args({4096});
BENCHMARK(f64_divide)->Args({4096});
BENCHMARK(f64_multiple)->Args({4096});
BENCHMARK(f64_add)->Args({4096});
BENCHMARK(f64_add_sat)->Args({4096});
BENCHMARK(f64_add_sat_boundary)->Args({4096});
BENCHMARK(f64_sub_sat)->Args({4096});
BENCHMARK(f64_mul_sat)->Args({4096});
BENCHMARK(f64_mul_sat_boundary)->Args({4096});
BENCHMARK(f64_div_sat)->Args({4096});
BENCHMARK(f64_modwarp_add)->Args({4096});
BENCHMARK(f64_modwarp_sub)->Args({4096});
BENCHMARK(f64_modwarp_mul)->Args({4096});
BENCHMARK(f64_modwarp_div)->Args({4096});
BENCHMARK(f64_minus)->Args({4096});
BENCHMARK(f64_sqrt)->Args({4096});
BENCHMARK(f64_log2)->Args({4096});
BENCHMARK(f64_log)->Args({4096});
BENCHMARK(f64_log10)->Args({4096});
BENCHMARK(f64_exp)->Args({4096});
BENCHMARK(f64_pow)->Args({4096});
BENCHMARK(f64_pow_fast)->Args({4096});
BENCHMARK(f64_sin)->Args({4096});
BENCHMARK(f64_cos)->Args({4096});
BENCHMARK(f64_tan)->Args({4096});
BENCHMARK(f64_atan)->Args({4096});
BENCHMARK(f64_acos)->Args({4096});
BENCHMARK(f64_asin)->Args({4096});
BENCHMARK(f64_cordic_sin)->Args({4096});
BENCHMARK(f64_cbrt)->Args({4096});
#endif

BENCHMARK_MAIN();

// on windows/msvc, -Wmaybe-uninitialized is not available
// so we can use #pragma to ignore the warning there
#ifdef _MSC_VER
// restore original warning levels.
#    pragma warning(pop)
#endif
