#pragma once

#include <iosfwd>
#include <span>
#include <string>

// The example program: a few commands that do little, written as a user of the library
// writes them. Its tests are the library's end to end: help, messages and exit codes.
namespace app {

// Runs the program on the words after its name. Returns its exit code.
int run(std::span<const std::string> words, std::ostream& out, std::ostream& err);

} // namespace app
