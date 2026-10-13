cmake_minimum_required(VERSION 3.29)

# vcpkg is loaded before project(), where it installs the dependencies, through cpp-policy's
# files in tools/vcpkg/: they build the dependencies like the project (cpp-policy POLICY.md 1.1).
include("${CMAKE_CURRENT_SOURCE_DIR}/tools/vcpkg/setup.cmake")

project(CliCpparser VERSION 0.3.0 LANGUAGES CXX)

# cpp-policy, pinned by commit (POLICY.md 10). Upgrade with its tools/upgrade.sh, which moves
# this pin and the one in .github/workflows/ci.yml together.
include(FetchContent)
FetchContent_Declare(cpp_policy
    GIT_REPOSITORY https://github.com/luiserh1/cpp-policy.git
    GIT_TAG        1f48e6634bd5b0b7643d7cbb39e556decc582030) # v0.37.0
FetchContent_MakeAvailable(cpp_policy)

# The library (POLICY.md 11.7): lib/cli_cpparser/, declared in the file its users include too.
include(library.cmake)

# The example program, as a user of the library writes one: everything except main(), so
# tests can link it.
add_library(cli_cpparser_core STATIC
    src/app/commands.cpp
    src/app/run.cpp)
target_include_directories(cli_cpparser_core PUBLIC src)
target_link_libraries(cli_cpparser_core PUBLIC cli_cpparser::cli_cpparser)
target_compile_definitions(cli_cpparser_core PRIVATE APP_VERSION="${PROJECT_VERSION}")
cpp_policy_apply(cli_cpparser_core)

add_executable(cli_cpparser src/main.cpp)
target_link_libraries(cli_cpparser PRIVATE cli_cpparser_core)
cpp_policy_apply(cli_cpparser)
# The most the program may weigh, stripped, on each system: its size plus 5% (cpp-policy
# POLICY.md 13.5). The release workflow checks it; the owner approves raising a budget.
cpp_policy_size_budget(cli_cpparser MACOS 182272 LINUX 184320 WINDOWS 371712)

enable_testing()
add_subdirectory(tests)

# The example program's layers (POLICY.md 11.1), all above the library's, which are in
# library.cmake; the audit checks every #include against them. README.md ("Layout") says what
# each module holds.
cpp_policy_layers(
    LAYER app)
cpp_policy_add_checks()
