# One doctest program per kind and module (cpp-policy POLICY.md 13). After each build, every
# test case becomes a CTest test named <kind>/<module>/<test case>, labelled with its kind.
# command, values and program are the library's modules; app is the example program, whose
# tests are the library's end to end.
# A module's tests are in unit/<module>.cpp, with more files for command and program.
set(more_unit_command unit/command_refusals.cpp unit/command_rules.cpp
    unit/command_usage.cpp)
set(more_unit_program unit/program_help.cpp)
foreach(module IN ITEMS command values program)
    cpp_policy_add_tests(unit ${module} SOURCES unit/${module}.cpp ${more_unit_${module}}
                         LIBRARIES cli_cpparser::cli_cpparser)
endforeach()
cpp_policy_add_tests(unit app SOURCES unit/app.cpp LIBRARIES cli_cpparser_core)
