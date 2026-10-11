# One doctest program per kind and module (cpp-policy POLICY.md 13). After each build, every
# test case becomes a CTest test named <kind>/<module>/<test case>, labelled with its kind.
cpp_policy_add_tests(unit app SOURCES test_app.cpp LIBRARIES cli_cpparser_core)
cpp_policy_add_tests(unit lowlevel SOURCES test_lowlevel.cpp LIBRARIES cli_cpparser_core)
