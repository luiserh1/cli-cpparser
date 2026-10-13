#include "cli_cpparser/command/command.hpp"
#include "cli_cpparser/program/program.hpp"
#include "program_tables.hpp"

#include <doctest/doctest.h>

#include <array>
#include <format>
#include <sstream>
#include <string>
#include <string_view>

// The help of a program and of its commands, whole: it is what a person reads first.
using namespace tables;

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

TEST_CASE("a command without details or summary has its listed forms in its help") {
    CHECK(cli_cpparser::help_text(commands.at(1), alfar) ==
          std::format("{}"
                      "\n"
                      "  zones --auto   Compute the zones from the model's shape\n"
                      "  zones --check  Validate every zone map\n"
                      "  zones --auto --review DIR --today DIR\n"
                      "                 The same, with pictures to review\n"
                      "\n"
                      "Options:\n"
                      "  --auto\n"
                      "  --check\n"
                      "  --project DIR  The project's folder\n"
                      "  --work DIR     In place of <project>/work\n"
                      "  -h, --help     Show this help\n",
                      zones_usage));
}

TEST_CASE("a command's help has its details where it has them in place of its summary") {
    static constexpr Command detailed{.name = "zones",
                                      .summary = "Compute or check the zones",
                                      .forms = zones_forms,
                                      .details = "Works on the project's zone maps.\n"
                                                 "They are PNG files."};
    static constexpr Program plain{.name = "alfar"};
    CHECK(cli_cpparser::help_text(detailed, plain) ==
          "usage: alfar zones --auto\n"
          "       alfar zones --check\n"
          "       alfar zones --auto --review DIR --today DIR\n"
          "       alfar zones --hidden\n"
          "\n"
          "Works on the project's zone maps.\n"
          "They are PNG files.\n"
          "\n"
          "Options:\n"
          "  -h, --help  Show this help\n");
}

TEST_CASE("a command with neither summary nor options has a help of its usage and -h") {
    static constexpr Command bare{.name = "bare"};
    static constexpr Program plain{.name = "alfar"};
    CHECK(cli_cpparser::help_text(bare, plain) == "usage: alfar bare\n"
                                                  "\n"
                                                  "Options:\n"
                                                  "  -h, --help  Show this help\n");
}

TEST_CASE("a program without shared options or notes has neither in its help") {
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
