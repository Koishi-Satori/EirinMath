#include <benchmark/benchmark.h>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <vector>
#include <eirin/numeric.hpp>

using namespace eirin;

namespace
{
    template <typename T>
    std::vector<T> make_int_set(std::size_t n, bool boundary)
    {
        std::vector<T> vec(n);
        std::mt19937_64 rng(boundary ? 0xA11CE : 0x5EED);
        using U = std::make_unsigned_t<T>;
        constexpr bool sign = std::is_signed_v<T>;
        for(auto& v : vec)
        {
            if(boundary)
            {
                // values near +/-max, so most operations saturate
                const U near_max =
                    static_cast<U>(std::numeric_limits<T>::max()) - static_cast<U>(rng() % 256);
                if constexpr(sign)
                    v = (rng() & 1) ? static_cast<T>(near_max)
                                     : static_cast<T>(static_cast<U>(0) - near_max);
                else
                    v = static_cast<T>(near_max);
            }
            else
            {
                // full range: the low N bits of a uniform 64-bit draw are uniform
                v = static_cast<T>(rng());
            }
        }
        return vec;
    }

    template <typename T, typename Op>
    void bench_sat_int(benchmark::State& state, bool boundary, Op op)
    {
        const auto a = make_int_set<T>(state.range(0), boundary);
        const auto b = make_int_set<T>(state.range(0), boundary);
        std::size_t i = 0;
        for(auto _ : state)
        {
            const auto x = a[i % a.size()];
            const auto y = b[(i + 1) % b.size()];
            ++i;
            benchmark::DoNotOptimize(x);
            benchmark::DoNotOptimize(y);
            const auto r = op(x, y);
            benchmark::DoNotOptimize(r);
            benchmark::ClobberMemory();
        }
    }

    template <typename T, typename Op>
    void bench_int_1(benchmark::State& state, bool boundary, Op op)
    {
        const auto a = make_int_set<T>(state.range(0), boundary);
        std::size_t i = 0;
        for(auto _ : state)
        {
            const auto x = a[i % a.size()];
            ++i;
            benchmark::DoNotOptimize(x);
            const auto r = op(x);
            benchmark::DoNotOptimize(r);
            benchmark::ClobberMemory();
        }
    }

    template <typename T, typename Op>
    void bench_sat_div(benchmark::State& state, bool boundary, Op op)
    {
        auto a = make_int_set<T>(state.range(0), boundary);
        auto b = make_int_set<T>(state.range(0), boundary);
        for(auto& v : b)
            if(v == 0) // divisor must not be zero (UB precondition)
                v = 1;
        std::size_t i = 0;
        for(auto _ : state)
        {
            const auto x = a[i % a.size()];
            const auto y = b[(i + 1) % b.size()];
            ++i;
            benchmark::DoNotOptimize(x);
            benchmark::DoNotOptimize(y);
            const auto r = op(x, y);
            benchmark::DoNotOptimize(r);
            benchmark::ClobberMemory();
        }
    }

    static void i32_add_sat(benchmark::State& state)
    {
        bench_sat_int<std::int32_t>(state, false, [](auto x, auto y)
                                    { return saturating_add(x, y); });
    }

    static void i32_add_sat_boundary(benchmark::State& state)
    {
        bench_sat_int<std::int32_t>(state, true, [](auto x, auto y)
                                    { return saturating_add(x, y); });
    }

    static void i64_add_sat(benchmark::State& state)
    {
        bench_sat_int<std::int64_t>(state, false, [](auto x, auto y)
                                    { return saturating_add(x, y); });
    }

    static void i64_add_sat_boundary(benchmark::State& state)
    {
        bench_sat_int<std::int64_t>(state, true, [](auto x, auto y)
                                    { return saturating_add(x, y); });
    }

    static void u64_add_sat_boundary(benchmark::State& state)
    {
        bench_sat_int<std::uint64_t>(state, true, [](auto x, auto y)
                                     { return saturating_add(x, y); });
    }

    static void i64_sub_sat(benchmark::State& state)
    {
        bench_sat_int<std::int64_t>(state, false, [](auto x, auto y)
                                    { return saturating_sub(x, y); });
    }

    static void i64_sub_sat_boundary(benchmark::State& state)
    {
        bench_sat_int<std::int64_t>(state, true, [](auto x, auto y)
                                    { return saturating_sub(x, y); });
    }

    static void i64_mul_sat(benchmark::State& state)
    {
        bench_sat_int<std::int64_t>(state, false, [](auto x, auto y)
                                    { return saturating_mul(x, y); });
    }

    static void i64_mul_sat_boundary(benchmark::State& state)
    {
        bench_sat_int<std::int64_t>(state, true, [](auto x, auto y)
                                    { return saturating_mul(x, y); });
    }

    static void i32_div_sat(benchmark::State& state)
    {
        bench_sat_div<std::int32_t>(state, false, [](auto x, auto y)
                                    { return saturating_div(x, y); });
    }

    static void i64_div_sat(benchmark::State& state)
    {
        bench_sat_div<std::int64_t>(state, false, [](auto x, auto y)
                                    { return saturating_div(x, y); });
    }

    static void sat_cast_i64_to_i32(benchmark::State& state)
    {
        bench_int_1<std::int64_t>(state, false, [](auto x)
                                  { return saturating_cast<std::int32_t>(x); });
    }

    static void sat_cast_i64_to_u32(benchmark::State& state)
    {
        bench_int_1<std::int64_t>(state, false, [](auto x)
                                  { return saturating_cast<std::uint32_t>(x); });
    }

    static void sat_cast_u64_to_u32(benchmark::State& state)
    {
        bench_int_1<std::uint64_t>(state, false, [](auto x)
                                   { return saturating_cast<std::uint32_t>(x); });
    }
} // namespace

BENCHMARK(i32_add_sat)->Args({4096});
BENCHMARK(i32_add_sat_boundary)->Args({4096});
BENCHMARK(i64_add_sat)->Args({4096});
BENCHMARK(i64_add_sat_boundary)->Args({4096});
BENCHMARK(u64_add_sat_boundary)->Args({4096});
BENCHMARK(i64_sub_sat)->Args({4096});
BENCHMARK(i64_sub_sat_boundary)->Args({4096});
BENCHMARK(i64_mul_sat)->Args({4096});
BENCHMARK(i64_mul_sat_boundary)->Args({4096});
BENCHMARK(i32_div_sat)->Args({4096});
BENCHMARK(i64_div_sat)->Args({4096});
BENCHMARK(sat_cast_i64_to_i32)->Args({4096});
BENCHMARK(sat_cast_i64_to_u32)->Args({4096});
BENCHMARK(sat_cast_u64_to_u32)->Args({4096});

BENCHMARK_MAIN();
