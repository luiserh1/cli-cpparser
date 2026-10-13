#include "cli_cpparser/command/internal/checks.hpp"

#include "cli_cpparser/command/usage.hpp"

#include <algorithm>
#include <cstddef>
#include <format>
#include <span>
#include <string_view>
#include <vector>

namespace cli_cpparser::internal {

namespace {

// The required options among these, each after a space: " --project DIR".
std::string required_part(std::span<const Option> options) {
    std::string text;
    for (const Option& option : options) {
        if (option.required) {
            text += std::format(" {}", spelled(option));
        }
    }
    return text;
}

// How many words the command takes, in words: "2", "2 or 3", "2 to 5", "2 or more", "at
// most 3".
std::string count_text(const Words& takes) {
    if (takes.most == any_number) {
        return std::format("{} or more", takes.least);
    }
    if (takes.least == takes.most) {
        return std::format("{}", takes.least);
    }
    if (takes.least == 0) {
        return std::format("at most {}", takes.most);
    }
    if (takes.most == takes.least + 1) {
        return std::format("{} or {}", takes.least, takes.most);
    }
    return std::format("{} to {}", takes.least, takes.most);
}

// Why the words that are no options are not what the command takes; empty when they are.
std::string words_refusal(std::span<const std::string> given, const Command& command,
                          const Program& program) {
    const Words& takes = command.words;
    if (given.size() >= takes.least && given.size() <= takes.most) {
        return {};
    }
    const std::string called = invocation(command, program);
    if (takes.most == 0) {
        return std::format("'{}' is not an option, and '{}' takes nothing else. {}", given.front(),
                           called, options_text(command, program));
    }
    if (takes.least == 1 && takes.most == 1 && given.empty()) {
        return std::format("'{}' needs its {}: write {}{} {}{}.", called, takes.name, called,
                           required_part(command.options), takes.name,
                           required_part(program.shared));
    }
    if (takes.least == 1 && takes.most == 1) {
        return std::format("'{}' takes one {}, and {} were given: '{}' and '{}' are two of "
                           "them. Run it once for each.",
                           called, takes.name, given.size(), given[0], given[1]);
    }
    if (given.empty()) {
        return std::format("'{}' takes {} {}, and none was given.", called, count_text(takes),
                           takes.name);
    }
    return std::format("'{}' takes {} {}, and {} {} given.", called, count_text(takes), takes.name,
                       given.size(), given.size() == 1 ? "was" : "were");
}

// Why the options given are not all that these need; empty when they are.
std::string required_refusal(const Arguments& arguments, std::span<const Option> options,
                             std::string_view called) {
    for (const Option& option : options) {
        if (!option.required || has(arguments, option.name)) {
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

// "--generate, --from-texture, --composite"
std::string names_text(std::span<const std::string_view> names) {
    std::string text;
    for (const std::string_view name : names) {
        text += std::format("{}{}", text.empty() ? "" : ", ", name);
    }
    return text;
}

// Why a choice is not made as it must be; empty when it is.
std::string choice_refusal(const Arguments& arguments, const Choice& choice) {
    std::vector<std::string_view> among;
    std::vector<std::string_view> given;
    for (const std::string_view name : choice.among) {
        if (name.empty()) {
            continue;
        }
        among.push_back(name);
        if (has(arguments, name)) {
            given.push_back(name);
        }
    }
    if (given.size() > 1) {
        return std::format("{} and {} are both given. Give one of them.", given[0], given[1]);
    }
    if (!given.empty() || !choice.required) {
        return {};
    }
    if (among.size() == 2) {
        return std::format("Neither {} nor {} is given. Give one of them.", among[0], among[1]);
    }
    return std::format("None of {} is given. Give one of them.", names_text(among));
}

// Why an option is given without the one it goes with; empty when it is not.
std::string needs_refusal(const Arguments& arguments, const Needs& rule,
                          std::span<const Option> options) {
    if (!has(arguments, rule.option) || has(arguments, rule.needs)) {
        return {};
    }
    const auto needed = std::ranges::find(options, rule.needs, &Option::name);
    const std::string written =
        needed == options.end() ? std::string{rule.needs} : spelled(*needed);
    return std::format("{} goes with {}: give {} too, or leave {} out.", rule.option, rule.needs,
                       written, rule.option);
}

} // namespace

std::string options_text(const Command& command, const Program& program) {
    const std::vector<Option> options = every_option(command, program);
    if (options.empty()) {
        return "It has no options.";
    }
    std::string text = "Its options are ";
    for (const Option& option : options) {
        text += std::format("{}{}", text.ends_with(' ') ? "" : ", ", spelled(option));
    }
    return text + ".";
}

std::string refusal(const Arguments& arguments, const Command& command, const Program& program) {
    const std::string called = invocation(command, program);
    if (std::string words = words_refusal(arguments.words, command, program); !words.empty()) {
        return words;
    }
    for (const std::span<const Option> options : {program.shared, command.options}) {
        if (std::string missing = required_refusal(arguments, options, called); !missing.empty()) {
            return missing;
        }
    }
    for (const Choice& choice : command.choices) {
        if (std::string wrong = choice_refusal(arguments, choice); !wrong.empty()) {
            return wrong;
        }
    }
    const std::vector<Option> options = every_option(command, program);
    for (const Needs& rule : command.needs) {
        if (std::string alone = needs_refusal(arguments, rule, options); !alone.empty()) {
            return alone;
        }
    }
    return {};
}

} // namespace cli_cpparser::internal
