# EirinMath

- Other language: [中文](README.zh-CN.md)

A flexible, high-performance and header-only C++ fixed point number mathematics library, provides fixed point template class, high precision mathematical operations and basic input and output functions. You can run the benchmarks ```fixed.benchmark``` and ```double.benchmark``` to test for performance differences between the fixed types and the C++ double. 

*EirinMath* also provides a pre-defined 32bit-width fixed point, with 16bit precision(```fixed32```), and 64bit-width fixed point, with 32bit precision(```fixed64```).
The fixed points require same calculation result in different platforms, devices, operator systems and compilers, and this library fulfills this requirement.
Notice that the fixed64 uses some int128 compiler extension as its IntermediateType, and some compiler might not support it. The ```__msvc_int128.hpp``` in MSVC provides ```std::_Signed128``` and ```std::_Unsigned128```, Clang and GCC in Linux provide ```__int128```.

As an extra function, *EirinMath* provides the vector class `tvec<T, N>`, and its partial specialization versions in `vec.hpp`. *EirinMath* allows you to use swizzle operators like GLSL, but with function call `()`, e.g.

``` c++
eirin::f64vec4 v{495, 514_f64, 19, 0};
eirin::ivec4 vi{114, 514.0, 0, 0};
v.xyz() += 1_f32;
vi.xyzw() = eirin::ivec4{1, 2, 3, 4} + v.xyxy();
```

## Fixed point library

### Create Fixed Point

You can create a fixed point number with integral or floating types using constructor and literals.

- The literals now only provides for fixed32 and fixed64.
- Constructing from floating‑point types is not recommended in cross‑platform code, because the binary representation of floating‑point constants may differ across platforms/compilers, leading to inconsistent results. Use string‑based literals instead.

```c++
#include <eirin/fixed.hpp>

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

You can also create a fixed point from std::basic_istream or strings.

- ```f32_from_cstring``` and ```fixed_from_cstring``` will return true on success.
- the ```parse``` functions needs to pass the end of the string.
    - it will stops when meeting the **first char** of the end string.
    - for example, 114a.514a will parse only "114".

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

### Fixed Point Output

You can use `std::ostream` or `std::format`.
Note that the formatter requires including additional header file.

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

### From Internal Value

- **NOTE: this is not recommended, UNLESS you know what you're doing.**

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

### Fixed Operator Functions

You can convert fixed point to integral or floating types.

```c++
#include <eirin/fixed.hpp>

using namespace eirin;

(int) "114.5"_f32;
(float) "114.5"_f32;
```

You can perform quadratic operations and module between fixed and fixed, fixed and integral, or integral and fixed.
The output will always be fixed point.

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

The compare functions are just like normal compare.

### Fixed Mathematical Functions

Provides high precision and fast pure c++ implemention.

The pre-defined math constants are defined in the fixed point number class.

References:
- Trigonometric Functions: [Efficient Approximations for the Arctangent Function](https://ieeexplore.ieee.org/document/1628884)
- Square Root: [Fixed point sqrt](https://groups.google.com/g/comp.lang.c/c/IpwKbw0MAxw)
- Binary Logarithm: [A Fast Binary Logarithm Algorithm](http://www.claysturner.com/dsp/BinaryLogarithm.pdf)

Supported functions:
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

## Vector Library

*EirinMath* provides a vector library with a GLSL swizzle support.

### Create Vector

Vectors are provided by the `tvec<N, T>` template, with aliases for common types:
- `vec2<T>`, `vec3<T>`, `vec4<T>` (using `tvec<N, T>`)
- Pre‑defined aliases: `vec2i`, `vec3i`, `vec4fixed64`, `vec4fixed32`, etc. (see `vec.hpp`)
- The aliases naming is `vec[dimensions][type][width,optional]`, e.g. `vec2i32`, `vec3f128`, `vec3fixed64`, etc.

| Vec Type |  int  | float |  fixed-int   | fixed-float  |  fixed point number   |
| -------- | ----- | ----- | ------------ | ------------ | --------------------- |
| vec2     | vec2i | vec2f | vec2i[width] | vec2f[width] | vec2fixed<T, I, f, r> |
| vec3     | vec2i | vec2f | vec2i[width] | vec2f[width] | vec3fixed<T, I, f, r> |
| vec4     | vec2i | vec2f | vec2i[width] | vec2f[width] | vec4fixed<T, I, f, r> |

Constructors:

```c++
#include <eirin/vec.hpp>

using namespace eirin;

