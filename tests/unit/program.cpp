#include "cli_cpparser/program/program.hpp"

#include "cli_cpparser/command/command.hpp"

#include <doctest/doctest.h>

#include <array>
#include <expected>
#include <ostream>
#include <print>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// A program's table of commands run on words: which command runs, what is printed where,
// and the exit code.
namespace {

using cli_cpparser::Arguments;
using cli_cpparser::Command;
using cli_cpparser::Count;
using cli_cpparser::exit_failure;
using cli_cpparser::exit_ok;
using cli_cpparser::exit_usage;
using cli_cpparser::Form;
using cli_cpparser::Option;
using cli_cpparser::Program;
using Words = std::vector<std::string>;

// Says what it was given, on both streams, and exits with what --exit asks.
std::expected<int, std::string> run_build(const Arguments& arguments, std::ostream& out,
                                          std::ostream& err) {
    std::println(out, "built piece {}", cli_cpparser::value_or(arguments, "--piece", "none"));
    std::println(err, "a note");
    return cli_cpparser::has(arguments, "--fail") ? exit_failure : exit_ok;
}

// Finds its arguments wrong, as only a command can.
std::expected<int, std::string> run_zones(const Arguments& /*arguments*/, std::ostream& out,
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
    Form{.usage = "alfar zones --auto --project DIR",
         .listed = "zones --auto",
         .summary = "Compute the zones from the model's shape"},
    Form{.usage = "alfar zones --check --project DIR",
         .listed = "zones --check",
         .summary = "Validate every zone map"},
    Form{.usage = "alfar zones --auto --review DIR --today DIR --project DIR",
         .listed = "zones --auto --review DIR --today DIR",
         .summary = "The same, with pictures to review"},
    Form{.usage = "alfar zones --hidden"},
};
constexpr std::array commands{
    Command{
        .name = "build",
        .summary = "Fill one piece and write its texture file",
        .options = build_options,
        .run = run_build,
    },
    Command{
        .name = "zones",
        .summary = "Compute or check the zones",
        .options = zones_options,
        .forms = zones_forms,
        .details = "Works on the project's zone maps.\nThey are PNG files.",
        .run = run_zones,
    },
    Command{.name = "later", .summary = "Not written yet"},
};
constexpr Program alfar{
    .name = "alfar",
    .version = "0.5.0",
    .summary = "makes the textures of the urn's pieces",
    .commands = commands,
    .notes = "Every command takes --project DIR.\nIt is the project's folder.",
};

constexpr std::string_view alfar_help =
    "alfar 0.5.0: makes the textures of the urn's pieces\n"
    "\n"
    "Usage: alfar <command> [options]\n"
    "\n"
    "Commands:\n"
    "  build          Fill one piece and write its texture file\n"
    "  zones --auto   Compute the zones from the model's shape\n"
    "  zones --check  Validate every zone map\n"
    "  zones --auto --review DIR --today DIR\n"
    "                 The same, with pictures to review\n"
    "  later          Not written yet\n"
    "  help           Show this list\n"
    "  --version      Print the program's version\n"
    "\n"
    "Every command takes --project DIR.\n"
    "It is the project's folder.\n"
    "\n"
    "Run 'alfar <command> --help' for a command's options.\n";

constexpr std::string_view build_usage = "usage: alfar build --piece N [--fail] "
                                         "[--zone Front|Back|Side|Rim]\n";
constexpr std::string_view build_help = "usage: alfar build --piece N [--fail] "
                                        "[--zone Front|Back|Side|Rim]\n"
                                        "\n"
                                        "Fill one piece and write its texture file\n"
                                        "\n"
                                        "Options:\n"
                                        "  --piece N   The piece to fill\n"
                                        "  --fail      Fail, for the test\n"
                                        "  --zone Front|Back|Side|Rim\n"
                                        "  -h, --help  Show this help\n";

struct Ran {
    int code{};
    std::string out;
    std::string err;
};

Ran ran(const Words& words, const Program& program = alfar) {
    std::ostringstream out;
    std::ostringstream err;
    const int code = cli_cpparser::run(program, words, out, err);
    return Ran{.code = code, .out = out.str(), .err = err.str()};
}

} // namespace

TEST_CASE("without words the help goes to the error stream and the usage is wrong") {
    const Ran result = ran(Words{});
    CHECK(result.code == exit_usage);
    CHECK(result.out == "");
    CHECK(result.err == alfar_help);
}

