#include "app/run.hpp"

#include <doctest/doctest.h>

#include <array>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// The example program end to end: a command line in, and the text on each stream and the
// exit code out. It is the library as a user has it.
namespace {

using Words = std::vector<std::string>;

struct Ran {
    int code{};
    std::string out;
    std::string err;
};

Ran ran(const Words& words) {
    std::ostringstream out;
    std::ostringstream err;
    const int code = app::run(words, out, err);
    return Ran{.code = code, .out = out.str(), .err = err.str()};
}

struct Row {
    Words words;
    int code;
    std::string_view out;
    std::string_view err;
};

void check_rows(std::span<const Row> rows) {
    for (const Row& row : rows) {
        std::string line;
        for (const std::string& word : row.words) {
            line += word + " ";
        }
        CAPTURE(line);
        const Ran result = ran(row.words);
        CHECK(result.code == row.code);
        CHECK(result.out == row.out);
        CHECK(result.err == row.err);
    }
}

constexpr std::string_view repeat_usage =
    "usage: cli_cpparser repeat --times N [--separator TEXT] [--prefix TEXT] WORD\n";
constexpr std::string_view sum_usage =
    "usage: cli_cpparser sum [--scale X] [--show total|mean] [--prefix TEXT] [NUMBER..]\n";
constexpr std::string_view pick_usage =
    "usage: cli_cpparser pick --from A,B.. --at N [--prefix TEXT]\n"
    "       cli_cpparser pick --from A,B.. --last [--prefix TEXT]\n";

std::string with(std::string_view message, std::string_view usage) {
    return std::string{message} + std::string{usage};
}

} // namespace

TEST_CASE("repeat prints its word the times asked") {
    const std::array rows{
        Row{.words = {"repeat", "ha", "--times", "3"}, .code = 0, .out = "ha ha ha\n", .err = ""},
        Row{.words = {"repeat", "--times=1", "ha"}, .code = 0, .out = "ha\n", .err = ""},
        Row{.words = {"repeat", "--separator=-", "ha", "--times", "2", "--prefix", "> "},
            .code = 0,
            .out = "> ha-ha\n",
            .err = ""},
    };
    check_rows(rows);
}

TEST_CASE("repeat with a wrong command line says what to write and exits with 2") {
    const std::string no_times =
        with("error: --times is missing: 'cli_cpparser repeat' needs it. Write --times N.\n",
             repeat_usage);
    const std::string no_word = with("error: 'cli_cpparser repeat' needs its WORD: write "
                                     "cli_cpparser repeat --times N WORD.\n",
                                     repeat_usage);
    const std::string too_many = with(
        "error: --times takes a whole number from 1 to 100, and '101' is not one.\n", repeat_usage);
    const std::string empty_separator =
        with("error: --separator needs a value: write --separator TEXT. (A value that starts "
             "with -- is written --separator=VALUE.)\n",
             repeat_usage);
    const std::array rows{
        Row{.words = {"repeat", "ha"}, .code = 2, .out = "", .err = no_times},
        Row{.words = {"repeat", "--times", "3"}, .code = 2, .out = "", .err = no_word},
        Row{.words = {"repeat", "ha", "--times", "101"}, .code = 2, .out = "", .err = too_many},
        Row{.words = {"repeat", "ha", "--times", "2", "--separator", ""},
            .code = 2,
            .out = "",
            .err = empty_separator},
    };
    check_rows(rows);
}

TEST_CASE("sum adds its numbers negative ones and decimals too") {
    const std::array rows{
        Row{.words = {"sum", "1", "-2.5", ".25"}, .code = 0, .out = "-1.25\n", .err = ""},
        Row{.words = {"sum"}, .code = 0, .out = "0\n", .err = ""},
        Row{.words = {"sum", "1", "2", "--scale", "2.5"}, .code = 0, .out = "7.5\n", .err = ""},
        Row{.words = {"sum", "--show=mean", "1", "2", "6"}, .code = 0, .out = "3\n", .err = ""},
        Row{.words = {"sum", "--prefix", "total: ", "4"},
            .code = 0,
            .out = "total: 4\n",
            .err = ""},
    };
    check_rows(rows);
}

TEST_CASE("sum refuses what is no number scale or thing to show and exits with 2") {
    const std::string not_a_number =
        with("error: NUMBER takes a number, and 'two' is not one.\n", sum_usage);
    const std::string scale =
        with("error: --scale takes a number from 0 to 1000, and '-1' is not one.\n", sum_usage);
    const std::string show =
        with("error: --show takes one of total, mean, and 'median' is none of them.\n", sum_usage);
    const std::array rows{
        Row{.words = {"sum", "1", "two"}, .code = 2, .out = "", .err = not_a_number},
        Row{.words = {"sum", "1", "--scale=-1"}, .code = 2, .out = "", .err = scale},
        Row{.words = {"sum", "1", "--show", "median"}, .code = 2, .out = "", .err = show},
    };
    check_rows(rows);
}

TEST_CASE("a command that cannot do its work says why and exits with 1") {
    const std::array rows{
        Row{.words = {"sum", "--show", "mean"},
            .code = 1,
            .out = "",
            .err = "error: there is no mean of no numbers. Give one NUMBER or more.\n"},
        Row{.words = {"pick", "--from", "a,b,c", "--at", "4"},
            .code = 1,
            .out = "",
            .err = "error: --at 4 is past the end: --from has 3 parts.\n"},
    };
    check_rows(rows);
}

