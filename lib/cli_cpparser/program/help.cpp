#include "cli_cpparser/command/usage.hpp"
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

// The width of the first column for these rows: the longest name that isn't too long for a
// column, so that one long name doesn't push every meaning away.
std::size_t column_width(std::span<const Row> rows) {
    constexpr std::size_t widest_column = 24;
    std::size_t width = 0;
    for (const Row& row : rows) {
        if (row.name.size() <= widest_column) {
            width = std::max(width, row.name.size());
        }
    }
    return width;
}

// The rows in two columns, two spaces in. A name longer than the first column has its
// meaning on a line of its own.
std::string columns(std::span<const Row> rows, std::size_t width) {
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

// The forms of a command that have a line of their own in the program's help.
std::vector<Row> listed_forms(const Command& command) {
    std::vector<Row> rows;
    for (const Form& form : command.forms) {
        if (!form.listed.empty()) {
            rows.push_back(Row{.name = std::string{form.listed}, .meaning = form.summary});
        }
    }
    return rows;
}

std::vector<Row> rows_of(std::span<const Option> options) {
    std::vector<Row> rows;
    rows.reserve(options.size());
    for (const Option& option : options) {
        rows.push_back(Row{.name = spelled(option), .meaning = option.help});
    }
    return rows;
}

// The program's commands as the rows of its help.
std::vector<Row> rows_of(const Program& program) {
    std::vector<Row> rows;
    for (const Command& command : program.commands) {
        std::vector<Row> forms = listed_forms(command);
        if (forms.empty()) {
            const std::string_view listed = command.listed.empty() ? command.name : command.listed;
            forms.push_back(Row{.name = std::string{listed}, .meaning = command.summary});
        }
        rows.insert(rows.end(), forms.begin(), forms.end());
    }
    rows.push_back(Row{.name = "help", .meaning = "Show this list"});
    rows.push_back(Row{.name = "--version", .meaning = "Print the program's version"});
    return rows;
}

} // namespace

std::string help_text(const Program& program) {
    const std::vector<Row> commands = rows_of(program);
    const std::vector<Row> shared = rows_of(program.shared);
    // One column for both lists, so that they line up.
    const std::size_t width = std::max(column_width(commands), column_width(shared));
    std::string required;
    for (const Option& option : program.shared) {
        if (option.required) {
            required += std::format(" {}", spelled(option));
        }
    }
    std::string text = std::format("{} {}: {}\n\n", program.name, program.version, program.summary);
    text += std::format("Usage: {} <command>{} [options]\n\n", program.name, required);
    text += std::format("Commands:\n{}\n", columns(commands, width));
    if (!shared.empty()) {
        text += std::format("Every command takes:\n{}\n", columns(shared, width));
    }
    if (!program.notes.empty()) {
        text += std::format("{}\n\n", program.notes);
    }
    text += std::format("Run '{} <command> --help' for a command's options.\n", program.name);
    return text;
}

std::string help_text(const Command& command, const Program& program) {
    std::string text = usage_text(command, program);
    const std::string_view about = command.details.empty() ? command.summary : command.details;
    if (!about.empty()) {
        text += std::format("\n{}\n", about);
    } else if (const std::vector<Row> forms = listed_forms(command); !forms.empty()) {
        text += std::format("\n{}", columns(forms, column_width(forms)));
    }
    std::vector<Row> rows = rows_of(every_option(command, program));
    rows.push_back(Row{.name = "-h, --help", .meaning = "Show this help"});
    text += std::format("\nOptions:\n{}", columns(rows, column_width(rows)));
    return text;
}

} // namespace cli_cpparser
