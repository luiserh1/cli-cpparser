#include "cli_cpparser/command/usage.hpp"

#include <format>

namespace cli_cpparser {

namespace {

// The command's one form when it gives none by hand.
std::string form_of_table(const Command& command, std::string_view program) {
    std::string form = invocation(command, program);
    for (const Option& option : command.options) {
        if (option.required) {
            form += std::format(" {}", spelled(option));
        }
    }
    for (const Option& option : command.options) {
        if (!option.required) {
            form += std::format(" [{}]", spelled(option));
        }
    }
    if (command.words.count == Count::one) {
        form += std::format(" {}", command.words.name);
    }
    if (command.words.count == Count::any) {
        form += std::format(" [{}..]", command.words.name);
    }
    return form;
}

} // namespace

std::string invocation(const Command& command, std::string_view program) {
    return command.name.empty() ? std::string{program}
                                : std::format("{} {}", program, command.name);
}

std::string spelled(const Option& option) {
    return option.value.empty() ? std::string{option.name}
                                : std::format("{} {}", option.name, option.value);
}

std::string usage_text(const Command& command, std::string_view program) {
    if (command.forms.empty()) {
        return std::format("usage: {}\n", form_of_table(command, program));
    }
    std::string text;
    for (const Form& form : command.forms) {
        text += std::format("{}{}\n", text.empty() ? "usage: " : "       ", form.usage);
    }
    return text;
}

} // namespace cli_cpparser
