#include "cli_cpparser/program/program.hpp"

#include "cli_cpparser/command/command.hpp"
#include "program_tables.hpp"

#include <doctest/doctest.h>

#include <array>
#include <format>
#include <sstream>
#include <string>
#include <string_view>

// A program's table of commands run on words: which command runs, what is printed where,
// and the exit code. The help texts are in program_help.cpp.
using namespace tables;

TEST_CASE("--version prints the program's name and version") {
    const Ran result = ran(Words{"--version"});
    CHECK(result.code == exit_ok);
    CHECK(result.out == "alfar 0.5.0\n");
    CHECK(result.err == "");
}

TEST_CASE("a word that is no command is refused with where the list is") {
    // "version" without its dashes is no command either, and neither is a command's start.
    for (const std::string_view word : {"paint", "version", "buil", "Build", "--build", ""}) {
        CAPTURE(word);
        const Ran result = ran(Words{std::string{word}, "--piece", "1"});
        CHECK(result.code == exit_usage);
        CHECK(result.out == "");
        CHECK(result.err == std::format("error: unknown command '{}'. Run 'alfar help' for the "
                                        "list of commands.\n",
                                        word));
    }
}

TEST_CASE("a command runs with its arguments and both streams and its code is the exit code") {
    const Ran done = ran(Words{"build", "--piece", "7", "--project", "p"});
    CHECK(done.code == exit_ok);
    CHECK(done.out == "built piece 7\n");
    CHECK(done.err == "a note\n");
    const Ran failed = ran(Words{"build", "--project=p", "--piece=8", "--fail"});
    CHECK(failed.code == exit_failure);
    CHECK(failed.out == "built piece 8\n");
}

TEST_CASE("a command with wrong words does not run: the message and its usage are printed") {
    const Ran result = ran(Words{"build", "--piece"});
    CHECK(result.code == exit_usage);
    CHECK(result.out == "");
    CHECK(result.err == std::format("error: --piece needs a value: write --piece N. (A value "
                                    "that starts with -- is written --piece=VALUE.)\n{}",
                                    build_usage));
}

TEST_CASE("a message a command returns is printed with its usage as a wrong command line") {
    const Ran result = ran(Words{"zones", "--auto", "--check", "--project", "p"});
    CHECK(result.code == exit_usage);
    CHECK(result.out == "looked at the zones\n");
    CHECK(result.err ==
          std::format("error: --auto and --check are both given. Give one of them.\n{}",
                      zones_usage));
}

TEST_CASE("a command asked for help prints it and does not run") {
    const Ran result = ran(Words{"build", "--nonsense", "--help"});
    CHECK(result.code == exit_ok);
    CHECK(result.out == build_help);
    CHECK(result.err == "");
}

TEST_CASE("a command whose table names nothing to run fails when it is run") {
    const Ran result = ran(Words{"later", "--project", "p"});
    CHECK(result.code == exit_failure);
    CHECK(result.out == "");
    CHECK(result.err == "error: 'alfar later' can't be run: its table names nothing to run.\n");
}

TEST_CASE("a usage error prints the message and the usage and is the wrong-usage code") {
    std::ostringstream err;
    CHECK(cli_cpparser::usage_error(commands.at(0), alfar, "--piece 0 is no piece.", err) ==
          exit_usage);
    CHECK(err.str() == std::format("error: --piece 0 is no piece.\n{}", build_usage));
}

TEST_CASE("one command is run on its words without the program's other commands") {
    struct Row {
        std::string_view what;
        Words words;
        int code;
        std::string_view out;
        std::string err;
    };
    const std::array rows{
        Row{.what = "it runs",
            .words = Words{"--piece", "7", "--project", "p"},
            .code = exit_ok,
            .out = "built piece 7\n",
            .err = "a note\n"},
        Row{.what = "help", .words = Words{"-h"}, .code = exit_ok, .out = build_help, .err = ""},
        Row{.what = "wrong words",
            .words = Words{"--piece", "7"},
            .code = exit_usage,
            .out = "",
            .err = std::format("error: --project is missing: 'alfar build' needs it. Write "
                               "--project DIR.\n{}",
                               build_usage)},
    };
    for (const Row& row : rows) {
        CAPTURE(row.what);
        std::ostringstream out;
        std::ostringstream err;
        CHECK(cli_cpparser::run_command(commands.at(0), row.words, alfar, out, err) == row.code);
        CHECK(out.str() == row.out);
        CHECK(err.str() == row.err);
    }
}

TEST_CASE("a program with no commands reads its words with one table") {
    static constexpr std::array options{Option{.name = "--port", .value = "N", .help = "Where"}};
    static constexpr Command serve{
        .summary = "Serves a folder", .options = options, .words = cli_cpparser::one_word("DIR")};
    static constexpr Program server{.name = "serve", .version = "1.0.0"};
    struct Row {
        std::string_view what;
        Words words;
        int code;
        std::string_view out;
        std::string_view err;
    };
    const std::array rows{
        Row{.what = "help",
            .words = Words{"site", "-h"},
            .code = exit_ok,
            .out = "usage: serve [--port N] DIR\n\nServes a folder\n\nOptions:\n"
                   "  --port N    Where\n  -h, --help  Show this help\n",
            .err = ""},
        Row{.what = "wrong words",
            .words = Words{},
            .code = exit_usage,
            .out = "",
            .err = "error: 'serve' needs its DIR: write serve DIR.\n"
                   "usage: serve [--port N] DIR\n"},
    };
    for (const Row& row : rows) {
        CAPTURE(row.what);
        std::ostringstream out;
        std::ostringstream err;
        const auto arguments = cli_cpparser::read(row.words, serve, server, out, err);
        REQUIRE_FALSE(arguments.has_value());
        CHECK(arguments.error() == row.code);
        CHECK(out.str() == row.out);
        CHECK(err.str() == row.err);
    }
    std::ostringstream out;
    std::ostringstream err;
    const auto arguments =
        cli_cpparser::read(Words{"site", "--port=8080"}, serve, server, out, err);
    REQUIRE(arguments.has_value());
    CHECK(arguments->words == Words{"site"});
    CHECK(cli_cpparser::value_or(*arguments, "--port", "") == "8080");
    CHECK(out.str() == "");
    CHECK(err.str() == "");
}
