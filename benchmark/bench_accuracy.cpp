#include <eirin/fixed.hpp>
#include <eirin/math.hpp>
#include <eirin/ext/cordic.hpp>
#include <eirin/detail/util.hpp>
#include <benchmark/benchmark.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <random>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>
#include <numbers>

using namespace eirin;

// 配置参数
struct TestConfig
{
    double interval_start = 0.0;
    double interval_end = 2 * std::numbers::pi;
    double subinterval_size = std::numbers::pi / 6.0; // π/6 = 30度
    int samples_per_subinterval = 1000;
    int critical_points_per_subinterval = 10; // 每个子区间额外测试的关键点
    std::string output_csv = "sin_accuracy_report.csv";
};

// 精度统计
struct SubintervalAccuracy
{
    double start;
    double end;
    int total_samples;

    // 泰勒级数
    double taylor_max_error;
    double taylor_avg_error;
    double taylor_max_rel_error;
    double taylor_avg_rel_error;

    // CORDIC
    double cordic_max_error;
    double cordic_avg_error;
    double cordic_max_rel_error;
    double cordic_avg_rel_error;

    // LUT
    double lut_max_error;
    double lut_avg_error;
    double lut_max_rel_error;
    double lut_avg_rel_error;

    // 子区间标记（如"[0, π/6]")
    std::string label;
};

class SinAccuracyTester
{
private:
    TestConfig config;
    std::vector<SubintervalAccuracy> results;

    // 生成子区间标签
    std::string generate_label(double start, double end)
    {
        std::ostringstream oss;

        // 使用特殊符号表示π的倍数
        auto format_as_pi = [](double value) -> std::string
        {
            if(value == 0) return "0";

            // 检查是否是π的简单分数
            const double eps = 1e-10;

            // 检查是否为π的倍数
            for(int n = 1; n <= 6; ++n)
            {
                for(int d = 1; d <= 6; ++d)
                {
                    double fraction = static_cast<double>(n) / d;
                    if(std::abs(value - fraction * std::numbers::pi) < eps)
                    {
                        if(n == 1 && d == 1) return "π";
                        if(n == 1) return "π/" + std::to_string(d);
                        if(d == 1) return std::to_string(n) + "π";
                        return std::to_string(n) + "π/" + std::to_string(d);
                    }
                }
            }

            // 检查是否为2π的倍数
            for(int n = 1; n <= 3; ++n)
            {
                if(std::abs(value - n * 2 * std::numbers::pi) < eps)
                {
                    return std::to_string(n) + "·2π";
                }
            }

            // 否则返回数值
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3) << value;
            return oss.str();
        };

