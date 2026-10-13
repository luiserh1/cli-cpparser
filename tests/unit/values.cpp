#include "cli_cpparser/values/readers.hpp"

#include <doctest/doctest.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <format>
#include <limits>
#include <numbers>
#include <string>
#include <string_view>
#include <vector>

// An option's value read as a number, a list or one word of a set, and what is said when it
// is none.
namespace {

using cli_cpparser::decimal_number;
using cli_cpparser::list;
using cli_cpparser::one_of;
using cli_cpparser::whole_number;

// Why the text is refused; empty when it is not.
template <typename Read>
std::string refusal(const Read& read) {
    return read ? std::string{} : read.error();
}

} // namespace

TEST_CASE("a whole number is read from its digits") {
    struct Row {
        std::string_view text;
        int number;
    };
    const std::array rows{
        Row{.text = "23", .number = 23},
        Row{.text = "023", .number = 23},
        Row{.text = "0", .number = 0},
        Row{.text = "-4", .number = -4},
        Row{.text = "2147483647", .number = std::numeric_limits<int>::max()},
        Row{.text = "-2147483648", .number = std::numeric_limits<int>::min()},
    };
    for (const Row& row : rows) {
        CAPTURE(row.text);
        const auto number = whole_number(row.text, "--piece");
        REQUIRE(number.has_value());
        CHECK(*number == row.number);
    }
}

TEST_CASE("text that is not all a whole number is refused") {
    const std::array<std::string_view, 14> texts{
        "",  " 1",  "1 ",  "+1", "0x10", "1e3",        "1.5",
        "x", "1,2", "--1", "-",  "1-",   "2147483648", "-2147483649",
    };
    for (const std::string_view text : texts) {
        CAPTURE(text);
        CHECK(refusal(whole_number(text, "--piece")) ==
              std::format("--piece takes a whole number, and '{}' is not one.", text));
    }
}

TEST_CASE("a whole number outside its range is refused with the range") {
    CHECK(refusal(whole_number("0", "--piece", {.least = 1})) ==
          "--piece takes a whole number, 1 or more, and '0' is not one.");
    CHECK(refusal(whole_number("x", "--piece", {.least = 1})) ==
          "--piece takes a whole number, 1 or more, and 'x' is not one.");
    CHECK(refusal(whole_number("11", "--tries", {.most = 10})) ==
          "--tries takes a whole number, 10 or less, and '11' is not one.");
    CHECK(refusal(whole_number("8", "--size", {.least = 16, .most = 4096})) ==
          "--size takes a whole number from 16 to 4096, and '8' is not one.");
    CHECK(refusal(whole_number("4097", "--size", {.least = 16, .most = 4096})) ==
          "--size takes a whole number from 16 to 4096, and '4097' is not one.");
}

TEST_CASE("both ends of a whole number's range are in it") {
    const auto least = whole_number("16", "--size", {.least = 16, .most = 4096});
    const auto most = whole_number("4096", "--size", {.least = 16, .most = 4096});
    REQUIRE(least.has_value());
    REQUIRE(most.has_value());
    CHECK(*least == 16);
    CHECK(*most == 4096);
}

TEST_CASE("a decimal number is read with or without its decimals") {
    struct Row {
        std::string_view text;
        double number;
    };
    const std::array rows{
        Row{.text = "3", .number = 3.0},       Row{.text = "3.0", .number = 3.0},
        Row{.text = "-0.25", .number = -0.25}, Row{.text = ".5", .number = 0.5},
        Row{.text = "-.5", .number = -0.5},    Row{.text = "0.1", .number = 0.1},
        Row{.text = "007.50", .number = 7.5},  Row{.text = "0", .number = 0.0},
        Row{.text = "5.", .number = 5.0},      Row{.text = "1234567.125", .number = 1234567.125},
    };
    for (const Row& row : rows) {
        CAPTURE(row.text);
        const auto number = decimal_number(row.text, "--scale");
        REQUIRE(number.has_value());
        CHECK(*number == row.number);
    }
}

TEST_CASE("a decimal number is the double nearest to it however many digits it has") {
    struct Row {
        std::string_view text;
        double number;
    };
    const std::array rows{
        Row{.text = "0.3", .number = 0.3},
        Row{.text = "123456.789012345", .number = 123456.789012345},
        Row{.text = "-999999999999999", .number = -999999999999999.0},
        // As another program prints a double in full: 17 digits.
        Row{.text = "0.30000000000000004", .number = 0.1 + 0.2},
        Row{.text = "3.141592653589793238462643383279502884", .number = std::numbers::pi},
        Row{.text = "0.000000000000000000001", .number = 1e-21},
        Row{.text = "100000000000000000000000", .number = 1e23},
        Row{.text = "2.5000000000000000000000000", .number = 2.5},
        Row{.text = "00000000000000000000042", .number = 42.0},
    };
    for (const Row& row : rows) {
        CAPTURE(row.text);
        const auto number = decimal_number(row.text, "--scale");
        REQUIRE(number.has_value());
        CHECK(*number == row.number);
        CHECK(cli_cpparser::as_decimal_number(row.text) == row.number);
    }
}

