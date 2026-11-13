Fixed Point Number IO
=======================

Standard Stream I/O
-----------------

EirinMath provides a simple interface for input and output of fixed point numbers using standard C++ streams.
You can use the standard input and output streams to read and write fixed point numbers.
For example, you can use the following code to read a fixed point number from standard input and print it to standard output:

.. code-block:: c++

    #include <iostream>
    #include <eirin/fixed.hpp>

    int main() {
        using namespace eirin;

        // Read a fixed point number from standard input
        fixed64 x;
        std::cin >> x;

        // Print the fixed point number to standard output
        std::cout << "The fixed point number is: " << x << std::endl;

        return 0;
    }


Other I/O Methods
-----------------

- You can use the std::format.
- The default format is ``{}``, which will print the fixed point number in decimal format.
- For other format arguments, you should branch it with ``{:}`` and use the format specifier as you would in C++. For example, you can use ``{:x}`` to print the fixed point number in hexadecimal format.
- Some arguments will override other format arguments declared before. To illustrate this, the following table shows the format arguments, and the ``Conflict`` column shows the format arguments that will be overridden by the current format argument.
- When radix argument is specified, only the integral part will be printed.

.. list-table:: The format arguments
    :header-rows: 1

    * - Format
      - Description
      - Conflict
      - Note
    * - ``?``
      - This will print the internal value of the fixed point number in hexadecimal format.
      - NO CONFLICT
      - No need to note.
    * - ``#``
      - Specify the radix prefix.
      - NO CONFLICT
      - This will only works with radix arguments.
    * - ``x``
      - This will print the fixed point number in hexadecimal format, with lowercase letters.
      - ``X``, ``d``, ``b``, ``o``
      - The ``0x`` prefix will only be added with ``#`` argument specified.
    * - ``X``
      - This will print the fixed point number in hexadecimal format, with uppercase letters.
      - ``x``, ``d``, ``b``, ``o``
      - The ``0X`` prefix will only be added with ``#`` argument specified.
    * - ``d``
      - This will print the fixed point number in decimal format.
      - ``x``, ``X``, ``b``, ``o``
      - The prefix of decimal will never be added, for no need.
    * - ``b``
      - This will print the fixed point number in binary format.
      - ``x``, ``X``, ``d``, ``o``
      - The ``0b`` prefix will only be added with ``#`` argument specified.
    * - ``o``
      - This will print the fixed point number in octal format.
      - ``x``, ``X``, ``d``, ``b``
      - The prefix will only be added with ``#`` argument specified.

.. code-block:: c++

    #include <iostream>
    #include <eirin/fixed.hpp>
    #include <eirin/io/format.hpp>

    int main() {
        using namespace eirin;

        fixed64 x = 114.514_f64;

        std::cout << std::format("fixed number: {}", x) << std::endl;
        std::cout << std::format("fixed number internal value: {:?}", x) << std::endl;

        return 0;
    }


Builtin Input/Output
--------------------

There are two builtin input functions, ``f32_from_cstring``, and ``fixed_from_cstring``.
