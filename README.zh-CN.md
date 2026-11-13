# EirinMath

- 其他语言：[English](README.md)

*EirinMath* 是一个 C++20 header-only数学库，提供了可配置精度的定点数模板、高性能高精度且确定性的定点数学库、GLSL 风格向量、列主序矩阵，以及面向可移植 SIMD 的内核结构。

可以运行 `eirin_math.benchmark` 和 `double.benchmark`，比较定点数与 C++ `double` 的性能；也可以选择性运行 `eirin_math.perf`，查看定点数运算和数学函数的精度与性能。

*EirinMath* 提供定点数模板 `fixed_num<T, I, f, r>`，并预定义了 32 位宽度、16 位精度的 `fixed32`，以及 64 位宽度、32 位精度的 `fixed64`。

本库可以满足定点数的主要需求：保证在不同平台、设备、操作系统和编译器上都能得到一致的计算结果。
此外，`fixed64` 会使用编译器扩展提供的128位整数作为中间计算类型，而在某些没有int128拓展的编译器会回退到库中内建的int128实现。MSVC上现在默认提供内建的int128实现（MSVC原先的128位乘法存在性能问题，维护者发起的优化PR已经被合入MSVC-STL。），但当设置了`EIRIN_USE_EXT_BUILTIN_INT128 == EIRIN_FALSE`时会强制使用`__msvc_int128.hpp`提供的`std::Signed128`和`std::Unsigned128`；GCC和Clang上则默认直接使用`__int128`。

C++26 风格的饱和算术实现在 [numeric.hpp](include/eirin/numeric.hpp) 中，支持定点数、整数，以及所有满足 `eirin::detail::is_integral_v` 的用户自定义整数类型。

作为额外功能，*EirinMath* 还提供了向量类 `tvec<T, N>` 及其在 `vec.hpp` 中的部分特化。它支持类似 GLSL 的 swizzle，但使用函数调用语法 `()`，例如：

```c++
eirin::f64vec4 v{495, 514_f64, 19, 0};
eirin::ivec4 vi{114, 514.0, 0, 0};
v.xyz() += 1_f32;
vi.xyzw() = eirin::ivec4{1, 2, 3, 4} + v.xyxy();
```

矩阵库提供 `tmat<C, R, T>` 以及方阵和矩形矩阵的常用别名。矩阵采用列主序：`m[i]` 表示第 `i` 列，`m(row, column)` 表示特定位置的元素。在形状和元素类型允许的情况下，矩阵支持逐元素运算、标量运算、代数矩阵乘法、矩阵与向量乘法、转置、行列式和逆矩阵。行列式和逆矩阵仅支持元素类型为小数类型的方阵。当前矩阵实现仍使用标量计算内核；SIMD 矩阵内核只有特化脚手架，尚未实现。

## 定点数库

- 库中和全文所指的定点数均为Q格式定点数，基于二进制小数位表示。

### 创建定点数

可以通过构造函数或字面量，用整数或浮点类型创建定点数。

- 字面量目前只支持 `fixed32` 和 `fixed64`。
- 不建议在跨平台代码中使用浮点数构造定点数。浮点常量的二进制表示可能随平台和编译器变化，从而引入结果差异；更推荐使用字符串字面量。

```c++
#include <eirin/fixed.hpp>

using namespace eirin;

int main(int argc, char** argv)
{
    auto fp32_1 = fixed32(495);
    auto fp32_2 = fixed32(114.514); // 更推荐使用 "114.5"_f32
    auto fp1 = fixed_num<int32_t, int64_t, 20, false>(5);
    auto fp2 = fixed_num<int32_t, int64_t, 20, false>(5.14);
    fp32_1 = 114_f32;
    fp32_2 = 5.14_f32;
    fp32_1 = "49.5"_f32;
    fp32_2 = "114.5"_f32; // 跨平台结果更稳定
    fp32_2 = -"114.5"_f32;
    fp32_2 = "-114.5"_f32; // 两种写法都可以
    return 0;
}
```

也可以通过 `std::basic_istream` 或字符串创建定点数。

- `f32_from_cstring` 和 `fixed_from_cstring` 成功时返回 `true`。
- `parse` 函数需要传入结束字符串。
    - 当遇到结束字符串的**第一个字符**时，函数会立即停止解析。
    - 例如，解析 `114a.514a` 时只会得到 `114`。