        oss << "[" << format_as_pi(start) << ", " << format_as_pi(end) << "]";
        return oss.str();
    }

    // 在子区间内生成测试点
    std::vector<double> generate_test_points(double start, double end)
    {
        std::vector<double> points;

        // 1. 生成随机点
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist(start, end);

        for(int i = 0; i < config.samples_per_subinterval; ++i)
        {
            points.push_back(dist(gen));
        }

        // 2. 添加关键点
        std::vector<double> critical_points = {
            start, // 区间起点
            end, // 区间终点
            (start + end) / 2.0, // 中点
            start + (end - start) * 0.25, // 1/4处
            start + (end - start) * 0.75 // 3/4处
        };

        // 添加特殊的三角函数关键点（如果在区间内）
        std::vector<double> trig_points = {
            0, std::numbers::pi / 6, std::numbers::pi / 4, std::numbers::pi / 3, std::numbers::pi / 2, 2 * std::numbers::pi / 3, 3 * std::numbers::pi / 4, 5 * std::numbers::pi / 6, std::numbers::pi, 7 * std::numbers::pi / 6, 5 * std::numbers::pi / 4, 4 * std::numbers::pi / 3, 3 * std::numbers::pi / 2, 5 * std::numbers::pi / 3, 7 * std::numbers::pi / 4, 11 * std::numbers::pi / 6, 2 * std::numbers::pi
        };

        for(double pt : trig_points)
        {
            if(pt >= start - 1e-10 && pt <= end + 1e-10)
            {
                critical_points.push_back(pt);
            }
        }

        // 去重并添加到points中
        std::sort(critical_points.begin(), critical_points.end());
        critical_points.erase(
            std::unique(critical_points.begin(), critical_points.end(), [](double a, double b)
                        { return std::abs(a - b) < 1e-10; }),
            critical_points.end()
        );

        for(double pt : critical_points)
        {
            points.push_back(pt);
        }

        return points;
    }

    // 测试单个子区间
    SubintervalAccuracy test_subinterval(double start, double end)
    {
        SubintervalAccuracy result;
        result.start = start;
        result.end = end;
        result.label = generate_label(start, end);

        auto test_points = generate_test_points(start, end);
        result.total_samples = test_points.size();

        // 初始化统计
        result.taylor_max_error = 0;
        result.taylor_avg_error = 0;
        result.taylor_max_rel_error = 0;
        result.taylor_avg_rel_error = 0;

        result.cordic_max_error = 0;
        result.cordic_avg_error = 0;
        result.cordic_max_rel_error = 0;
        result.cordic_avg_rel_error = 0;

        result.lut_max_error = 0;
        result.lut_avg_error = 0;
        result.lut_max_rel_error = 0;
        result.lut_avg_rel_error = 0;

        // 累积误差
        double taylor_error_sum = 0;
        double taylor_rel_error_sum = 0;
        int taylor_rel_samples = 0;

        double cordic_error_sum = 0;
        double cordic_rel_error_sum = 0;
        int cordic_rel_samples = 0;

        double lut_error_sum = 0;
        double lut_rel_error_sum = 0;
        int lut_rel_samples = 0;

        for(double x : test_points)
        {
            // 参考值（标准库）
            double ref = std::sin(x);

            // convert to fixed64
            std::string x_str = std::to_string(x);
            auto fp_x = operator""_f64(x_str.c_str(), x_str.size());

            // 计算各个实现的结果
            double taylor_result = (double)sin(fp_x);
            double cordic_result = (double)cordic_sine(fp_x);
            double lut_result = (double)util::lut::lut_calc_sin(fp_x);

            // 计算泰勒级数误差
            double taylor_error = std::abs(taylor_result - ref);
            result.taylor_max_error = std::max(result.taylor_max_error, taylor_error);
            taylor_error_sum += taylor_error;

            if(ref != 0)
            {
                double rel_error = taylor_error / std::abs(ref);
                result.taylor_max_rel_error = std::max(result.taylor_max_rel_error, rel_error);
                taylor_rel_error_sum += rel_error;
                taylor_rel_samples++;
            }

            // 计算CORDIC误差
            double cordic_error = std::abs(cordic_result - ref);
            result.cordic_max_error = std::max(result.cordic_max_error, cordic_error);
            cordic_error_sum += cordic_error;

            if(ref != 0)
            {
                double rel_error = cordic_error / std::abs(ref);
                result.cordic_max_rel_error = std::max(result.cordic_max_rel_error, rel_error);
                cordic_rel_error_sum += rel_error;
                cordic_rel_samples++;
            }

            // 计算LUT误差
            double lut_error = std::abs(lut_result - ref);
            result.lut_max_error = std::max(result.lut_max_error, lut_error);
            lut_error_sum += lut_error;

            if(ref != 0)
            {
                double rel_error = lut_error / std::abs(ref);
                result.lut_max_rel_error = std::max(result.lut_max_rel_error, rel_error);
                lut_rel_error_sum += rel_error;
                lut_rel_samples++;
            }
        }

        // 计算平均值
        result.taylor_avg_error = taylor_error_sum / test_points.size();
        result.taylor_avg_rel_error = taylor_rel_samples > 0 ?
                                          taylor_rel_error_sum / taylor_rel_samples :
                                          0;

        result.cordic_avg_error = cordic_error_sum / test_points.size();
        result.cordic_avg_rel_error = cordic_rel_samples > 0 ?
                                          cordic_rel_error_sum / cordic_rel_samples :
                                          0;

        result.lut_avg_error = lut_error_sum / test_points.size();
        result.lut_avg_rel_error = lut_rel_samples > 0 ?
                                       lut_rel_error_sum / lut_rel_samples :
                                       0;

        return result;
    }

