#include "app/commands.hpp"

#include "cli_cpparser/program/program.hpp"
#include "cli_cpparser/values/readers.hpp"

#include <array>
#include <cstddef>
#include <format>
#include <ostream>
#include <print>
#include <string_view>
#include <vector>

namespace app {

namespace {

using cli_cpparser::Arguments;
using cli_cpparser::exit_failure;
using cli_cpparser::exit_ok;

// Prints a command's result after what --prefix gave, the option every command shares.
void print_result(const Arguments& arguments, std::string_view result, std::ostream& out) {
    std::println(out, "{}{}", cli_cpparser::value_or(arguments, "--prefix", ""), result);
}

} // namespace

std::expected<int, std::string> run_repeat(const Arguments& arguments, std::ostream& out,
                                           std::ostream& /*err*/) {
    // --times is required and the command takes one word: the parser has seen to both.
    const auto times = cli_cpparser::whole_number(*cli_cpparser::value_of(arguments, "--times"),
                                                  "--times", {.least = 1, .most = 100});
    if (!times) {
        return std::unexpected(times.error());
    }
    const std::string separator = cli_cpparser::value_or(arguments, "--separator", " ");
    std::string result;
    for (int count = 0; count < *times; ++count) {
        result += std::format("{}{}", count == 0 ? "" : separator, arguments.words.front());
    }
    print_result(arguments, result, out);
    return exit_ok;
}

std::expected<int, std::string> run_sum(const Arguments& arguments, std::ostream& out,
                                        std::ostream& err) {
    constexpr std::array<std::string_view, 2> shown{"total", "mean"};
    const auto show =
        cli_cpparser::one_of(cli_cpparser::value_or(arguments, "--show", "total"), "--show", shown);
    if (!show) {
        return std::unexpected(show.error());
    }
    const auto scale = cli_cpparser::decimal_number(
        cli_cpparser::value_or(arguments, "--scale", "1"), "--scale", {.least = 0, .most = 1000});
    if (!scale) {
        return std::unexpected(scale.error());
    }
    double total = 0;
    for (const std::string& word : arguments.words) {
        const auto number = cli_cpparser::decimal_number(word, "NUMBER");
        if (!number) {
            return std::unexpected(number.error());
        }
        total += *number;
    }
    if (shown.at(*show) == "total") {
        print_result(arguments, std::format("{}", total * *scale), out);
        return exit_ok;
    }
    if (arguments.words.empty()) {
        std::println(err, "error: there is no mean of no numbers. Give one NUMBER or more.");
        return exit_failure;
    }
    const auto count = static_cast<double>(arguments.words.size());
    print_result(arguments, std::format("{}", total * *scale / count), out);
    return exit_ok;
}

std::expected<int, std::string> run_pick(const Arguments& arguments, std::ostream& out,
                                         std::ostream& err) {
    const std::string* const at = cli_cpparser::value_of(arguments, "--at");
    const bool last = cli_cpparser::has(arguments, "--last");
    // A rule between two options is the command's own: the table can't say it.
    if ((at != nullptr) == last) {
        return std::unexpected(last ? "--at and --last are both given. Give one of them."
                                    : "Neither --at nor --last is given. Write --at N for the "
                                      "Nth part, or --last for the last one.");
    }
    const auto parts = cli_cpparser::list(*cli_cpparser::value_of(arguments, "--from"), "--from");
    if (!parts) {
        return std::unexpected(parts.error());
    }
    if (last) {
        print_result(arguments, parts->back(), out);
        return exit_ok;
    }
    const auto place = cli_cpparser::whole_number(*at, "--at", {.least = 1});
    if (!place) {
        return std::unexpected(place.error());
    }
    const auto index = static_cast<std::size_t>(*place) - 1;
    if (index >= parts->size()) {
        std::println(err, "error: --at {} is past the end: --from has {} parts.", *place,
                     parts->size());
        return exit_failure;
    }
    print_result(arguments, parts->at(index), out);
    return exit_ok;
}

} // namespace app
