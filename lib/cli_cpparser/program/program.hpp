#pragma once

#include "cli_cpparser/command/command.hpp"
#include "cli_cpparser/command/usage.hpp"

#include <expected>
#include <iosfwd>
#include <span>
#include <string>
#include <string_view>

// A program's table of commands, its help, and the exit codes. Nothing here writes to
// stdout or stderr: every function takes the streams.
namespace cli_cpparser {

constexpr int exit_ok = 0;
constexpr int exit_failure = 1; // a command ran and could not do its work
constexpr int exit_usage = 2;   // the command line is wrong

struct Program {
    std::string_view name;    // "alfar"
    std::string_view version; // "0.5.0"
    std::string_view summary; // one line: what the program is for
    std::span<const Command> commands;
    // Shown at the end of the program's help; may have several lines. Empty: nothing.
    std::string_view notes;
};

// Runs the program on the words after its name (argv without argv[0]).
//
//   no words                       the help to `err`; exit_usage
//   help, -h, --help               the help to `out`; exit_ok
//   --version                      "alfar 0.5.0" to `out`; exit_ok
//   a command's name               its words are parsed. With -h or --help among them, the
//                                  command's help goes to `out`; exit_ok. When they are
//                                  wrong, "error: <message>" and the command's usage go to
//                                  `err`; exit_usage. Otherwise the command runs, and what
//                                  it returns is the exit code (a message: as when the
//                                  words are wrong).
//   anything else                  "error: unknown command 'x'. Run 'alfar help' for the
//                                  list of commands." to `err`; exit_usage
int run(const Program& program, std::span<const std::string> words, std::ostream& out,
        std::ostream& err);

// The program's help:
//
//   alfar 0.5.0: makes the textures of the urn's pieces from the museum photos
//
//   Usage: alfar <command> [options]
//
//   Commands:
//     status         Per piece: tag, fills, coverage, whether built
//     zones --auto   Compute Front, Back, Side and Rim from the model's shape
//     zones --check  Validate every zone map
//     help           Show this list
//     --version      Print the program's version
//
//   <notes>
//
//   Run 'alfar <command> --help' for a command's options.
//
// A command with forms has a line for each form that is `listed`; one without, a line with
// its name and summary. A line too long for the column has its summary under it.
[[nodiscard]] std::string help_text(const Program& program);

// A command's help: its usage, its details (or its summary), and its options in a column.
//
//   usage: alfar build --piece N [--zone Z]
//          alfar build --all
//
//   Fill a piece by its recipe and write its texture file
//
//   Options:
//     --all       Fill every piece, reusing the cache
//     --piece N   The piece to fill
//     -h, --help  Show this help
[[nodiscard]] std::string help_text(const Command& command, std::string_view program);

// Prints "error: <message>" and the command's usage to `err`. Returns exit_usage.
int usage_error(const Command& command, std::string_view program, std::string_view message,
                std::ostream& err);

// For a program with no commands: its words, parsed. When they ask for help, the command's
// help is printed to `out` and the unexpected value is exit_ok; when they are wrong, the
// message and the usage go to `err` and it is exit_usage. Either way it is the exit code to
// return at once. (run() does this for a program that has commands.)
[[nodiscard]] std::expected<Arguments, int> read(std::span<const std::string> words,
                                                 const Command& command, std::string_view program,
                                                 std::ostream& out, std::ostream& err);

} // namespace cli_cpparser