```c++
#include <eirin/fixed.hpp>

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

### 输出定点数

可以使用 `std::ostream` 或 `std::format` 输出定点数。使用格式化功能时需要额外包含格式头文件。

```c++
#include <eirin/fixed.hpp>
#include <eirin/io/format.hpp>

using namespace eirin;

int main(int argc, char** argv)
{
    std::cout << "114.5625"_f32 << std::endl;
    std::cout << "-114.5625"_f32 << std::endl;
    std::cout << std::format("{:s}", 114.5625_f32) << std::endl;
    std::cout << std::format("{:i}", 114.5625_f32) << std::endl;
    std::cout << std::format("{:f}", 114.5625_f32) << std::endl;
    return 0;
}
```

### 从内部值创建

- **注意：除非你很清楚内部表示的语义，否则不推荐使用这种方式。**

```c++
#include <eirin/fixed.hpp>

using namespace eirin;

int main(int argc, char** argv)
{
    auto fp = fixed32::from_internal_value(7504789);
    auto log2_10 = fixed32::template from_fixed_num_value<60>(0x35269E12F346E200ll);
    return 0;
}
```

### 定点数运算符

定点数可以转换为整数或浮点类型。

```c++
#include <eirin/fixed.hpp>

using namespace eirin;

(int) "114.5"_f32;
(float) "114.5"_f32;
```

可以在定点数与定点数、定点数与整数、整数与定点数之间执行四则运算和取模运算。结果始终是定点数。

```c++
#include <eirin/fixed.hpp>

using namespace eirin;

fp1 = "114.5"_f32;
fp2 = "514"_f32;
fp1 + fp2;
fp1 + 100;
100 + fp2;
fp1 *= 2;
fp1 /= fp2;
```

比较函数的用法与普通数值一致。

### 定点数学函数

提供高精度、高性能的纯 C++ 实现。预定义数学常量定义在定点数类型中。

参考实现：

- 三角函数：[Efficient Approximations for the Arctangent Function](https://ieeexplore.ieee.org/document/1628884)
- 平方根：[Fixed point sqrt](https://groups.google.com/g/comp.lang.c/c/IpwKbw0MAxw)
- 二进制对数：[A Fast Binary Logarithm Algorithm](http://www.claysturner.com/dsp/BinaryLogarithm.pdf)

支持的函数：

- 舍入函数
    - `ceil` / `floor`
    - `trunc`
    - `round`
- `abs`
- `min` / `max`
- `sqrt`
- 三角函数
    - `sin` / `cos` / `tan`
    - `asin` / `acos` / `atan`
- `cbrt`
- `log2` / `log` / `log10`
- `pow` / `exp`

## 向量库

*EirinMath* 提供支持 GLSL swizzle 的向量库。

### 创建向量

向量由 `tvec<N, T>` 模板提供，并带有常用类型的别名：

- `vec2<T>`、`vec3<T>`、`vec4<T>`，底层均为 `tvec<N, T>`。
- 预定义别名包括 `vec2i`、`vec3i`、`vec4fixed64`、`vec4fixed32` 等，详见 `vec.hpp`。
- 别名命名规则为 `vec[维度][类型][可选宽度]`，例如 `vec2i32`、`vec3f128`、`vec3fixed64`。

| 向量类型 | int | float | 定宽整数 | 定宽浮点 | 定点数 |
| --- | --- | --- | --- | --- | --- |
| vec2 | `vec2i` | `vec2f` | `vec2i[width]` | `vec2f[width]` | `vec2fixed<T, I, f, r>` |
| vec3 | `vec3i` | `vec3f` | `vec3i[width]` | `vec3f[width]` | `vec3fixed<T, I, f, r>` |
| vec4 | `vec4i` | `vec4f` | `vec4i[width]` | `vec4f[width]` | `vec4fixed<T, I, f, r>` |

构造函数：

```c++
#include <eirin/vec.hpp>

using namespace eirin;

