#include "cli_cpparser/values/readers.hpp"

#include <algorithm>
#include <charconv>
#include <format>
#include <memory>
#include <optional>
#include <ranges>
#include <system_error>

namespace cli_cpparser {

namespace {

// A number from text, strictly: the whole text is the number, or there is none.
template <typename Number, typename... Format>
std::optional<Number> number_of(std::string_view text, Format... format) {
    const char* const end = std::to_address(text.end());
    Number number{};
    const auto [last, error] =
        std::from_chars(std::to_address(text.begin()), end, number, format...);
    if (error != std::errc{} || last != end) {
        return std::nullopt;
    }
    return number;
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
    return number_of<int>(text);
}

std::optional<double> as_decimal_number(std::string_view text) {
    // std::from_chars reads "inf" and "nan" as numbers; a number here is written with digits,
    // and a number too large for a double is none.
    constexpr std::string_view written_with = "0123456789.-";
    if (text.find_first_not_of(written_with) != std::string_view::npos) {
        return std::nullopt;
    }
    const auto number = number_of<double>(text, std::chars_format::fixed);
    if (number && *number == 0) {
        return 0.0; // "-0" is zero
    }
    return number;
}

std::expected<int, std::string> whole_number(std::string_view text, std::string_view option,
                                             WholeRange range) {
    return in_range(as_whole_number(text), range,
                    std::format("{} takes a whole number{} and '{}' is not one.", option,
                                range_text(range), text));
}

std::expected<double, std::string> decimal_number(std::string_view text, std::string_view option,
                                                  DecimalRange range) {
    return in_range(
        as_decimal_number(text), range,
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
