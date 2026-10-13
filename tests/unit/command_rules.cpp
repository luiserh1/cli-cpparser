#include "cli_cpparser/command/command.hpp"
#include "cli_cpparser/command/parse.hpp"
#include "command_tables.hpp"

#include <doctest/doctest.h>

#include <array>
#include <string>
#include <string_view>

// What the table says beyond its options one by one: how many words a command takes, the
// options a program shares, and the rules between options.
namespace {

using namespace tables;
using cli_cpparser::Choice;
using cli_cpparser::Needs;

struct Row {
    std::string_view what;
    Words words;
    std::string_view says;
};

constexpr std::array zones_options{
    Option{.name = "--auto"},
    Option{.name = "--check"},
    Option{.name = "--reset"},
    Option{.name = "--pieces", .value = "N,N.."},
    Option{.name = "--review", .value = "DIR"},
    Option{.name = "--today", .value = "DIR"},
    Option{.name = "--out", .value = "DIR", .required = true},
};
constexpr std::array zones_choices{Choice{.among = {"--auto", "--check"}, .required = true}};
constexpr std::array zones_needs{
    Needs{.option = "--reset", .needs = "--auto"},
    Needs{.option = "--pieces", .needs = "--auto"},
    Needs{.option = "--review", .needs = "--today"},
    Needs{.option = "--today", .needs = "--review"},
};
constexpr Command zones{
    .name = "zones", .options = zones_options, .choices = zones_choices, .needs = zones_needs};

void check_rows(std::span<const Row> rows, const Command& command, const Program& of = program) {
    for (const Row& row : rows) {
        CAPTURE(row.what);
        CHECK(refusal(row.words, command, of) == row.says);
    }
}

} // namespace

TEST_CASE("a command that takes two or three words is refused with any other number") {
    static constexpr Command diff{.name = "diff", .words = {.least = 2, .most = 3, .name = "DIR"}};
    const std::array rows{
        Row{.what = "none",
            .words = Words{},
            .says = "'alfar diff' takes 2 or 3 DIR, and none was given."},
        Row{.what = "one",
            .words = Words{"a"},
            .says = "'alfar diff' takes 2 or 3 DIR, and 1 was given."},
        Row{.what = "two", .words = Words{"a", "b"}, .says = ""},
        Row{.what = "three", .words = Words{"a", "b", "c"}, .says = ""},
        Row{.what = "four",
            .words = Words{"a", "b", "c", "d"},
            .says = "'alfar diff' takes 2 or 3 DIR, and 4 were given."},
    };
    check_rows(rows, diff);
}

TEST_CASE("the number of words a command takes is said as its table has it") {
    struct Counted {
        cli_cpparser::Words takes;
        Words words;
        std::string_view says;
    };
    const std::array rows{
        Counted{.takes = {.least = 2, .most = 2, .name = "DIR"},
                .words = Words{"a"},
                .says = "'alfar go' takes 2 DIR, and 1 was given."},
        Counted{.takes = {.least = 2, .most = 5, .name = "DIR"},
                .words = Words{"a"},
                .says = "'alfar go' takes 2 to 5 DIR, and 1 was given."},
        Counted{.takes = {.least = 1, .most = cli_cpparser::any_number, .name = "FILE"},
                .words = Words{},
                .says = "'alfar go' takes 1 or more FILE, and none was given."},
        Counted{.takes = {.least = 0, .most = 1, .name = "FILE"},
                .words = Words{"a", "b"},
                .says = "'alfar go' takes at most 1 FILE, and 2 were given."},
        Counted{.takes = {.least = 0, .most = 1, .name = "FILE"}, .words = Words{}, .says = ""},
        Counted{.takes = {.least = 1, .most = cli_cpparser::any_number, .name = "FILE"},
                .words = Words{"a", "b", "c", "d", "e", "f"},
                .says = ""},
    };
    for (const Counted& row : rows) {
        CAPTURE(row.says);
        const Command go{.name = "go", .words = row.takes};
        CHECK(refusal(row.words, go) == row.says);
    }
}

TEST_CASE("a choice that must be made is refused with none of its options and with two") {
    const std::array rows{
        Row{.what = "one", .words = Words{"--auto", "--out", "o"}, .says = ""},
        Row{.what = "the other", .words = Words{"--out=o", "--check"}, .says = ""},
        Row{.what = "both",
            .words = Words{"--check", "--out", "o", "--auto"},
            .says = "--auto and --check are both given. Give one of them."},
        Row{.what = "neither",
            .words = Words{"--out", "o"},
            .says = "Neither --auto nor --check is given. Give one of them."},
    };
    check_rows(rows, zones);
}

