#pragma once

#include "cli_cpparser/command/command.hpp"
#include "cli_cpparser/program/program.hpp"

#include <array>
#include <expected>
#include <ostream>
#include <print>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// The program the tests of the program module run, "alfar", with what its help and its
// commands' usage must read.
namespace tables {

using cli_cpparser::Arguments;
using cli_cpparser::Command;
using cli_cpparser::exit_failure;
using cli_cpparser::exit_ok;
using cli_cpparser::exit_usage;
using cli_cpparser::Form;
using cli_cpparser::Option;
using cli_cpparser::Program;
using Words = std::vector<std::string>;

// Says what it was given, on both streams, and exits with what --exit asks.
inline std::expected<int, std::string> run_build(const Arguments& arguments, std::ostream& out,
                                                 std::ostream& err) {
    std::println(out, "built piece {}", cli_cpparser::value_or(arguments, "--piece", "none"));
    std::println(err, "a note");
    return cli_cpparser::has(arguments, "--fail") ? exit_failure : exit_ok;
}

// Finds its arguments wrong, as only a command can.
inline std::expected<int, std::string> run_zones(const Arguments& /*arguments*/, std::ostream& out,
                                                 std::ostream& /*err*/) {
    std::println(out, "looked at the zones");
    return std::unexpected("--auto and --check are both given. Give one of them.");
}

constexpr std::array build_options{
    Option{.name = "--piece", .value = "N", .help = "The piece to fill", .required = true},
    Option{.name = "--fail", .help = "Fail, for the test"},
    Option{.name = "--zone", .value = "Front|Back|Side|Rim"},
};
constexpr std::array zones_options{Option{.name = "--auto"}, Option{.name = "--check"}};
constexpr std::array zones_forms{
    Form{.usage = "alfar zones --auto",
         .listed = "zones --auto",
         .summary = "Compute the zones from the model's shape"},
    Form{.usage = "alfar zones --check",
         .listed = "zones --check",
         .summary = "Validate every zone map"},
    Form{.usage = "alfar zones --auto --review DIR --today DIR",
         .listed = "zones --auto --review DIR --today DIR",
         .summary = "The same, with pictures to review"},
    Form{.usage = "alfar zones --hidden"},
};
constexpr std::array commands{
    Command{
        .name = "build",
        .summary = "Fill one piece and write its texture file",
        .listed = "build --piece N",
        .options = build_options,
        .run = run_build,
    },
    Command{
        .name = "zones",
        .options = zones_options,
        .forms = zones_forms,
        .run = run_zones,
    },
    Command{.name = "later", .summary = "Not written yet"},
};
constexpr std::array folders{
    Option{.name = "--project", .value = "DIR", .help = "The project's folder", .required = true},
    Option{.name = "--work", .value = "DIR", .help = "In place of <project>/work"},
};
constexpr Program alfar{
    .name = "alfar",
    .version = "0.5.0",
    .summary = "makes the textures of the urn's pieces",
    .commands = commands,
    .shared = folders,
    .notes = "A command writes what it did to <work>/reports/.\nIt can be deleted.",
};

constexpr std::string_view alfar_help =
    "alfar 0.5.0: makes the textures of the urn's pieces\n"
    "\n"
    "Usage: alfar <command> --project DIR [options]\n"
    "\n"
    "Commands:\n"
    "  build --piece N  Fill one piece and write its texture file\n"
    "  zones --auto     Compute the zones from the model's shape\n"
    "  zones --check    Validate every zone map\n"
    "  zones --auto --review DIR --today DIR\n"
    "                   The same, with pictures to review\n"
    "  later            Not written yet\n"
    "  help             Show this list\n"
    "  --version        Print the program's version\n"
    "\n"
    "Every command takes:\n"
    "  --project DIR    The project's folder\n"
    "  --work DIR       In place of <project>/work\n"
    "\n"
    "A command writes what it did to <work>/reports/.\n"
    "It can be deleted.\n"
    "\n"
    "Run 'alfar <command> --help' for a command's options.\n";

constexpr std::string_view build_usage = "usage: alfar build --piece N [--fail] "
                                         "[--zone Front|Back|Side|Rim] --project DIR "
                                         "[--work DIR]\n";
constexpr std::string_view build_help = "usage: alfar build --piece N [--fail] "
                                        "[--zone Front|Back|Side|Rim] --project DIR "
                                        "[--work DIR]\n"
                                        "\n"
                                        "Fill one piece and write its texture file\n"
                                        "\n"
                                        "Options:\n"
                                        "  --piece N      The piece to fill\n"
                                        "  --fail         Fail, for the test\n"
                                        "  --zone Front|Back|Side|Rim\n"
                                        "  --project DIR  The project's folder\n"
                                        "  --work DIR     In place of <project>/work\n"
                                        "  -h, --help     Show this help\n";
constexpr std::string_view zones_usage =
    "usage: alfar zones --auto --project DIR [--work DIR]\n"
    "       alfar zones --check --project DIR [--work DIR]\n"
    "       alfar zones --auto --review DIR --today DIR --project DIR [--work DIR]\n"
    "       alfar zones --hidden --project DIR [--work DIR]\n";

struct Ran {
    int code{};
    std::string out;
    std::string err;
};

inline Ran ran(const Words& words, const Program& program = alfar) {
    std::ostringstream out;
    std::ostringstream err;
    const int code = cli_cpparser::run(program, words, out, err);
    return Ran{.code = code, .out = out.str(), .err = err.str()};
}

} // namespace tables
