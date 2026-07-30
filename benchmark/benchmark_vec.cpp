#include <eirin/eirin.hpp>
#include <benchmark/benchmark.h>
#include <bench.hpp>

using namespace eirin;

static void vec2_f32_create(benchmark::State& state)
{
    double arg = state.range(0);
    for(auto _ : state)
    {
        auto input = arg;
        benchmark::DoNotOptimize(input);

        auto result = vec2<fixed32>{input, input};
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(vec2_f32_create)->Args({1145});

BENCHMARK_MAIN();
