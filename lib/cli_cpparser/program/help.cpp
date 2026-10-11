#include "cli_cpparser/program/program.hpp"

#include <algorithm>
#include <cstddef>
#include <format>
#include <utility>
#include <vector>

namespace cli_cpparser {

namespace {

// One line of a list: what is written, and what it is.
struct Row {
    std::string name;
    std::string_view meaning;
};

// The rows in two columns, two spaces in. A name too long for the first column has its
// meaning on a line of its own, so that one long name doesn't push every meaning away.
std::string columns(std::span<const Row> rows) {
    constexpr std::size_t widest_column = 24;
    std::size_t width = 0;
    for (const Row& row : rows) {
        if (row.name.size() <= widest_column) {
            width = std::max(width, row.name.size());
        }
    }
    std::string text;
    for (const Row& row : rows) {
        if (row.meaning.empty()) {
            text += std::format("  {}\n", row.name);
        } else if (row.name.size() > width) {
            text += std::format("  {}\n  {:<{}}  {}\n", row.name, "", width, row.meaning);
        } else {
            text += std::format("  {:<{}}  {}\n", row.name, width, row.meaning);
        }
    }
    return text;
}

// A command's lines in the program's help: one for each form that is listed, or one with
// its name.
void add_rows(std::vector<Row>& rows, const Command& command) {
    const std::size_t before = rows.size();
    for (const Form& form : command.forms) {
        if (!form.listed.empty()) {
            rows.push_back(Row{.name = std::string{form.listed}, .meaning = form.summary});
        }
    }
    if (rows.size() == before) {
        rows.push_back(Row{.name = std::string{command.name}, .meaning = command.summary});
    }
}

} // namespace

std::string help_text(const Program& program) {
    std::vector<Row> rows;
    for (const Command& command : program.commands) {
        add_rows(rows, command);
    }
    rows.push_back(Row{.name = "help", .meaning = "Show this list"});
    rows.push_back(Row{.name = "--version", .meaning = "Print the program's version"});
    std::string text = std::format("{} {}: {}\n\n", program.name, program.version, program.summary);
    text += std::format("Usage: {} <command> [options]\n\n", program.name);
    text += std::format("Commands:\n{}\n", columns(rows));
    if (!program.notes.empty()) {
        text += std::format("{}\n\n", program.notes);
    }
    text += std::format("Run '{} <command> --help' for a command's options.\n", program.name);
    return text;
}

std::string help_text(const Command& command, std::string_view program) {
    std::string text = usage_text(command, program);
    const std::string_view about = command.details.empty() ? command.summary : command.details;
    if (!about.empty()) {
        text += std::format("\n{}\n", about);
    }
    std::vector<Row> rows;
    for (const Option& option : command.options) {
        rows.push_back(Row{.name = spelled(option), .meaning = option.help});
    }
    rows.push_back(Row{.name = "-h, --help", .meaning = "Show this help"});
    text += std::format("\nOptions:\n{}", columns(rows));
    return text;
}

} // namespace cli_cpparser