TEST_CASE("pick prints the part asked for in either of its forms") {
    const std::array rows{
        Row{.words = {"pick", "--from", "red,green,blue", "--at", "2"},
            .code = 0,
            .out = "green\n",
            .err = ""},
        Row{.words = {"pick", "--last", "--from=red,green,blue"},
            .code = 0,
            .out = "blue\n",
            .err = ""},
        Row{.words = {"pick", "--from", "one", "--at=1", "--prefix=the "},
            .code = 0,
            .out = "the one\n",
            .err = ""},
    };
    check_rows(rows);
}

TEST_CASE("pick refuses what breaks its own rules with its two forms and exits with 2") {
    const std::string neither = with("error: Neither --at nor --last is given. Write --at N for "
                                     "the Nth part, or --last for the last one.\n",
                                     pick_usage);
    const std::string both =
        with("error: --at and --last are both given. Give one of them.\n", pick_usage);
    const std::string no_from = with("error: --from is missing: give the list to pick from, as "
                                     "in --from red,green,blue.\n",
                                     pick_usage);
    const std::string empty_part = with("error: --from takes a list with commas between its "
                                        "parts, and 'a,,c' has an empty part.\n",
                                        pick_usage);
    const std::string no_place =
        with("error: --at takes a whole number, 1 or more, and '0' is not one.\n", pick_usage);
    const std::array rows{
        Row{.words = {"pick", "--from", "a,b"}, .code = 2, .out = "", .err = neither},
        Row{.words = {"pick", "--from", "a,b", "--at", "1", "--last"},
            .code = 2,
            .out = "",
            .err = both},
        Row{.words = {"pick", "--last"}, .code = 2, .out = "", .err = no_from},
        Row{.words = {"pick", "--from", "a,,c", "--last"}, .code = 2, .out = "", .err = empty_part},
        Row{.words = {"pick", "--from", "a,b", "--at", "0"}, .code = 2, .out = "", .err = no_place},
    };
    check_rows(rows);
}

TEST_CASE("the program's help lists its commands and pick's two forms") {
    const Ran result = ran(Words{"help"});
    CHECK(result.code == 0);
    CHECK(result.err == "");
    // The first line has the version, which changes with each release.
    REQUIRE(result.out.starts_with("cli_cpparser "));
    const std::string after_first_line = result.out.substr(result.out.find('\n'));
    CHECK(after_first_line ==
          "\n"
          "\n"
          "Usage: cli_cpparser <command> [options]\n"
          "\n"
          "Commands:\n"
          "  repeat                    Print a word several times\n"
          "  sum                       Add numbers up\n"
          "  pick --from A,B.. --at N  Print the Nth part of a list\n"
          "  pick --from A,B.. --last  Print the last part of a list\n"
          "  help                      Show this list\n"
          "  --version                 Print the program's version\n"
          "\n"
          "Every command takes --prefix TEXT. It exits with 0 when it did its work, with 1 "
          "when it\n"
          "could not, and with 2 when the command line is wrong.\n"
          "\n"
          "Run 'cli_cpparser <command> --help' for a command's options.\n");
}

TEST_CASE("the program's version is one line with its name") {
    const Ran result = ran(Words{"--version"});
    CHECK(result.code == 0);
    CHECK(result.err == "");
    CHECK(result.out.starts_with("cli_cpparser "));
    CHECK(result.out.ends_with('\n'));
    CHECK(result.out.find('\n') == result.out.size() - 1);
}

TEST_CASE("a command's help has its usage what it does and its options") {
    const std::array rows{
        Row{.words = {"sum", "--help"},
            .code = 0,
            .out = "usage: cli_cpparser sum [--scale X] [--show total|mean] [--prefix TEXT] "
                   "[NUMBER..]\n"
                   "\n"
                   "Prints the total of the numbers given, or their mean. A number may be "
                   "negative\n"
                   "or have decimals: sum 1 -2.5 .25\n"
                   "\n"
                   "Options:\n"
                   "  --scale X          Multiply the result by X, from 0 to 1000 (default: 1)\n"
                   "  --show total|mean  What to print (default: total)\n"
                   "  --prefix TEXT      Text to print before the result\n"
                   "  -h, --help         Show this help\n",
            .err = ""},
        Row{.words = {"pick", "-h"},
            .code = 0,
            .out = "usage: cli_cpparser pick --from A,B.. --at N [--prefix TEXT]\n"
                   "       cli_cpparser pick --from A,B.. --last [--prefix TEXT]\n"
                   "\n"
                   "Print one part of a list\n"
                   "\n"
                   "Options:\n"
                   "  --from A,B..   The list, with commas between its parts\n"
                   "  --at N         Pick the Nth part, counting from 1\n"
                   "  --last         Pick the last part\n"
                   "  --prefix TEXT  Text to print before the result\n"
                   "  -h, --help     Show this help\n",
            .err = ""},
    };
    check_rows(rows);
}

TEST_CASE("a word that is no command is refused and exits with 2") {
    const std::array rows{
        Row{.words = {"multiply", "2", "3"},
            .code = 2,
            .out = "",
            .err = "error: unknown command 'multiply'. Run 'cli_cpparser help' for the list of "
                   "commands.\n"},
    };
    check_rows(rows);
}