int main()
{
    // 默认构造，不初始化
    vec2<int> v1;

    // 标量构造，所有分量取相同值
    vec3<float> v2(1.0f);

    // 分别指定各分量
    vec4<int> v3(1, 2, 3, 4);

    // 从另一个向量隐式转换
    vec2<float> v4(v3);   // 将各分量转换为 float

    // 从较小向量和额外分量构造
    vec3<double> v5(vec2<double>(1.0, 2.0), 3.0);

    // 使用花括号统一初始化
    vec4<int> v6{1, 2, 3, 4};
    return 0;
}
```

### 访问元素

可以通过命名成员 `.x`、`.y`、`.z`、`.w` 访问元素，也可以通过 `operator[]` 访问。

```c++
vec4<int> v{1, 2, 3, 4};
v.x = 10;
int a = v[2];   // a = 3
```

### Swizzle 操作

*EirinMath* 支持使用函数调用语法 `.xy()`、`.xyz()`、`.yxw()` 等构造 swizzle。掩码长度为 2、3 或 4，也允许重复分量，例如 `.xx()`、`.xyx()`。

- 读取 swizzle 会返回一个对应长度的新向量。
- 赋值写入时，右侧向量维度必须与 swizzle 掩码长度一致。
- `+=`、`-=`、`*=`、`/=` 等复合赋值同样适用于 swizzle 表达式，但维度必须匹配。

```c++
vec4<int> v{1, 2, 3, 4};

// 读取
auto v2 = v.xy();               // v2 = (1, 2)
auto v3 = v.zyx();              // v3 = (3, 2, 1)

// 写入
v.xy() = vec2<int>{5, 6};       // v = (5, 6, 3, 4)
v.xz() += vec2<int>{1, 1};      // v = (6, 6, 4, 4)

// 读取时允许重复索引
auto xx = v.xx();               // (6, 6)

// 写入时不允许重复索引作为左值
v.xx() = vec2<int>{7, 8};       // 编译失败
```

> 与 GLSL/GLM 类似，*EirinMath* 不允许通过重复索引进行修改，例如 `v.xx() += 1`。

### 向量运算符和函数

向量支持常见的算术运算符：`+`、`-`、`*`、`/`、`%`、位运算 `&`、`|`、`^` 和移位，并可用于标量与维度匹配的向量。

```c++
vec2<int> a{1, 2}, b{3, 4};
auto c = a + b;               // (4, 6)
auto d = a * 2;               // (2, 4)
auto e = 3 * a;               // (3, 6)
a += b;                       // a 变为 (4, 6)
```

`tvec` 还提供常用向量函数：

- `dot`：点积。
- `cross`：叉积，仅用于 2 维或 3 维向量；标准叉积主要定义在 3 维和 7 维空间中。

## 矩阵库

矩阵由 `mat.hpp` 中的 `tmat<C, R, T>` 提供，其中 `C` 是列数，`R` 是行数。常用别名包括 `mat2`、`mat3`、`mat4`，以及矩形矩阵 `mat2x3` 和 `mat3x2`。

矩阵采用列主序存储和索引：

```c++
#include <eirin/mat.hpp>

using namespace eirin;

mat3<int> m(
    tvec<3, int>(1, 2, 3), // 第 0 列
    tvec<3, int>(4, 5, 6), // 第 1 列
    tvec<3, int>(7, 8, 9)  // 第 2 列
);

