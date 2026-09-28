add_requires("gtest")

target("eirin_fixed.test")
    set_warnings("all", "error")
    set_kind("binary")
    if is_plat("wasm") then
        set_extension(".js")
        set_exceptions("no-cxx")
        add_defines("EIRIN_NO_EXCEPTIONS")
    end
    add_tests("test_eirin_fixed")
    add_files("**.cpp")
    add_deps("eirin_fixed")
    add_packages("gtest")
    if has_config("eirin_dev_test") and get_config("eirin_dev_test") == true then
        add_defines("EIRIN_DEV_TEST_MODE")
        add_cxxflags("-mavx", "-mavx2", "-msse")
    end

target("eirin_fixed.test.feat_mat_init_identify")
    set_warnings("all", "error")
    set_kind("binary")
    add_defines("EIRIN_MATRIX_ENABLE_INIT_IDENTIFY", "EIRIN_TEST_MAT_INIT_IDENTIFY")
    if is_plat("wasm") then
        set_extension(".js")
        set_exceptions("no-cxx")
        add_defines("EIRIN_NO_EXCEPTIONS")
    end
    add_tests("test_eirin_fixed")
    add_files("test_mat.cpp", "main.cpp")
    add_deps("eirin_fixed")
    add_packages("gtest")
    if has_config("eirin_dev_test") and get_config("eirin_dev_test") == true then
        add_defines("EIRIN_DEV_TEST_MODE")
        add_cxxflags("-mavx", "-mavx2", "-msse")
    end
