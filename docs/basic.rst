Fixed Point Number
===================

The ``eirin::fixed_point`` class is a template class that represents a fixed point number, it contains four template parameters:

- Type: The type that represents the fixed point number, it stores the integer part and the fractional part bits.
- IntermediateType: The type that represents the intermediate value, it is used to store the intermediate result of the arithmetic operations.
- fraction: The number of bits that represent the fractional part.
- rounding: The rounding mode, default and recommended to be ``false``.

For example, you can create a fixed point number with 32 bits integer part and 64 bits fractional part, and 20 bits for the fractional part, and no rounding with ``eirin::fixed_num<int32_t, int64_t, 20, false>``.

Constraints
----------------

The ``eirin::fixed_point`` class has some constraints:

- concept ``eirin::fixed_num_fraction``: The fraction must be less than or equal to the number of bits of the type, and greater than 0.
- concept ``eirin::fixed_num_size``: the size of IntermediateType must be greater than the size of Type.
- concept ``eirin::fixed_num_signness``: The Type and IntermediateType must have the same signness.

The logic to check the signness is as follows:
- Check if template struct ``eirin::traits::is_signed`` is defined.
- If it is defined, check the ``eirin::traits::is_signed`` of the Type and IntermediateType.
- If it is not defined, ``eirin::traits::is_signed`` will redirect to ``std::is_signed`` of the Type and IntermediateType.

Pre-defined Types
------------------

There are two pre-defined fixed point types in the library:

- ``eirin::fixed32``: A fixed point number with 1 bit for sign, 31 bits for integer part and 32 bits for fractional part.
    - ``eirin::fixed32`` is a typedef of ``eirin::fixed_num<int32_t, int64_t, 32, false>``.
- ``eirin::fixed64``: A fixed point number with 1 bit for sign, 63 bits for integer part and 64 bits for fractional part.
    - ``eirin::fixed64`` is a typedef of ``eirin::fixed_num<int64_t, eirin::detail::int128_t, 64, false>``.
    - Some compiler might not provide int128 type, and in this situation, the ``eirin::fixed64`` will not be available.

These types can meet most of the requirements for fixed point numbers, and you can use them directly without specifying the template parameters.

Construct a Fixed Point Number
===============================

Construct with Integral or Floating Types
--------------------------------

You can create a fixed point number with integral or floating types using constructor.

The ``eirin::fixed_point`` class provides two constructors which allows you to create a fixed point number with integral or floating types.

.. code-block:: c++

    #include <iostream>
    #include <eirin/fixed.hpp>
    
    int main() {
        using namespace eirin;
        // Create a fixed point number with integral type
        fixed_num<int32_t, int64_t, 20, false> fp1(12345);
        std::cout << "fp1: " << fp1 << std::endl;
    
        // Create a fixed point number with floating point type
        fixed_num<int32_t, int64_t, 20, false> fp2(123.45f);
        std::cout << "fp2: " << fp2 << std::endl;

        // predefined types
        fixed32 fp3(514);
        std::cout << "fp3: " << fp3 << std::endl;

        fixed64 fp4(114.514f);
        std::cout << "fp4: " << fp4 << std::endl;

        return 0;
    }

.. note::
    It is not recommended to use floating point types as the template parameter, because the floating point types are not exact and may cause precision loss.
    The fixed point number is designed to avoid precision loss, and constructing with floating point types may cause problems when cross-platform.
    But constructing with integral types is safe and recommended.

Literal
----------------

You can create the pre-defined fixed point number with a literal ``_f32`` and ``_f64``.

The ``_f32`` literal provides to create from integral, floating point types or string.
The ``_f64`` literal provides to create from integral or floating point types or raw literal.

.. code-block:: c++

    #include <iostream>
    #include <eirin/fixed.hpp>
    
    int main() {
        using namespace eirin;
        fixed32 fp1 = "123.45"_f32; // string literal
        std::cout << "fp1: " << fp1 << std::endl;

        fixed64 fp2 = 123.45_f64; // raw literal
        std::cout << "fp2: " << fp2 << std::endl;

        return 0;
    }

Construct from Another Fixed Point Number
------------------------------------------------

You can create a fixed point number from another fixed point number with the same type or different type.
The constructor will automatically convert the fixed point number to the target type.

.. code-block:: c++

    #include <iostream>
    #include <eirin/fixed.hpp>
    
    int main() {
        using namespace eirin;
        fixed32 fp1 = "123.45"_f32;
        std::cout << "fp1: " << fp1 << std::endl;

        fixed64 fp2(fp1);
        std::cout << "fp2: " << fp2 << std::endl;

        fixed32 fp3(fp2);
        std::cout << "fp3: " << fp3 << std::endl;

        return 0;
    }

Convert from Integer or Floating Point Types
------------------------------------------------

You can convert a fixed point number from an integer or floating point.

.. code-block:: c++

    #include <iostream>
    #include <eirin/fixed.hpp>
    
    int main() {
        using namespace eirin;
        auto fp1 = (fixed32) 12345;
        std::cout << "fp1: " << fp1 << std::endl;

        auto fp2 = (fixed32) 123.45f;
        std::cout << "fp2: " << fp2 << std::endl;

        using fixed_10_32 = fixed_num<int32_t, int64_t, 10, false>;
        auto fp3 = (fixed_10_32) 123.45f;
        std::cout << "fp3: " << fp3 << std::endl;
        auto fp4 = (fixed_10_32) 12345;
        std::cout << "fp4: " << fp4 << std::endl;

        return 0;
    }

