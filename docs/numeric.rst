Numeric Arithmetic
==================

Besides plain arithmetic operators, the library provides free functions for
arithmetic with defined overflow behavior: ``saturating_*`` clamps the result
to the representable range, while ``modwarp_*`` wraps modulo 2^N like two's
complement. A value-preserving saturating cast between two fixed point types
is also provided.

These functions are implemented in the ``numeric.hpp`` header, which is
already included by the umbrella header ``eirin.hpp``. They accept any
``fixed_point`` type.

Saturating Arithmetic
---------------------

- ``saturating_add(x, y)``: returns ``x + y``, or the nearest representable
  value when the exact sum does not fit in the storage type.
- ``saturating_sub(x, y)``: returns ``x - y``, or the nearest representable
  value when the exact difference does not fit.
- ``saturating_mul(x, y)``: returns the fixed point product, or the nearest
  representable value when it does not fit.
- ``saturating_div(x, y)``: returns the fixed point quotient, or the nearest
  representable value when it does not fit.

The result is always deterministic: no undefined behavior is invoked, unlike
plain operators which overflow the storage type. Unsigned types clamp at zero
on subtraction underflow and at the maximum on overflow.

Multiplication and division are computed in the intermediate type, and follow
the rounding mode of the fixed point type when rounding is enabled. Division
keeps the same contract as ``operator/``:

.. warning::
    Division by zero is undefined behavior for ``saturating_div`` and
    ``modwarp_div``. The divisor must not be zero.

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
        fixed32 zero = saturating_cast<fixed32>(-0.1_f64);      // truncates toward zero
        fixed32 truncated = saturating_cast<fixed32>(3.75_f64); // 3.75

        std::cout << wide << '\n';
        std::cout << narrow << '\n';
        std::cout << zero << '\n';
        std::cout << truncated << '\n';

        return 0;
    }