TEST_CASE("help -h and --help print the program's help") {
    for (const std::string_view word : {"help", "-h", "--help"}) {
        CAPTURE(word);
        const Ran result = ran(Words{std::string{word}, "ignored"});
        CHECK(result.code == exit_ok);
        CHECK(result.out == alfar_help);
        CHECK(result.err == "");
    }
}

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
    const Ran done = ran(Words{"build", "--piece", "7"});
    CHECK(done.code == exit_ok);
    CHECK(done.out == "built piece 7\n");
    CHECK(done.err == "a note\n");
    const Ran failed = ran(Words{"build", "--piece=8", "--fail"});
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
    const Ran result = ran(Words{"zones", "--auto", "--check"});
    CHECK(result.code == exit_usage);
    CHECK(result.out == "looked at the zones\n");
    CHECK(result.err == "error: --auto and --check are both given. Give one of them.\n"
                        "usage: alfar zones --auto --project DIR\n"
                        "       alfar zones --check --project DIR\n"
                        "       alfar zones --auto --review DIR --today DIR --project DIR\n"
                        "       alfar zones --hidden\n");
}

TEST_CASE("a command asked for help prints it and does not run") {
    const Ran result = ran(Words{"build", "--nonsense", "--help"});
    CHECK(result.code == exit_ok);
    CHECK(result.out == build_help);
    CHECK(result.err == "");
}

TEST_CASE("a command's help has its details where it has them in place of its summary") {
    CHECK(cli_cpparser::help_text(commands.at(1), "alfar") ==
          "usage: alfar zones --auto --project DIR\n"
          "       alfar zones --check --project DIR\n"
          "       alfar zones --auto --review DIR --today DIR --project DIR\n"
          "       alfar zones --hidden\n"
          "\n"
          "Works on the project's zone maps.\n"
          "They are PNG files.\n"
          "\n"
          "Options:\n"
          "  --auto\n"
          "  --check\n"
          "  -h, --help  Show this help\n");
}

TEST_CASE("a command with neither summary nor options has a help of its usage and -h") {
    static constexpr Command bare{.name = "bare"};
    CHECK(cli_cpparser::help_text(bare, "alfar") == "usage: alfar bare\n"
                                                    "\n"
                                                    "Options:\n"
                                                    "  -h, --help  Show this help\n");
}

TEST_CASE("a program without notes has none in its help") {
    static constexpr std::array one{Command{.name = "go", .summary = "Go"}};
    static constexpr Program small{
        .name = "tool", .version = "1.2.3", .summary = "a tool", .commands = one};
    CHECK(cli_cpparser::help_text(small) == "tool 1.2.3: a tool\n"
                                            "\n"
                                            "Usage: tool <command> [options]\n"
                                            "\n"
                                            "Commands:\n"
                                            "  go         Go\n"
                                            "  help       Show this list\n"
                                            "  --version  Print the program's version\n"
                                            "\n"
                                            "Run 'tool <command> --help' for a command's "
                                            "options.\n");
}

TEST_CASE("a command whose table names nothing to run fails when it is run") {
    const Ran result = ran(Words{"later"});
    CHECK(result.code == exit_failure);
    CHECK(result.out == "");
    CHECK(result.err == "error: 'alfar later' can't be run: its table names nothing to run.\n");
}

TEST_CASE("a usage error prints the message and the usage and is the wrong-usage code") {
    std::ostringstream err;
    CHECK(cli_cpparser::usage_error(commands.at(0), "alfar", "--piece 0 is no piece.", err) ==
          exit_usage);
    CHECK(err.str() == std::format("error: --piece 0 is no piece.\n{}", build_usage));
}

TEST_CASE("a program with no commands reads its words with one table") {
    static constexpr std::array options{Option{.name = "--port", .value = "N", .help = "Where"}};
    static constexpr Command serve{.summary = "Serves a folder",
                                   .options = options,
                                   .words = {.count = Count::one, .name = "DIR"}};
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
        const auto arguments = cli_cpparser::read(row.words, serve, "serve", out, err);
        REQUIRE_FALSE(arguments.has_value());
        CHECK(arguments.error() == row.code);
        CHECK(out.str() == row.out);
        CHECK(err.str() == row.err);
    }
    std::ostringstream out;
    std::ostringstream err;
    const auto arguments =
        cli_cpparser::read(Words{"site", "--port=8080"}, serve, "serve", out, err);
    REQUIRE(arguments.has_value());
    CHECK(arguments->words == Words{"site"});
    CHECK(cli_cpparser::value_or(*arguments, "--port", "") == "8080");
    CHECK(out.str() == "");
    CHECK(err.str() == "");
}
