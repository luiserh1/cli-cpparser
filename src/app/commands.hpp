#pragma once

#include "cli_cpparser/command/command.hpp"

#include <expected>
#include <iosfwd>
#include <string>

// What each command does with its arguments (cli_cpparser::Runner). The tables that say
// what they accept are in run.cpp.
namespace app {

// "repeat WORD --times N [--separator TEXT]": the word, N times.
std::expected<int, std::string> run_repeat(const cli_cpparser::Arguments& arguments,
                                           std::ostream& out, std::ostream& err);

// "sum [NUMBER..] [--scale X] [--show total|mean]": the numbers' total or their mean.
std::expected<int, std::string> run_sum(const cli_cpparser::Arguments& arguments, std::ostream& out,
                                        std::ostream& err);

// "pick --from A,B.. --at N" or "pick --from A,B.. --last": one part of a list.
std::expected<int, std::string> run_pick(const cli_cpparser::Arguments& arguments,
                                         std::ostream& out, std::ostream& err);

} // namespace app
