#include "cli_cpparser/command/command.hpp"
#include "cli_cpparser/command/usage.hpp"
#include "command_tables.hpp"

#include <doctest/doctest.h>

#include <array>
#include <string_view>

// The usage written from a table whose options have rules between them, or whose words
// are shown by another name.
namespace {

using namespace tables;
using cli_cpparser::Needs;

constexpr std::array export_options{
    Option{.name = "--with", .value = "PROGRAM"},
    Option{.name = "--allow-blank"},
    Option{.name = "--result", .value = "OUT"},
    Option{.name = "--out", .value = "DIR", .required = true},
};

std::string usage_with(std::span<const Needs> needs, const Program& of = program) {
    const Command command{.name = "export", .options = export_options, .needs = needs};
    return cli_cpparser::usage_text(command, of);
}

} // namespace

TEST_CASE("two options that each need the other share their brackets in the usage") {
    static constexpr std::array needs{Needs{.option = "--with", .needs = "--result"},
                                      Needs{.option = "--result", .needs = "--with"}};
    // The second is written beside the first, wherever the table has it.
    CHECK(usage_with(needs) ==
          "usage: alfar export --out DIR [--with PROGRAM --result OUT] [--allow-blank]\n");
    CHECK(usage_with(needs, with_folders) ==
          "usage: alfar export --out DIR [--with PROGRAM --result OUT] [--allow-blank] "
          "--project DIR [--work DIR]\n");
}

TEST_CASE("an option that needs another one way only has its own brackets in the usage") {
    static constexpr std::array one_way{Needs{.option = "--result", .needs = "--with"}};
    CHECK(usage_with(one_way) ==
          "usage: alfar export --out DIR [--with PROGRAM] [--allow-blank] [--result OUT]\n");
}

TEST_CASE("an option that needs a required one has its own brackets in the usage") {
    static constexpr std::array needs{Needs{.option = "--result", .needs = "--out"},
                                      Needs{.option = "--out", .needs = "--result"}};
    CHECK(usage_with(needs) ==
          "usage: alfar export --out DIR [--with PROGRAM] [--allow-blank] [--result OUT]\n");
}

TEST_CASE("three options that all need each other share one pair of brackets in the usage") {
    static constexpr std::array needs{
        Needs{.option = "--with", .needs = "--result"},
        Needs{.option = "--result", .needs = "--with"},
        Needs{.option = "--with", .needs = "--allow-blank"},
        Needs{.option = "--allow-blank", .needs = "--with"},
        Needs{.option = "--result", .needs = "--allow-blank"},
        Needs{.option = "--allow-blank", .needs = "--result"},
    };
    CHECK(usage_with(needs) ==
          "usage: alfar export --out DIR [--with PROGRAM --allow-blank --result OUT]\n");
}

TEST_CASE("an option that goes with two others that do not go together is written once") {
    static constexpr std::array needs{
        Needs{.option = "--with", .needs = "--result"},
        Needs{.option = "--result", .needs = "--with"},
        Needs{.option = "--result", .needs = "--allow-blank"},
        Needs{.option = "--allow-blank", .needs = "--result"},
    };
    CHECK(usage_with(needs) ==
          "usage: alfar export --out DIR [--with PROGRAM --result OUT] [--allow-blank]\n");
}

TEST_CASE("a word is written in the usage as it is shown and called by its name elsewhere") {
    static constexpr Command imports{
        .name = "import", .words = {.least = 1, .most = 1, .name = "FILE", .shown = "FILE|FOLDER"}};
    CHECK(cli_cpparser::usage_text(imports, with_folders) ==
          "usage: alfar import FILE|FOLDER --project DIR [--work DIR]\n");
    CHECK(refusal(Words{"--project", "p"}, imports, with_folders) ==
          "'alfar import' needs its FILE: write alfar import FILE --project DIR.");
    static constexpr Command add{
        .name = "add",
        .words = {.least = 1, .most = cli_cpparser::any_number, .name = "FILE", .shown = "PATH"}};
    CHECK(cli_cpparser::usage_text(add, program) == "usage: alfar add PATH [PATH..]\n");
}
