#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <expected>
#include <iosfwd>
#include <limits>
#include <optional>
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

// How many words that are no options a command takes: from `least` to `most`. Left as they
// are, none.
inline constexpr std::size_t any_number = std::numeric_limits<std::size_t>::max();
struct Words {
    std::size_t least{};
    std::size_t most{};
    std::string_view name; // what the messages call one of them: "FILE"
    // How the usage writes one, when that is not its name: "FILE|FOLDER". Empty: `name`.
    std::string_view shown;
};
// Exactly one word, and any number of them, none included.
[[nodiscard]] constexpr Words one_word(std::string_view name) {
    return Words{.least = 1, .most = 1, .name = name};
}
[[nodiscard]] constexpr Words any_words(std::string_view name) {
    return Words{.least = 0, .most = any_number, .name = name};
}

// One way of writing a command, when it has several. A form with only its usage is the
// usual case: a command whose forms differ in their options is listed once in the
// program's help, by its name and summary.
struct Form {
    // The command line as the usage shows it, without the options the program shares:
    // "alfar zones --check". The library writes those after it (Program::shared).
    std::string_view usage;
    // The same in short, as the program's help lists it: "zones --check". Empty: the form
    // has no line of its own there.
    std::string_view listed;
    std::string_view summary; // what this form does, beside `listed`
};

// Options of which one is given, and no more: "--auto" or "--check".
//   "--auto and --check are both given. Give one of them."
//   "Neither --auto nor --check is given. Give one of them."        (when `required`)
struct Choice {
    std::array<std::string_view, 6> among; // their names; the places left over stay empty
    bool required{};                       // one must be given; otherwise none may be
    // What to say in place of those two sentences, as Option::missing is for a required
    // option: when several are given, and when none is and one must be. Empty: the
    // sentence above.
    std::string_view several;
    std::string_view missing;
};

// An option that is only given with another: "--zone" with "--piece".
//   "--zone goes with --piece: give --piece N too, or leave --zone out."
// Two options that go together are two of these, one each way; the usage written from the
// table then has them in one pair of brackets: "[--tonematcher PROGRAM --result OUT]".
struct Needs {
    std::string_view option;
    std::string_view needs;
    std::string_view message; // what to say in place of that sentence; empty: the sentence
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
    std::vector<std::string> words; // the words that are no options, in order
    bool help{};                    // -h or --help was among the words: nothing else was looked at
};

// What a command does with its arguments. It returns its exit code (exit_ok or exit_failure,
// program/program.hpp; a program's own codes start at 3), having printed what it did to
// `out` and why it failed to `err`. When the arguments are wrong in a way only the command
// knows, it returns the message instead: run() prints it with the command's usage and exits
// with exit_usage. A typed reader's error (values/readers.hpp) is such a message.
using Runner = std::expected<int, std::string> (*)(const Arguments& arguments, std::ostream& out,
                                                   std::ostream& err);

struct Command {
    std::string_view name;    // the word after the program's: "zones". Empty in a program
                              // that has no commands.
    std::string_view summary; // one line: what it does
    // How the program's help lists it, when the name alone says too little: "export --out
    // DIR". Empty: by its name. (A command with listed forms has their lines instead.)
    std::string_view listed;
    std::span<const Option> options; // its own; the program's shared ones are added to them
    Words words;
    // Its forms, written by hand, for a command one line cannot describe ("zones --auto .."
    // / "zones --check"). Empty: one form is written from the table: its required options,
    // the others in brackets, its words, and the options the program shares.
    std::span<const Form> forms;
    // More for the command's help, under the usage and above the options; may have several
    // lines. Empty: the summary is shown there, or the listed forms when there is none.
    std::string_view details;
    std::span<const Choice> choices; // rules between its options, checked by the parser
    std::span<const Needs> needs;
    Runner run{}; // null only in a program that has no commands, which calls read() itself
};

// A program: its name, for the messages and the help, and what its commands share.
struct Program {
    std::string_view name;    // "alfar"
    std::string_view version; // "0.5.0"
    std::string_view summary; // one line: what the program is for
    std::span<const Command> commands;
    // Options every command takes, besides its own ("--project DIR"). They are written
    // after a command's own in its usage and its help, the program's help lists them once,
    // and a required one that is missing is reported before a command's own. Options that
    // only some commands share are joined to those commands' own (joined()).
    std::span<const Option> shared;
    // Shown at the end of the program's help; may have several lines. Empty: nothing.
    std::string_view notes;
};

// Several lists of options as one, in the order given: a command's own with a list that
// some commands share.
//
//   constexpr std::array gimp{Option{"--gimp", "PATH", "..."}, ...};
//   constexpr std::array own{Option{"--tree", {}, "..."}, Option{"--dir", "PATH", "..."}};
//   constexpr auto layers_options = joined(own, gimp);
//   constexpr Command layers{.name = "layers", .options = layers_options, ...};
template <std::size_t... Sizes>
[[nodiscard]] constexpr std::array<Option, (Sizes + ...)>
joined(const std::array<Option, Sizes>&... lists) {
    std::array<Option, (Sizes + ...)> all{};
    auto at = all.begin();
    ((at = std::ranges::copy(lists, at).out), ...);
    return all;
}

// Whether an option was given, and its value (nothing when it was not; empty for a flag).
// The value lives as long as `arguments`. It is a view: where a string is wanted, as by a
// file's stream on Linux, make a std::string or a std::filesystem::path of it.
[[nodiscard]] bool has(const Arguments& arguments, std::string_view name);
[[nodiscard]] std::optional<std::string_view> value_of(const Arguments& arguments,
                                                       std::string_view name);
// The value, or `fallback` when the option was not given.
[[nodiscard]] std::string value_or(const Arguments& arguments, std::string_view name,
                                   std::string_view fallback);
// The value of an option the table marks `required`, which the parser has seen to. (For
// any other option that was not given, it is empty.)
[[nodiscard]] std::string_view required_value(const Arguments& arguments, std::string_view name);

} // namespace cli_cpparser
