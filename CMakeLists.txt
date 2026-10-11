cmake_minimum_required(VERSION 3.29)

# vcpkg is loaded before project(), where it installs the dependencies, through cpp-policy's
# files in tools/vcpkg/: they build the dependencies like the project (cpp-policy POLICY.md 1.1).
include("${CMAKE_CURRENT_SOURCE_DIR}/tools/vcpkg/setup.cmake")

project(CliCpparser VERSION 0.1.0 LANGUAGES CXX)

# cpp-policy, pinned by commit (POLICY.md 10). Upgrade with its tools/upgrade.sh, which moves
# this pin and the one in .github/workflows/ci.yml together.
include(FetchContent)
FetchContent_Declare(cpp_policy
    GIT_REPOSITORY https://github.com/luiserh1/cpp-policy.git
    GIT_TAG        a49bbea5111428a2d2c59d186b2317af78e19089) # v0.33.5
FetchContent_MakeAvailable(cpp_policy)

# Everything except main(), so tests can link it.
add_library(cli_cpparser_core STATIC
    src/app/greeting.cpp
    src/lowlevel/environment.cpp)
target_include_directories(cli_cpparser_core PUBLIC src)
cpp_policy_apply(cli_cpparser_core)

add_executable(cli_cpparser src/main.cpp)
target_link_libraries(cli_cpparser PRIVATE cli_cpparser_core)
cpp_policy_apply(cli_cpparser)
# The most the program may weigh, stripped, on each system: its size plus 5% (cpp-policy
# POLICY.md 13.5). The release workflow checks it; the owner approves raising a budget.
cpp_policy_size_budget(cli_cpparser MACOS 109568 LINUX 93184 WINDOWS 285696)

enable_testing()
add_subdirectory(tests)

# Layers from the lowest up (POLICY.md 11.1); the audit checks every #include against them.
# README.md ("Layout") and AGENTS.md describe what each layer holds.
cpp_policy_layers(
    LAYER lowlevel
    LAYER app)
cpp_policy_add_checks()
