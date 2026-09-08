#ifndef EIRIN_MATH_PERF_HPP
#define EIRIN_MATH_PERF_HPP

#pragma once

#include <cmath>
#include <cstdio>
#include "../eirin.hpp"
#include "int128.hpp"

namespace eirin
{
namespace perf
{
    template <typename T>
    struct esp_ret
    {
        T max_esp;
        T min_esp;
        T max_esp_input;
        T min_esp_input;

        esp_ret(T max_esp, T min_esp, T max_esp_input, T min_esp_input)
            : max_esp(max_esp), min_esp(min_esp), max_esp_input(max_esp_input), min_esp_input(min_esp_input)
        {}
    };

    template <typename T, typename Func, typename StdFunc>
    esp_ret<T> measure_esp(T start, T end, T step, Func func, StdFunc std_func, T(initializer)(int64_t))
    {
        T max_esp = initializer(0), min_esp = initializer(0XFF), max_input = initializer(0), min_input = initializer(0);
        for(T x = start; x <= end; x += step)
        {
            auto sin_val = func(x);
            auto std_sin_val = std_func(x);
            auto esp = abs(std_sin_val - sin_val);
            if(esp > max_esp)
            {
                max_esp = esp;
                max_input = x;
            }
            if(esp < min_esp)
            {
                min_esp = esp;
                min_input = x;
            }
        }
        return {max_esp, min_esp, max_input, min_input};
    }
} // namespace perf
} // namespace eirin

#endif // EIRIN_MATH_PERF_HPP
