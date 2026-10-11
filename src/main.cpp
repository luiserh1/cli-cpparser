#include "app/run.hpp"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <span>
#include <string>
#include <vector>

// Program boundary: the only place where every exception is caught (POLICY.md 3).
// The handlers use std::fputs because std::println could itself throw.
int main(int argc, char** argv) {
    try {
        const std::span<char*> raw{argv, static_cast<std::size_t>(argc)};
        std::vector<std::string> words;
        for (std::size_t at = 1; at < raw.size(); ++at) {
            words.emplace_back(raw[at]);
        }
        return app::run(words, std::cout, std::cerr);
    } catch (const std::exception& e) {
        std::fputs("unhandled exception: ", stderr);
        std::fputs(e.what(), stderr);
        std::fputs("\n", stderr);
    } catch (...) { // boundary: program
        std::fputs("unhandled unknown exception\n", stderr);
    }
    return EXIT_FAILURE;
}
