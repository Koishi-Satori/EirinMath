cmake_minimum_required(VERSION 3.24)

project(EirinMath VERSION 1.5.2 LANGUAGES CXX)

include(GNUInstallDirs)

# ---------------------------------------------------------------------------
# Build options
# ---------------------------------------------------------------------------
option(eirin_build_tests "Build unit tests" ON)
option(eirin_build_benchmarks "Build fixed number and double benchmarks" ON)
option(eirin_build_advanced_benchmark "Build advanced fixed number benchmarks" OFF)
option(eirin_dev_test "Enable the development test include" OFF)
option(
    eirin_prefer_system_gtest
    "Prefer a system-installed GoogleTest; fall back to FetchContent (v1.14.0) when it is not found"
    ON
)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Keep all runnable artifacts in one directory so that the generated .js files
# of the Emscripten builds sit next to their .wasm siblings.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# ---------------------------------------------------------------------------
# Warning flags
# ---------------------------------------------------------------------------
if(MSVC)
    set(EIRIN_WARNINGS /W3)
    set(EIRIN_WARNINGS_AS_ERRORS /WX)
else()
    set(EIRIN_WARNINGS -Wall)
    set(EIRIN_WARNINGS_AS_ERRORS -Werror)
endif()

function(eirin_enable_warnings target)
    target_compile_options(${target} PRIVATE ${EIRIN_WARNINGS})
endfunction()

function(eirin_enable_warnings_as_errors target)
    target_compile_options(${target} PRIVATE ${EIRIN_WARNINGS} ${EIRIN_WARNINGS_AS_ERRORS})
endfunction()

# ---------------------------------------------------------------------------
# The header-only library target
# ---------------------------------------------------------------------------
add_library(eirin_math INTERFACE)
add_library(eirin::math ALIAS eirin_math)

target_compile_features(eirin_math INTERFACE cxx_std_20)
target_include_directories(
    eirin_math
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
              $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ---------------------------------------------------------------------------
# Installation: this is a header-only library, so only the headers are copied
# into the GNUInstallDirs include directory.
# ---------------------------------------------------------------------------
install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# ---------------------------------------------------------------------------
# Tests and benchmarks
# ---------------------------------------------------------------------------
if(eirin_build_tests)
    enable_testing()
    add_subdirectory(test)
endif()

if(eirin_build_benchmarks)
    add_subdirectory(benchmark)
endif()
