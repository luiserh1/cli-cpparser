# The library this project offers (cpp-policy POLICY.md 11.7): lib/cli_cpparser/. This
# project's CMakeLists.txt includes this file, and so does a project that uses the library,
# so nothing here is about tests, checks or the example program.

# Layers from the lowest up; the audit checks every #include against them. README.md
# ("Layout") says what each module holds.
cpp_policy_library(cli_cpparser
    LAYER command values
    LAYER program
    SOURCES
        command/command.cpp
        command/internal/checks.cpp
        command/parse.cpp
        command/usage.cpp
        values/readers.cpp
        program/help.cpp
        program/program.cpp)
