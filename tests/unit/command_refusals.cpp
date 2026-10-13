#include "cli_cpparser/command/command.hpp"
#include "cli_cpparser/command/parse.hpp"
#include "command_tables.hpp"

#include <doctest/doctest.h>

#include <array>
#include <string>
#include <string_view>

// What is said when a command's words are wrong: each message whole, since the message is
// what the person at the command line gets.
namespace {

using namespace tables;

struct Row {
    std::string_view what;
    Words words;
    std::string_view says;
};

constexpr std::string_view piece_needs =
    "--piece needs a value: write --piece N. (A value that starts with -- is written "
    "--piece=VALUE.)";

} // namespace

TEST_CASE("an option given twice is refused with what it was given") {
    const std::array rows{
        Row{.what = "an option with a value",
            .words = Words{"--piece", "1", "--piece", "2"},
            .says = "--piece is given twice, with '1' and with '2'. Give it once."},
        Row{.what = "in both spellings",
            .words = Words{"--piece=1", "--all", "--piece", "1"},
            .says = "--piece is given twice, with '1' and with '1'. Give it once."},
        Row{.what = "a flag",
            .words = Words{"--all", "--zone", "Rim", "--all"},
            .says = "--all is given twice. Give it once."},
    };
    for (const Row& row : rows) {
        CAPTURE(row.what);
        CHECK(refusal(row.words) == row.says);
    }
}

TEST_CASE("an option without its value is refused with how to write it") {
    const std::array rows{
        Row{.what = "the last word", .words = Words{"--all", "--piece"}, .says = piece_needs},
        Row{.what = "before an option", .words = Words{"--piece", "--all"}, .says = piece_needs},
        Row{.what = "an empty value", .words = Words{"--piece", ""}, .says = piece_needs},
        Row{.what = "nothing after the equals sign",
            .words = Words{"--piece="},
            .says = piece_needs},
        Row{.what = "a value that starts with two dashes",
            .words = Words{"--zone", "--odd"},
            .says = "--zone needs a value: write --zone Z. (A value that starts with -- is "
                    "written --zone=VALUE.)"},
    };
    for (const Row& row : rows) {
        CAPTURE(row.what);
        CHECK(refusal(row.words) == row.says);
    }
}

TEST_CASE("a flag given a value is refused") {
    CHECK(refusal(Words{"--all=yes"}) ==
          "--all takes no value, and '--all=yes' gives it one. Write --all alone.");
    CHECK(refusal(Words{"--all="}) ==
          "--all takes no value, and '--all=' gives it one. Write --all alone.");
}

TEST_CASE("an option the command has not is refused with the ones it has") {
    CHECK(refusal(Words{"--fast"}) == "'alfar build' has no option --fast. Its options are "
                                      "--all, --piece N, --zone Z.");
    CHECK(refusal(Words{"--fast=1"}) == "'alfar build' has no option --fast. Its options are "
                                        "--all, --piece N, --zone Z.");
    // Names are whole and exact: no shortening, no other case.
    CHECK(refusal(Words{"--pie", "1"}) == "'alfar build' has no option --pie. Its options are "
                                          "--all, --piece N, --zone Z.");
    CHECK(refusal(Words{"--All"}) == "'alfar build' has no option --All. Its options are "
                                     "--all, --piece N, --zone Z.");
    CHECK(refusal(Words{"--fast"}, status) ==
          "'alfar status' has no option --fast. It has no options.");
}

TEST_CASE("a word is refused by a command that takes none") {
    CHECK(refusal(Words{"--all", "x.png", "y.png"}) ==
          "'x.png' is not an option, and 'alfar build' takes nothing else. Its options are "
          "--all, --piece N, --zone Z.");
    CHECK(refusal(Words{"-v"}, status) ==
          "'-v' is not an option, and 'alfar status' takes nothing else. It has no options.");
}

TEST_CASE("a command that takes one word is refused without it and with two") {
    CHECK(refusal(Words{"--project", "p"}, import) ==
          "'alfar import' needs its FILE: write alfar import --project DIR FILE.");
    CHECK(refusal(Words{"a", "--project", "p", "b", "c"}, import) ==
          "'alfar import' takes one FILE, and 3 were given: 'a' and 'b' are two of them. Run "
          "it once for each.");
}

TEST_CASE("a required option that is missing is refused in the default words or its own") {
    CHECK(refusal(Words{"--project", "p"}, exporting) ==
          "--out is missing: 'alfar export' needs it. Write --out DIR.");
    CHECK(refusal(Words{"--out=o", "--allow-blank"}, exporting) ==
          "--project DIR is missing: give the project's folder.");
    // Both missing: the first of the table is the one named.
    CHECK(refusal(Words{}, exporting) ==
          "--out is missing: 'alfar export' needs it. Write --out DIR.");
    CHECK(refusal(Words{"--out", "o", "--project", "p"}, exporting) == "");
}

TEST_CASE("the first mistake among the words is the one reported") {
    // A wrong option before a missing word and a missing option; then the word before the
    // option.
    CHECK(refusal(Words{"--fast"}, import) ==
          "'alfar import' has no option --fast. Its options are --project DIR, --replace.");
    CHECK(refusal(Words{}, import) ==
          "'alfar import' needs its FILE: write alfar import --project DIR FILE.");
    CHECK(refusal(Words{"a.png"}, import) ==
          "--project is missing: 'alfar import' needs it. Write --project DIR.");
}

TEST_CASE("a program with no commands is named alone in the messages") {
    static constexpr std::array options{Option{.name = "--port", .value = "N", .required = true}};
    static constexpr Command serve{.options = options, .words = cli_cpparser::one_word("DIR")};
    static constexpr Program server{.name = "serve"};
    const std::array rows{
        Row{.what = "an option it has not",
            .words = Words{"--fast"},
            .says = "'serve' has no option --fast. Its options are --port N."},
        Row{.what = "no word",
            .words = Words{},
            .says = "'serve' needs its DIR: write serve --port N DIR."},
        Row{.what = "no port",
            .words = Words{"site"},
            .says = "--port is missing: 'serve' needs it. Write --port N."},
    };
    for (const Row& row : rows) {
        CAPTURE(row.what);
        CHECK(refusal(row.words, serve, server) == row.says);
    }
}