TEST_CASE("a choice among three names them all when none is given") {
    static constexpr std::array options{Option{.name = "--generate"},
                                        Option{.name = "--from-texture"},
                                        Option{.name = "--composite"}};
    static constexpr std::array choices{
        Choice{.among = {"--generate", "--from-texture", "--composite"}, .required = true}};
    static constexpr Command normal_map{
        .name = "normal-map", .options = options, .choices = choices};
    const std::array rows{
        Row{.what = "none",
            .words = Words{},
            .says = "None of --generate, --from-texture, --composite is given. Give one of "
                    "them."},
        Row{.what = "the last two",
            .words = Words{"--composite", "--from-texture"},
            .says = "--from-texture and --composite are both given. Give one of them."},
        Row{.what = "one", .words = Words{"--composite"}, .says = ""},
    };
    check_rows(rows, normal_map);
}

TEST_CASE("a choice that need not be made is refused only with two of its options") {
    static constexpr std::array options{Option{.name = "--suffix", .value = "S"},
                                        Option{.name = "--no-suffix"}};
    static constexpr std::array choices{Choice{.among = {"--suffix", "--no-suffix"}}};
    static constexpr Command match{.name = "match", .options = options, .choices = choices};
    const std::array rows{
        Row{.what = "none", .words = Words{}, .says = ""},
        Row{.what = "one", .words = Words{"--no-suffix"}, .says = ""},
        Row{.what = "both",
            .words = Words{"--no-suffix", "--suffix=_x"},
            .says = "--suffix and --no-suffix are both given. Give one of them."},
    };
    check_rows(rows, match);
}

TEST_CASE("an option without the one it goes with is refused with how to write both") {
    const std::array rows{
        Row{.what = "a flag that goes with a flag",
            .words = Words{"--check", "--out=o", "--reset"},
            .says = "--reset goes with --auto: give --auto too, or leave --reset out."},
        Row{.what = "an option that goes with one that takes a value",
            .words = Words{"--auto", "--out=o", "--review", "r"},
            .says = "--review goes with --today: give --today DIR too, or leave --review out."},
        Row{.what = "the other way round",
            .words = Words{"--auto", "--out=o", "--today", "t"},
            .says = "--today goes with --review: give --review DIR too, or leave --today out."},
        Row{.what = "both together",
            .words = Words{"--auto", "--out=o", "--today", "t", "--review=r", "--reset", "--pieces",
                           "1,2"},
            .says = ""},
        Row{.what = "neither", .words = Words{"--auto", "--out=o"}, .says = ""},
    };
    check_rows(rows, zones);
}

TEST_CASE("required options are checked before choices and choices before what goes together") {
    const std::array rows{
        Row{.what = "all three wrong",
            .words = Words{"--reset"},
            .says = "--out is missing: 'alfar zones' needs it. Write --out DIR."},
        Row{.what = "a choice and a pair wrong",
            .words = Words{"--reset", "--out", "o"},
            .says = "Neither --auto nor --check is given. Give one of them."},
        Row{.what = "a word too many before them all",
            .words = Words{"--reset", "stray"},
            .says = "'stray' is not an option, and 'alfar zones' takes nothing else. Its options "
                    "are --auto, --check, --reset, --pieces N,N.., --review DIR, --today DIR, "
                    "--out DIR."},
    };
    check_rows(rows, zones);
}

TEST_CASE("the options a program shares are named after a command's own") {
    const std::array rows{
        Row{.what = "an option neither has",
            .words = Words{"--fast"},
            .says = "'alfar build' has no option --fast. Its options are --all, --piece N, "
                    "--zone Z, --project DIR, --work DIR."},
        Row{.what = "a shared option twice",
            .words = Words{"--project", "p", "--project=q"},
            .says = "--project is given twice, with 'p' and with 'q'. Give it once."},
        Row{.what = "a shared option without its value",
            .words = Words{"--project"},
            .says = "--project needs a value: write --project DIR. (A value that starts with "
                    "-- is written --project=VALUE.)"},
    };
    check_rows(rows, build, with_folders);
}

TEST_CASE("a shared option that is required and missing is reported before a command's own") {
    static constexpr std::array options{Option{.name = "--out", .value = "DIR", .required = true}};
    static constexpr Command exports{.name = "export", .options = options};
    const std::array rows{
        Row{.what = "both missing",
            .words = Words{},
            .says = "--project is missing: 'alfar export' needs it. Write --project DIR."},
        Row{.what = "the command's own missing",
            .words = Words{"--project", "p"},
            .says = "--out is missing: 'alfar export' needs it. Write --out DIR."},
        Row{.what = "neither missing", .words = Words{"--project", "p", "--out=o"}, .says = ""},
    };
    check_rows(rows, exports, with_folders);
}

TEST_CASE("a missing word is asked for with the required options on both sides of it") {
    static constexpr std::array options{Option{.name = "--as", .value = "NAME", .required = true}};
    static constexpr Command imports{
        .name = "import", .options = options, .words = cli_cpparser::one_word("FILE")};
    CHECK(refusal(Words{"--as", "x", "--project", "p"}, imports, with_folders) ==
          "'alfar import' needs its FILE: write alfar import --as NAME FILE --project DIR.");
}
