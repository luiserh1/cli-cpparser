#include "cli_cpparser/command/parse.hpp"

#include "cli_cpparser/command/usage.hpp"

#include <algorithm>
#include <cstddef>
#include <format>
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

// "Its options are --all, --piece N, --zone Z."
std::string options_text(const Command& command) {
    if (command.options.empty()) {
        return "It has no options.";
    }
    std::string text = "Its options are ";
    for (const Option& option : command.options) {
        text += std::format("{}{}", text.ends_with(' ') ? "" : ", ", spelled(option));
    }
    return text + ".";
}

// An option with its value, and how many words it took: two when the value is the next word.
struct Taken {
    Given option;
    std::size_t words{1};
};

// The option that `words` starts with.
std::expected<Taken, std::string> option_at(std::span<const std::string> words,
                                            const Command& command, std::string_view called) {
    const std::string& word = words.front();
    const std::size_t equals = word.find('=');
    const bool has_equals = equals != std::string::npos;
    Taken taken{.option = Given{.name = word.substr(0, equals), .value = {}}};
    const std::string& name = taken.option.name;
    const auto declared = std::ranges::find(command.options, name, &Option::name);
    if (declared == command.options.end()) {
        return std::unexpected(
            std::format("'{}' has no option {}. {}", called, name, options_text(command)));
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

// The command as it is written with its required options: "alfar import --project DIR".
std::string with_required(const Command& command, std::string_view called) {
    std::string text{called};
    for (const Option& option : command.options) {
        if (option.required) {
            text += std::format(" {}", spelled(option));
        }
    }
    return text;
}

// Why the words that are no options are not what the command takes; empty when they are.
std::string words_refusal(std::span<const std::string> others, const Command& command,
                          std::string_view called) {
    const Words& takes = command.words;
    if (takes.count == Count::any) {
        return {};
    }
    if (takes.count == Count::none) {
        if (others.empty()) {
            return {};
        }
        return std::format("'{}' is not an option, and '{}' takes nothing else. {}", others.front(),
                           called, options_text(command));
    }
    if (others.empty()) {
        return std::format("'{}' needs its {}: write {} {}.", called, takes.name,
                           with_required(command, called), takes.name);
    }
    if (others.size() > 1) {
        return std::format("'{}' takes one {}, and {} were given: '{}' and '{}' are two of "
                           "them. Run it once for each.",
                           called, takes.name, others.size(), others[0], others[1]);
    }
    return {};
}

// Why the options given are not all the command needs; empty when they are.
std::string required_refusal(std::span<const Given> given, const Command& command,
                             std::string_view called) {
    for (const Option& option : command.options) {
        if (!option.required || std::ranges::contains(given, option.name, &Given::name)) {
            continue;
        }
        if (!option.missing.empty()) {
            return std::string{option.missing};
        }
        return std::format("{} is missing: '{}' needs it. Write {}.", option.name, called,
                           spelled(option));
    }
    return {};
}

} // namespace

std::expected<Arguments, std::string> parse(std::span<const std::string> words,
                                            const Command& command, std::string_view program) {
    Arguments arguments;
    if (std::ranges::any_of(words, asks_for_help)) {
        arguments.help = true;
        return arguments;
    }
    const std::string called = invocation(command, program);
    for (std::size_t at = 0; at < words.size();) {
        if (!is_option(words[at])) {
            arguments.words.push_back(words[at]);
            ++at;
            continue;
        }
        auto taken = option_at(words.subspan(at), command, called);
        if (!taken) {
            return std::unexpected(std::move(taken.error()));
        }
        const auto before = std::ranges::find(arguments.options, taken->option.name, &Given::name);
        if (before != arguments.options.end()) {
            return std::unexpected(twice_text(*before, taken->option));
        }
        at += taken->words;
        arguments.options.push_back(std::move(taken->option));
    }
    if (std::string refusal = words_refusal(arguments.words, command, called); !refusal.empty()) {
        return std::unexpected(std::move(refusal));
    }
    if (std::string refusal = required_refusal(arguments.options, command, called);
        !refusal.empty()) {
        return std::unexpected(std::move(refusal));
    }
    return arguments;
}

} // namespace cli_cpparser
