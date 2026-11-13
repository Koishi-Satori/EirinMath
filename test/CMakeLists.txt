# ---------------------------------------------------------------------------
# GoogleTest: prefer a system installation, fall back to FetchContent (1.14.0)
# ---------------------------------------------------------------------------
set(_eirin_gtest_source "FetchContent")

if (eirin_prefer_system_gtest)
    find_package(GTest QUIET)
    if (TARGET GTest::gtest)
        set(_eirin_gtest_source "system")
    endif ()
endif ()

if (NOT TARGET GTest::gtest)
    include(FetchContent)

    # Keep the dependency build minimal and self-contained.
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(gtest_build_samples OFF CACHE BOOL "" FORCE)
    set(gtest_build_tests OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)
endif ()

message(STATUS "EirinMath: GoogleTest from ${_eirin_gtest_source}")

# ---------------------------------------------------------------------------
# Unit tests
# ---------------------------------------------------------------------------
set(EIRIN_TEST_SOURCES main.cpp test_io.cpp test_sat_arith.cpp test_vec.cpp)

add_executable(eirin_math.test ${EIRIN_TEST_SOURCES})
target_link_libraries(eirin_math.test PRIVATE EirinMath::math GTest::gtest)
eirin_enable_warnings_as_errors(eirin_math.test)

if (eirin_dev_test)
    # The development tests include the AVX2 SIMD helpers, which are only
    # available for x86 GCC/Clang builds.
    if (NOT MSVC AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|i[3-6]86)$")
        target_compile_definitions(eirin_math.test PRIVATE EIRIN_DEV_TEST_MODE)
        target_compile_options(eirin_math.test PRIVATE -mavx -mavx2 -msse)
    else ()
        message(STATUS "EirinMath: eirin_dev_test is only supported on x86 GCC/Clang; ignoring it")
    endif ()
endif ()

# add this for simd test first, will refactor all cmake configure later.
function(eirin_add_simd_test target)
    cmake_parse_arguments(
            SIMD_TEST
            ""
            ""
            "OPTIONS;DEFINITIONS;LABELS"
            ${ARGN}
    )

    add_executable(${target} test_simd.cpp)

    target_link_libraries(
            ${target}
            PRIVATE EirinMath::math GTest::gtest_main
    )

    target_compile_options(
            ${target}
            PRIVATE ${SIMD_TEST_OPTIONS}
    )

    target_compile_definitions(
            ${target}
            PRIVATE ${SIMD_TEST_DEFINITIONS}
    )

    eirin_enable_warnings_as_errors(${target})

    if (EMSCRIPTEN)
        target_compile_options(${target} PRIVATE -fno-exceptions)
        target_compile_definitions(${target} PRIVATE EIRIN_NO_EXCEPTIONS)
        target_link_options(${target} PRIVATE -sEXIT_RUNTIME=1)
        set_target_properties(${target} PROPERTIES SUFFIX ".js")

        add_test(
                NAME ${target}
                COMMAND node $<TARGET_FILE:${target}>
        )
    else ()
        add_test(NAME ${target} COMMAND ${target})
    endif ()

    set_tests_properties(
            ${target}
            PROPERTIES LABELS "simd;${SIMD_TEST_LABELS}"
    )
endfunction()

eirin_add_simd_test(
        eirin_math.test.simd
        LABELS native
)

if (eirin_test_simd_scalar)
    eirin_add_simd_test(
            eirin_math.test.simd.scalar
            DEFINITIONS EIRIN_MATH_NO_SIMD
            LABELS scalar
    )
endif ()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|i[3-6]86)$")
    if (MSVC)
        set(EIRIN_SSE2_OPTIONS)
        set(EIRIN_AVX2_OPTIONS /arch:AVX2)
        set(EIRIN_AVX512_OPTIONS /arch:AVX512)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(EIRIN_SSE2_OPTIONS -msse2 -mno-avx)
        set(EIRIN_AVX2_OPTIONS -mavx2 -mno-avx512f)
        set(EIRIN_AVX512_OPTIONS
                -mavx512f
                -mavx512dq
                -mavx512bw
                -mavx512vl
        )
    endif ()

    if (EIRIN_SSE2_OPTIONS OR MSVC)
        eirin_add_simd_test(
                eirin_math.test.simd.sse2
                OPTIONS ${EIRIN_SSE2_OPTIONS}
                LABELS sse2
        )
    endif ()

    if (eirin_test_simd_avx2 AND EIRIN_AVX2_OPTIONS)
        eirin_add_simd_test(
                eirin_math.test.simd.avx2
                OPTIONS ${EIRIN_AVX2_OPTIONS}
                DEFINITIONS EIRIN_DEV_TEST_MODE EIRIN_SIMD_TEST_EXPECT_AVX2=1
                LABELS avx2
        )
    endif ()

    if (eirin_test_simd_avx512 AND EIRIN_AVX512_OPTIONS)
        eirin_add_simd_test(
                eirin_math.test.simd.avx512
                OPTIONS ${EIRIN_AVX512_OPTIONS}
                LABELS avx512
        )
    endif ()
endif ()

if (EMSCRIPTEN)
    # Match the previous WASM configuration: no C++ exceptions.
    target_compile_options(eirin_math.test PRIVATE -fno-exceptions)
    target_compile_definitions(eirin_math.test PRIVATE EIRIN_NO_EXCEPTIONS)
    target_link_options(eirin_math.test PRIVATE -sEXIT_RUNTIME=1)
    set_target_properties(eirin_math.test PROPERTIES SUFFIX ".js")
endif ()

if (EMSCRIPTEN)
    add_test(
            NAME test_eirin_math
            COMMAND node $<TARGET_FILE:eirin_math.test>
    )
else ()
    add_test(NAME test_eirin_math COMMAND eirin_math.test)
endif ()
