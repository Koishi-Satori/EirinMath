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
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.4
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
    target_link_libraries(${target} PRIVATE EirinMath::math)
    target_link_libraries(${target} PRIVATE benchmark::benchmark)
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

    if(EMSCRIPTEN)
        target_link_options(${target} PRIVATE -sEXIT_RUNTIME=1)
        set_target_properties(${target} PROPERTIES SUFFIX ".js")
    endif()
endfunction()

eirin_add_benchmark(eirin_math.benchmark benchmark.cpp bench.cpp)
eirin_add_benchmark(eirin_math.tvec.benchmark benchmark_vec.cpp bench.cpp)
eirin_add_benchmark(eirin_math.sat_arith.benchmark bench_sat_arith.cpp bench.cpp)
eirin_add_benchmark(double.benchmark double_bench.cpp bench.cpp)

if(eirin_build_advanced_benchmark)
    eirin_add_benchmark(eirin_math.perf bench_perf.cpp bench.cpp)
    eirin_add_benchmark(eirin_math.accuracy bench_accuracy.cpp bench.cpp)
endif()
