#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iosfwd>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// A command's table: what it accepts on its command line, written once as a constant. The
// parser, the help and the error messages all read this table, so they cannot disagree.
namespace cli_cpparser {

// One option of a command.
struct Option {
    std::string_view name;  // as it is written: "--pieces"
    std::string_view value; // what follows it, as the help calls it ("N,N.."); empty: a flag
    std::string_view help;  // one line for the command's help
    bool required{};        // the command line is wrong without it
    // What to say when a required option is missing; empty: "--out is missing: 'alfar
    // export' needs it. Write --out DIR."
    std::string_view missing;
};

// How many words that are no options a command takes.
enum class Count : std::uint8_t {
    none,
    one, // exactly one
    any, // any number, none included
};

struct Words {
    Count count{Count::none};
    std::string_view name; // what the help and the messages call one of them: "FILE"
};

// One way of writing a command, when it has several.
struct Form {
    // The whole command line, as the usage shows it: "alfar zones --check --project DIR
    // [--inputs DIR] [--work DIR]". The library adds nothing to it.
    std::string_view usage;
    // The same in short, as the program's help lists it: "zones --check". Empty: the form
    // has no line of its own there.
    std::string_view listed;
    std::string_view summary; // what this form does, beside `listed`
};

// An option as it was given.
struct Given {
    std::string name;
    std::string value; // empty for a flag
    [[nodiscard]] bool operator==(const Given&) const = default;
};

// A command line, read. A command asks this and never the words.
struct Arguments {
    std::vector<Given> options;     // in the order given, each at most once
    std::vector<std::string> words; // the words that are no options: one for Count::one
    bool help{};                    // -h or --help was among the words: nothing else was looked at
};

// What a command does with its arguments. It returns its exit code (exit_ok or exit_failure,
// program/program.hpp), having printed what it did to `out` and why it failed to `err`. When
// the arguments are wrong in a way only the command knows ("--zone needs --piece"), it
// returns the message instead: run() prints it with the command's usage and exits with
// exit_usage. A typed reader's error (values/readers.hpp) is such a message.
using Runner = std::expected<int, std::string> (*)(const Arguments& arguments, std::ostream& out,
                                                   std::ostream& err);

struct Command {
    std::string_view name;           // the word after the program's: "zones". Empty in a
                                     // program that has no commands.
    std::string_view summary;        // one line: what it does
    std::span<const Option> options; // every option it takes; see joined() for shared lists
    Words words;
    // Its forms, written by hand, for a command a table cannot describe ("zones --auto .." /
    // "zones --check"). Empty: one form is written from the table, the required options
    // first and the others in brackets, and the program's help lists the command by its name
    // and summary.
    std::span<const Form> forms;
    // More for the command's help, under the usage and above the options; may have several
    // lines. Empty: the summary is shown there.
    std::string_view details;
    Runner run{}; // null only in a program that has no commands, which calls read() itself
};

// Several lists of options as one, in the order given: a command's own with the lists every
// command of the program shares.
//
//   constexpr std::array folders{Option{"--project", "DIR", "..."}, ...};
//   constexpr std::array own{Option{"--all", {}, "..."}, Option{"--piece", "N", "..."}};
//   constexpr auto build_options = joined(own, folders);
//   constexpr Command build{.name = "build", .options = build_options, ...};
template <std::size_t... Sizes>
[[nodiscard]] constexpr std::array<Option, (Sizes + ...)>
joined(const std::array<Option, Sizes>&... lists) {
    std::array<Option, (Sizes + ...)> all{};
    auto at = all.begin();
    ((at = std::ranges::copy(lists, at).out), ...);
    return all;
}

// Whether an option was given, and its value (nothing when it was not; empty for a flag).
// The value lives as long as `arguments`.
[[nodiscard]] bool has(const Arguments& arguments, std::string_view name);
[[nodiscard]] const std::string* value_of(const Arguments& arguments, std::string_view name);
// The value, or `fallback` when the option was not given.
[[nodiscard]] std::string value_or(const Arguments& arguments, std::string_view name,
                                   std::string_view fallback);

} // namespace cli_cpparser
