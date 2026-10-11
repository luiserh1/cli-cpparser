#include "cli_cpparser/values/readers.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <ranges>
#include <system_error>

namespace cli_cpparser {

namespace {

// A whole number from text, strictly: the whole text is the number, or there is none.
std::optional<int> whole_of(std::string_view text) {
    const char* const end = std::to_address(text.end());
    int number{};
    const auto [last, error] = std::from_chars(std::to_address(text.begin()), end, number);
    if (error != std::errc{} || last != end) {
        return std::nullopt;
    }
    return number;
}

// Ten to a power that a double holds exactly.
constexpr int most_exact_power = 22;
double ten_to(int power) {
    double result = 1;
    for (int at = 0; at < power; ++at) {
        result *= 10;
    }
    return result;
}

// The digits of a decimal number without its point, and the power of ten that puts it back:
// "12.5" is 125 and -1.
struct Digits {
    std::uint64_t digits{};
    int power{};
};

// Text as digits with at most one point among them; nothing for any other text. Digits past
// those a std::uint64_t holds are dropped, which keeps more of them than a double has.
std::optional<Digits> digits_of(std::string_view text) {
    constexpr std::uint64_t most_before_another = 999'999'999'999'999'999;
    Digits read;
    bool any_digit = false;
    bool after_point = false;
    for (const char letter : text) {
        if (letter == '.' && !after_point) {
            after_point = true;
            continue;
        }
        if (letter < '0' || letter > '9') {
            return std::nullopt;
        }
        any_digit = true;
        if (read.digits <= most_before_another) {
            read.digits = (read.digits * 10) + static_cast<std::uint64_t>(letter - '0');
            read.power -= after_point ? 1 : 0;
        } else {
            read.power += after_point ? 0 : 1;
        }
    }
    if (!any_digit) {
        return std::nullopt;
    }
    return read;
}

// A decimal number from text, strictly, as the header says. It is this file's own and not
// std::from_chars, which macOS has for a double only from version 26 on, nor the C
// runtime's, which reads by the machine's language. A number of up to 15 digits is one
// division or one product of two exact numbers, so it is the nearest double.
std::optional<double> decimal_of(std::string_view text) {
    const bool negative = text.starts_with('-');
    const auto read = digits_of(negative ? text.substr(1) : text);
    if (!read) {
        return std::nullopt;
    }
    auto number = static_cast<double>(read->digits);
    for (int left = -read->power; left > 0; left -= most_exact_power) {
        number /= ten_to(std::min(left, most_exact_power));
    }
    for (int left = read->power; left > 0; left -= most_exact_power) {
        number *= ten_to(std::min(left, most_exact_power));
    }
    if (!std::isfinite(number)) {
        return std::nullopt;
    }
    if (number == 0) {
        return 0.0; // "-0" is zero
    }
    return negative ? -number : number;
}

// What follows "a whole number" in a message: ", 1 or more,", " from 16 to 4096,", or the
// comma alone when the range has no limit.
template <typename Range>
std::string range_text(const Range& range) {
    const Range all{};
    const bool has_least = range.least != all.least;
    const bool has_most = range.most != all.most;
    if (has_least && has_most) {
        return std::format(" from {} to {},", range.least, range.most);
    }
    if (has_least) {
        return std::format(", {} or more,", range.least);
    }
    if (has_most) {
        return std::format(", {} or less,", range.most);
    }
    return ",";
}

template <typename Number, typename Range>
std::expected<Number, std::string> in_range(std::optional<Number> number, const Range& range,
                                            std::string refusal) {
    if (!number || *number < range.least || *number > range.most) {
        return std::unexpected(std::move(refusal));
    }
    return *number;
}

} // namespace

std::expected<int, std::string> whole_number(std::string_view text, std::string_view option,
                                             WholeRange range) {
    return in_range(whole_of(text), range,
                    std::format("{} takes a whole number{} and '{}' is not one.", option,
                                range_text(range), text));
}

std::expected<double, std::string> decimal_number(std::string_view text, std::string_view option,
                                                  DecimalRange range) {
    return in_range(
        decimal_of(text), range,
        std::format("{} takes a number{} and '{}' is not one.", option, range_text(range), text));
}

std::expected<std::vector<std::string>, std::string> list(std::string_view text,
                                                          std::string_view option) {
    std::vector<std::string> parts;
    // Splitting no text gives no parts, where it has one that is empty.
    if (text.empty()) {
        parts.emplace_back();
    }
    for (const auto part : std::views::split(text, ',')) {
        parts.emplace_back(std::string_view{part});
    }
    for (const std::string& part : parts) {
        if (part.empty()) {
            return std::unexpected(std::format("{} takes a list with commas between its parts, "
                                               "and '{}' has an empty part.",
                                               option, text));
        }
    }
    return parts;
}

std::expected<std::size_t, std::string> one_of(std::string_view text, std::string_view option,
                                               std::span<const std::string_view> set) {
    const auto found = std::ranges::find(set, text);
    if (found != set.end()) {
        return static_cast<std::size_t>(std::ranges::distance(set.begin(), found));
    }
    std::string words;
    for (const std::string_view word : set) {
        words += std::format("{}{}", words.empty() ? "" : ", ", word);
    }
    return std::unexpected(
        std::format("{} takes one of {}, and '{}' is none of them.", option, words, text));
}

} // namespace cli_cpparser