auto col0 = m[0];       // (1, 2, 3)
int value = m(2, 0);    // 第 2 行、第 0 列，值为 7
```

矩阵 API 包括：

- 单位矩阵和零矩阵构造、分量类型转换以及矩阵嵌入；
- 逐元素 `+`、`-`，以及标量 `+`、`-`、`*`、`/`；
- 代数矩阵乘法、矩阵与向量乘法，以及合法的矩形矩阵乘法；
- 所有支持形状的 `transpose`；
- 方阵的 `determinant` 和 `inverse`；`inverse` 要求小数类型，且奇异矩阵的逆矩阵行为未定义；
- `data()` 和 `to_array()`，均按列主序连续排列。

与 `tvec` 的逐元素 `*` 不同，`mat * mat` 执行的是代数矩阵乘法。矩阵别名的元素类型覆盖整数、浮点数、定点数，以及平台支持时的 128 位整数。

当前 `matrix_kernel_selector` 仍选择标量内核。`matrix_func_simd.hpp` 目前只是特化脚手架，且矩阵库还没有 benchmark。在这些内核和 benchmark 完成前，不对矩阵库作 SIMD 或相对性能承诺。

## 精度与性能快照

仓库包含 Google Benchmark 测试套件，覆盖定点算术、数学函数、饱和算术和 `tvec`。下面的数据是代表性测量，不构成性能保证。测试环境为 AMD Ryzen 7 9700X、GCC 14.2、Release 构建（`-O3 -DNDEBUG`）和 Google Benchmark；性能数据为三次运行的中位数，输入范围来自 `benchmark/bench_perf.cpp`。

精度以 `double` 为参考，并基于量化后的定点输入进行统计。真实结果超出定点数可表示范围的样本会被排除。ULP 相对于定点数精度计算。

| 函数 | 类型 | 最大绝对误差 | 最大相对误差 | 最大 ULP 误差 |
| --- | --- | ---: | ---: | ---: |
| `sin` | `fixed32` | 1.09e-4 | 7.13e-1 | 7.13 |
| `sin` | `fixed64` | 2.02e-8 | 1.03e-5 | 86.7 |
| `sqrt` | `fixed32` | 1.53e-5 | 1.80e-4 | 1.00 |
| `sqrt` | `fixed64` | 2.33e-10 | 1.85e-8 | 1.00 |
| `exp` | `fixed32` | 1.09e-4 | 3.32e-1 | 7.16 |
| `exp` | `fixed64` | 9.94e-5 | 6.53e-6 | 4.27e5 |

`exp(fixed64)` 的大 ULP 误差出现在结果幅值较大的区间，其相对误差仍在约 `1e-6`。这些数据用于展示精度与性能之间的取舍，并不表示所有场景都具有统一的 ULP 上界。由于 `sin` 的零点附近参考值趋近于 0，相对误差也可能明显变大；此时绝对误差和 ULP 更有参考价值。

单输入代表性微基准：

| 函数 | `fixed32` | `fixed64` | `double` |
| --- | ---: | ---: | ---: |
| `sin` | 4.65 ns | 4.98 ns | 5.95 ns |
| `sqrt` | 23.3 ns | 3.92 ns | 2.70 ns |
| `exp` | 4.59 ns | 6.31 ns | 2.04 ns |

同一机器上现有的 `fixed64` 向量微基准结果约为：

| 操作 | `vec2<fixed64>` | `vec3<fixed64>` | `vec4<fixed64>` |
| --- | ---: | ---: | ---: |
| 加法 | 0.185 ns | 0.375 ns | 0.457 ns |
| 点积 | 0.737 ns | 1.10 ns | — |
| 叉积 | 0.735 ns | 2.39 ns | — |

可以按以下方式复现测量结果：

```sh
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -Deirin_build_advanced_benchmark=ON
cmake --build build
./build/bin/eirin_math.perf
./build/bin/eirin_math.tvec.benchmark
```

`eirin_math.perf` 还会将聚合精度数据写入 `accuracy_report.csv`。

### 饱和算术

在 C++26 之前，标准库没有提供饱和算术。*EirinMath* 提供一个尽量精确、与数据无关、性能较高且尽量避免分支的实现。

饱和算术函数声明在 `numeric.hpp` 中，可用于定点数和整数，也支持用户自定义类型。

```c++
int32_t a = INT32_MAX, b = INT32_MIN;
int32_t res = eirin::saturating_add(a, a); // INT32_MAX
res = eirin::saturating_add(b, b);         // INT32_MIN
res = eirin::saturating_sub(a, b);         // INT32_MAX
res = eirin::saturating_mul(a, a);         // INT32_MAX
res = eirin::saturating_mul(a, b);         // INT32_MIN
res = eirin::saturating_div(b, -1);        // INT32_MAX
res = eirin::saturating_cast(INT64_MAX);   // INT32_MAX
```

### 自定义整数类型

`numeric.hpp` 中的饱和助手可用于任何类整数存储类型，但用户自定义整数不会被自动识别，需要通过下面的 traits 显式 opt-in。`include/eirin/ext/builtin_ints.hpp` 中针对 `eirin::ext::int128` / `uint128` 的实现是完整参考。

要让自定义类型 `my_int` 使用 `saturating_*`，需要特化：

- `eirin::detail::is_integral<my_int>`：启用饱和算术重载的开关；
- `eirin::detail::is_signed<my_int>` 或 `eirin::detail::is_unsigned<my_int>`：两者必须恰好有一个为 `true`，用于判断溢出方向；
- `std::numeric_limits<my_int>`：提供 `digits`、`min()`、`max()` 和 `lowest()`，作为饱和边界。

```c++
namespace eirin::detail
{
template <>
struct is_integral<my_int> : public std::true_type
{};

template <>
struct is_signed<my_int> : public std::true_type
{};
} // namespace eirin::detail

namespace std
{
template <>
class numeric_limits<my_int>
{
public:
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = true;
    static constexpr bool is_integer = true;
    static constexpr int digits = 127;
    static constexpr int radix = 2;

