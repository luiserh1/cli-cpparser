#pragma once

#include "cli_cpparser/command/command.hpp"

#include <string>
#include <string_view>

// How a command is written, as text: for the usage, the help and the messages.
namespace cli_cpparser {

// The command as it is called: "alfar zones", or "serve" in a program with no commands.
[[nodiscard]] std::string invocation(const Command& command, std::string_view program);

// An option as it is written: "--pieces N,N..", or "--auto" for a flag.
[[nodiscard]] std::string spelled(const Option& option);

// A command's forms: "usage: " and the first, the others under it, each ending its line.
//
//   usage: alfar zones --auto [--pieces N,N..]
//          alfar zones --check
//
// A command without forms has one, written from its table: the required options, the others
// in brackets, and its words ("FILE" for one, "[FILE..]" for any number).
//
//   usage: alfar export --out DIR [--allow-blank]
[[nodiscard]] std::string usage_text(const Command& command, std::string_view program);

} // namespace cli_cpparser
