#include "cli_cpparser/command/usage.hpp"

#include <cstddef>
#include <format>

namespace cli_cpparser {

namespace {

// The options as a usage line has them, each after a space: the required ones, then the
// others in brackets.
std::string options_part(std::span<const Option> options) {
    std::string text;
    for (const Option& option : options) {
        if (option.required) {
            text += std::format(" {}", spelled(option));
        }
    }
    for (const Option& option : options) {
        if (!option.required) {
            text += std::format(" [{}]", spelled(option));
        }
    }
    return text;
}

// The words as a usage line has them: " FILE", " [FILE..]", " DIR DIR [DIR]".
std::string words_part(const Words& words) {
    // A usage line names each word up to a few; a command that takes more says so in its
    // forms.
    constexpr std::size_t most_named = 4;
    std::string text;
    for (std::size_t at = 0; at < words.least && at < most_named; ++at) {
        text += std::format(" {}", words.name);
    }
    if (words.most == any_number) {
        return text + std::format(" [{}..]", words.name);
    }
    for (std::size_t at = words.least; at < words.most && at < most_named; ++at) {
        text += std::format(" [{}]", words.name);
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
                           options_part(command.options), words_part(command.words), shared);
    }
    std::string text;
    for (const Form& form : command.forms) {
        text += std::format("{}{}{}\n", text.empty() ? "usage: " : "       ", form.usage, shared);
    }
    return text;
}

} // namespace cli_cpparser
