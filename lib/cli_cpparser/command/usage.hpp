#pragma once

#include "cli_cpparser/command/command.hpp"

#include <string>
#include <vector>

// How a command is written, as text: for the usage, the help and the messages.
namespace cli_cpparser {

// The command as it is called: "alfar zones", or "serve" in a program with no commands.
[[nodiscard]] std::string invocation(const Command& command, const Program& program);

// An option as it is written: "--pieces N,N..", or "--auto" for a flag.
[[nodiscard]] std::string spelled(const Option& option);

// Every option the command takes: its own, then those the program shares.
[[nodiscard]] std::vector<Option> every_option(const Command& command, const Program& program);

// A command's forms: "usage: " and the first, the others under it, each ending its line.
// The options the program shares are written after each form.
//
//   usage: alfar zones --auto [--pieces N,N..] --project DIR [--work DIR]
//          alfar zones --check --project DIR [--work DIR]
//
// A command without forms has one, written from its table: its required options, the others
// in brackets, its words ("FILE" for one, "[FILE..]" for any number), and the shared ones.
//
//   usage: alfar export --out DIR [--allow-blank] --project DIR [--work DIR]
[[nodiscard]] std::string usage_text(const Command& command, const Program& program);

} // namespace cli_cpparser