Create From String
---------------------

You can create a fixed point number from a string with the ``eirin::fixed_from_cstring`` function.

This function has 3 parameters:

- ``const char* str``: The string to be converted.
- ``size_t len``: The length of the string.
- ``fixed_num<T, I, f, r>& fp``: The fixed point number to be created.

.. code-block:: c++

    #include <iostream>
    #include <eirin/fixed.hpp>
    
    int main() {
        using namespace eirin;
        fixed32 fp1;
        fixed_from_cstring("123.45", 6, fp1);
        std::cout << "fp1: " << fp1 << std::endl;

        fixed64 fp2;
        fixed_from_cstring("-123.45", 7, fp2);
        std::cout << "fp2: " << fp2 << std::endl;

        return 0;
    }

Create From Internal Representation
------------------------------------------------

.. warning::
    This method is not recommended to use, because it is not safe, unless you know what you are doing.

The function ``eirin::fixed_num::from_internal_value`` is used to create a fixed point number from the internal representation, but it is not that directive like mentioned.
Before using it, you need to know the internal representation of the fixed point number.

- For signed types, the first bit is the sign bit, and the rest is the integer part and the fractional part. The length of the integer part is ``sizeof(Type) * 8 - fraction - 1``.
- For unsigned types, the first bit is the integer part, and the rest is the fractional part. The length of the integer part is ``sizeof(Type) * 8 - fraction``.
- The integral part are just like the integral types, so the actual value is ``m_value >> fraction``, marked as ``m_int_actual`` here.
- Each bits of the fractional part are a value of ``2^(-i)`` where ``i`` is the index of the bit, starting from 1, if this bit is 1. For example, the first bit is ``2^(-1)``, the second bit is ``2^(-2)``, and so on. So it is easy to calculate the actual value of the fractional part, which is the summation from i equals 1 to ``fraction`` of ``2^i + f(i)``, ``f(i)`` same as the bit value of the i-th bit.
- The actual value of the fixed point number are ``m_int_actual + m_frac_actual``.

There is another function ``eirin::fixed_num::from_fixed_num_value`` which is used to create a fixed point number from the internal value of another fixed point number.

.. code-block:: c++

    #include <iostream>
    #include <eirin/fixed.hpp>
    
    int main() {
        using namespace eirin;
        fixed32 fp1 = "123.45"_f32;
        std::cout << "fp1: " << fp1 << std::endl;

        // Get the internal representation
        auto internal_value = fp1.internal_value();
        std::cout << "internal_value: " << internal_value << std::endl;

        // Create a fixed point number from the internal representation
        fixed32 fp2 = fixed32::from_internal_value(internal_value);
        std::cout << "fp2: " << fp2 << std::endl;

        // waring: if you know what you are doing
        auto fp3 = fixed32::from_internal_value(0x12345678);

        return 0;
    }


Useful Functions
=================

Get the Integral or Fractional Part
------------------------------------------------

- You can get the integral part of the fixed point number with the ``eirin::fixed_num::integral_part`` function.
- Also, you can get the fractional part of the fixed point number with the ``eirin::fixed_num::fractional_part`` function.

.. code-block:: c++

    #include <iostream>
    #include <eirin/fixed.hpp>
    
    int main() {
        using namespace eirin;
        fixed32 fp1 = "123.45"_f32;
        std::cout << "fp1: " << fp1 << std::endl;

        // Get the integral part
        auto integral_part = fp1.integral_part();
        std::cout << "integral_part: " << integral_part << std::endl;

        // Get the fractional part
        auto fractional_part = fp1.fractional_part();
        std::cout << "fractional_part: " << fractional_part << std::endl;

        return 0;
    }

Get the Internal Representation
------------------------------------------------

- You can get the internal representation of the fixed point number with the ``eirin::fixed_num::internal_value`` function.
- You can also get the internal representation of the integral part with the ``eirin::fixed_num::raw_integral_value`` function.

.. warning::
    This method is not recommended to use, because it is not safe, unless you know what you are doing.

.. code-block:: c++

    #include <iostream>
    #include <eirin/fixed.hpp>
    
    int main() {
        using namespace eirin;
        fixed32 fp1 = "123.45"_f32;
        std::cout << "fp1: " << fp1 << std::endl;

        // Get the internal representation
        auto internal_value = fp1.internal_value();
        std::cout << "internal_value: " << internal_value << std::endl;

        // Get the integral part
        auto integral_value = fp1.raw_integral_value();
        std::cout << "integral_value: " << integral_value << std::endl;

        return 0;
    }

Sign Bit
----------------

- You can get the sign bit of the fixed point number with the ``eirin::fixed_num::signbit`` function.
- Also, you can get the sign bit mask with the ``eirin::fixed_num::signbit_mask`` function.

.. code-block:: c++

    #include <iostream>
    #include <eirin/fixed.hpp>
    
    int main() {
        using namespace eirin;
        fixed32 fp1 = "123.45"_f32;
        std::cout << "fp1: " << fp1 << std::endl;

        // Get the sign bit
        auto signbit = fp1.signbit();
        std::cout << "signbit: " << signbit << std::endl;

        // Get the sign bit mask
        auto signbit_mask = fp1.signbit_mask();
        std::cout << "signbit_mask: " << signbit_mask << std::endl;

        return 0;
    }