public:
    SinAccuracyTester(const TestConfig& cfg = TestConfig()) : config(cfg) {}

    void run_tests()
    {
        std::cout << "开始精度测试..." << std::endl;
        std::cout << "测试区间: [0, 2π]" << std::endl;
        std::cout << "子区间大小: π/6 ≈ " << std::numbers::pi / 6 << std::endl;
        std::cout << "每个子区间样本数: " << config.samples_per_subinterval << std::endl;
        std::cout << std::endl;

        results.clear();

        // 生成子区间
        double current = config.interval_start;
        int interval_count = 0;

        while(current < config.interval_end - 1e-10)
        {
            double next = std::min(current + config.subinterval_size, config.interval_end);

            std::cout << "测试子区间 " << (interval_count + 1) << ": ";
            std::cout << "[" << current << ", " << next << "]" << std::endl;

            auto result = test_subinterval(current, next);
            results.push_back(result);

            current = next;
            interval_count++;
        }

        std::cout << "\n完成 " << interval_count << " 个子区间的测试" << std::endl;
    }

    void save_to_csv()
    {
        std::ofstream file(config.output_csv);
        if(!file.is_open())
        {
            std::cerr << "无法打开文件: " << config.output_csv << std::endl;
            return;
        }

        // 写入CSV头部
        file << "subinterval_label,start,end,total_samples,"
             << "taylor_max_error,taylor_avg_error,taylor_max_rel_error,taylor_avg_rel_error,"
             << "cordic_max_error,cordic_avg_error,cordic_max_rel_error,cordic_avg_rel_error,"
             << "lut_max_error,lut_avg_error,lut_max_rel_error,lut_avg_rel_error\n";

        // 写入数据
        for(const auto& result : results)
        {
            file << "\"" << result.label << "\","
                 << result.start << "," << result.end << "," << result.total_samples << ","
                 << std::scientific << std::setprecision(15)
                 << result.taylor_max_error << ","
                 << result.taylor_avg_error << ","
                 << result.taylor_max_rel_error << ","
                 << result.taylor_avg_rel_error << ","
                 << result.cordic_max_error << ","
                 << result.cordic_avg_error << ","
                 << result.cordic_max_rel_error << ","
                 << result.cordic_avg_rel_error << ","
                 << result.lut_max_error << ","
                 << result.lut_avg_error << ","
                 << result.lut_max_rel_error << ","
                 << result.lut_avg_rel_error << "\n";
        }

        file.close();
        std::cout << "结果已保存到: " << config.output_csv << std::endl;
    }

    void print_summary()
    {
        std::cout << "\n=== 精度测试汇总 ===" << std::endl;
        std::cout << std::string(80, '=') << std::endl;

        // 打印表格头部
        std::cout << std::setw(15) << "子区间"
                  << std::setw(15) << "泰勒最大误差"
                  << std::setw(15) << "CORDIC最大误差"
                  << std::setw(15) << "LUT最大误差"
                  << std::endl;
        std::cout << std::string(60, '-') << std::endl;

        // 打印每个子区间的结果
        for(const auto& result : results)
        {
            std::cout << std::setw(15) << result.label
                      << std::setw(15) << std::scientific << std::setprecision(2)
                      << result.taylor_max_error
                      << std::setw(15) << result.cordic_max_error
                      << std::setw(15) << result.lut_max_error
                      << std::endl;
        }

        // 计算全局统计数据
        double global_taylor_max_error = 0;
        double global_cordic_max_error = 0;
        double global_lut_max_error = 0;

        for(const auto& result : results)
        {
            global_taylor_max_error = std::max(global_taylor_max_error, result.taylor_max_error);
            global_cordic_max_error = std::max(global_cordic_max_error, result.cordic_max_error);
            global_lut_max_error = std::max(global_lut_max_error, result.lut_max_error);
        }

        std::cout << std::string(60, '-') << std::endl;
        std::cout << std::setw(15) << "全局最大误差"
                  << std::setw(15) << global_taylor_max_error
                  << std::setw(15) << global_cordic_max_error
                  << std::setw(15) << global_lut_max_error
                  << std::endl;
    }
};

int main()
{
    TestConfig config;
    config.samples_per_subinterval = 5000; // 每个子区间5000个样本
    config.output_csv = "sin_accuracy_by_subinterval.csv";

    SinAccuracyTester tester(config);
    tester.run_tests();
    tester.save_to_csv();
    tester.print_summary();

    return 0;
}
