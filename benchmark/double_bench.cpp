#include <benchmark/benchmark.h>
#include "bench.hpp"
#include <cmath>

using namespace std;

static void double_create(benchmark::State& state)
{
    for(auto _ : state)
    {
        auto result = db_identity(1145.14);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_divide(benchmark::State& state)
{
    auto d1 = DOUBLE_FROM_BENCH(0);
    auto d2 = DOUBLE_FROM_BENCH(1);
    for(auto _ : state)
    {
        auto input1 = d1, input2 = d2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = input1 / input2;
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_multiple(benchmark::State& state)
{
    auto d1 = DOUBLE_FROM_BENCH(0);
    auto d2 = DOUBLE_FROM_BENCH(1);
    for(auto _ : state)
    {
        auto input1 = d1, input2 = d2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = input1 * input2;
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_add(benchmark::State& state)
{
    auto d1 = DOUBLE_FROM_BENCH(0);
    auto d2 = DOUBLE_FROM_BENCH(1);
    for(auto _ : state)
    {
        auto input1 = d1, input2 = d2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = input1 + input2;
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_minus(benchmark::State& state)
{
    auto d1 = DOUBLE_FROM_BENCH(0);
    auto d2 = DOUBLE_FROM_BENCH(1);
    for(auto _ : state)
    {
        auto input1 = d1, input2 = d2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = input1 - input2;
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_sqrt(benchmark::State& state)
{
    auto d1 = DOUBLE_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = d1;
        benchmark::DoNotOptimize(input);

        auto result = sqrt(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_log2(benchmark::State& state)
{
    auto d1 = DOUBLE_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = d1;
        benchmark::DoNotOptimize(input);

        auto result = log2(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_log(benchmark::State& state)
{
    auto d1 = DOUBLE_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = d1;
        benchmark::DoNotOptimize(input);

        auto result = log(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_log10(benchmark::State& state)
{
    auto d1 = DOUBLE_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = d1;
        benchmark::DoNotOptimize(input);

        auto result = log10(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_exp(benchmark::State& state)
{
    auto d1 = DOUBLE_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = d1;
        benchmark::DoNotOptimize(input);

        auto result = exp(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_pow(benchmark::State& state)
{
    auto d1 = DOUBLE_FROM_BENCH(0);
    auto d2 = DOUBLE_FROM_BENCH(1);
    for(auto _ : state)
    {
        auto input1 = d1, input2 = d2;
        benchmark::DoNotOptimize(input1);
        benchmark::DoNotOptimize(input2);

        auto result = pow(input1, input2);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_sin(benchmark::State& state)
{
    auto d1 = DOUBLE_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = d1;
        benchmark::DoNotOptimize(input);

        auto result = sin(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_cos(benchmark::State& state)
{
    auto d1 = DOUBLE_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = d1;
        benchmark::DoNotOptimize(input);

        auto result = cos(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_tan(benchmark::State& state)
{
    auto d1 = DOUBLE_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = d1;
        benchmark::DoNotOptimize(input);

        auto result = tan(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_asin(benchmark::State& state)
{
    auto d1 = DOUBLE_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = d1;
        benchmark::DoNotOptimize(input);

        auto result = asin(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_acos(benchmark::State& state)
{
    auto d1 = DOUBLE_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = d1;
        benchmark::DoNotOptimize(input);

        auto result = acos(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_atan(benchmark::State& state)
{
    auto d1 = DOUBLE_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = d1;
        benchmark::DoNotOptimize(input);

        auto result = atan(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(double_create)->Args({BENCH_DOUBLE_VAL(1145.14)});
BENCHMARK(double_divide)->Args({BENCH_DOUBLE_VAL(1145.14), BENCH_DOUBLE_VAL(4.95)});
BENCHMARK(double_multiple)->Args({BENCH_DOUBLE_VAL(1145.14), BENCH_DOUBLE_VAL(4.95)});
BENCHMARK(double_add)->Args({BENCH_DOUBLE_VAL(1145.14), BENCH_DOUBLE_VAL(4.95)});
BENCHMARK(double_minus)->Args({BENCH_DOUBLE_VAL(1145.14), BENCH_DOUBLE_VAL(4.95)});
BENCHMARK(double_sqrt)->Args({BENCH_DOUBLE_VAL(1145.14)});
BENCHMARK(double_log2)->Args({BENCH_DOUBLE_VAL(1145.14)});
BENCHMARK(double_log)->Args({BENCH_DOUBLE_VAL(1145.14)});
BENCHMARK(double_log10)->Args({BENCH_DOUBLE_VAL(1145.14)});
BENCHMARK(double_exp)->Args({BENCH_DOUBLE_VAL(1145.14)});
BENCHMARK(double_pow)->Args({BENCH_DOUBLE_VAL(1145.14), BENCH_DOUBLE_VAL(3.5)});
BENCHMARK(double_sin)->Args({BENCH_DOUBLE_VAL(1145.14)});
BENCHMARK(double_cos)->Args({BENCH_DOUBLE_VAL(1145.14)});
BENCHMARK(double_tan)->Args({BENCH_DOUBLE_VAL(1145.14)});
BENCHMARK(double_atan)->Args({BENCH_DOUBLE_VAL(0.5)});
BENCHMARK(double_acos)->Args({BENCH_DOUBLE_VAL(0.5)});
BENCHMARK(double_asin)->Args({BENCH_DOUBLE_VAL(0.5)});

BENCHMARK_MAIN();
