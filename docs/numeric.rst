Numeric Arithmetic
==================

Besides the plain arithmetic operators, the library provides free functions
with defined overflow behavior in two overload families:

- overloads for **integral types** (signed or unsigned, ``bool`` excluded),
  modelled after the C++26 ``<numeric>`` saturating arithmetic;
- overloads for ``fixed_point`` types, whose storage overflow is detected in
  the wider intermediate type.

For fixed point values there are also the ``modwarp_*`` functions, which wrap
modulo 2^N like two's-complement arithmetic, and a value-preserving
``saturating_cast`` between two fixed point types or between two integral
types.

Everything below is provided by the ``numeric.hpp`` header, which is already
included by the umbrella header ``eirin.hpp``.

Integral Saturating Arithmetic
------------------------------

The four functions below accept any library integral type: the signed and
unsigned standard integer types, and the extended 128-bit types
(``detail::int128_t``/``detail::uint128_t``) when the underlying compiler
support is available. ``bool`` is intentionally not accepted, mirroring the
C++26 saturating arithmetic family in ``<numeric>``.

- ``saturating_add(x, y)``: returns ``x + y``, or ``max``/``min`` of ``T``
  when the exact sum does not fit.
- ``saturating_sub(x, y)``: returns ``x - y``, or ``max``/``min`` of ``T``
  when the exact difference does not fit.
- ``saturating_mul(x, y)``: returns ``x * y``, or ``max``/``min`` of ``T``
  when the exact product does not fit.
- ``saturating_div(x, y)``: returns ``x / y``; the only overflow case is
  ``min / -1`` for signed types, which saturates to ``max``.

For signed types the saturation direction follows the sign of the exact
result. For unsigned types addition and multiplication saturate to the
maximum, subtraction saturates to zero on underflow, and division cannot
overflow. When the operation fits, the exact result is returned.

All four functions are ``constexpr`` and ``noexcept``, so they can be used in
constant expressions and never rely on compiler-specific overflow behavior.

.. warning::
    Division by zero is undefined behavior for ``saturating_div``, for both
    integral and fixed point operands. The divisor must not be zero.

