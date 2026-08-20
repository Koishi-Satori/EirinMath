#include <eirin/fixed.hpp>
#include <eirin/math.hpp>
#include <benchmark/benchmark.h>
#include "bench.hpp"
#include <eirin/ext/cordic.hpp>
#include <eirin/detail/util.hpp>
#include <array>
#include <vector>
#include <random>
#include <bit>
#include <limits>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>

#ifdef EIRIN_MATH_HAS_INT128
// on windows/msvc, -Wmaybe-uninitialized is not available
// so we can use #pragma to ignore the warning there
#    ifdef _MSC_VER
// save warning levels, and drop it to level 3
#        pragma warning(push, 3)
// turn two warnings off
#        pragma warning(disable : 4701 4703)
#    endif

using namespace eirin;

struct eirin_test_register
{};

template <typename I, typename R>
static inline R force_convert(I in)
{
    R out = 0;
    std::memcpy(&out, &in, sizeof(R));
    return out;
}

#    define BM_DOUBLE_PARAM_OUT(out) force_convert<int64_t, double>(out)
#    define BM_DOUBLE_PARAM_IN(in)   force_convert<double, int64_t>(in)
#    define BM_FLOAT_PARAM_OUT(out)  force_convert<int64_t, float>(out)
#    define BM_FLOAT_PARAM_IN(in)    force_convert<float, int64_t>(in)
#    define EIRIN_BENCH_PERF_PUT_ARGS(min, max, samples) \
        ->Args({BM_DOUBLE_PARAM_IN(min), BM_DOUBLE_PARAM_IN(max), samples})

template <typename Fixed>
requires fixed_point<Fixed>
static std::array<Fixed, 8> make_varying_inputs(Fixed base)
{
    std::array<Fixed, 8> inputs;
    for(std::size_t k = 0; k < inputs.size(); ++k)
        inputs[k] = Fixed::from_internal_value(
            base.internal_value() + static_cast<int64_t>(k) * 4096
        );
    return inputs;
}

template <typename FloatPoint>
requires std::floating_point<FloatPoint>
static std::array<FloatPoint, 8> make_varying_inputs(FloatPoint base)
{
    std::array<FloatPoint, 8> inputs;
    if constexpr(std::numeric_limits<FloatPoint>::is_specialized)
    {
        for(std::size_t k = 0; k < inputs.size(); ++k)
            inputs[k] += std::numeric_limits<FloatPoint>::epsilon() * k;
    }
    else if constexpr(sizeof(FloatPoint) == sizeof(uint32_t) || sizeof(FloatPoint) == sizeof(uint64_t))
    {
        using Bits = std::conditional_t<sizeof(FloatPoint) == sizeof(uint32_t), uint32_t, uint64_t>;
        Bits bits = std::bit_cast<Bits>(base);
        for(std::size_t k = 0; k < inputs.size(); ++k)
            inputs[k] = std::bit_cast<FloatPoint>(bits + static_cast<Bits>(k) * 4096);
    }
    else
    {
        const FloatPoint step = base == FloatPoint(0) ? FloatPoint(1e-6) : std::numeric_limits<FloatPoint>::epsilon() * std::abs(base);
        for(std::size_t k = 0; k < inputs.size(); ++k)
            inputs[k] = base + step * static_cast<FloatPoint>(k + 1);
    }
    return inputs;
}

