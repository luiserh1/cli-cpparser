#include "cli_cpparser/values/readers.hpp"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <ranges>
#include <system_error>

namespace cli_cpparser {

namespace {

// Ten to a power, exactly: a double holds every power up to the 22nd.
double ten_to(std::size_t power) {
    double result = 1;
    for (std::size_t at = 0; at < power; ++at) {
        result *= 10;
    }
    return result;
}

bool all_digits(std::string_view text) {
    return std::ranges::all_of(text, [](char letter) { return letter >= '0' && letter <= '9'; });
}

// Why text is no decimal number that can be read.
struct NotRead {
    std::size_t digits{}; // how many it has, when they are too many; 0: it is no number
};

// A decimal number from text, strictly, as the header says. It is this file's own and not
// std::from_chars, which macOS has for a double only from version 26 on, nor the C
// runtime's, which reads by the machine's language. With at most 15 digits, the digits are
// a whole number a double holds exactly, and so is the power of ten they are divided by:
// one division of two exact numbers gives the nearest double.
std::expected<double, NotRead> decimal_of(std::string_view text) {
    const bool negative = text.starts_with('-');
    const std::string_view written = negative ? text.substr(1) : text;
    const std::size_t point = std::min(written.find('.'), written.size());
    std::string_view whole = written.substr(0, point);
    std::string_view decimals = written.substr(std::min(point + 1, written.size()));
    if (!all_digits(whole) || !all_digits(decimals) || (whole.empty() && decimals.empty())) {
        return std::unexpected(NotRead{});
    }
    whole = whole.substr(std::min(whole.find_first_not_of('0'), whole.size()));
    decimals = decimals.substr(0, decimals.find_last_not_of('0') + 1);
    const std::size_t digits = whole.size() + decimals.size();
    if (digits > most_decimal_digits) {
        return std::unexpected(NotRead{.digits = digits});
    }
    std::uint64_t value = 0;
    for (const std::string_view part : {whole, decimals}) {
        for (const char letter : part) {
            value = (value * 10) + static_cast<std::uint64_t>(letter - '0');
        }
    }
    const double number = static_cast<double>(value) / ten_to(decimals.size());
    return negative && value != 0 ? -number : number; // "-0" is zero
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

std::optional<int> as_whole_number(std::string_view text) {
    const char* const end = std::to_address(text.end());
    int number{};
    const auto [last, error] = std::from_chars(std::to_address(text.begin()), end, number);
    if (error != std::errc{} || last != end) {
        return std::nullopt;
    }
    return number;
}

std::optional<double> as_decimal_number(std::string_view text) {
    const auto number = decimal_of(text);
    return number ? std::optional{*number} : std::nullopt;
}

std::expected<int, std::string> whole_number(std::string_view text, std::string_view option,
                                             WholeRange range) {
    return in_range(as_whole_number(text), range,
                    std::format("{} takes a whole number{} and '{}' is not one.", option,
                                range_text(range), text));
}

std::expected<double, std::string> decimal_number(std::string_view text, std::string_view option,
                                                  DecimalRange range) {
    const auto number = decimal_of(text);
    if (!number && number.error().digits != 0) {
        return std::unexpected(std::format("{} takes a number of at most {} digits, and '{}' "
                                           "has {}.",
                                           option, most_decimal_digits, text,
                                           number.error().digits));
    }
    return in_range(
        number ? std::optional{*number} : std::nullopt, range,
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
