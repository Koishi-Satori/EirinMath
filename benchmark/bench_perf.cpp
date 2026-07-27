#include <eirin/fixed.hpp>
#include <eirin/math.hpp>
#include <benchmark/benchmark.h>
#include "bench.hpp"
#include <eirin/ext/cordic.hpp>
#include <eirin/detail/util.hpp>
#include <vector>
#include <random>
#include <chrono>
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

static void taylor_sin(benchmark::State& state)
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

static void cordic_sin(benchmark::State& state)
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

static void lut_sin(benchmark::State& state)
{
    auto fp1 = F64_FROM_BENCH(0);
    for(auto _ : state)
    {
        auto input = fp1;
        benchmark::DoNotOptimize(input);

        auto result = util::lut::lut_calc_sin(input);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

static void double_sin(benchmark::State& state)
{
    double double_val = state.range(0) / 1000.0;
    for(auto _ : state)
    {
        auto input = double_val;
        benchmark::DoNotOptimize(input);

        auto result = std::sin(double_val);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(taylor_sin)->Args({BENCH_F64_VAL(11.4514)});
BENCHMARK(cordic_sin)->Args({BENCH_F64_VAL(11.4514)});
BENCHMARK(lut_sin)->Args({BENCH_F64_VAL(11.4514)});
BENCHMARK(double_sin)->Args({114514});

// 参数化基准测试模板
template <typename SinFunc>
static void BM_SinFunction(benchmark::State& state, SinFunc sin_func, const std::string& name)
{
    // 获取输入范围参数
    double min_val = state.range(0);
    double max_val = state.range(1);
    int num_samples = state.range(2);

    // 生成测试样本
    std::vector<double> samples;
    samples.reserve(num_samples);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(min_val, max_val);

    for(int i = 0; i < num_samples; ++i)
    {
        samples.push_back(dist(gen));
    }

    // 性能统计
    double total_time = 0;
    int iteration_count = 0;
    for(auto _ : state)
    {
        auto fp1 = fixed64(samples[iteration_count % num_samples]);
        auto start = std::chrono::high_resolution_clock::now();

        sin_func(fp1);

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        total_time += elapsed.count();
        iteration_count++;

        // 设置迭代时间
        state.SetIterationTime(elapsed.count() / 1e9);
    }

    // 设置统计信息
    state.counters["avg_time_ns"] = total_time / iteration_count;
    state.counters["throughput"] = benchmark::Counter(
        1e9 / (total_time / iteration_count),
        benchmark::Counter::kIsRate
    );

    // 记录测试范围
    state.SetLabel(("[" + std::to_string(min_val) + ", " +
                    std::to_string(max_val) + "], samples=" +
                    std::to_string(num_samples))
                       .c_str());
}

// 包装函数
static void BM_TaylorSin(benchmark::State& state)
{
    BM_SinFunction(state, [](const auto& x)
                   { return sin(x); },
                   "TaylorSin");
}

static void BM_CordicSin(benchmark::State& state)
{
    BM_SinFunction(state, [](const auto& x)
                   { return cordic_sine(x); },
                   "CordicSin");
}

static void BM_LUTSin(benchmark::State& state)
{
    BM_SinFunction(state, [](const auto& x)
                   { return util::lut::lut_calc_sin(x); },
                   "LUTSin");
}

// 使用BENCHMARK宏注册参数化测试
BENCHMARK(BM_TaylorSin)
    ->Name("Sin/Taylor")
    ->Unit(benchmark::kNanosecond)
    ->Args({0, static_cast<int64_t>(M_PI / 2), 1000}) // 第一象限
    ->Args({static_cast<int64_t>(M_PI / 2), static_cast<int64_t>(M_PI), 1000}) // 第二象限
    ->Args({static_cast<int64_t>(M_PI), static_cast<int64_t>(3 * M_PI / 2), 1000}) // 第三象限
    ->Args({static_cast<int64_t>(3 * M_PI / 2), static_cast<int64_t>(2 * M_PI), 1000}) // 第四象限
    ->Args({static_cast<int64_t>(-M_PI), static_cast<int64_t>(M_PI), 2000}) // 主值区间
    ->Args({static_cast<int64_t>(-10 * M_PI), static_cast<int64_t>(10 * M_PI), 5000}) // 大范围
    ->Args({0, 0, 500}) // 接近0，注意：这里min=max=0，实际使用时会特殊处理
    ->UseRealTime();

BENCHMARK(BM_CordicSin)
    ->Name("Sin/CORDIC")
    ->Unit(benchmark::kNanosecond)
    ->Args({0, static_cast<int64_t>(M_PI / 2), 1000})
    ->Args({static_cast<int64_t>(M_PI / 2), static_cast<int64_t>(M_PI), 1000})
    ->Args({static_cast<int64_t>(M_PI), static_cast<int64_t>(3 * M_PI / 2), 1000})
    ->Args({static_cast<int64_t>(3 * M_PI / 2), static_cast<int64_t>(2 * M_PI), 1000})
    ->Args({static_cast<int64_t>(-M_PI), static_cast<int64_t>(M_PI), 2000})
    ->Args({static_cast<int64_t>(-10 * M_PI), static_cast<int64_t>(10 * M_PI), 5000})
    ->Args({0, 0, 500})
    ->UseRealTime();

BENCHMARK(BM_LUTSin)
    ->Name("Sin/LUT")
    ->Unit(benchmark::kNanosecond)
    ->Args({0, static_cast<int64_t>(M_PI / 2), 1000})
    ->Args({static_cast<int64_t>(M_PI / 2), static_cast<int64_t>(M_PI), 1000})
    ->Args({static_cast<int64_t>(M_PI), static_cast<int64_t>(3 * M_PI / 2), 1000})
    ->Args({static_cast<int64_t>(3 * M_PI / 2), static_cast<int64_t>(2 * M_PI), 1000})
    ->Args({static_cast<int64_t>(-M_PI), static_cast<int64_t>(M_PI), 2000})
    ->Args({static_cast<int64_t>(-10 * M_PI), static_cast<int64_t>(10 * M_PI), 5000})
    ->Args({0, 0, 500})
    ->UseRealTime();

// 精度验证结构体
struct AccuracyStats
{
    double max_abs_error = 0;
    double avg_abs_error = 0;
    double max_rel_error = 0;
    double avg_rel_error = 0;
    int total_samples = 0;

    void update(double computed, double reference)
    {
        double abs_error = std::abs(computed - reference);
        max_abs_error = std::max(max_abs_error, abs_error);
        avg_abs_error += (abs_error - avg_abs_error) / (total_samples + 1);

        if(reference != 0)
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
        std::cout << "  Samples: " << total_samples << "\n";
    }
};

// 全局精度统计
AccuracyStats taylor_accuracy, cordic_accuracy, lut_accuracy;

// 带精度验证的基准测试
template <typename SinFunc, typename ReferenceFunc>
static void BM_SinWithAccuracy(benchmark::State& state, SinFunc sin_func, ReferenceFunc ref_func, AccuracyStats& accuracy_stats, const std::string& func_name)
{
    // 参数解析
    double min_val = state.range(0);
    double max_val = state.range(1);
    int num_samples = state.range(2);

    // 特殊处理：如果min_val == max_val，测试特定点
    if(min_val == max_val && min_val == 0)
    {
        // 测试接近0的点
        min_val = -0.001;
        max_val = 0.001;
    }

    // 生成测试样本
    std::vector<double> samples;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(min_val, max_val);

    for(int i = 0; i < num_samples; ++i)
    {
        samples.push_back(dist(gen));
    }

    // 添加关键点
    std::vector<double> critical_points = {
        0.0, M_PI / 6, M_PI / 4, M_PI / 3, M_PI / 2, M_PI, 2 * M_PI, -M_PI / 6, -M_PI / 4, -M_PI / 3, -M_PI / 2, -M_PI
    };

    for(auto pt : critical_points)
    {
        if(pt >= min_val && pt <= max_val)
        {
            samples.push_back(pt);
        }
    }

    // 基准测试
    size_t sample_index = 0;
    double total_time = 0;
    int iteration_count = 0;

    for(auto _ : state)
    {
        double input = samples[sample_index % samples.size()];

        // 创建输入
        auto fp1 = fixed64(input);

        // 执行计算并计时
        auto start = std::chrono::high_resolution_clock::now();

        auto result = sin_func(fp1);

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        // 更新精度统计
        double ref_value = ref_func(input);
        accuracy_stats.update((double)result, ref_value);

        // 更新性能统计
        total_time += elapsed.count();
        iteration_count++;
        sample_index++;

        // 防止编译器优化
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();

        // 设置迭代时间
        state.SetIterationTime(elapsed.count() / 1e9);
    }

    // 设置计数器
    state.counters["avg_ns"] = total_time / iteration_count;
    state.counters["throughput"] = benchmark::Counter(
        1e9 / (total_time / iteration_count),
        benchmark::Counter::kIsRate
    );

    state.counters["max_rel_error"] = accuracy_stats.max_rel_error;
    state.counters["avg_rel_error"] = accuracy_stats.avg_rel_error;

    // 设置标签
    state.SetLabel(func_name.c_str());
}

// 包装函数
static void BM_TaylorSin_Accuracy(benchmark::State& state)
{
    BM_SinWithAccuracy(state, [](const auto& x)
                       { return sin(x); },
                       [](double x)
                       { return std::sin(x); },
                       taylor_accuracy,
                       "TaylorSin");
}

static void BM_CordicSin_Accuracy(benchmark::State& state)
{
    BM_SinWithAccuracy(state, [](const auto& x)
                       { return cordic_sine(x); },
                       [](double x)
                       { return std::sin(x); },
                       cordic_accuracy,
                       "CordicSin");
}

static void BM_LUTSin_Accuracy(benchmark::State& state)
{
    BM_SinWithAccuracy(state, [](const auto& x)
                       { return util::lut::lut_calc_sin(x); },
                       [](double x)
                       { return std::sin(x); },
                       lut_accuracy,
                       "LUTSin");
}

// 注册测试
BENCHMARK(BM_TaylorSin_Accuracy)
    ->Name("SinAccuracy/Taylor")
    ->Args({0, static_cast<int64_t>(M_PI / 2), 1000})
    ->Args({static_cast<int64_t>(M_PI / 2), static_cast<int64_t>(M_PI), 1000})
    ->Args({static_cast<int64_t>(M_PI), static_cast<int64_t>(3 * M_PI / 2), 1000})
    ->Args({static_cast<int64_t>(3 * M_PI / 2), static_cast<int64_t>(2 * M_PI), 1000})
    ->Args({static_cast<int64_t>(-M_PI), static_cast<int64_t>(M_PI), 2000})
    ->UseRealTime();

BENCHMARK(BM_CordicSin_Accuracy)
    ->Name("SinAccuracy/CORDIC")
    ->Args({0, static_cast<int64_t>(M_PI / 2), 1000})
    ->Args({static_cast<int64_t>(M_PI / 2), static_cast<int64_t>(M_PI), 1000})
    ->Args({static_cast<int64_t>(M_PI), static_cast<int64_t>(3 * M_PI / 2), 1000})
    ->Args({static_cast<int64_t>(3 * M_PI / 2), static_cast<int64_t>(2 * M_PI), 1000})
    ->Args({static_cast<int64_t>(-M_PI), static_cast<int64_t>(M_PI), 2000})
    ->UseRealTime();

BENCHMARK(BM_LUTSin_Accuracy)
    ->Name("SinAccuracy/LUT")
    ->Args({0, static_cast<int64_t>(M_PI / 2), 1000})
    ->Args({static_cast<int64_t>(M_PI / 2), static_cast<int64_t>(M_PI), 1000})
    ->Args({static_cast<int64_t>(M_PI), static_cast<int64_t>(3 * M_PI / 2), 1000})
    ->Args({static_cast<int64_t>(3 * M_PI / 2), static_cast<int64_t>(2 * M_PI), 1000})
    ->Args({static_cast<int64_t>(-M_PI), static_cast<int64_t>(M_PI), 2000})
    ->UseRealTime();

int main(int argc, char** argv)
{
    ::benchmark::Initialize(&argc, argv);

    if(::benchmark::ReportUnrecognizedArguments(argc, argv))
    {
        return 1;
    }

    // 运行基准测试
    ::benchmark::RunSpecifiedBenchmarks();

    // 输出精度报告
    std::cout << "\n"
              << std::string(60, '=') << "\n";
    std::cout << "ACCURACY REPORT\n";
    std::cout << std::string(60, '=') << "\n";

    taylor_accuracy.print("Taylor Series Sin");
    cordic_accuracy.print("CORDIC Sin");
    lut_accuracy.print("LUT Sin");

    // 输出到文件
    std::ofstream report("accuracy_report.csv");
    if(report.is_open())
    {
        report << "Function,MaxAbsError,AvgAbsError,MaxRelError,AvgRelError,Samples\n";
        report << "Taylor," << taylor_accuracy.max_abs_error << ","
               << taylor_accuracy.avg_abs_error << ","
               << taylor_accuracy.max_rel_error << ","
               << taylor_accuracy.avg_rel_error << ","
               << taylor_accuracy.total_samples << "\n";
        report << "CORDIC," << cordic_accuracy.max_abs_error << ","
               << cordic_accuracy.avg_abs_error << ","
               << cordic_accuracy.max_rel_error << ","
               << cordic_accuracy.avg_rel_error << ","
               << cordic_accuracy.total_samples << "\n";
        report << "LUT," << lut_accuracy.max_abs_error << ","
               << lut_accuracy.avg_abs_error << ","
               << lut_accuracy.max_rel_error << ","
               << lut_accuracy.avg_rel_error << ","
               << lut_accuracy.total_samples << "\n";
    }

    return 0;
}
#endif
