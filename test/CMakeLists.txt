# ---------------------------------------------------------------------------
# GoogleTest: prefer a system installation, fall back to FetchContent (1.14.0)
# ---------------------------------------------------------------------------
set(_eirin_gtest_source "FetchContent")

if(eirin_prefer_system_gtest)
    find_package(GTest QUIET)
    if(TARGET GTest::gtest)
        set(_eirin_gtest_source "system")
    endif()
endif()

if(NOT TARGET GTest::gtest)
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
endif()

message(STATUS "EirinMath: GoogleTest from ${_eirin_gtest_source}")

# ---------------------------------------------------------------------------
# Unit tests
# ---------------------------------------------------------------------------
set(EIRIN_TEST_SOURCES main.cpp test_io.cpp test_sat_arith.cpp test_vec.cpp)

add_executable(eirin_fixed.test ${EIRIN_TEST_SOURCES})
target_link_libraries(eirin_fixed.test PRIVATE eirin::math GTest::gtest)
eirin_enable_warnings_as_errors(eirin_fixed.test)

if(eirin_dev_test)
    # The development tests include the AVX2 SIMD helpers, which are only
    # available for x86 GCC/Clang builds.
    if(NOT MSVC AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|i[3-6]86)$")
        target_compile_definitions(eirin_fixed.test PRIVATE EIRIN_DEV_TEST_MODE)
        target_compile_options(eirin_fixed.test PRIVATE -mavx -mavx2 -msse)
    else()
        message(STATUS "EirinMath: eirin_dev_test is only supported on x86 GCC/Clang; ignoring it")
    endif()
endif()

if(EMSCRIPTEN)
    # Match the previous WASM configuration: no C++ exceptions.
    target_compile_options(eirin_fixed.test PRIVATE -fno-exceptions)
    target_compile_definitions(eirin_fixed.test PRIVATE EIRIN_NO_EXCEPTIONS)
    target_link_options(eirin_fixed.test PRIVATE -sEXIT_RUNTIME=1)
    set_target_properties(eirin_fixed.test PROPERTIES SUFFIX ".js")
endif()

if(EMSCRIPTEN)
    add_test(
        NAME test_eirin_fixed
        COMMAND node $<TARGET_FILE:eirin_fixed.test>
    )
else()
    add_test(NAME test_eirin_fixed COMMAND eirin_fixed.test)
endif()
