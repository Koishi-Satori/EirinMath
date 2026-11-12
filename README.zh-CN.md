# EirinFixed

- Original: [en-us](README.md)

一个灵活且高精度的C++定点数库，提供了定点数模板类、高精度数学运算和基本都输入输出函数。可以运行```eirin_fixed.benchmark```和```double.benchmark```这两个基准测试来测试定点数类型与C++ double类型之间的性能差异，或者查看在benchmark目录下[-O3](benchmark/fixed_benchmark_O3.txt)与[-O2](benchmark/fixed_benchmark_O2.txt)优化下的运行结果，测试环境为AMD R7-7735H。

同时提供了已定义的32位宽度、16位精度定点数(```fixed32```)和64位宽度、32位精度定点数(```fixed64```)。
`fixed64`使用int128作为中间计算类型，部分编译器可能没有int128拓展导致无法使用fixed64。

### 创建定点数

可以使用字面量或者将浮点数整形类型传入构造函数来创建定点数。

- 字面量目前只支持 `fixed32`与`fixed64`
- 不建议传入浮点数来创建定点数
    - 浮点数在不同平台/编译器/设备/操作系统上可能存在误差
    - 定点数设计目的为避免该误差。

```c++
#include <fixed.hpp>

using namespace eirin;

int main(int argc, char** argv)
{
    auto fp32_1 = fixed32(495);
    auto fp32_2 = fixed32(114.514); // using "114.5"_f32 is better.
    auto fp1 = fixed_num<int32_t, int64_t, 20, false>(5);
    auto fp2 = fixed_num<int32_t, int64_t, 20, false>(5.14);
    fp32_1 = 114_f32;
    fp32_2 = 5.14_f32;
    fp32_1 = "49.5"_f32;
    fp32_2 = "114.5"_f32; // multi-platforms work-well!
    fp32_2 = -"114.5"_f32;
    fp32_2 = "-114.5"_f32; // both this and the one above are well.
    return 0;
}
```

也可以通过std::basic_istream或者字符串来创建定点数。

- 函数`f32_from_cstring`和`fixed_from_cstring`在成功时返回true。
- 函数`parse`需要传入字符串的末尾。
    - 该函数在碰到末尾字符串的第一个字符时会立刻停止解析
    - 例如，"114a.514a"只会解析"114"部分

```c++
#include <fixed.hpp>

using namespace eirin;

int main(int argc, char** argv)
{
    fp1 = 0_f32;
    fp2 = fixed_num<int32_t, int64_t, 20, false>(0);
    f32_from_cstring("-114.514", 8, fp1);
    fixed_from_cstring("-5", 2, fp2);
    std::cin >> fp;
    parse("114a.514a", "a", fp);
    return 0;
}
```

### 定点数数学计算

提供了纯C++实现的高精度、高性能实现。

部分数学常量在定点数模板类中已有定义。

引用:
- Trigonometric Functions: [Efficient Approximations for the Arctangent Function](https://ieeexplore.ieee.org/document/1628884)
- Square Root: [Fixed point sqrt](https://groups.google.com/g/comp.lang.c/c/IpwKbw0MAxw)
- Binary Logarithm: [A Fast Binary Logarithm Algorithm](http://www.claysturner.com/dsp/BinaryLogarithm.pdf)

支持的函数:
- rounding functions
    - ceil/floor
    - trunc
    - round
- abs
- min/max
- sqrt
- trigonometric functions
    - sin/cos/tan
    - asin/acos/atan
- cbrt
- log2/log/log10
- pow/exp

### 支持的编译器

需要至少>=C++20.

- MSVC 19.3+ (Visual Studio 2022)
- GCC 12+
- Clang 15+ with libc++
- Clang 16+ with libstdc++

## License
[MIT](LICENSE) License
