#pragma once

#include "cli_cpparser/command/command.hpp"

#include <expected>
#include <span>
#include <string>

namespace cli_cpparser {

// The words after a command's name as its arguments. `program` gives the program's name,
// for the messages ("'alfar zones' has no option --fast"), and the options every command
// shares.
//
// The rules:
//   - An option is "--name value" or "--name=value". A flag stands alone.
//   - "-h" or "--help" anywhere among the words wins: `help` is set and nothing else is
//     looked at, not even a mistake. There are no other short options: "-x" is a word.
//   - A value that starts with "--" must be written "--name=VALUE".
//   - Every other word is one of the command's words, checked against how many it takes.
//
// The error is one message that names the option and says what to write:
//   an option the command has not   "'alfar build' has no option --fast. Its options are
//                                    --all, --piece N, --zone Z."   (or "It has no options.")
//   an option given twice           "--piece is given twice, with '1' and with '2'. Give it
//                                    once."          (a flag: "--all is given twice. ...")
//   no value, or an empty one       "--piece needs a value: write --piece N. (A value that
//                                    starts with -- is written --piece=VALUE.)"
//   a flag with a value             "--all takes no value, and '--all=yes' gives it one.
//                                    Write --all alone."
//   a word, and it takes none       "'x.png' is not an option, and 'alfar build' takes
//                                    nothing else. Its options are ..."
//   no word, and it takes one       "'alfar import' needs its FILE: write alfar import FILE
//                                    --project DIR."          (with the required options)
//   two words, and it takes one     "'alfar import' takes one FILE, and 3 were given: 'a'
//                                    and 'b' are two of them. Run it once for each."
//   any other number of words       "'alfar diff' takes 2 or 3 DIR, and 1 was given."
//   a required option missing       "--out is missing: 'alfar export' needs it. Write --out
//                                    DIR."          (or the option's own `missing` text)
//   a choice not made, or made twice   see Choice (command.hpp)
//   an option without the one it goes with   see Needs (command.hpp)
//
// The words are checked in order, and the first mistake among them is the one reported.
// Then, in this order: how many words there are; the required options, the program's
// shared ones first, each list in its own order; the choices; the options that go with
// another.
//
// In a program with no commands (`command.name` empty) the messages say "'serve'" where
// these say "'alfar build'".
[[nodiscard]] std::expected<Arguments, std::string>
parse(std::span<const std::string> words, const Command& command, const Program& program);

} // namespace cli_cpparser
