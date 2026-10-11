#include "app/run.hpp"

#include "app/commands.hpp"
#include "cli_cpparser/command/command.hpp"
#include "cli_cpparser/program/program.hpp"

#include <array>
#include <string_view>

namespace app {

namespace {

using cli_cpparser::Command;
using cli_cpparser::Count;
using cli_cpparser::Form;
using cli_cpparser::Option;

// The project's version, from project() in CMakeLists.txt.
constexpr std::string_view version = APP_VERSION;

// The options every command takes: an ordinary list, joined to each command's own.
constexpr std::array shared_options{
    Option{.name = "--prefix", .value = "TEXT", .help = "Text to print before the result"},
};

constexpr std::array repeat_own{
    Option{
        .name = "--times", .value = "N", .help = "How many times, from 1 to 100", .required = true},
    Option{.name = "--separator",
           .value = "TEXT",
           .help = "What goes between them (default: a space)"},
};
constexpr auto repeat_options = cli_cpparser::joined(repeat_own, shared_options);

constexpr std::array sum_own{
    Option{.name = "--scale",
           .value = "X",
           .help = "Multiply the result by X, from 0 to 1000 (default: 1)"},
    Option{.name = "--show", .value = "total|mean", .help = "What to print (default: total)"},
};
constexpr auto sum_options = cli_cpparser::joined(sum_own, shared_options);

constexpr std::array pick_own{
    Option{.name = "--from",
           .value = "A,B..",
           .help = "The list, with commas between its parts",
           .required = true,
           .missing = "--from is missing: give the list to pick from, as in "
                      "--from red,green,blue."},
    Option{.name = "--at", .value = "N", .help = "Pick the Nth part, counting from 1"},
    Option{.name = "--last", .help = "Pick the last part"},
};
constexpr auto pick_options = cli_cpparser::joined(pick_own, shared_options);

// pick has two forms, which a table can't say: they are written by hand.
constexpr std::array pick_forms{
    Form{.usage = "cli_cpparser pick --from A,B.. --at N [--prefix TEXT]",
         .listed = "pick --from A,B.. --at N",
         .summary = "Print the Nth part of a list"},
    Form{.usage = "cli_cpparser pick --from A,B.. --last [--prefix TEXT]",
         .listed = "pick --from A,B.. --last",
         .summary = "Print the last part of a list"},
};

constexpr std::array commands{
    Command{
        .name = "repeat",
        .summary = "Print a word several times",
        .options = repeat_options,
        .words = {.count = Count::one, .name = "WORD"},
        .run = run_repeat,
    },
    Command{
        .name = "sum",
        .summary = "Add numbers up",
        .options = sum_options,
        .words = {.count = Count::any, .name = "NUMBER"},
        .details = "Prints the total of the numbers given, or their mean. A number may be "
                   "negative\nor have decimals: sum 1 -2.5 .25",
        .run = run_sum,
    },
    Command{
        .name = "pick",
        .summary = "Print one part of a list",
        .options = pick_options,
        .forms = pick_forms,
        .run = run_pick,
    },
};

constexpr cli_cpparser::Program program{
    .name = "cli_cpparser",
    .version = version,
    .summary = "an example of the cli-cpparser library",
    .commands = commands,
    .notes = "Every command takes --prefix TEXT. It exits with 0 when it did its work, with 1 "
             "when it\ncould not, and with 2 when the command line is wrong.",
};

} // namespace

int run(std::span<const std::string> words, std::ostream& out, std::ostream& err) {
    return cli_cpparser::run(program, words, out, err);
}

} // namespace app