int main()
{
    // Default construction (uninitialised)
    vec2<int> v1;

    // Scalar – all components set to same value
    vec3<float> v2(1.0f);

    // Explicit component‑wise
    vec4<int> v3(1, 2, 3, 4);

    // From another vector (implicit conversion)
    vec2<float> v4(v3);   // converts components to float

    // From a smaller vector plus extra components
    vec3<double> v5(vec2<double>(1.0, 2.0), 3.0);

    // Uniform initialisation with braces
    vec4<int> v6{1, 2, 3, 4};
    return 0;
}
```

### Access Elements

Elements are accessed via named members `.x`, `.y`, `.z`, `.w` and also via `operator[]`.

```c++
vec4<int> v{1, 2, 3, 4};
v.x = 10;
int a = v[2];   // a = 3
```

### Swizzle Operations

*EirinMath* supports swizzle masks using function‑call syntax `.xy()`, `.xyz()`, `.yxw()`, etc. The mask length can be 2, 3, or 4 components, and components may be repeated (e.g. `.xx()`, `.xyx()`).

- Read from a swizzle returns a new vector of the mask length.
- Write to a swizzle (via assignment) is allowed if the mask length matches the right‑hand side vector dimension.
- Compound assignments (`+=`, `-=`, `*=`, `/=`, etc.) are supported for swizzle expressions, provided the mask length matches the vector dimension.

```c++
vec4<int> v{1, 2, 3, 4};

// Read
auto v2 = v.xy();               // v2 = (1, 2)
auto v3 = v.zyx();              // v3 = (3, 2, 1)

// Write
v.xy() = vec2<int>{5, 6};       // v = (5, 6, 3, 4)
v.xz() += vec2<int>{1, 1};      // v = (6, 6, 4, 4)

// Repeated indices are allowed for reads
auto xx = v.xx();               // (6, 6) – reading twice

// For writes, repeated indices as lvalue are disallowed:
v.xx() = vec2<int>{7, 8};       // compile failed.
```

> Note: Just Like GLSL/GLM, *EirinMath* also does not permit repeated indices in modifying swizzle expressions (e.g. `v.xx() += 1`).

### Vector Operators and Functions

Vectors support the usual arithmetic operators (`+`, `-`, `*`, `/`, `%`, bitwise `&`, `|`, `^`, shifts) both with scalars and vectors of compatible dimensions.

```c++
vec2<int> a{1, 2}, b{3, 4};
auto c = a + b;               // (4, 6)
auto d = a * 2;               // (2, 4)
auto e = 3 * a;               // (3, 6)
a += b;                       // a becomes (4, 6)
```

`tvec` provides a rich set of utility functions:
- `dot` – dot product
- `cross` – cross product (only for 2/3‑component vectors, because vector cross products are only defines on 3 or 7 dim.)

### Saturating Arithmetic

Since before C++26, the saturating arithmetic is not provided by the C++ Standard Library, *EirinMath* provides an implementation that is accurate, data-independent, as high-performance as possible, and as branch-free as possible.

The saturating arithmetic functions are declared in `numeric.hpp`, and available for both fixed point number and integral types (can be custom type).

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

### Custom Integral Type

The saturating helpers in `numeric.hpp` work on any integer-like storage type,
but a user-defined integer is not recognized automatically: it must opt in
through the traits below. `include/eirin/ext/builtin_ints.hpp` (the
`eirin::ext::int128` / `uint128` specializations) is a complete reference
implementation you can copy from.

To use a custom type `my_int` with the `saturating_*` functions, specialize:

- `eirin::detail::is_integral<my_int>` — the opt-in switch that enables the
  saturating overloads;
- `eirin::detail::is_signed<my_int>` or `eirin::detail::is_unsigned<my_int>` —
  exactly one of them must report `true` so overflow direction is known;
- `std::numeric_limits<my_int>` — provides `digits` and `min()`/`max()`/
  `lowest()`, which are the saturation bounds.

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

The type itself must also provide the plain integer operations the algorithms
rely on (`+ - * / %`, shifts, unary minus, comparisons and conversions), all
`constexpr`.

Two more traits are optional:

- `eirin::detail::make_unsigned<my_int>` (and `make_signed` for the unsigned
  counterpart) enables the branchless overflow path in `saturating_*`, and is
  required when the type is used as `fixed_num` storage. When no unsigned
  counterpart exists, the saturating helpers detect that and fall back to the
  generic path.

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

- `eirin::int_sequence_generator<my_int, Word>` lets `detail::bit_width` (and
  therefore `fixed_num::bit_width()`) split a big integer into words instead of
  using the slower generic loop. Inherit
  `eirin::detail::__int_sequence_generator_base`, set
  `is_specialized = true`, and provide `constexpr` `begin()`/`end()` returning
  an iterator with `operator*`, `operator++` and `operator!=`. Words are
  yielded least significant first; `test/main.cpp` contains working examples
  for both 64-bit and 16-bit word types.

## Supported Compilers

Requires at least **C++20**.

- Build System: xmake >= v2.2.2
- Compilers:
    - Any C++ compiler that supports **C++20**.
    - Clang >= 10
    - GCC >= 10
    - MSVC >= 19.22 (VS 2019 16.2)

## License
[MIT](LICENSE) License
