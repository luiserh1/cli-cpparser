#pragma once

#include "cli_cpparser/command/command.hpp"

#include <string>

// What parse() checks once every word is read: the command's words against what it takes,
// its required options, and the rules between its options.
namespace cli_cpparser::internal {

// Why the arguments are not what the command takes; empty when they are. The first of:
// the number of words, a required option missing (the program's shared ones first), a
// choice with none or two given, an option without the one it goes with.
[[nodiscard]] std::string refusal(const Arguments& arguments, const Command& command,
                                  const Program& program);

// "Its options are --all, --piece N, --project DIR.", or "It has no options."
[[nodiscard]] std::string options_text(const Command& command, const Program& program);

} // namespace cli_cpparser::internal
