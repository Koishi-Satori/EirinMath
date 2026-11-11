# ---------------------------------------------------------------------------
# Google Benchmark is always obtained through FetchContent.
# ---------------------------------------------------------------------------
include(FetchContent)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    google_benchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.9.4.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(google_benchmark)

# Newer Clang (shipped by recent Emscripten SDKs) diagnoses the __COUNTER__
# feature probe in benchmark's header as a C2y extension.  The benchmark
# library compiles itself with -pedantic-errors, which turns that diagnostic
# into a hard error, so silence it for the library targets only.
if(EMSCRIPTEN)
    foreach(_benchmark_lib benchmark benchmark_main)
        if(TARGET ${_benchmark_lib})
            target_compile_options(${_benchmark_lib} PRIVATE -Wno-counter-extension)
        endif()
    endforeach()
endif()

message(STATUS "EirinMath: Google Benchmark from FetchContent (v1.9.4)")

# ---------------------------------------------------------------------------
# Benchmark executables
# ---------------------------------------------------------------------------
function(eirin_add_benchmark target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE eirin::math benchmark::benchmark)
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    eirin_enable_warnings(${target})

    # Catch uninitialized variable usage on Linux.  The flag is GCC-only.
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE -Wmaybe-uninitialized)
    endif()

    if(EMSCRIPTEN)
        target_link_options(${target} PRIVATE -sEXIT_RUNTIME=1)
        set_target_properties(${target} PROPERTIES SUFFIX ".js")
    endif()
endfunction()

eirin_add_benchmark(eirin_fixed.benchmark benchmark.cpp bench.cpp)
eirin_add_benchmark(eirin_fixed.tvec.benchmark benchmark_vec.cpp bench.cpp)
eirin_add_benchmark(eirin_fixed.sat_arith.benchmark bench_sat_arith.cpp bench.cpp)
eirin_add_benchmark(double.benchmark double_bench.cpp bench.cpp)

if(eirin_build_advanced_benchmark)
    eirin_add_benchmark(eirin_fixed.perf bench_perf.cpp bench.cpp)
    eirin_add_benchmark(eirin_fixed.accuracy bench_accuracy.cpp bench.cpp)
endif()
