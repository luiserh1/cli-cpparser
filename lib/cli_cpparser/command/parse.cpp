#include "cli_cpparser/command/parse.hpp"

#include "cli_cpparser/command/internal/checks.hpp"
#include "cli_cpparser/command/usage.hpp"

#include <algorithm>
#include <cstddef>
#include <format>
#include <string_view>
#include <utility>
#include <vector>

namespace cli_cpparser {

namespace {

bool is_option(std::string_view word) {
    return word.starts_with("--") && word.size() > 2;
}

bool asks_for_help(const std::string& word) {
    return word == "-h" || word == "--help";
}

// An option with its value, and how many words it took: two when the value is the next word.
struct Taken {
    Given option;
    std::size_t words{1};
};

// The option that `words` starts with, by the table `options`. Nothing when the table has
// no option of that name.
std::expected<std::optional<Taken>, std::string> option_at(std::span<const std::string> words,
                                                           std::span<const Option> options) {
    const std::string& word = words.front();
    const std::size_t equals = word.find('=');
    const bool has_equals = equals != std::string::npos;
    Taken taken{.option = Given{.name = word.substr(0, equals), .value = {}}};
    const std::string& name = taken.option.name;
    const auto declared = std::ranges::find(options, name, &Option::name);
    if (declared == options.end()) {
        return std::nullopt;
    }
    if (declared->value.empty()) {
        if (has_equals) {
            return std::unexpected(std::format(
                "{} takes no value, and '{}' gives it one. Write {} alone.", name, word, name));
        }
        return taken;
    }
    if (has_equals) {
        taken.option.value = word.substr(equals + 1);
    } else if (words.size() > 1 && !is_option(words[1])) {
        taken.option.value = words[1];
        taken.words = 2;
    }
    if (taken.option.value.empty()) {
        return std::unexpected(std::format(
            "{} needs a value: write {}. (A value that starts with -- is written {}=VALUE.)", name,
            spelled(*declared), name));
    }
    return taken;
}

std::string twice_text(const Given& first, const Given& second) {
    if (first.value.empty()) {
        return std::format("{} is given twice. Give it once.", first.name);
    }
    return std::format("{} is given twice, with '{}' and with '{}'. Give it once.", first.name,
                       first.value, second.value);
}

} // namespace

std::expected<Arguments, std::string> parse(std::span<const std::string> words,
                                            const Command& command, const Program& program) {
    Arguments arguments;
    if (std::ranges::any_of(words, asks_for_help)) {
        arguments.help = true;
        return arguments;
    }
    const std::vector<Option> options = every_option(command, program);
    for (std::size_t at = 0; at < words.size();) {
        if (!is_option(words[at])) {
            arguments.words.push_back(words[at]);
            ++at;
            continue;
        }
        auto taken = option_at(words.subspan(at), options);
        if (!taken) {
            return std::unexpected(std::move(taken.error()));
        }
        if (!*taken) {
            const std::string_view word = words[at];
            return std::unexpected(std::format(
                "'{}' has no option {}. {}", invocation(command, program),
                word.substr(0, word.find('=')), internal::options_text(command, program)));
        }
        Given& given = (*taken)->option;
        const auto before = std::ranges::find(arguments.options, given.name, &Given::name);
        if (before != arguments.options.end()) {
            return std::unexpected(twice_text(*before, given));
        }
        at += (*taken)->words;
        arguments.options.push_back(std::move(given));
    }
    if (std::string refusal = internal::refusal(arguments, command, program); !refusal.empty()) {
        return std::unexpected(std::move(refusal));
    }
    return arguments;
}

} // namespace cli_cpparser
