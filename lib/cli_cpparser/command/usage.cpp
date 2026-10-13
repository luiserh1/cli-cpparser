#include "cli_cpparser/command/usage.hpp"

#include <cstddef>
#include <format>
#include <span>
#include <string_view>
#include <vector>

namespace cli_cpparser {

namespace {

// Whether each of two options is only given with the other.
bool go_together(std::string_view one, std::string_view other, std::span<const Needs> needs) {
    bool one_way = false;
    bool other_way = false;
    for (const Needs& rule : needs) {
        one_way = one_way || (rule.option == one && rule.needs == other);
        other_way = other_way || (rule.option == other && rule.needs == one);
    }
    return one_way && other_way;
}

// The options as a usage line has them, each after a space: the required ones, then the
// others in brackets. Those that go with one before them are in its brackets.
std::string options_part(std::span<const Option> options, std::span<const Needs> needs = {}) {
    std::string text;
    for (const Option& option : options) {
        if (option.required) {
            text += ' ' + spelled(option);
        }
    }
    std::vector<bool> written(options.size());
    for (std::size_t at = 0; at < options.size(); ++at) {
        if (options[at].required || written[at]) {
            continue;
        }
        text += " [" + spelled(options[at]);
        for (std::size_t next = at + 1; next < options.size(); ++next) {
            if (!options[next].required && !written[next] &&
                go_together(options[at].name, options[next].name, needs)) {
                text += ' ' + spelled(options[next]);
                written[next] = true;
            }
        }
        text += ']';
    }
    return text;
}

// The words as a usage line has them: " FILE", " [FILE..]", " DIR DIR [DIR]".
std::string words_part(const Words& words) {
    // A usage line names each word up to a few; a command that takes more says so in its
    // forms.
    constexpr std::size_t most_named = 4;
    const std::string_view name = words.shown.empty() ? words.name : words.shown;
    std::string text;
    for (std::size_t at = 0; at < words.least && at < most_named; ++at) {
        text.append(" ").append(name);
    }
    if (words.most == any_number) {
        return text.append(" [").append(name).append("..]");
    }
    for (std::size_t at = words.least; at < words.most && at < most_named; ++at) {
        text.append(" [").append(name).append("]");
    }
    return text;
}

} // namespace

std::string invocation(const Command& command, const Program& program) {
    return command.name.empty() ? std::string{program.name}
                                : std::format("{} {}", program.name, command.name);
}

std::string spelled(const Option& option) {
    return option.value.empty() ? std::string{option.name}
                                : std::format("{} {}", option.name, option.value);
}

std::vector<Option> every_option(const Command& command, const Program& program) {
    std::vector<Option> all{command.options.begin(), command.options.end()};
    all.insert(all.end(), program.shared.begin(), program.shared.end());
    return all;
}

std::string usage_text(const Command& command, const Program& program) {
    const std::string shared = options_part(program.shared);
    if (command.forms.empty()) {
        return std::format("usage: {}{}{}{}\n", invocation(command, program),
                           options_part(command.options, command.needs), words_part(command.words),
                           shared);
    }
    std::string text;
    for (const Form& form : command.forms) {
        text += std::format("{}{}{}\n", text.empty() ? "usage: " : "       ", form.usage, shared);
    }
    return text;
}

} // namespace cli_cpparser