template <typename Fixed, typename MathFunc>
requires fixed_point<Fixed>
static void bench_math_func(benchmark::State& state, MathFunc test_func)
{
    using value_type = typename Fixed::value_type;
    const auto base = Fixed::from_internal_value(
        static_cast<value_type>(state.range(0))
    );
    const auto inputs = make_varying_inputs(base);
    std::size_t i = 0;
    for(auto _ : state)
    {
        auto input = inputs[i++ & 7];
        benchmark::DoNotOptimize(input);

        auto result = test_func(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

template <typename FloatPoint, typename MathFunc>
requires std::floating_point<FloatPoint>
static void bench_math_func(benchmark::State& state, MathFunc test_func)
{
    const auto base = force_convert<int64_t, FloatPoint>(state.range(0));
    const auto inputs = make_varying_inputs(base);
    std::size_t i = 0;
    for(auto _ : state)
    {
        auto input = inputs[i++ & 7];
        benchmark::DoNotOptimize(input);

        auto result = test_func(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

#    define EIRIN_BENCH_PERF_SIMPLE(name, type, func, arg)                             \
        static void name /**/ (benchmark::State & state)                               \
        {                                                                              \
            bench_math_func<type>(state, [](const type& x) { return func /**/ (x); }); \
        }                                                                              \
        BENCHMARK(name)->Args({arg});

EIRIN_BENCH_PERF_SIMPLE(taylor_sin, fixed64, detail::sin_taylor, BENCH_F64_VAL(11.4514))
EIRIN_BENCH_PERF_SIMPLE(cordic_sin, fixed64, cordic_sine, BENCH_F64_VAL(11.4514))
EIRIN_BENCH_PERF_SIMPLE(lut_sin, fixed64, util::lut::lut_calc_sin, BENCH_F64_VAL(11.4514))
EIRIN_BENCH_PERF_SIMPLE(minimax_sin, fixed64, detail::sin_minimax, BENCH_F64_VAL(11.4514))
EIRIN_BENCH_PERF_SIMPLE(best_sin, fixed64, sin, BENCH_F64_VAL(11.4514))
EIRIN_BENCH_PERF_SIMPLE(taylor_sin_f32, fixed32, detail::sin_taylor, BENCH_F32_VAL(11.4514))
EIRIN_BENCH_PERF_SIMPLE(minimax_sin_f32, fixed32, detail::sin_minimax, BENCH_F32_VAL(11.4514))
EIRIN_BENCH_PERF_SIMPLE(best_sin_f32, fixed32, sin, BENCH_F32_VAL(11.4514))
EIRIN_BENCH_PERF_SIMPLE(sqrt_f64, fixed64, sqrt, BENCH_F64_VAL(1145.14))
EIRIN_BENCH_PERF_SIMPLE(sqrt_f32, fixed32, sqrt, BENCH_F32_VAL(1145.14))
EIRIN_BENCH_PERF_SIMPLE(exp_f64, fixed64, exp, BENCH_F64_VAL(1145.14))
EIRIN_BENCH_PERF_SIMPLE(exp_f32, fixed32, exp, BENCH_F32_VAL(1145.14))
EIRIN_BENCH_PERF_SIMPLE(double_sin, double, std::sin, BM_DOUBLE_PARAM_IN(11.4514))
EIRIN_BENCH_PERF_SIMPLE(float_sin, float, std::sin, BM_FLOAT_PARAM_IN(11.4514f))
EIRIN_BENCH_PERF_SIMPLE(double_sqrt, double, std::sqrt, BM_DOUBLE_PARAM_IN(1145.14))
EIRIN_BENCH_PERF_SIMPLE(float_sqrt, float, std::sqrt, BM_FLOAT_PARAM_IN(1145.14f))
EIRIN_BENCH_PERF_SIMPLE(double_exp, double, std::exp, BM_DOUBLE_PARAM_IN(1145.14))
EIRIN_BENCH_PERF_SIMPLE(float_exp, float, std::exp, BM_FLOAT_PARAM_IN(1145.14f))

#    define EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_TEST(name, special_process) \
        template <typename Fixed, typename MathFunc>                         \
        static void name /**/ (benchmark::State & state, MathFunc test_func) \
        {                                                                    \
            double min_val = BM_DOUBLE_PARAM_OUT(state.range(0));            \
            double max_val = BM_DOUBLE_PARAM_OUT(state.range(1));            \
            int num_samples = state.range(2);                                \
                                                                             \
            special_process;                                                 \
                                                                             \
            std::vector<Fixed> samples;                                      \
            samples.reserve(num_samples);                                    \
                                                                             \
            std::random_device rd;                                           \
            std::mt19937 gen(rd());                                          \
            std::uniform_real_distribution<double> dist(min_val, max_val);   \
                                                                             \
            for(int i = 0; i < num_samples; ++i)                             \
            {                                                                \
                samples.emplace_back(dist(gen));                             \
            }                                                                \
                                                                             \
            std::size_t sample_index = 0;                                    \
            for(auto _ : state)                                              \
            {                                                                \
                auto input = samples[sample_index++ % samples.size()];       \
                benchmark::DoNotOptimize(input);                             \
                                                                             \
                auto result = test_func(input);                              \
                benchmark::DoNotOptimize(result);                            \
                benchmark::ClobberMemory();                                  \
            }                                                                \
                                                                             \
            state.SetLabel(("[" + std::to_string(min_val) + ", " +           \
                            std::to_string(max_val) + "], samples=" +        \
                            std::to_string(num_samples))                     \
                               .c_str());                                    \
        }

#    define EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_SPECIAL_PROCESS_TRI \
        if(min_val == max_val && min_val == 0)                       \
        {                                                            \
            min_val = -0.001;                                        \
            max_val = 0.001;                                         \
        }

EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_TEST(BM_SinFunction, EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_SPECIAL_PROCESS_TRI)
EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_TEST(BM_SqrtFunction, )
EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_TEST(BM_ExpFunction, )
EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_TEST(BM_Log2Function, )
EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_TEST(BM_AtanFunction, )

#    define EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_FUNC(name, bench_func, type, func) \
        static void name /**/ (benchmark::State & state)                            \
        {                                                                           \
            bench_func<type>(state, [](const type& x) { return func /**/ (x); });   \
        }

#    define EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_ARGS_TRI       \
        EIRIN_BENCH_PERF_PUT_ARGS(0, M_PI / 2, 1000)            \
        EIRIN_BENCH_PERF_PUT_ARGS(M_PI / 2, M_PI, 1000)         \
        EIRIN_BENCH_PERF_PUT_ARGS(M_PI, 3 * M_PI / 2, 1000)     \
        EIRIN_BENCH_PERF_PUT_ARGS(3 * M_PI / 2, 2 * M_PI, 1000) \
        EIRIN_BENCH_PERF_PUT_ARGS(-M_PI, M_PI, 2000)            \
        EIRIN_BENCH_PERF_PUT_ARGS(-10 * M_PI, 10 * M_PI, 5000)  \
        EIRIN_BENCH_PERF_PUT_ARGS(0, 0, 500);

#    define EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_ARGS_SQRT \
        EIRIN_BENCH_PERF_PUT_ARGS(0, 1, 750)               \
        EIRIN_BENCH_PERF_PUT_ARGS(1, 2, 750)               \
        EIRIN_BENCH_PERF_PUT_ARGS(2, 9, 1000)              \
        EIRIN_BENCH_PERF_PUT_ARGS(9, 32767, 4000)          \
        EIRIN_BENCH_PERF_PUT_ARGS(32768, 2100000000, 5000);

#    define EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_ARGS_EXP \
        EIRIN_BENCH_PERF_PUT_ARGS(0, 1, 2000)             \
        EIRIN_BENCH_PERF_PUT_ARGS(1, 2, 2000)             \
        EIRIN_BENCH_PERF_PUT_ARGS(2, 9, 2000);

#    define EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_ARGS_LOG2 \
        EIRIN_BENCH_PERF_PUT_ARGS(0.01, 1, 2000)           \
        EIRIN_BENCH_PERF_PUT_ARGS(1, 2, 2000)              \
        EIRIN_BENCH_PERF_PUT_ARGS(2, 9, 2000)              \
        EIRIN_BENCH_PERF_PUT_ARGS(9, 32767, 4000)          \
        EIRIN_BENCH_PERF_PUT_ARGS(32768, 2100000000, 5000);

#    define EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_ARGS_LOG2_32 \
        EIRIN_BENCH_PERF_PUT_ARGS(0.01, 1, 2000)              \
        EIRIN_BENCH_PERF_PUT_ARGS(1, 2, 2000)                 \
        EIRIN_BENCH_PERF_PUT_ARGS(2, 9, 2000)                 \
        EIRIN_BENCH_PERF_PUT_ARGS(9, 32767, 4000);

#    define EIRIN_BENCH_PERF(arg_type, name, bench_func, type, func, prefix, label) \
        EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_FUNC(name, bench_func, type, func)     \
        BENCHMARK(name)                                                             \
            ->Name(prefix #label)                                                   \
            ->Unit(benchmark::kNanosecond)                                          \
                EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_ARGS_##arg_type

EIRIN_BENCH_PERF(TRI, BM_TaylorSin, BM_SinFunction, fixed64, detail::sin_taylor, "Sin/", Taylor)
EIRIN_BENCH_PERF(TRI, BM_CordicSin, BM_SinFunction, fixed64, cordic_sine, "Sin/", CORDIC)
EIRIN_BENCH_PERF(TRI, BM_LUTSin, BM_SinFunction, fixed64, util::lut::lut_calc_sin, "Sin/", LUT)
EIRIN_BENCH_PERF(TRI, BM_MinimaxSin, BM_SinFunction, fixed64, detail::sin_minimax, "Sin/", MiniMax)
EIRIN_BENCH_PERF(TRI, BM_BestSin, BM_SinFunction, fixed64, sin, "Sin/", AutoFit)
EIRIN_BENCH_PERF(TRI, BM_TaylorSinF32, BM_SinFunction, fixed32, detail::sin_taylor, "Sin32/", Taylor)
EIRIN_BENCH_PERF(TRI, BM_MinimaxSinF32, BM_SinFunction, fixed32, detail::sin_minimax, "Sin32/", MiniMax)
EIRIN_BENCH_PERF(TRI, BM_BestSinF32, BM_SinFunction, fixed32, sin, "Sin32/", AutoFit)
EIRIN_BENCH_PERF(SQRT, BM_Sqrt, BM_SqrtFunction, fixed64, sqrt, "Sqrt/", AutoFit)
EIRIN_BENCH_PERF(SQRT, BM_SqrtF32, BM_SqrtFunction, fixed32, sqrt, "Sqrt32/", AutoFit)
EIRIN_BENCH_PERF(EXP, BM_Exp32, BM_ExpFunction, fixed32, exp, "Exp32/", Best)
EIRIN_BENCH_PERF(EXP, BM_Exp, BM_ExpFunction, fixed64, exp, "Exp/", Best)
EIRIN_BENCH_PERF(LOG2, BM_Log2, BM_Log2Function, fixed64, log2, "Log2/", Best)
EIRIN_BENCH_PERF(LOG2_32, BM_Log2F32, BM_Log2Function, fixed32, log2, "Log232/", Best)

#    define EIRIN_BENCH_PERF_COMPLEX_PERFORMANCE_ARGS_ATAN \
        EIRIN_BENCH_PERF_PUT_ARGS(-1, 1, 2000)         \
        EIRIN_BENCH_PERF_PUT_ARGS(-10, 10, 2000)       \
        EIRIN_BENCH_PERF_PUT_ARGS(-100, 100, 2000)     \
        EIRIN_BENCH_PERF_PUT_ARGS(-32767, 32767, 4000);

EIRIN_BENCH_PERF(ATAN, BM_Atan, BM_AtanFunction, fixed64, atan, "Atan/", AutoFit)
EIRIN_BENCH_PERF(ATAN, BM_AtanF32, BM_AtanFunction, fixed32, atan, "Atan32/", AutoFit)

// two-input (pow) performance benchmark: random (base, exponent) pairs
#    define EIRIN_BENCH_PERF_PUT_ARGS_POW(bmin, bmax, emin, emax, samples) \
        ->Args({BM_DOUBLE_PARAM_IN(bmin), BM_DOUBLE_PARAM_IN(bmax),         \
                BM_DOUBLE_PARAM_IN(emin), BM_DOUBLE_PARAM_IN(emax), samples})

#    define EIRIN_BENCH_PERF_POW_TEST(name)                                  \
        template <typename Fixed, typename MathFunc>                         \
        static void name /**/ (benchmark::State & state, MathFunc test_func) \
        {                                                                    \
            double bmin = BM_DOUBLE_PARAM_OUT(state.range(0));               \
            double bmax = BM_DOUBLE_PARAM_OUT(state.range(1));               \
            double emin = BM_DOUBLE_PARAM_OUT(state.range(2));               \
            double emax = BM_DOUBLE_PARAM_OUT(state.range(3));               \
            int num_samples = state.range(4);                                \
                                                                             \
            std::vector<Fixed> bases, exps;                                  \
            bases.reserve(num_samples);                                      \
            exps.reserve(num_samples);                                       \
                                                                             \
            std::random_device rd;                                           \
            std::mt19937 gen(rd());                                          \
            std::uniform_real_distribution<double> dist_b(bmin, bmax);       \
            std::uniform_real_distribution<double> dist_e(emin, emax);       \
                                                                             \
            for(int i = 0; i < num_samples; ++i)                             \
            {                                                                \
                bases.emplace_back(dist_b(gen));                             \
                exps.emplace_back(dist_e(gen));                              \
            }                                                                \
                                                                             \
            std::size_t sample_index = 0;                                    \
            for(auto _ : state)                                              \
            {                                                                \
                auto b = bases[sample_index % bases.size()];                 \
                auto e = exps[sample_index % exps.size()];                   \
                benchmark::DoNotOptimize(b);                                 \
                benchmark::DoNotOptimize(e);                                 \
                                                                             \
                auto result = test_func(b, e);                               \
                benchmark::DoNotOptimize(result);                            \
                benchmark::ClobberMemory();                                  \
                ++sample_index;                                              \
            }                                                                \
                                                                             \
            state.SetLabel(("b[" + std::to_string(bmin) + ", " +             \
                            std::to_string(bmax) + "], e[" +                 \
                            std::to_string(emin) + ", " +                    \
                            std::to_string(emax) + "], samples=" +           \
                            std::to_string(num_samples))                     \
                               .c_str());                                    \
        }

EIRIN_BENCH_PERF_POW_TEST(BM_PowFunction)

#    define EIRIN_BENCH_PERF_ARGS_POW        \
        EIRIN_BENCH_PERF_PUT_ARGS_POW(0.5, 2, -1, 1, 2000) \
        EIRIN_BENCH_PERF_PUT_ARGS_POW(1, 2, 0.5, 2, 2000)  \
        EIRIN_BENCH_PERF_PUT_ARGS_POW(2, 9, 0, 4, 2000)    \
        EIRIN_BENCH_PERF_PUT_ARGS_POW(0.1, 9, -4, 4, 4000);

#    define EIRIN_BENCH_PERF_POW(name, bench_func, type, func, prefix, label) \
        static void name /**/ (benchmark::State & state)                      \
        {                                                                     \
            bench_func<type>(state, [](const type& b, const type& e) {        \
                return func /**/ (b, e);                                      \
            });                                                               \
        }                                                                     \
        BENCHMARK(name)                                                       \
            ->Name(prefix #label)                                             \
            ->Unit(benchmark::kNanosecond)                                    \
                EIRIN_BENCH_PERF_ARGS_POW

EIRIN_BENCH_PERF_POW(BM_Pow32, BM_PowFunction, fixed32, pow_fast, "Pow32/", Old)
EIRIN_BENCH_PERF_POW(BM_PowNew32, BM_PowFunction, fixed32, pow, "Pow32/", New)
EIRIN_BENCH_PERF_POW(BM_Pow, BM_PowFunction, fixed64, pow_fast, "Pow/", Old)
EIRIN_BENCH_PERF_POW(BM_PowNew, BM_PowFunction, fixed64, pow, "Pow/", New)

struct AccuracyStats
{
    double max_abs_error = 0;
    double avg_abs_error = 0;
    double max_rel_error = 0;
    double avg_rel_error = 0;
    double max_ulp_error = 0;
    double avg_ulp_error = 0;
    double worst_abs_x = 0;
    int total_samples = 0;

    void update(double computed, double reference, double input_x, double ulp)
    {
        double abs_error = std::abs(computed - reference);
        if(abs_error > max_abs_error)
        {
            max_abs_error = abs_error;
            worst_abs_x = input_x;
        }
        avg_abs_error += (abs_error - avg_abs_error) / (total_samples + 1);

        double ulp_error = abs_error / ulp;
        max_ulp_error = std::max(max_ulp_error, ulp_error);
        avg_ulp_error += (ulp_error - avg_ulp_error) / (total_samples + 1);

        if(std::abs(reference) > 1e-6)
        {
            double rel_error = abs_error / std::abs(reference);
            max_rel_error = std::max(max_rel_error, rel_error);
            avg_rel_error += (rel_error - avg_rel_error) / (total_samples + 1);
        }

        total_samples++;
    }

    void print(const std::string& func_name) const
    {
        std::cout << func_name << " Accuracy:\n";
        std::cout << "  Max Absolute Error: " << std::scientific << max_abs_error << "\n";
        std::cout << "  Avg Absolute Error: " << std::scientific << avg_abs_error << "\n";
        std::cout << "  Max Relative Error: " << std::scientific << max_rel_error << "\n";
        std::cout << "  Avg Relative Error: " << std::scientific << avg_rel_error << "\n";
        std::cout << "  Max Ulp Error:      " << std::scientific << max_ulp_error << "\n";
        std::cout << "  Avg Ulp Error:      " << std::scientific << avg_ulp_error << "\n";
        std::cout << "  Worst Abs At x:     " << std::fixed << worst_abs_x << "\n";
        std::cout << "  Samples: " << total_samples << "\n";
    }
};

static std::vector<std::pair<const char*, AccuracyStats*>> accuracy_results;

static inline eirin_test_register push_accuracy_stats(const char* name, AccuracyStats& stats)
{
    static eirin_test_register instance;
    accuracy_results.emplace_back(std::make_pair(name, &stats));
    return instance;
}

// Accuracy measurement notes:
//  - The reference is evaluated on the exact quantized fixed-point input, so input
//    quantization is not attributed to the function under test.
//  - Samples whose true result is outside the representable range are excluded:
//    saturation is a strategy-specific overflow behavior, not an algorithm error.
//  - A fixed RNG seed keeps runs reproducible.
#    define EIRIN_BENCH_PERF_ACCURACY_TEST(name, special_process, ...)                                                                                             \
        template <typename Fixed, typename TestFunc, typename ReferenceFunc>                                                                                       \
        static void name /**/ (benchmark::State & state, TestFunc test_func, ReferenceFunc ref_func, AccuracyStats & accuracy_stats, const std::string& func_name) \
        {                                                                                                                                                          \
            double min_val = BM_DOUBLE_PARAM_OUT(state.range(0));                                                                                                  \
            double max_val = BM_DOUBLE_PARAM_OUT(state.range(1));                                                                                                  \
            int num_samples = state.range(2);                                                                                                                      \
                                                                                                                                                                   \
            special_process;                                                                                                                                       \
                                                                                                                                                                   \
            std::vector<double> refs;                                                                                                                              \
            std::vector<Fixed> samples;                                                                                                                            \
            const double critical_points[] = {__VA_ARGS__};                                                                                                        \
            refs.reserve(num_samples + sizeof(critical_points) / sizeof(double));                                                                                  \
            samples.reserve(num_samples + sizeof(critical_points) / sizeof(double));                                                                               \
                                                                                                                                                                   \
            std::mt19937 gen(0x5EED);                                                                                                                              \
            std::uniform_real_distribution<double> dist(min_val, max_val);                                                                                         \
            const double ulp = std::ldexp(1.0, -static_cast<int>(Fixed::precision));                                                                               \
                                                                                                                                                                   \
            auto add_sample = [&](double v)                                                                                                                        \
            {                                                                                                                                                      \
                samples.emplace_back(v);                                                                                                                           \
                const double ref_value = ref_func(static_cast<double>(samples.back()));                                                                            \
                if(ref_value > static_cast<double>(std::numeric_limits<Fixed>::max()))                                                                             \
                {                                                                                                                                                  \
                    samples.pop_back();                                                                                                                            \
                    return;                                                                                                                                        \
                }                                                                                                                                                  \
                refs.push_back(ref_value);                                                                                                                         \
            };                                                                                                                                                     \
                                                                                                                                                                   \
            for(int i = 0; i < num_samples; ++i)                                                                                                                   \
                add_sample(dist(gen));                                                                                                                             \
            for(double pt : critical_points)                                                                                                                       \
            {                                                                                                                                                      \
                if(pt >= min_val && pt <= max_val)                                                                                                                 \
                    add_sample(pt);                                                                                                                                \
            }                                                                                                                                                      \
            size_t sample_index = 0;                                                                                                                               \
            if(samples.empty())                                                                                                                                    \
            {                                                                                                                                                      \
                state.SkipWithError("all samples are outside the representable range");                                                                            \
                return;                                                                                                                                            \
            }                                                                                                                                                      \
                                                                                                                                                                   \
            for(auto _ : state)                                                                                                                                    \
            {                                                                                                                                                      \
                auto input = samples[sample_index % samples.size()];                                                                                               \
                benchmark::DoNotOptimize(input);                                                                                                                   \
                                                                                                                                                                   \
                auto result = test_func(input);                                                                                                                    \
                benchmark::DoNotOptimize(result);                                                                                                                  \
                                                                                                                                                                   \
                double ref_value = refs[sample_index % refs.size()];                                                                                               \
                accuracy_stats.update((double)result, ref_value, static_cast<double>(input), ulp);                                                                 \
                sample_index++;                                                                                                                                    \
            }                                                                                                                                                      \
                                                                                                                                                                   \
            state.SetLabel(func_name.c_str());                                                                                                                     \
        }

#    define EIRIN_BENCH_PERF_ACCURACY_SPECIAL_PROCESS_TRI \
        if(min_val == max_val && min_val == 0)            \
        {                                                 \
            min_val = -0.001;                             \
            max_val = 0.001;                              \
        }

#    define EIRIN_BENCH_PERF_ACCURACY_ARGS_TRI                  \
        EIRIN_BENCH_PERF_PUT_ARGS(0, M_PI / 2, 1000)            \
        EIRIN_BENCH_PERF_PUT_ARGS(M_PI / 2, M_PI, 1000)         \
        EIRIN_BENCH_PERF_PUT_ARGS(M_PI, 3 * M_PI / 2, 1000)     \
        EIRIN_BENCH_PERF_PUT_ARGS(3 * M_PI / 2, 2 * M_PI, 1000) \
        EIRIN_BENCH_PERF_PUT_ARGS(-M_PI, M_PI, 2000)            \
        EIRIN_BENCH_PERF_PUT_ARGS(-10 * M_PI, 10 * M_PI, 5000)  \
        EIRIN_BENCH_PERF_PUT_ARGS(0, 0, 500);

#    define EIRIN_BENCH_PERF_ACCURACY_ARGS_SQRT   \
        EIRIN_BENCH_PERF_PUT_ARGS(0, 1, 500)      \
        EIRIN_BENCH_PERF_PUT_ARGS(1, 2, 500)      \
        EIRIN_BENCH_PERF_PUT_ARGS(2, 9, 1000)     \
        EIRIN_BENCH_PERF_PUT_ARGS(9, 32767, 4000) \
        EIRIN_BENCH_PERF_PUT_ARGS(32768, 2100000000, 5000);

#    define EIRIN_BENCH_PERF_ACCURACY_ARGS_SQRT_32 \
        EIRIN_BENCH_PERF_PUT_ARGS(0, 1, 1000)      \
        EIRIN_BENCH_PERF_PUT_ARGS(1, 2, 1000)      \
        EIRIN_BENCH_PERF_PUT_ARGS(2, 9, 2000)      \
        EIRIN_BENCH_PERF_PUT_ARGS(9, 32767, 5000);

#    define EIRIN_BENCH_PERF_ACCURACY_ARGS_EXP   \
        EIRIN_BENCH_PERF_PUT_ARGS(-11, 0, 1500)  \
        EIRIN_BENCH_PERF_PUT_ARGS(0, 1, 1500)    \
        EIRIN_BENCH_PERF_PUT_ARGS(1, 2, 1500)    \
        EIRIN_BENCH_PERF_PUT_ARGS(2, 9, 2000)    \
        EIRIN_BENCH_PERF_PUT_ARGS(9, 10.5, 1500) \
        EIRIN_BENCH_PERF_PUT_ARGS(10.5, 21.5, 2000);

#    define EIRIN_BENCH_PERF_ACCURACY_ARGS_LOG2   \
        EIRIN_BENCH_PERF_PUT_ARGS(0.01, 1, 1500)  \
        EIRIN_BENCH_PERF_PUT_ARGS(1, 2, 1500)     \
        EIRIN_BENCH_PERF_PUT_ARGS(2, 9, 2000)     \
        EIRIN_BENCH_PERF_PUT_ARGS(9, 32767, 4000) \
        EIRIN_BENCH_PERF_PUT_ARGS(32768, 2100000000, 5000);

#    define EIRIN_BENCH_PERF_ACCURACY_ARGS_LOG2_32 \
        EIRIN_BENCH_PERF_PUT_ARGS(0.01, 1, 1500)   \
        EIRIN_BENCH_PERF_PUT_ARGS(1, 2, 1500)      \
        EIRIN_BENCH_PERF_PUT_ARGS(2, 9, 2000)      \
        EIRIN_BENCH_PERF_PUT_ARGS(9, 32767, 4000);

#    define EIRIN_BENCH_PERF_ACCURACY_ARGS_ATAN \
        EIRIN_BENCH_PERF_PUT_ARGS(-1, 1, 2000)     \
        EIRIN_BENCH_PERF_PUT_ARGS(-1.5, 1.5, 2000);

// wrapper functions
#    define EIRIN_BENCH_PERF_ACCURACY_FUNC(name, bench_func, test_type, test_func, ref_type, ref_func, result, label)                                          \
        static void name /**/ (benchmark::State & state)                                                                                                       \
        {                                                                                                                                                      \
            bench_func<test_type>(state, [](const test_type& x) { return test_func /**/ (x); }, [](ref_type x) { return ref_func /**/ (x); }, result, #label); \
        }

#    define EIRIN_BENCH_PERF_ACCURACY(arg_type, name, bench_func, test_type, test_func, ref_type, ref_func, result, label, func_name) \
        AccuracyStats result;                                                                                                         \
        EIRIN_BENCH_PERF_ACCURACY_FUNC(name, bench_func, test_type, test_func, ref_type, ref_func, result, label)                     \
        [[maybe_unused]] static const eirin_test_register helper_##name = push_accuracy_stats(func_name, result);                     \
        BENCHMARK(name)                                                                                                               \
            ->Name("MathAccuracy/" #label)                                                                                            \
                EIRIN_BENCH_PERF_ACCURACY_ARGS_##arg_type

EIRIN_BENCH_PERF_ACCURACY_TEST(BM_SinWithAccuracy, EIRIN_BENCH_PERF_ACCURACY_SPECIAL_PROCESS_TRI, 0, )
EIRIN_BENCH_PERF_ACCURACY_TEST(BM_SqrtWithAccuracy, , 0, 1, 2, 4, 9, 16, 25, 36, 49, 81, 100)
EIRIN_BENCH_PERF_ACCURACY_TEST(BM_ExpWithAccuracy, , 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -11.090354888959125, -9.704060527839234, -6.931471805599453, -3.4657359027997265, -0.6931471805599453, 0.6931471805599453, 1.3862943611198906, 2.772588722239781, 4.1588830833596715, 5.545177444479562, 6.931471805599453, 8.317766166719343, 9.704060527839234, 10.39720770839918, 11.090354888959125)
EIRIN_BENCH_PERF_ACCURACY_TEST(BM_Log2WithAccuracy, , 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 1.5, 3, 5, 10, 100, 1000, 10000, 32767)
EIRIN_BENCH_PERF_ACCURACY_TEST(BM_AtanWithAccuracy, , 0, 1, -1, 0.5, -0.5, 0.5773502691896258, -0.5773502691896258, 1.7320508075688772, -1.7320508075688772, 2, -2)

EIRIN_BENCH_PERF_ACCURACY(TRI, BM_TaylorSin_Accuracy, BM_SinWithAccuracy, fixed64, detail::sin_taylor, double, std::sin, taylor_accuracy, TaylorSin, "Taylor Series Sin (fixed64)")
EIRIN_BENCH_PERF_ACCURACY(TRI, BM_CordicSin_Accuracy, BM_SinWithAccuracy, fixed64, cordic_sine, double, std::sin, cordic_accuracy, CordicSin, "CORDIC Sin (fixed64)")
EIRIN_BENCH_PERF_ACCURACY(TRI, BM_LUTSin_Accuracy, BM_SinWithAccuracy, fixed64, util::lut::lut_calc_sin, double, std::sin, lut_accuracy, LUTSin, "LUT Sin (fixed64)")
EIRIN_BENCH_PERF_ACCURACY(TRI, BM_MinimaxSin_Accuracy, BM_SinWithAccuracy, fixed64, detail::sin_minimax, double, std::sin, minimax_accuracy, MinimaxSin, "Minimax Sin (fixed64)")
EIRIN_BENCH_PERF_ACCURACY(TRI, BM_BestSin_Accuracy, BM_SinWithAccuracy, fixed64, sin, double, std::sin, best_accuracy, AutoFitSin, "AutoFit Sin (fixed64)")
EIRIN_BENCH_PERF_ACCURACY(TRI, BM_TaylorSinF32_Accuracy, BM_SinWithAccuracy, fixed32, detail::sin_taylor, double, std::sin, taylor_f32_accuracy, TaylorSinF32, "Taylor Series Sin (fixed32)")
EIRIN_BENCH_PERF_ACCURACY(TRI, BM_MinimaxSinF32_Accuracy, BM_SinWithAccuracy, fixed32, detail::sin_minimax, double, std::sin, minimax_f32_accuracy, MinimaxSinF32, "Minimax Sin (fixed32)")
EIRIN_BENCH_PERF_ACCURACY(TRI, BM_BestSinF32_Accuracy, BM_SinWithAccuracy, fixed32, sin, double, std::sin, best_f32_accuracy, AutoFitSinF32, "AutoFit Sin (fixed32)")
EIRIN_BENCH_PERF_ACCURACY(SQRT, BM_BestSqrt_Accuracy, BM_SqrtWithAccuracy, fixed64, sqrt, double, std::sqrt, sqrt_accuracy, AutoFitSqrt, "AutoFit Sqrt (fixed64)")
EIRIN_BENCH_PERF_ACCURACY(SQRT_32, BM_BestSqrtF32_Accuracy, BM_SqrtWithAccuracy, fixed32, sqrt, double, std::sqrt, sqrt_f32_accuracy, AutoFitSqrtF32, "AutoFit Sqrt (fixed32)")
EIRIN_BENCH_PERF_ACCURACY(EXP, BM_BestExp_Accuracy, BM_ExpWithAccuracy, fixed64, exp, double, std::exp, exp_accuracy, PloyExp, "Poly Exp (fixed64)")
EIRIN_BENCH_PERF_ACCURACY(EXP, BM_BestExpF32_Accuracy, BM_ExpWithAccuracy, fixed32, exp, double, std::exp, exp_f32_accuracy, PloyExpF32, "Poly Exp (fixed32)")
EIRIN_BENCH_PERF_ACCURACY(LOG2, BM_BestLog2_Accuracy, BM_Log2WithAccuracy, fixed64, log2, double, std::log2, log2_accuracy, PloyLog2, "Poly Log2 (fixed64)")
EIRIN_BENCH_PERF_ACCURACY(LOG2_32, BM_BestLog2F32_Accuracy, BM_Log2WithAccuracy, fixed32, log2, double, std::log2, log2_f32_accuracy, PloyLog2F32, "Poly Log2 (fixed32)")

// two-input (pow) accuracy benchmark
#    define EIRIN_BENCH_PERF_ACCURACY_TEST_POW(name)                                                                                                    \
        template <typename Fixed, typename TestFunc, typename ReferenceFunc>                                                                           \
        static void name /**/ (benchmark::State & state, TestFunc test_func, ReferenceFunc ref_func, AccuracyStats & accuracy_stats, const std::string& func_name) \
        {                                                                                                                                              \
            double bmin = BM_DOUBLE_PARAM_OUT(state.range(0));                                                                                         \
            double bmax = BM_DOUBLE_PARAM_OUT(state.range(1));                                                                                         \
            double emin = BM_DOUBLE_PARAM_OUT(state.range(2));                                                                                         \
            double emax = BM_DOUBLE_PARAM_OUT(state.range(3));                                                                                         \
            int num_samples = state.range(4);                                                                                                          \
                                                                                                                                                       \
            std::vector<double> refs;                                                                                                                  \
            std::vector<Fixed> bases, exps;                                                                                                            \
            refs.reserve(num_samples);                                                                                                                 \
            bases.reserve(num_samples);                                                                                                                \
            exps.reserve(num_samples);                                                                                                                 \
                                                                                                                                                       \
            std::mt19937 gen(0x5EED);                                                                                                                  \
            std::uniform_real_distribution<double> dist_b(bmin, bmax);                                                                                 \
            std::uniform_real_distribution<double> dist_e(emin, emax);                                                                                 \
            const double ulp = std::ldexp(1.0, -static_cast<int>(Fixed::precision));                                                                   \
                                                                                                                                                       \
            for(int i = 0; i < num_samples; ++i)                                                                                                       \
            {                                                                                                                                          \
                Fixed b(dist_b(gen)), e(dist_e(gen));                                                                                                  \
                const double ref_value = ref_func(static_cast<double>(b), static_cast<double>(e));                                                     \
                if(ref_value > static_cast<double>(std::numeric_limits<Fixed>::max()))                                                                 \
                {                                                                                                                                      \
                    --i;                                                                                                                               \
                    continue;                                                                                                                          \
                }                                                                                                                                      \
                bases.push_back(b);                                                                                                                    \
                exps.push_back(e);                                                                                                                     \
                refs.push_back(ref_value);                                                                                                             \
            }                                                                                                                                          \
                                                                                                                                                       \
            if(bases.empty())                                                                                                                          \
            {                                                                                                                                          \
                state.SkipWithError("all samples are outside the representable range");                                                                \
                return;                                                                                                                                \
            }                                                                                                                                          \
                                                                                                                                                       \
            std::size_t sample_index = 0;                                                                                                              \
            for(auto _ : state)                                                                                                                        \
            {                                                                                                                                          \
                auto b = bases[sample_index % bases.size()];                                                                                           \
                auto e = exps[sample_index % exps.size()];                                                                                             \
                benchmark::DoNotOptimize(b);                                                                                                           \
                benchmark::DoNotOptimize(e);                                                                                                           \
                                                                                                                                                       \
                auto result = test_func(b, e);                                                                                                         \
                benchmark::DoNotOptimize(result);                                                                                                      \
                                                                                                                                                       \
                double ref_value = refs[sample_index % refs.size()];                                                                                   \
                accuracy_stats.update((double)result, ref_value, static_cast<double>(b), ulp);                                                         \
                sample_index++;                                                                                                                        \
            }                                                                                                                                          \
                                                                                                                                                       \
            state.SetLabel(func_name.c_str());                                                                                                         \
        }

EIRIN_BENCH_PERF_ACCURACY_TEST_POW(BM_PowWithAccuracy)

#    define EIRIN_BENCH_PERF_ACCURACY_POW_ARGS \
        EIRIN_BENCH_PERF_PUT_ARGS_POW(0.5, 2, -1, 1, 2000) \
        EIRIN_BENCH_PERF_PUT_ARGS_POW(1, 2, 0.5, 2, 2000)  \
        EIRIN_BENCH_PERF_PUT_ARGS_POW(2, 9, 0, 4, 2000)    \
        EIRIN_BENCH_PERF_PUT_ARGS_POW(0.1, 9, -4, 4, 4000);

#    define EIRIN_BENCH_PERF_ACCURACY_POW(name, bench_func, test_type, test_func, ref_type, ref_func, result, label, func_name) \
        AccuracyStats result;                                                                                                   \
        static void name /**/ (benchmark::State & state)                                                                        \
        {                                                                                                                       \
            bench_func<test_type>(state,                                                                                        \
                                  [](const test_type& b, const test_type& e) { return test_func /**/ (b, e); },                 \
                                  [](ref_type b, ref_type e) { return ref_func /**/ (b, e); }, result, #label);                 \
        }                                                                                                                       \
        [[maybe_unused]] static const eirin_test_register helper_##name = push_accuracy_stats(func_name, result);                \
        BENCHMARK(name)                                                                                                         \
            ->Name("MathAccuracy/" #label)                                                                                      \
                EIRIN_BENCH_PERF_ACCURACY_POW_ARGS

EIRIN_BENCH_PERF_ACCURACY_POW(BM_Pow32_Accuracy, BM_PowWithAccuracy, fixed32, pow_fast, double, std::pow, pow32_accuracy, PowOld32, "Pow Old (fixed32)")
EIRIN_BENCH_PERF_ACCURACY_POW(BM_PowNew32_Accuracy, BM_PowWithAccuracy, fixed32, pow, double, std::pow, pow_new32_accuracy, PowNew32, "Pow New (fixed32)")
EIRIN_BENCH_PERF_ACCURACY_POW(BM_Pow_Accuracy, BM_PowWithAccuracy, fixed64, pow_fast, double, std::pow, pow_accuracy, PowOld64, "Pow Old (fixed64)")
EIRIN_BENCH_PERF_ACCURACY_POW(BM_PowNew_Accuracy, BM_PowWithAccuracy, fixed64, pow, double, std::pow, pow_new_accuracy, PowNew64, "Pow New (fixed64)")

EIRIN_BENCH_PERF_ACCURACY(ATAN, BM_Atan_Accuracy, BM_AtanWithAccuracy, fixed64, atan, double, std::atan, atan_accuracy, Atan, "Atan (fixed64)")
EIRIN_BENCH_PERF_ACCURACY(ATAN, BM_AtanF32_Accuracy, BM_AtanWithAccuracy, fixed32, atan, double, std::atan, atan_f32_accuracy, Atan32, "Atan (fixed32)")

int main(int argc, char** argv)
{
    ::benchmark::Initialize(&argc, argv);

    if(::benchmark::ReportUnrecognizedArguments(argc, argv))
    {
        return 1;
    }

    ::benchmark::RunSpecifiedBenchmarks();

    std::cout << "\n"
              << std::string(60, '=') << "\n";
    std::cout << "ACCURACY REPORT\n";
    std::cout << std::string(60, '=') << "\n";

    for(auto&& pair : accuracy_results)
    {
        pair.second->print(pair.first);
    }

    std::ofstream report("accuracy_report.csv");
    if(report.is_open())
    {
        report << "Function,MaxAbsError,AvgAbsError,MaxRelError,AvgRelError,MaxUlpError,AvgUlpError,Samples\n";
        auto dump = [&report](const char* n, const AccuracyStats& s)
        {
            report << n << "," << s.max_abs_error << "," << s.avg_abs_error << ","
                   << s.max_rel_error << "," << s.avg_rel_error << "," << s.max_ulp_error << ","
                   << s.avg_ulp_error << "," << s.total_samples << "\n";
        };
        for(auto&& pair : accuracy_results)
        {
            dump(pair.first, *pair.second);
        }
    }

    return 0;
}
#endif
