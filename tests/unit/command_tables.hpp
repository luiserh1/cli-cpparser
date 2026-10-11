#pragma once

#include "cli_cpparser/command/command.hpp"
#include "cli_cpparser/command/parse.hpp"

#include <array>
#include <string>
#include <string_view>
#include <vector>

// The commands the tests of the command module parse for, of a program called "alfar".
namespace tables {

using cli_cpparser::Command;
using cli_cpparser::Count;
using cli_cpparser::Option;
using Words = std::vector<std::string>;

constexpr std::string_view program = "alfar";

// A flag and two options that take a value; no words.
constexpr std::array build_options{
    Option{.name = "--all", .help = "Fill every piece"},
    Option{.name = "--piece", .value = "N", .help = "The piece to fill"},
    Option{.name = "--zone", .value = "Z", .help = "One zone of it"},
};
constexpr Command build{.name = "build", .options = build_options};

// One word, and a required option.
constexpr std::array import_options{
    Option{.name = "--project", .value = "DIR", .required = true},
    Option{.name = "--replace"},
};
constexpr Command import{
    .name = "import",
    .options = import_options,
    .words = {.count = Count::one, .name = "FILE"},
};

// Any number of words.
constexpr std::array photos_options{Option{.name = "--add", .value = "FILE"}};
constexpr Command photos{
    .name = "photos",
    .options = photos_options,
    .words = {.count = Count::any, .name = "FILE"},
};

// Two required options, one with its own words for when it is missing.
constexpr std::array export_options{
    Option{.name = "--allow-blank"},
    Option{.name = "--out", .value = "DIR", .required = true},
    Option{.name = "--project",
           .value = "DIR",
           .required = true,
           .missing = "--project DIR is missing: give the project's folder."},
};
constexpr Command exporting{.name = "export", .options = export_options};

// No options at all.
constexpr Command status{.name = "status"};

// Why the words are refused; empty when they are not.
inline std::string refusal(const Words& words, const Command& command = build) {
    const auto arguments = cli_cpparser::parse(words, command, program);
    return arguments ? std::string{} : arguments.error();
}

} // namespace tables
