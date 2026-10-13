# cli-cpparser

A small C++23 library that reads a program's command line: a table per command, one parse
function, help written from the table, and messages that say what to write. It is the
common part of the parsers four programs each wrote for themselves, with their tests in
one place.

It is a [cpp-policy](https://github.com/luiserh1/cpp-policy) library (POLICY.md 11.7):
another policy project uses it at a pinned commit, and it is built in that project's build.
Licence: MIT.

## The rules of the command line

- An option is `--name value` or `--name=value`. A flag stands alone.
- There are no short options, except `-h` for help. `-x` is a plain word.
- `-h` or `--help` anywhere among the words wins: nothing else is looked at.
- An option given twice is an error. So is one the command has not, one without its value
  or with an empty one, a flag given a value, and a required option left out.
- A value that starts with `--` is written `--name=VALUE`.
- The words that are no options are checked against how many the command takes: none, one,
  two or three, any number.
- A table can say two rules between options: that one of several is given and no more
  (`--auto` or `--check`), and that one goes only with another (`--zone` with `--piece`).
- A number is read strictly: the whole text is the number or it is refused, and what is
  read is the number nearest to what was written, the same on every system.
- A program exits with 0 when it did its work, with 1 when it could not, and with 2 when
  the command line is wrong.
  Code 2 is the library's on every command: a program's own codes, if it has any, start
  at 3.

Every error is one message that names the option and says what to write:

```
error: --times takes a whole number from 1 to 100, and '101' is not one.
usage: cli_cpparser repeat --times N [--separator TEXT] [--final TEXT] WORD [--prefix TEXT]
```

## Using it

A command is a constant table, and a function that gets its arguments already read:

```cpp
#include "cli_cpparser/program/program.hpp"
#include "cli_cpparser/values/readers.hpp"

using namespace cli_cpparser;

std::expected<int, std::string> run_resize(const Arguments& arguments, std::ostream& out,
                                           std::ostream& err) {
    const auto size = whole_number(required_value(arguments, "--size"), "--size",
                                   {.least = 16, .most = 4096});
    if (!size) {
        return std::unexpected(size.error()); // printed with the usage; exit code 2
    }
    // ... arguments.words.front() is the FILE ...
    return exit_ok;
}

constexpr std::array resize_options{
    Option{.name = "--size", .value = "N", .help = "The new width, 16 to 4096",
           .required = true},
    Option{.name = "--keep", .help = "Keep the original file"},
    Option{.name = "--out", .value = "FILE", .help = "Where to write it"},
};
constexpr std::array resize_choices{Choice{.among = {"--keep", "--out"}}}; // not both
constexpr std::array commands{
    Command{.name = "resize",
            .summary = "Resize an image",
            .options = resize_options,
            .words = one_word("FILE"),
            .choices = resize_choices,
            .run = run_resize},
};
constexpr std::array shared{
    Option{.name = "--quiet", .help = "Print nothing but errors"}, // every command takes it
};
constexpr Program program{.name = "imagetool", .version = "1.0.0",
                          .summary = "works on images", .commands = commands,
                          .shared = shared};

// In main, with the words after the program's name:
//     return cli_cpparser::run(program, words, std::cout, std::cerr);
```

`run()` handles `help`, `-h`, `--help`, `--version`, an unknown command, each command's
`--help`, and every wrong command line. The headers say the rest:

| Header | What it has |
|---|---|
| `cli_cpparser/command/command.hpp` | The table (`Option`, `Words`, `Form`, `Choice`, `Needs`, `Command`, `Program`), the arguments read (`Arguments`, `has`, `value_of`, `value_or`, `required_value`), and `joined()` for lists of options some commands share |
| `cli_cpparser/command/parse.hpp` | `parse()`, with every rule and every message |
| `cli_cpparser/command/usage.hpp` | A command's usage as text |
| `cli_cpparser/values/readers.hpp` | An option's value as a whole number or a decimal number in a range, a list with commas, or one word of a set; and the two numbers without a message |
| `cli_cpparser/program/program.hpp` | `run()`, `run_command()` for one command alone, the help texts, the exit codes, and `read()` for a program with no commands |

- **Options every command takes** are given once, in `Program::shared`: they are added to
  every command's table, usage and help, and listed once in the program's help. Options
  only some commands share are joined to those commands' own with `joined()`.
- **A command with several forms** (`zones --auto ..` / `zones --check`) writes them by hand
  as `Form`s; the library writes the shared options after each.
- **A rule the table can't say** ("words only with `--add`") is the command's own: it
  returns the message, and the library prints it with the usage.
- **A program with no commands** has one `Command` without a name and calls `read()`.
- **A program's own exit codes** start at 3: 0, 1 and 2 are the library's on every command.

`src/app/` is a whole example.

In the project that uses it (`<commit>` is a release's commit; see `CHANGELOG.md`):

```cmake
cpp_policy_use_library(cli_cpparser
    GIT_REPOSITORY https://github.com/luiserh1/cli-cpparser.git
    GIT_TAG        <commit>)   # v0.4.0
target_link_libraries(app_core PRIVATE cli_cpparser::cli_cpparser)
cpp_policy_confine_includes(cli_cpparser/ TO app)
```

The link is `PUBLIC` when the target's own headers name the library's types
(`cli_cpparser::Arguments` in a header that declares the runners), since its tests include
them.

It needs no package, and cpp-policy v0.34.0 or later in the project that uses it (for the
macOS it builds for).

### What it leaves out

Short options and their bundles, binding results to variables, environment variables,
configuration files, shell completion and colours. The library is worth having only while
it stays small.

## Building

Requirements: LLVM 23 (clang, clang-tidy, clang-format), CMake 3.29 or later
and Ninja, plus vcpkg, with `VCPKG_ROOT` set to its folder.
The cpp-policy README ("Setup per platform") says how to install them.

Dependencies come from vcpkg (`vcpkg.json`, pinned by its
`builtin-baseline`); set `VCPKG_ROOT` before configuring.

```
sh tools/hooks/gate                    # the gate: audit, format, build with clang-tidy, tests
cmake --workflow --preset unit         # quick: no clang-tidy, -O1, unit tests only
cmake --workflow --preset release      # optimized build, size budget, benchmarks
```

On Windows the presets carry a `win-` prefix (`win-check`, `win-unit`,
`win-release`).

Enable the git hooks once per clone:

```
git config core.hooksPath tools/hooks
```

`pre-commit` runs the suppression audit, the format check and clang-tidy on
the staged C++ files; `commit-msg` checks the message (Conventional Commits
with a body that says why); `pre-push` runs the full gate. After editing C++
files, `sh tools/hooks/tidy-files <file>...` checks just those files.

This project follows [cpp-policy](https://github.com/luiserh1/cpp-policy),
pinned in `CMakeLists.txt` (v0.39.0). `.clang-tidy`, `.clang-format`,
`CMakePresets.json` and `tools/hooks/` are its copies and must not be edited
here; upgrade with its `tools/upgrade.sh`.

Every push runs the gate in GitHub Actions (`.github/workflows/ci.yml`).

## Layout

The repository builds a library and, over it, an example program (cpp-policy POLICY.md
11.7). The library is in `lib/cli_cpparser/`, declared in `library.cmake`. The example's
modules are in `src/`. Every include of the library is written
`"cli_cpparser/<module>/<header>.hpp"`, and of the example `"<module>/<header>.hpp"`.

Both are split into modules, grouped into layers, here from the top down. Code may include
headers from its own layer or a lower one, and the library never includes the example.
`cpp_policy_layers()` in `CMakeLists.txt` declares the example's layers,
`cpp_policy_library()` in `library.cmake` the library's, and the audit enforces them.

The example program, in `src/`:

| Layer | Path | Contents |
|---|---|---|
| 2 | `src/main.cpp` | Program boundary: `argv` into words, and every exception caught |
| 1 | `src/app/` | Three commands (`repeat`, `sum`, `pick`) written as a user of the library writes them: their tables and what they do |

The library, in `lib/cli_cpparser/`:

| Layer | Module | Contents |
|---|---|---|
| 2 | `program/` | A program's table of commands: the dispatch, the help texts, the exit codes |
| 1 | `command/` | A command's and a program's table, the parser with its checks, the arguments read, the usage text |
| 1 | `values/` | Typed readers of an option's value |

`tests/unit/` has one doctest program per module (`cpp_policy_add_tests()` in
`tests/CMakeLists.txt`): the library's three, and `app`, which runs the example program end
to end, command line in and text and exit code out. CTest runs each test case on its own,
named `<kind>/<module>/<test case>`: `ctest --preset dev -L unit` runs the unit tests,
`-R <name>` one test.
