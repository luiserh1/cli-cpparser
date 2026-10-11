#include "cli_cpparser/command/command.hpp"

#include "cli_cpparser/command/parse.hpp"
#include "cli_cpparser/command/usage.hpp"
#include "command_tables.hpp"

#include <doctest/doctest.h>

#include <array>
#include <string>
#include <string_view>
#include <vector>

// A command's words read into its arguments, and the command written as text. What is said
// when the words are wrong is in command_refusals.cpp.
namespace {

using namespace tables;
using cli_cpparser::Form;
using cli_cpparser::Given;
using cli_cpparser::has;
using cli_cpparser::parse;
using cli_cpparser::value_of;

} // namespace

TEST_CASE("an option's value is the next word or what follows its equals sign") {
    const std::array<Words, 3> spellings{
        Words{"--piece", "23", "--zone", "Front"},
        Words{"--piece=23", "--zone=Front"},
        Words{"--zone=Front", "--piece", "23"},
    };
    for (const Words& words : spellings) {
        CAPTURE(words.front());
        const auto arguments = parse(words, build, program);
        REQUIRE(arguments.has_value());
        REQUIRE(value_of(*arguments, "--piece") != nullptr);
        REQUIRE(value_of(*arguments, "--zone") != nullptr);
        CHECK(*value_of(*arguments, "--piece") == "23");
        CHECK(*value_of(*arguments, "--zone") == "Front");
        CHECK(arguments->words == Words{});
        CHECK_FALSE(arguments->help);
    }
}

TEST_CASE("a flag stands alone and has no value") {
    const auto arguments = parse(Words{"--all"}, build, program);
    REQUIRE(arguments.has_value());
    CHECK(arguments->options == std::vector<Given>{Given{.name = "--all", .value = ""}});
}

TEST_CASE("the options are kept in the order they were given") {
    const auto arguments = parse(Words{"--zone=Rim", "--all", "--piece", "7"}, build, program);
    REQUIRE(arguments.has_value());
    CHECK(arguments->options == std::vector<Given>{
                                    Given{.name = "--zone", .value = "Rim"},
                                    Given{.name = "--all", .value = ""},
                                    Given{.name = "--piece", .value = "7"},
                                });
}

TEST_CASE("a value that starts with two dashes is taken after an equals sign") {
    const auto arguments = parse(Words{"--zone=--odd"}, build, program);
    REQUIRE(arguments.has_value());
    REQUIRE(value_of(*arguments, "--zone") != nullptr);
    CHECK(*value_of(*arguments, "--zone") == "--odd");
}

TEST_CASE("only the first equals sign parts an option from its value") {
    const auto arguments = parse(Words{"--zone=a=b"}, build, program);
    REQUIRE(arguments.has_value());
    REQUIRE(value_of(*arguments, "--zone") != nullptr);
    CHECK(*value_of(*arguments, "--zone") == "a=b");
}

TEST_CASE("no words at all are the arguments of a command that needs nothing") {
    const auto arguments = parse(Words{}, build, program);
    REQUIRE(arguments.has_value());
    CHECK(arguments->options.empty());
    CHECK(arguments->words.empty());
    CHECK_FALSE(arguments->help);
}

TEST_CASE("a command that takes one word gets it wherever it is among the options") {
    const std::array<Words, 3> orders{
        Words{"a.png", "--project", "p", "--replace"},
        Words{"--project", "p", "a.png", "--replace"},
        Words{"--replace", "--project=p", "a.png"},
    };
    for (const Words& words : orders) {
        CAPTURE(words.front());
        const auto arguments = parse(words, import, program);
        REQUIRE(arguments.has_value());
        CHECK(arguments->words == Words{"a.png"});
        CHECK(has(*arguments, "--replace"));
    }
}

TEST_CASE("a command that takes any number of words gets them in order none included") {
    const auto several = parse(Words{"b.png", "--add", "a.png", "c.png"}, photos, program);
    REQUIRE(several.has_value());
    CHECK(several->words == Words{"b.png", "c.png"});
    const auto none = parse(Words{"--add=a.png"}, photos, program);
    REQUIRE(none.has_value());
    CHECK(none->words.empty());
}