TEST_CASE("a number too large for a double is refused") {
    const std::string too_large(400, '9');
    CHECK(refusal(decimal_number(too_large, "--scale")) ==
          std::format("--scale takes a number, and '{}' is not one.", too_large));
    CHECK_FALSE(cli_cpparser::as_decimal_number(too_large).has_value());
}

TEST_CASE("minus zero is read as zero") {
    const auto zero = decimal_number("-0.0", "--scale");
    REQUIRE(zero.has_value());
    CHECK(*zero == 0.0);
    CHECK_FALSE(std::signbit(*zero));
}

TEST_CASE("text that is not all a decimal number is refused") {
    const std::array<std::string_view, 20> texts{
        "",    " 1",   "1 ", "+1", "1e3", "1E3", "0x1p3", "nan", "inf",   "-inf",
        "1,5", "1..2", ".",  "-",  "1-",  "x",   "--1",   "-.",  "1.2.3", "1-2",
    };
    for (const std::string_view text : texts) {
        CAPTURE(text);
        CHECK(refusal(decimal_number(text, "--scale")) ==
              std::format("--scale takes a number, and '{}' is not one.", text));
        CHECK_FALSE(cli_cpparser::as_decimal_number(text).has_value());
    }
}

TEST_CASE("a number is read without a message for a command that writes its own") {
    CHECK(cli_cpparser::as_whole_number("023") == 23);
    CHECK(cli_cpparser::as_whole_number("-4") == -4);
    CHECK_FALSE(cli_cpparser::as_whole_number("1.5").has_value());
    CHECK_FALSE(cli_cpparser::as_whole_number("").has_value());
    CHECK(cli_cpparser::as_decimal_number("-0.25") == -0.25);
    CHECK(cli_cpparser::as_decimal_number(".5") == 0.5);
}

TEST_CASE("a decimal number outside its range is refused with the range") {
    CHECK(refusal(decimal_number("0", "--scale", {.least = 0.1, .most = 10})) ==
          "--scale takes a number from 0.1 to 10, and '0' is not one.");
    CHECK(refusal(decimal_number("10.001", "--scale", {.least = 0.1, .most = 10})) ==
          "--scale takes a number from 0.1 to 10, and '10.001' is not one.");
    CHECK(refusal(decimal_number("-1", "--blur", {.least = 0})) ==
          "--blur takes a number, 0 or more, and '-1' is not one.");
    CHECK(refusal(decimal_number("1.5", "--part", {.most = 1})) ==
          "--part takes a number, 1 or less, and '1.5' is not one.");
    const auto least = decimal_number("0.1", "--scale", {.least = 0.1, .most = 10});
    const auto most = decimal_number("10", "--scale", {.least = 0.1, .most = 10});
    REQUIRE(least.has_value());
    REQUIRE(most.has_value());
    CHECK(*least == 0.1);
    CHECK(*most == 10.0);
}

TEST_CASE("a list is its parts between the commas in order") {
    struct Row {
        std::string_view text;
        std::vector<std::string> parts;
    };
    const std::array rows{
        Row{.text = "1,23,52", .parts = {"1", "23", "52"}},
        Row{.text = "one", .parts = {"one"}},
        Row{.text = "a b, c", .parts = {"a b", " c"}},
        Row{.text = "x,x", .parts = {"x", "x"}},
    };
    for (const Row& row : rows) {
        CAPTURE(row.text);
        const auto parts = list(row.text, "--pieces");
        REQUIRE(parts.has_value());
        CHECK(*parts == row.parts);
    }
}

TEST_CASE("a list with an empty part is refused") {
    const std::array<std::string_view, 5> texts{"", ",", "1,,3", ",1", "1,"};
    for (const std::string_view text : texts) {
        CAPTURE(text);
        CHECK(refusal(list(text, "--pieces")) ==
              std::format("--pieces takes a list with commas between its parts, and '{}' has "
                          "an empty part.",
                          text));
    }
}

TEST_CASE("one word of a set is read as its place in the set") {
    constexpr std::array<std::string_view, 4> zones{"Front", "Back", "Side", "Rim"};
    for (std::size_t at = 0; at < zones.size(); ++at) {
        CAPTURE(zones.at(at));
        const auto place = one_of(zones.at(at), "--zone", zones);
        REQUIRE(place.has_value());
        CHECK(*place == at);
    }
}

TEST_CASE("a word that is not of the set is refused with the set") {
    constexpr std::array<std::string_view, 4> zones{"Front", "Back", "Side", "Rim"};
    // The word is written exactly as in the set: no other case, no shortening.
    const std::array<std::string_view, 4> texts{"front", "Fro", "", "Front,Back"};
    for (const std::string_view text : texts) {
        CAPTURE(text);
        CHECK(refusal(one_of(text, "--zone", zones)) ==
              std::format("--zone takes one of Front, Back, Side, Rim, and '{}' is none of them.",
                          text));
    }
}
