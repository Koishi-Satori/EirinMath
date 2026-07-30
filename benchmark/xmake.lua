add_requires("benchmark")

target("eirin_fixed.benchmark")
    set_kind("binary")
    if is_plat("wasm") then
        set_extension(".js")
    end
    add_includedirs(".", {public = true})
    add_files("./benchmark.cpp", "./bench.cpp")
    add_deps("eirin_fixed")
    add_packages("benchmark")
    -- add -Wmaybe-uninitialized on linux to catch uninitialized variable usage
    -- to be noticed that in windows/msvc this flag is not available
    if is_os("linux") then
        add_cxxflags("-Wmaybe-uninitialized", {force = true})
    end
    -- after_build(function(target)
    --     os.cp("$(scriptdir)/*.in", target:targetdir() .. "/benchmark_input/")
    -- end)

target("eirin_fixed.tvec.benchmark")
    set_kind("binary")
    if is_plat("wasm") then
        set_extension(".js")
    end
    add_includedirs(".", {public = true})
    add_files("./benchmark_vec.cpp", "./bench.cpp")
    add_deps("eirin_fixed")
    add_packages("benchmark")
    -- add -Wmaybe-uninitialized on linux to catch uninitialized variable usage
    -- to be noticed that in windows/msvc this flag is not available
    if is_os("linux") then
        add_cxxflags("-Wmaybe-uninitialized", {force = true})
    end
    -- after_build(function(target)
    --     os.cp("$(scriptdir)/*.in", target:targetdir() .. "/benchmark_input/")
    -- end)

if has_config("eirin_build_advanced_benchmark") and get_config("eirin_build_advanced_benchmark") == true then
    target("eirin_fixed.perf")
        set_kind("binary")
        add_includedirs(".", {public = true})
        add_files("./bench_perf.cpp", "./bench.cpp")
        add_deps("eirin_fixed")
        add_packages("benchmark")
        -- add -Wmaybe-uninitialized on linux to catch uninitialized variable usage
        -- to be noticed that in windows/msvc this flag is not available
        if is_os("linux") then
            add_cxxflags("-Wmaybe-uninitialized", {force = true})
        end
        -- after_build(function(target)
        --     os.cp("$(scriptdir)/*.in", target:targetdir() .. "/benchmark_input/")
        -- end)

    target("eirin_fixed.accuracy")
        set_kind("binary")
        add_includedirs(".", {public = true})
        add_files("./bench_accuracy.cpp", "./bench.cpp")
        add_deps("eirin_fixed")
        add_packages("benchmark")
        -- add -Wmaybe-uninitialized on linux to catch uninitialized variable usage
        -- to be noticed that in windows/msvc this flag is not available
        if is_os("linux") then
            add_cxxflags("-Wmaybe-uninitialized", {force = true})
        end
        -- after_build(function(target)
        --     os.cp("$(scriptdir)/*.in", target:targetdir() .. "/benchmark_input/")
        -- end)
end

target("double.benchmark")
    set_kind("binary")
    if is_plat("wasm") then
        set_extension(".js")
    end
    add_includedirs(".", {public = true})
    add_files("./double_bench.cpp", "./bench.cpp")
    add_deps("eirin_fixed")
    add_packages("benchmark")
    -- add -Wmaybe-uninitialized on linux to catch uninitialized variable usage
    -- to be noticed that in windows/msvc this flag is not available
    if is_os("linux") then
        add_cxxflags("-Wmaybe-uninitialized", {force = true})
    end
    -- after_build(function(target)
    --     os.cp("$(scriptdir)/*.in", target:targetdir() .. "/benchmark_input/")
    -- end)
