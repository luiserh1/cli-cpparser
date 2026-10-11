#include "cli_cpparser/program/program.hpp"

#include "cli_cpparser/command/parse.hpp"

#include <algorithm>
#include <ostream>
#include <print>
#include <utility>

namespace cli_cpparser {

namespace {

int run_command(const Command& command, std::span<const std::string> words, const Program& program,
                std::ostream& out, std::ostream& err) {
    const auto arguments = read(words, command, program.name, out, err);
    if (!arguments) {
        return arguments.error();
    }
    if (command.run == nullptr) {
        std::println(err, "error: '{}' can't be run: its table names nothing to run.",
                     invocation(command, program.name));
        return exit_failure;
    }
    const auto code = command.run(*arguments, out, err);
    if (!code) {
        return usage_error(command, program.name, code.error(), err);
    }
    return *code;
}

} // namespace

int usage_error(const Command& command, std::string_view program, std::string_view message,
                std::ostream& err) {
    std::println(err, "error: {}", message);
    std::print(err, "{}", usage_text(command, program));
    return exit_usage;
}

std::expected<Arguments, int> read(std::span<const std::string> words, const Command& command,
                                   std::string_view program, std::ostream& out, std::ostream& err) {
    auto arguments = parse(words, command, program);
    if (!arguments) {
        return std::unexpected(usage_error(command, program, arguments.error(), err));
    }
    if (arguments->help) {
        std::print(out, "{}", help_text(command, program));
        return std::unexpected(exit_ok);
    }
    return std::move(*arguments);
}

int run(const Program& program, std::span<const std::string> words, std::ostream& out,
        std::ostream& err) {
    if (words.empty()) {
        std::print(err, "{}", help_text(program));
        return exit_usage;
    }
    const std::string_view name = words.front();
    if (name == "help" || name == "--help" || name == "-h") {
        std::print(out, "{}", help_text(program));
        return exit_ok;
    }
    if (name == "--version") {
        std::println(out, "{} {}", program.name, program.version);
        return exit_ok;
    }
    const auto command = std::ranges::find(program.commands, name, &Command::name);
    if (command == program.commands.end()) {
        std::println(err, "error: unknown command '{}'. Run '{} help' for the list of commands.",
                     name, program.name);
        return exit_usage;
    }
    return run_command(*command, words.subspan(1), program, out, err);
}

} // namespace cli_cpparser
