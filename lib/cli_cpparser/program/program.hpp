#pragma once

#include "cli_cpparser/command/command.hpp"

#include <expected>
#include <iosfwd>
#include <span>
#include <string>
#include <string_view>

// A program's commands run on its words, its help, and the exit codes. Nothing here writes
// to stdout or stderr: every function takes the streams.
namespace cli_cpparser {

constexpr int exit_ok = 0;
constexpr int exit_failure = 1; // a command ran and could not do its work
constexpr int exit_usage = 2;   // the command line is wrong; a program's own codes start at 3

// Runs the program on the words after its name (argv without argv[0]).
//
//   no words                       the help to `err`; exit_usage
//   help, -h, --help               the help to `out`; exit_ok
//   --version                      "alfar 0.5.0" to `out`; exit_ok
//   a command's name               run_command() on the words after it
//   anything else                  "error: unknown command 'x'. Run 'alfar help' for the
//                                  list of commands." to `err`; exit_usage
int run(const Program& program, std::span<const std::string> words, std::ostream& out,
        std::ostream& err);

// Runs one command on the words after its name. With -h or --help among them, the command's
// help goes to `out`; exit_ok. When they are wrong, "error: <message>" and the command's
// usage go to `err`; exit_usage. Otherwise the command runs, and what it returns is the
// exit code (a message: as when the words are wrong).
//
// It is what run() does once it has found the command, for a second caller of one command
// that should not link every other (a window's button that runs "model" again).
int run_command(const Command& command, std::span<const std::string> words, const Program& program,
                std::ostream& out, std::ostream& err);

// The program's help:
//
//   alfar 0.5.0: makes the textures of the urn's pieces from the museum photos
//
//   Usage: alfar <command> --project DIR [options]
//
//   Commands:
//     status            Per piece: tag, fills, coverage, whether built
//     export --out DIR  Write the textures and zone maps
//     zones --auto      Compute Front, Back, Side and Rim from the model's shape
//     zones --check     Validate every zone map
//     help              Show this list
//     --version         Print the program's version
//
//   Every command takes:
//     --project DIR     The project's folder
//     --work DIR        In place of <project>/work
//
//   <notes>
//
//   Run 'alfar <command> --help' for a command's options.
//
// A command with listed forms has a line for each; one without, a line with its `listed`
// text or its name, and its summary. A line too long for the column has its summary under
// it. The usage line names the shared options that are required, and "Every command takes"
// is there when the program has shared options.
[[nodiscard]] std::string help_text(const Program& program);

// A command's help: its usage, what it does, and its options in a column, its own and then
// the shared ones.
//
//   usage: alfar build --piece N [--zone Z] --project DIR
//          alfar build --all --project DIR
//
//   Fill a piece by its recipe and write its texture file
//
//   Options:
//     --all          Fill every piece, reusing the cache
//     --piece N      The piece to fill
//     --project DIR  The project's folder
//     -h, --help     Show this help
//
// What it does is its `details`; without them its summary; without that, its listed forms
// with their summaries.
[[nodiscard]] std::string help_text(const Command& command, const Program& program);

// Prints "error: <message>" and the command's usage to `err`. Returns exit_usage.
int usage_error(const Command& command, const Program& program, std::string_view message,
                std::ostream& err);

// For a program with no commands: its words, parsed by its one table. When they ask for
// help, the command's help is printed to `out` and the unexpected value is exit_ok; when
// they are wrong, the message and the usage go to `err` and it is exit_usage. Either way it
// is the exit code to return at once.
[[nodiscard]] std::expected<Arguments, int> read(std::span<const std::string> words,
                                                 const Command& command, const Program& program,
                                                 std::ostream& out, std::ostream& err);

} // namespace cli_cpparser
