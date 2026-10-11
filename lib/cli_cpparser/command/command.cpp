#include "cli_cpparser/command/command.hpp"

#include <algorithm>

namespace cli_cpparser {

bool has(const Arguments& arguments, std::string_view name) {
    return std::ranges::contains(arguments.options, name, &Given::name);
}

const std::string* value_of(const Arguments& arguments, std::string_view name) {
    const auto found = std::ranges::find(arguments.options, name, &Given::name);
    return found == arguments.options.end() ? nullptr : &found->value;
}

std::string value_or(const Arguments& arguments, std::string_view name, std::string_view fallback) {
    const std::string* const value = value_of(arguments, name);
    return value == nullptr ? std::string{fallback} : *value;
}

} // namespace cli_cpparser
