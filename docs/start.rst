Supported Platforms
===================

- CMake >= 3.24
- Any C++ compiler that supports C++20

Currently, the following platforms and compilers are officially tested.

.. note::
    The 64-bit fixed point number uses ``eirin::detail::int128_t`` as the intermediate type. Currently, it depends on the compiler extension, and for the compiler that does not support it, the 64-bit fixed point number will not be available.
    If you need the 64-bit fixed point number, please use the compiler that supports it or some third-party library to provide the 128-bit integer type (and typedef the fixed64 yourself).
    In MSVC, the int128_t is typedefed as ``std::_Signed128`` and ``std::_Unsigned128`` in ``__msvc_int128.hpp`` header file.
    And in Linux Clang/GCC, it is typedefed as ``__int128_t`` and ``unsigned __int128_t`` in ``__int128_t.h`` header file.

Integrate into Your Project
===========================

A. Copy into Your Project
-------------------------

The core library of EirinFixed is a header-only library, you can copy the ``include`` directory into your project, and include the header file in your code.

B. Build and Install with CMake
-------------------------------

EirinFixed is built with `CMake <https://cmake.org/>`_ (>= 3.24). Because the library is header-only, building produces no library artifact; the ``install`` step copies the headers into ``<prefix>/include/eirin/...`` (the GNUInstallDirs include directory of the chosen prefix).

From the repository root, configure, build and run the unit tests:

.. code-block:: sh

    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    ctest --test-dir build --output-on-failure

Install the headers into a prefix:

.. code-block:: sh

    cmake --install build --prefix /path/to/prefix

The install step also generates a CMake package configuration, so that other CMake projects can consume the installed library:

.. code-block:: cmake

    find_package(EirinMath 1.5 CONFIG REQUIRED)
    target_link_libraries(my_target PRIVATE EirinMath::math)

The library target is exported as ``EirinMath::math`` (the package version is compatible within the same major version). When the library is vendored with ``add_subdirectory``, the in-tree aliases ``EirinMath::math`` and ``eirin::math`` are available.

The examples use the Ninja generator, as the CI does. With a multi-config generator, pass ``--config Release`` to ``cmake --build``/``cmake --install`` and ``-C Release`` to ``ctest`` instead.

The following CMake options are available. All of them are boolean and are passed with ``-D<name>=ON`` or ``-D<name>=OFF``:

- ``eirin_build_tests`` (default ``ON``): build the unit tests.
- ``eirin_build_benchmarks`` (default ``ON``): build the fixed-number and double benchmarks.
- ``eirin_build_advanced_benchmark`` (default ``OFF``): build the advanced benchmarks (performance/accuracy).
- ``eirin_dev_test`` (default ``OFF``): enable development test includes (SIMD tests).
- ``eirin_prefer_system_gtest`` (default ``ON``): use a system-installed GoogleTest if found; otherwise FetchContent downloads GoogleTest v1.14.0. Set to ``OFF`` to force FetchContent.

Google Benchmark is always obtained through FetchContent.

For Emscripten/WASM builds, configure with ``emcmake`` and Ninja. Tests and benchmarks are emitted as ``.js`` files under ``build-wasm/bin`` and can be run with ``node``:

.. code-block:: sh

    emcmake cmake -S . -B build-wasm -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build build-wasm
    node build-wasm/bin/<name>.js

The benchmark executables are ``eirin_fixed.benchmark``, ``eirin_fixed.tvec.benchmark``, ``eirin_fixed.sat_arith.benchmark`` and ``double.benchmark``. On Unix single-config builds they are written to ``build/bin``.