.. code-block:: c++

    #include <cstdint>
    #include <iostream>
    #include <limits>
    #include <eirin/eirin.hpp>

    int main()
    {
        using namespace eirin;

        constexpr auto a = saturating_add(std::int32_t{2'000'000'000},
                                          std::int32_t{1'000'000'000}); // INT32_MAX
        constexpr auto b = saturating_sub(std::uint32_t{0}, std::uint32_t{1}); // 0
        const auto c = saturating_mul(std::int32_t{1'200'000'000},
                                      std::int32_t{-2});               // INT32_MIN
        const auto d = saturating_div(std::numeric_limits<std::int32_t>::min(),
                                      std::int32_t{-1});               // INT32_MAX

        std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
        return 0;
    }

Fixed-point Saturating Arithmetic
---------------------------------

The same names are overloaded for ``fixed_point`` types:

- ``saturating_add(x, y)``: returns ``x + y``, or the nearest representable
  value when the exact sum does not fit in the storage type.
- ``saturating_sub(x, y)``: returns ``x - y``, or the nearest representable
  value when the exact difference does not fit.
- ``saturating_mul(x, y)``: returns the fixed point product, or the nearest
  representable value when it does not fit.
- ``saturating_div(x, y)``: returns the fixed point quotient, or the nearest
  representable value when it does not fit.

The result is always deterministic: no undefined behavior is invoked, unlike
plain operators which overflow the storage type. Unsigned storage types clamp
at zero on subtraction underflow and at the maximum on overflow.

Multiplication and division are computed in the intermediate type and follow
the rounding mode of the fixed point type when rounding is enabled. Division
keeps the same contract as ``operator/`` (see the warning above: the divisor
must not be zero).

.. code-block:: c++

    #include <iostream>
    #include <eirin/eirin.hpp>

    int main()
    {
        using namespace eirin;

        const fixed32 max_f32 = fixed32::from_internal_value(0x7FFFFFFF);

        std::cout << saturating_add(max_f32, 1_f32) << '\n';   // stays at the maximum
        std::cout << saturating_sub(0_f32, 1_f32) << '\n';     // -1
        std::cout << saturating_mul(20000_f32, 2_f32) << '\n'; // clamps to the maximum
        std::cout << saturating_div(max_f32, 0.5_f32) << '\n'; // clamps to the maximum

        return 0;
    }

Mod-warp Arithmetic
-------------------

``modwarp_*`` applies to ``fixed_point`` values only:

- ``modwarp_add(x, y)``: returns ``(x + y) mod 2^N``.
- ``modwarp_sub(x, y)``: returns ``(x - y) mod 2^N``.
- ``modwarp_mul(x, y)``: returns the fixed point product ``mod 2^N``.
- ``modwarp_div(x, y)``: returns the fixed point quotient ``mod 2^N``.

``N`` is the bit width of the storage type. The result wraps around exactly
like two's-complement arithmetic, but it is computed through the wider
intermediate type, so no undefined behavior is involved. Division by zero is
undefined behavior, as with ``operator/``.

.. code-block:: c++

    #include <iostream>
    #include <eirin/eirin.hpp>

    int main()
    {
        using namespace eirin;

        const fixed32 max_f32 = fixed32::from_internal_value(0x7FFFFFFF);

        std::cout << modwarp_add(max_f32, 1_f32) << '\n';   // wraps to the minimum
        std::cout << modwarp_sub(0_f32, 1_f32) << '\n';     // -1
        std::cout << modwarp_mul(max_f32, 2_f32) << '\n';   // wrapped product

        return 0;
    }

Saturating Cast
---------------

Two overloads are provided, one for integral types and one for fixed point
types.

Integral types
~~~~~~~~~~~~~~

``saturating_cast<Res>(x)`` converts the integral value ``x`` into the
integral type ``Res``, preserving the value when it fits and clamping to the
representable range of ``Res`` otherwise. The conversion is ``constexpr`` and
``noexcept``. ``bool`` is not a valid source or target type.

- Negative values converted into an unsigned target saturate to zero.
- Values above the target maximum saturate to ``Res``'s maximum.
- Values below the target minimum (signed targets) saturate to ``Res``'s
  minimum.
- Widening conversions simply produce the exact value.

.. code-block:: c++

    #include <cstdint>
    #include <iostream>
    #include <eirin/eirin.hpp>

    int main()
    {
        using namespace eirin;

        const auto a = saturating_cast<std::int8_t>(300);        // 127
        const auto b = saturating_cast<std::int8_t>(-300);       // -128
        const auto c = saturating_cast<std::uint8_t>(-1);        // 0
        const auto d = saturating_cast<std::uint64_t>(-1);       // 0
        const auto e = saturating_cast<std::int64_t>(12345);     // 12345

        std::cout << static_cast<int>(a) << ' ' << static_cast<int>(b) << ' '
                  << static_cast<int>(c) << ' ' << d << ' ' << e << '\n';
        return 0;
    }

Fixed point types
~~~~~~~~~~~~~~~~~

``saturating_cast<T>(x)`` converts the fixed point value ``x`` into another
fixed point type ``T``, preserving the value when it fits and clamping to the
representable range of ``T`` otherwise.

Cross-fraction conversion keeps the numeric value: the source internal value
is scaled to the destination fraction with the same formulas as the
destination converting constructor. When the destination fraction is smaller,
the value is truncated, or rounded when ``T`` enables rounding. Negative
values cast into an unsigned destination saturate to zero. The conversion is
well defined even when the source storage type is wider than the destination
intermediate type.

.. code-block:: c++

    #include <iostream>
    #include <eirin/eirin.hpp>

    int main()
    {
        using namespace eirin;

        fixed64 wide = saturating_cast<fixed64>(3.75_f32);    // 3.75, exact in 2^-32 units
        fixed32 narrow = saturating_cast<fixed32>(40000.0_f64); // does not fit: max
        fixed32 zero = saturating_cast<fixed32>(-0.1_f64);      // negative: zero
        fixed32 truncated = saturating_cast<fixed32>(3.75_f64); // 3.75

        std::cout << wide << '\n';
        std::cout << narrow << '\n';
        std::cout << zero << '\n';
        std::cout << truncated << '\n';

        return 0;
    }

Implementation notes
--------------------

The integral overloads dispatch to the best available overflow check:
compiler builtins (GCC/Clang ``__builtin_*_overflow``), the MSVC overflow
intrinsics added in Visual Studio 2022 17.7 (``_add_overflow_*``,
``_sub_overflow_*`` and the ``_mul_overflow_*``/``_mul_full_overflow_*``
family on x86/x64), and a branchless portable fallback otherwise. All paths
are ``constexpr``-safe. The internal helpers live in
``eirin/detail/sat_arith.hpp``, which ``numeric.hpp`` includes; they are an
implementation detail and are not part of the public API.
