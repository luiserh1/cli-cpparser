#include "app/run.hpp"

#include "app/commands.hpp"
#include "cli_cpparser/command/command.hpp"
#include "cli_cpparser/program/program.hpp"

#include <array>
#include <string_view>

namespace app {

namespace {

using cli_cpparser::Choice;
using cli_cpparser::Command;
using cli_cpparser::Form;
using cli_cpparser::Needs;
using cli_cpparser::Option;

// The project's version, from project() in CMakeLists.txt.
constexpr std::string_view version = APP_VERSION;

// The option every command takes: it is given to the program once (Program::shared).
constexpr std::array shared_options{
    Option{.name = "--prefix", .value = "TEXT", .help = "Text to print before the result"},
};

constexpr std::array repeat_options{
    Option{
        .name = "--times", .value = "N", .help = "How many times, from 1 to 100", .required = true},
    Option{.name = "--separator",
           .value = "TEXT",
           .help = "What goes between them (default: a space)"},
    Option{.name = "--final",
           .value = "TEXT",
           .help = "What goes before the last one, in place of the separator"},
};
// A rule between two options, which the parser checks.
constexpr std::array repeat_needs{Needs{.option = "--final", .needs = "--separator"}};

constexpr std::array sum_options{
    Option{.name = "--scale",
           .value = "X",
           .help = "Multiply the result by X, from 0 to 1000 (default: 1)"},
    Option{.name = "--show", .value = "total|mean", .help = "What to print (default: total)"},
};

constexpr std::array pick_options{
    Option{.name = "--from",
           .value = "A,B..",
           .help = "The list, with commas between its parts",
           .required = true,
           .missing = "--from is missing: give the list to pick from, as in --from "
                      "red,green,blue."},
    Option{.name = "--at", .value = "N", .help = "Pick the Nth part, counting from 1"},
    Option{.name = "--last", .help = "Pick the last part"},
};
// pick has two forms, one for each option of its choice. They are written by hand, without
// the shared option, which the library writes after each.
constexpr std::array pick_forms{
    Form{.usage = "cli_cpparser pick --from A,B.. --at N",
         .listed = "pick --from A,B.. --at N",
         .summary = "Print the Nth part of a list"},
    Form{.usage = "cli_cpparser pick --from A,B.. --last",
         .listed = "pick --from A,B.. --last",
         .summary = "Print the last part of a list"},
};
constexpr std::array pick_choices{Choice{.among = {"--at", "--last"}, .required = true}};

constexpr std::array commands{
    Command{
        .name = "repeat",
        .summary = "Print a word several times",
        .listed = "repeat WORD --times N",
        .options = repeat_options,
        .words = cli_cpparser::one_word("WORD"),
        .needs = repeat_needs,
        .run = run_repeat,
    },
    Command{
        .name = "sum",
        .summary = "Add numbers up",
        .options = sum_options,
        .words = cli_cpparser::any_words("NUMBER"),
        .details = "Prints the total of the numbers given, or their mean. A number may be "
                   "negative\nor have decimals: sum 1 -2.5 .25",
        .run = run_sum,
    },
    Command{
        .name = "pick",
        .options = pick_options,
        .forms = pick_forms,
        .choices = pick_choices,
        .run = run_pick,
    },
};

constexpr cli_cpparser::Program program{
    .name = "cli_cpparser",
    .version = version,
    .summary = "an example of the cli-cpparser library",
    .commands = commands,
    .shared = shared_options,
    .notes = "A command exits with 0 when it did its work, with 1 when it could not, and "
             "with 2\nwhen the command line is wrong.",
};

} // namespace

int run(std::span<const std::string> words, std::ostream& out, std::ostream& err) {
    return cli_cpparser::run(program, words, out, err);
}

} // namespace app