TEST_CASE("a word that is not two dashes and a name is a plain word") {
    // There are no short options: "-x" is a word, and so are "-", "--" and a negative number.
    const Words plain{"-x", "-", "--", "-2.5", "a=b"};
    const auto arguments = parse(plain, photos, program);
    REQUIRE(arguments.has_value());
    CHECK(arguments->words == plain);
    // And so it can be an option's value.
    const auto value = parse(Words{"--piece", "-4"}, build, program);
    REQUIRE(value.has_value());
    REQUIRE(value_of(*value, "--piece") != nullptr);
    CHECK(*value_of(*value, "--piece") == "-4");
}

TEST_CASE("-h or --help anywhere wins and nothing else is looked at") {
    const std::array<Words, 5> askings{
        Words{"-h"},
        Words{"--help"},
        Words{"--piece", "1", "--help"},
        Words{"--no-such-option", "--piece", "--piece", "-h", "stray"},
        Words{"--piece", "--help"},
    };
    for (const Words& words : askings) {
        CAPTURE(words.front());
        const auto arguments = parse(words, build, program);
        REQUIRE(arguments.has_value());
        CHECK(arguments->help);
        CHECK(arguments->options.empty());
        CHECK(arguments->words.empty());
    }
}

TEST_CASE("an option is asked for by its name") {
    const auto arguments = parse(Words{"--all", "--piece", "3"}, build, program);
    REQUIRE(arguments.has_value());
    CHECK(has(*arguments, "--all"));
    CHECK(has(*arguments, "--piece"));
    CHECK_FALSE(has(*arguments, "--zone"));
    CHECK(value_of(*arguments, "--zone") == nullptr);
    REQUIRE(value_of(*arguments, "--all") != nullptr);
    CHECK(*value_of(*arguments, "--all") == "");
    CHECK(cli_cpparser::value_or(*arguments, "--piece", "9") == "3");
    CHECK(cli_cpparser::value_or(*arguments, "--zone", "Front") == "Front");
}

TEST_CASE("lists of options are joined in the order given") {
    static constexpr std::array shared{Option{.name = "--project", .value = "DIR"},
                                       Option{.name = "--work", .value = "DIR"}};
    static constexpr auto all = cli_cpparser::joined(build_options, shared);
    static_assert(all.size() == 5);
    std::vector<std::string_view> names;
    names.reserve(all.size());
    for (const Option& option : all) {
        names.push_back(option.name);
    }
    CHECK(names ==
          std::vector<std::string_view>{"--all", "--piece", "--zone", "--project", "--work"});
    CHECK(all.at(3).value == "DIR");
}

TEST_CASE("a command without forms has its usage written from its table") {
    struct Row {
        const Command* command;
        std::string_view usage;
    };
    const std::array rows{
        Row{.command = &build, .usage = "usage: alfar build [--all] [--piece N] [--zone Z]\n"},
        Row{.command = &import, .usage = "usage: alfar import --project DIR [--replace] FILE\n"},
        Row{.command = &photos, .usage = "usage: alfar photos [--add FILE] [FILE..]\n"},
        Row{.command = &exporting,
            .usage = "usage: alfar export --out DIR --project DIR [--allow-blank]\n"},
        Row{.command = &status, .usage = "usage: alfar status\n"},
    };
    for (const Row& row : rows) {
        CAPTURE(row.command->name);
        CHECK(cli_cpparser::usage_text(*row.command, program) == row.usage);
    }
}

TEST_CASE("a command's forms are its usage as they are written") {
    static constexpr std::array forms{
        Form{.usage = "alfar zones --auto [--pieces N,N..] --project DIR"},
        Form{.usage = "alfar zones --check --project DIR"},
        Form{.usage = "alfar zones --list"},
    };
    static constexpr Command zones{.name = "zones", .options = build_options, .forms = forms};
    CHECK(cli_cpparser::usage_text(zones, program) ==
          "usage: alfar zones --auto [--pieces N,N..] --project DIR\n"
          "       alfar zones --check --project DIR\n"
          "       alfar zones --list\n");
}

TEST_CASE("a command is called by the program's name and its own") {
    CHECK(cli_cpparser::invocation(build, "alfar") == "alfar build");
    // A program with no commands has one table, without a name.
    static constexpr Command alone{.options = build_options};
    CHECK(cli_cpparser::invocation(alone, "serve") == "serve");
    CHECK(cli_cpparser::usage_text(alone, "serve") ==
          "usage: serve [--all] [--piece N] [--zone Z]\n");
}

TEST_CASE("an option is written with the name of its value") {
    CHECK(cli_cpparser::spelled(build_options.at(0)) == "--all");
    CHECK(cli_cpparser::spelled(build_options.at(1)) == "--piece N");
}
