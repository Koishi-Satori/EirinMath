set_project("eirin_fixed")
set_version("1.5.2")

add_rules("mode.debug", "mode.release")

option("eirin_build_tests")
    set_default(true)
    set_showmenu(true)
    set_description("Build unit tests")
    option_end()
option("eirin_build_benchmarks")
    set_default(true)
    set_showmenu(true)
    set_description("Build fixed number and double benchmarks")
    option_end()
option("eirin_dev_test")
    set_default(false)
    set_showmenu(true)
    set_description("Develop Test Include")
    option_end()
option("eirin_build_advanced_benchmark")
    set_default(false)
    set_showmenu(true)
    set_description("Build advanced fixed number benchmarks")
    option_end()

set_warnings("all")
set_languages("cxx20")

target("eirin_fixed")
    set_kind("headeronly")
    add_includedirs("include", {public = true})
    add_headerfiles("include/(eirin/**.hpp)")
    target_end()

if has_config("eirin_build_tests") then
    includes("test")
end
if has_config("eirin_build_benchmarks") then
    includes("benchmark")
end