    static constexpr my_int(min)() noexcept { /* INT128_MIN */ }
    static constexpr my_int(max)() noexcept { /* INT128_MAX */ }
    static constexpr my_int lowest() noexcept { return (min)(); }
};
} // namespace std
```

该类型还需要提供算法依赖的基础整数运算（`+ - * / %`、移位、一元负号、比较和转换），并且全部必须是 `constexpr`。

还有两个可选 traits：

- `eirin::detail::make_unsigned<my_int>`，以及无符号类型对应的 `make_signed`，用于启用 `saturating_*` 的无分支溢出路径；当该类型用作 `fixed_num` 存储类型时，这个 trait 是必需的。没有对应无符号类型时，饱和助手会检测到并回退到通用路径。

```c++
namespace eirin::detail
{
template <>
struct make_unsigned<my_int>
{
    using type = my_uint;
};

template <>
struct make_signed<my_uint, true>
{
    using type = my_int;
};
} // namespace eirin::detail
```

- `eirin::int_sequence_generator<my_int, Word>` 可以让 `detail::bit_width` 以及 `fixed_num::bit_width()` 按 word 拆分大整数，避免使用较慢的通用循环。继承 `eirin::detail::__int_sequence_generator_base`，设置 `is_specialized = true`，并提供返回迭代器的 `constexpr begin()` / `end()`；迭代器需要支持 `operator*`、`operator++` 和 `operator!=`。word 按最低有效位在前输出；`test/main.cpp` 中有 64 位和 16 位 word 类型的完整示例。

## 构建与测试

需要 **CMake >= 3.24** 和支持 C++20 的编译器。这是一个header-only库；安装步骤会把头文件复制到 GNUInstallDirs 的 include 目录，即 `<prefix>/include/eirin/...`。

在仓库根目录执行：

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure        # 运行单元测试
cmake --install build --prefix /path/to/prefix    # 安装头文件
```

示例使用 Ninja 生成器，与 CI 保持一致。使用多配置生成器时，需要给 `cmake --build` / `cmake --install` 传入 `--config Release`，并给 `ctest` 传入 `-C Release`。

安装步骤还会生成 CMake package 配置，其他 CMake 项目可以这样使用：

```cmake
find_package(EirinMath 1.5 CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE EirinMath::math)
```

`EirinMath::math` 是导出的头文件库目标，包在同一个主版本内兼容。通过 `add_subdirectory` 引入项目时，也可以使用同一个 `EirinMath::math` 别名。

常用 CMake 选项如下，均为布尔值，使用 `-D<name>=ON` 或 `-D<name>=OFF` 设置：

| 选项 | 默认值 | 说明 |
| --- | --- | --- |
| `eirin_build_tests` | `ON` | 构建单元测试 |
| `eirin_build_benchmarks` | `ON` | 构建定点数和 double 基准测试 |
| `eirin_build_advanced_benchmark` | `OFF` | 构建高级性能/精度 benchmark |
| `eirin_dev_test` | `OFF` | 启用开发测试包含项（SIMD 测试） |
| `eirin_test_simd_scalar` | `ON` | 构建标量 SIMD 框架测试 |
| `eirin_test_simd_avx2` | `OFF` | 构建并运行 AVX2 backend 测试 |
| `eirin_test_simd_avx512` | `OFF` | 构建并运行 AVX-512 backend 测试 |
| `eirin_prefer_system_gtest` | `ON` | 优先使用系统 GoogleTest；未找到时通过 FetchContent 下载 GoogleTest v1.14.0。设为 `OFF` 可强制使用 FetchContent |

Google Benchmark 始终通过 FetchContent 获取。

Emscripten/WASM 构建：

```sh
emcmake cmake -S . -B build-wasm -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-wasm
node build-wasm/bin/<name>.js
```

基准测试可执行文件包括 `eirin_math.benchmark`、`eirin_math.tvec.benchmark`、`eirin_math.sat_arith.benchmark` 和 `double.benchmark`。在 Unix 单配置构建中，它们位于 `build/bin`。

## 支持的编译器

至少需要 **C++20**。

- 构建系统：CMake >= 3.24
- 编译器：
    - 任何支持 **C++20** 的 C++ 编译器。
    - Clang >= 10
    - GCC >= 10
    - MSVC >= 19.22（VS 2019 16.2）

## License

[MIT](LICENSE) License
