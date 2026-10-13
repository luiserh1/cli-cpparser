#include "cli_cpparser/command/command.hpp"

#include <algorithm>

namespace cli_cpparser {

bool has(const Arguments& arguments, std::string_view name) {
    return std::ranges::contains(arguments.options, name, &Given::name);
}

std::optional<std::string_view> value_of(const Arguments& arguments, std::string_view name) {
    const auto found = std::ranges::find(arguments.options, name, &Given::name);
    if (found == arguments.options.end()) {
        return std::nullopt;
    }
    return found->value;
}

std::string value_or(const Arguments& arguments, std::string_view name, std::string_view fallback) {
    return std::string{value_of(arguments, name).value_or(fallback)};
}

std::string_view required_value(const Arguments& arguments, std::string_view name) {
    return value_of(arguments, name).value_or(std::string_view{});
}

} // namespace cli_cpparser
