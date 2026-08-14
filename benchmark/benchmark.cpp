#include <eirin/fixed.hpp>
#include <eirin/math.hpp>
#include <benchmark/benchmark.h>
#include <eirin/ext/cordic.hpp>
#include <bench.hpp>

// on windows/msvc, -Wmaybe-uninitialized is not available
// so we can use #pragma to ignore the warning there
#ifdef _MSC_VER
// save warning levels, and drop it to level 3
#    pragma warning(push, 3)
// turn two warnings off
#    pragma warning(disable : 4701 4703)
#endif

using namespace eirin;

static void f32_create(benchmark::State& state)
{
    double arg = state.range(0);
    for(auto _ : state)
    {
        auto input = arg;
        benchmark::DoNotOptimize(input);

        auto result = fixed32(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_divide(benchmark::State& state)
{
    auto fp1 = F32_FROM_BENCH(0);
    auto fp2 = F32_FROM_BENCH(1);
    for(auto _ : state)
    {
        auto input1 = fp1, input2 = fp2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = input1 / input2;
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_multiple(benchmark::State& state)
{
    auto fp1 = "4.95"_f32;
    auto fp2 = "1145.14"_f32;
    for(auto _ : state)
    {
        auto input1 = fp1, input2 = fp2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = input1 * input2;
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_add(benchmark::State& state)
{
    auto fp1 = "4.95"_f32;
    auto fp2 = "1145.14"_f32;
    for(auto _ : state)
    {
        auto input1 = fp1, input2 = fp2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = input1 + input2;
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_minus(benchmark::State& state)
{
    auto fp1 = "4.95"_f32;
    auto fp2 = "1145.14"_f32;
    for(auto _ : state)
    {
        auto input1 = fp1, input2 = fp2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = input1 - input2;
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_sqrt(benchmark::State& state)
{
    auto fp1 = F32_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = sqrt(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_log2(benchmark::State& state)
{
    auto fp1 = F32_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto result = log2(fp1);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_log(benchmark::State& state)
{
    auto fp1 = F32_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = log(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_log10(benchmark::State& state)
{
    auto fp1 = F32_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto result = log10(fp1);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_exp(benchmark::State& state)
{
    auto fp1 = F32_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = exp(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_pow(benchmark::State& state)
{
    auto fp1 = F32_FROM_BENCH(0);
    auto fp2 = F32_FROM_BENCH(1);
    for(auto _ : state)
    {
        auto input1 = fp1, input2 = fp2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = pow(input1, input2);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}
static void f32_pow_fast(benchmark::State& state)
{
    auto fp1 = F32_FROM_BENCH(0);
    auto fp2 = F32_FROM_BENCH(1);
    for(auto _ : state)
    {
        auto input1 = fp1, input2 = fp2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = pow_fast(input1, input2);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_sin(benchmark::State& state)
{
    auto fp1 = F32_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = sin(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_cos(benchmark::State& state)
{
    auto fp1 = F32_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = cos(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_tan(benchmark::State& state)
{
    auto fp1 = F32_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = tan(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_atan(benchmark::State& state)
{
    auto fp1 = F32_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = atan(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_asin(benchmark::State& state)
{
    auto fp1 = F32_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = asin(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_acos(benchmark::State& state)
{
    auto fp1 = F32_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = acos(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f32_cordic_sin(benchmark::State& state)
{
    auto fp1 = F32_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = cordic_sine(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

#ifdef EIRIN_MATH_HAS_INT128
static void f64_create(benchmark::State& state)
{
    double arg = state.range(0);
    for(auto _ : state)
    {
        auto input = arg;
        benchmark::DoNotOptimize(input);

        auto fp1 = fixed64(input);
        benchmark::DoNotOptimize(fp1);
        benchmark::ClobberMemory();
    }
}

static void f64_divide(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    auto fp2 = F64_FROM_BENCH(1);
    for(auto _ : state)
    {
        auto input1 = fp1, input2 = fp2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = input1 / input2;
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_multiple(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    auto fp2 = F64_FROM_BENCH(1);
    for(auto _ : state)
    {
        auto input1 = fp1, input2 = fp2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = input1 * input2;
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_add(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    auto fp2 = F64_FROM_BENCH(1);
    for(auto _ : state)
    {
        auto input1 = fp1, input2 = fp2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = input1 + input2;
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_minus(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    auto fp2 = F64_FROM_BENCH(1);
    for(auto _ : state)
    {
        auto input1 = fp1, input2 = fp2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = input1 - input2;
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_sqrt(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = sqrt(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_log2(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto result = log2(fp1);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_log(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = log(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_log10(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto result = log10(fp1);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_exp(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = exp(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_pow_fast(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    auto fp2 = F64_FROM_BENCH(1);
    for(auto _ : state)
    {
        auto input1 = fp1, input2 = fp2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = pow_fast(input1, input2);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_pow(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    auto fp2 = F64_FROM_BENCH(1);
    for(auto _ : state)
    {
        auto input1 = fp1, input2 = fp2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = pow(input1, input2);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_sin(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = sin(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_cos(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = cos(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_tan(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = tan(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_atan(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = atan(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_asin(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = asin(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_acos(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = acos(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void f64_cordic_sin(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = cordic_sine(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}
#endif

BENCHMARK(f32_create)->Args({1145});
BENCHMARK(f32_divide)->Args({BENCH_F32_VAL(4.95), BENCH_F32_VAL(1145.14)});
BENCHMARK(f32_multiple)->Args({BENCH_F32_VAL(4.95), BENCH_F32_VAL(1145.14)});
BENCHMARK(f32_add)->Args({BENCH_F32_VAL(4.95), BENCH_F32_VAL(1145.14)});
BENCHMARK(f32_minus)->Args({BENCH_F32_VAL(4.95), BENCH_F32_VAL(1145.14)});
BENCHMARK(f32_sqrt)->Args({BENCH_F32_VAL(1145.14)});
BENCHMARK(f32_log2)->Args({BENCH_F32_VAL(1145.14)});
BENCHMARK(f32_log)->Args({BENCH_F32_VAL(1145.14)});
BENCHMARK(f32_log10)->Args({BENCH_F32_VAL(1145.14)});
BENCHMARK(f32_exp)->Args({BENCH_F32_VAL(11.4514)});
BENCHMARK(f32_pow)->Args({BENCH_F32_VAL(11.4514), BENCH_F32_VAL(3.5)});
BENCHMARK(f32_pow_fast)->Args({BENCH_F32_VAL(11.4514), BENCH_F32_VAL(3.5)});
BENCHMARK(f32_sin)->Args({BENCH_F32_VAL(1145.14)});
BENCHMARK(f32_cos)->Args({BENCH_F32_VAL(1145.14)});
BENCHMARK(f32_tan)->Args({BENCH_F32_VAL(1145.14)});
BENCHMARK(f32_atan)->Args({BENCH_F32_VAL(0.5)});
BENCHMARK(f32_acos)->Args({BENCH_F32_VAL(0.5)});
BENCHMARK(f32_asin)->Args({BENCH_F32_VAL(0.5)});
BENCHMARK(f32_cordic_sin)->Args({BENCH_F32_VAL(1145.14)});
#ifdef EIRIN_MATH_HAS_INT128
BENCHMARK(f64_create)->Args({1145});
BENCHMARK(f64_divide)->Args({BENCH_F64_VAL(1145.14), BENCH_F64_VAL(4.95)});
BENCHMARK(f64_multiple)->Args({BENCH_F64_VAL(1145.14), BENCH_F64_VAL(4.95)});
BENCHMARK(f64_add)->Args({BENCH_F64_VAL(1145.14), BENCH_F64_VAL(4.95)});
BENCHMARK(f64_minus)->Args({BENCH_F64_VAL(1145.14), BENCH_F64_VAL(4.95)});
BENCHMARK(f64_sqrt)->Args({BENCH_F64_VAL(1145.14)});
BENCHMARK(f64_log2)->Args({BENCH_F64_VAL(1145.14)});
BENCHMARK(f64_log)->Args({BENCH_F64_VAL(1145.14)});
BENCHMARK(f64_log10)->Args({BENCH_F64_VAL(1145.14)});
BENCHMARK(f64_exp)->Args({BENCH_F64_VAL(11.4514)});
BENCHMARK(f64_pow)->Args({BENCH_F64_VAL(11.4514), BENCH_F64_VAL(3.5)});
BENCHMARK(f64_pow_fast)->Args({BENCH_F32_VAL(11.4514), BENCH_F32_VAL(3.5)});
BENCHMARK(f64_sin)->Args({BENCH_F64_VAL(1145.14)});
BENCHMARK(f64_cos)->Args({BENCH_F64_VAL(1145.14)});
BENCHMARK(f64_tan)->Args({BENCH_F64_VAL(1145.14)});
BENCHMARK(f64_atan)->Args({BENCH_F64_VAL(0.5)});
BENCHMARK(f64_acos)->Args({BENCH_F64_VAL(0.5)});
BENCHMARK(f64_asin)->Args({BENCH_F64_VAL(0.5)});
BENCHMARK(f64_cordic_sin)->Args({BENCH_F64_VAL(1145.14)});
#endif

BENCHMARK_MAIN();

// on windows/msvc, -Wmaybe-uninitialized is not available
// so we can use #pragma to ignore the warning there
#ifdef _MSC_VER
// restore original warning levels.
#    pragma warning(pop)
#endif
