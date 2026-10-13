#pragma once

#include <cstddef>
#include <expected>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// An option's value, read as what the command needs. Each reader takes the text and the
// option it was given with, and its error is a whole message: it names the option, says
// what it takes and quotes what was given. A command returns that message as it is.
//
// The text is read strictly: the whole of it is the value, or the reader fails. No spaces
// around it, no "+", no "0x", no "1e3", no "nan" or "inf", and "." as the decimal point
// whatever the machine's language.
namespace cli_cpparser {

// The values a number may have, both ends included. Left as they are, there is no limit on
// that side.
struct WholeRange {
    int least{std::numeric_limits<int>::min()};
    int most{std::numeric_limits<int>::max()};
};
struct DecimalRange {
    double least{std::numeric_limits<double>::lowest()};
    double most{std::numeric_limits<double>::max()};
};

// The most digits a decimal number may be written with, not counting zeros before it or
// after its decimals: as many as a double holds whatever they are. So every number read is
// the double nearest to what was written, the same on every system.
inline constexpr std::size_t most_decimal_digits = 15;

// "23", "023", "-4".
//   "--piece takes a whole number, 1 or more, and 'x' is not one."
//   "--size takes a whole number from 16 to 4096, and '8' is not one."
//   "--shift takes a whole number, and '1.5' is not one."
[[nodiscard]] std::expected<int, std::string>
whole_number(std::string_view text, std::string_view option, WholeRange range = {});

// "3", "3.0", "-0.25", ".5", "5.", "007.500".
//   "--scale takes a number from 0.1 to 10, and '0' is not one."
//   "--scale takes a number of at most 15 digits, and '3.14159265358979323' has 18."
[[nodiscard]] std::expected<double, std::string>
decimal_number(std::string_view text, std::string_view option, DecimalRange range = {});

// The same two without a message, for a command that writes its own: the number, or nothing
// for text that is none (or, for a decimal number, that has too many digits).
[[nodiscard]] std::optional<int> as_whole_number(std::string_view text);
[[nodiscard]] std::optional<double> as_decimal_number(std::string_view text);

// "1,23,52" as its parts, in order. A part may not be empty, so neither may the text.
//   "--pieces takes a list with commas between its parts, and '1,,3' has an empty part."
// What a part must be is the command's to check, with another reader.
[[nodiscard]] std::expected<std::vector<std::string>, std::string> list(std::string_view text,
                                                                        std::string_view option);

// One word of a set, written exactly as in the set: its place in `set`.
//   "--zone takes one of Front, Back, Side, Rim, and 'front' is none of them."
[[nodiscard]] std::expected<std::size_t, std::string>
one_of(std::string_view text, std::string_view option, std::span<const std::string_view> set);

} // namespace cli_cpparser
