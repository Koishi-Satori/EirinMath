#include <eirin/eirin.hpp>
#include <benchmark/benchmark.h>
#include <bench.hpp>
#include <random>

using namespace eirin;

template <std::size_t N, typename T>
static tvec<N, T> random_vec()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<double> dist(-1000.0, 1000.0);
    tvec<N, T> v;
    for(std::size_t i = 0; i < N; ++i)
    {
        v[i] = static_cast<T>(dist(gen));
    }
    return v;
}

static void BM_Vec2_Create(benchmark::State& state)
{
    double arg = state.range(0);
    for(auto _ : state)
    {
        auto v = vec2<fixed64>{arg, arg};
        benchmark::DoNotOptimize(v);
    }
}

static void BM_Vec2_Add(benchmark::State& state)
{
    static auto a = random_vec<2, fixed64>();
    static auto b = random_vec<2, fixed64>();
    for(auto _ : state)
    {
        auto c = a + b;
        benchmark::DoNotOptimize(c);
    }
}

static void BM_Vec2_Sub(benchmark::State& state)
{
    static auto a = random_vec<2, fixed64>();
    static auto b = random_vec<2, fixed64>();
    for(auto _ : state)
    {
        auto c = a - b;
        benchmark::DoNotOptimize(c);
    }
}

static void BM_Vec2_Mul(benchmark::State& state)
{
    static auto a = random_vec<2, fixed64>();
    static auto b = random_vec<2, fixed64>();
    for(auto _ : state)
    {
        auto c = a * b;
        benchmark::DoNotOptimize(c);
    }
}

static void BM_Vec2_Div(benchmark::State& state)
{
    static auto a = random_vec<2, fixed64>();
    static auto b = random_vec<2, fixed64>();
    for(auto _ : state)
    {
        auto c = a / b;
        benchmark::DoNotOptimize(c);
    }
}

static void BM_Vec2_ScalarAdd(benchmark::State& state)
{
    static auto a = random_vec<2, fixed64>();
    fixed64 s = fixed64(state.range(0));
    for(auto _ : state)
    {
        auto c = a + s;
        benchmark::DoNotOptimize(c);
    }
}

static void BM_Vec2_ScalarMul(benchmark::State& state)
{
    static auto a = random_vec<2, fixed64>();
    fixed64 s = fixed64(state.range(0));
    for(auto _ : state)
    {
        auto c = a * s;
        benchmark::DoNotOptimize(c);
    }
}

static void BM_Vec2_Dot(benchmark::State& state)
{
    static auto a = random_vec<2, fixed64>();
    static auto b = random_vec<2, fixed64>();
    for(auto _ : state)
    {
        auto d = a.dot(b);
        benchmark::DoNotOptimize(d);
    }
}

static void BM_Vec2_Cross(benchmark::State& state)
{
    static auto a = random_vec<2, fixed64>();
    static auto b = random_vec<2, fixed64>();
    for(auto _ : state)
    {
        auto c = a.cross(b);
        benchmark::DoNotOptimize(c);
    }
}

static void BM_Vec2_SwizzleRead(benchmark::State& state)
{
    static auto v = random_vec<2, fixed64>();
    for(auto _ : state)
    {
        auto xy = v.xy();
        benchmark::DoNotOptimize(xy);
    }
}

static void BM_Vec2_SwizzleWrite(benchmark::State& state)
{
    static auto v = random_vec<2, fixed64>();
    static auto xy = random_vec<2, fixed64>();
    for(auto _ : state)
    {
        v.xy() = xy;
        benchmark::DoNotOptimize(v);
    }
}

// ==================== Vec3 ====================

static void BM_Vec3_Create(benchmark::State& state)
{
    double arg = state.range(0);
    for(auto _ : state)
    {
        auto v = vec3<fixed64>{arg, arg, arg};
        benchmark::DoNotOptimize(v);
    }
}

static void BM_Vec3_Add(benchmark::State& state)
{
    static auto a = random_vec<3, fixed64>();
    static auto b = random_vec<3, fixed64>();
    for(auto _ : state)
    {
        auto c = a + b;
        benchmark::DoNotOptimize(c);
    }
}

static void BM_Vec3_Dot(benchmark::State& state)
{
    static auto a = random_vec<3, fixed64>();
    static auto b = random_vec<3, fixed64>();
    for(auto _ : state)
    {
        auto d = a.dot(b);
        benchmark::DoNotOptimize(d);
    }
}

static void BM_Vec3_Cross(benchmark::State& state)
{
    static auto a = random_vec<3, fixed64>();
    static auto b = random_vec<3, fixed64>();
    for(auto _ : state)
    {
        auto c = a.cross(b);
        benchmark::DoNotOptimize(c);
    }
}

// ==================== Vec4 ====================

static void BM_Vec4_Create(benchmark::State& state)
{
    double arg = state.range(0);
    for(auto _ : state)
    {
        auto v = vec4<fixed64>{arg, arg, arg, arg};
        benchmark::DoNotOptimize(v);
    }
}

static void BM_Vec4_Add(benchmark::State& state)
{
    static auto a = random_vec<4, fixed64>();
    static auto b = random_vec<4, fixed64>();
    for(auto _ : state)
    {
        auto c = a + b;
        benchmark::DoNotOptimize(c);
    }
}

// Vec2
BENCHMARK(BM_Vec2_Create)->Args({1145});
BENCHMARK(BM_Vec2_Add);
BENCHMARK(BM_Vec2_Sub);
BENCHMARK(BM_Vec2_Mul);
BENCHMARK(BM_Vec2_Div);
BENCHMARK(BM_Vec2_ScalarAdd)->Args({1145});
BENCHMARK(BM_Vec2_ScalarMul)->Args({1145});
BENCHMARK(BM_Vec2_Dot);
BENCHMARK(BM_Vec2_Cross);
BENCHMARK(BM_Vec2_SwizzleRead);
BENCHMARK(BM_Vec2_SwizzleWrite);

// Vec3
BENCHMARK(BM_Vec3_Create)->Args({1145});
BENCHMARK(BM_Vec3_Add);
BENCHMARK(BM_Vec3_Dot);
BENCHMARK(BM_Vec3_Cross);

// Vec4
BENCHMARK(BM_Vec4_Create)->Args({1145});
BENCHMARK(BM_Vec4_Add);

BENCHMARK_MAIN();
