# Changelog

Each entry says what changed and why. The version is defined once, in
`project()` in `CMakeLists.txt`; each release is an annotated tag `v<version>`.

Policy: cpp-policy v0.39.0.

## Unreleased

## 0.4.0 (2026-10-13)

What Alfar's adoption of 0.2.0 asked for. A user of 0.3.0 changes nothing in its tables or
calls; one item changes what a program may print.

- A rule between options says the program's own words when it has them: `Needs::message`,
  and `Choice::several` and `Choice::missing` (several of the choice given; none given and
  one required), as `Option::missing` is for a required option. Empty, each keeps the
  library's sentence. Alfar adopted 0.2.0 on the condition that no message a user reads
  changes, and none of the library's sentences is Alfar's, so seven of its rules stayed
  checks inside four commands.
- **Changed, in what a program prints:** two options that each need the other (two `Needs`,
  one each way) share one pair of brackets in the usage written from the table:
  `[--tonematcher PROGRAM --result OUT]`, where it wrote `[--tonematcher PROGRAM]
  [--result OUT]`. The second is written beside the first, wherever the table has it. Both
  must be the command's own and not required. The usage says what the parser enforces;
  Alfar's `export` kept a `Form` only to write that line. A command with such a pair and
  no `Form` prints a different usage line from 0.3.0.
- `Words::shown`: how the usage writes a word, when that is not what the messages call it
  (`FILE|FOLDER` in the usage, "needs its FILE" in the message). Empty, it is
  `Words::name`, as before. Alfar's `import` kept a `Form` only for that line.
- The usage is written without `std::format`. With the three changes above the example
  was 512 bytes over its size budget on Windows; written so, its code is smaller than
  0.3.0's on macOS (138,030 bytes of sections against 139,647).
- The header says that a value is a view, and that a file's stream wants a string or a path
  made of it: `std::ofstream::open` takes a `std::string_view` with libc++ and not with
  libstdc++, so ToneMatcher's use of one passed on the Mac and failed on Linux.

## 0.3.0 (2026-10-13)

- A decimal number is read exactly whatever its length: `decimal_number` and
  `as_decimal_number` use `std::from_chars` again, and the limit of 15 digits that 0.2.0
  set is gone, with its message and the constant `most_decimal_digits`. A number another
  program printed in full has 17 digits (`0.30000000000000004`) and was refused, and
  ToneMatcher kept its own reader for it. 0.1.0 left `std::from_chars` because GitHub's
  macOS lacked it for a `double`; from cpp-policy v0.34.0 every Mac build is for macOS
  26.0, which has it. A project that uses the library needs that release or a later one.

## 0.2.0 (2026-10-13)

What Alfar's and ToneMatcher's adoption of 0.1.0 asked for. Both adopted it without
patching it; each kept code of its own for something the table could not say, and that is
what these changes take in. A user of 0.1.0 has to change its tables and calls: every item
marked **changed** breaks them.

- **Changed:** the functions that took the program's name (`parse`, `usage_text`,
  `invocation`, `help_text`, `usage_error`, `read`) take the `Program`. `Program` is
  defined in `command/command.hpp`. They need the options the program shares, below; a
  program with no commands passes a `Program` with its name alone.
- `Program::shared`: options every command takes, given once. They are added to every
  command's table, written after its own in the usage (after each hand-written form too)
  and in its help, listed once in the program's help under "Every command takes", and the
  required ones are named in its "Usage:" line. A required one that is missing is reported
  before a command's own. Alfar wrote its three folders in thirteen usage lines and in
  every table, and aligned its help's notes by hand to the library's column.
- **Changed:** `Form::usage` is written without the shared options, for the same reason.
- **Changed:** `Words` has `least` and `most` in place of `Count` (none, one, any):
  `one_word("FILE")` and `any_words("FILE")` say what `Count::one` and `Count::any` said.
  ToneMatcher's `diff DIR1 DIR2 [OUT_DIR]` takes two or three words and had to count them
  itself.
- `Command::choices` and `Command::needs`: one of several options and no more, and an
  option that goes only with another, checked by the parser with its own messages. Four of
  ToneMatcher's eleven hand-written messages and seven of Alfar's ten commands checked
  such a rule themselves. The owner had ruled groups out for 0.1.0; these two shapes are
  all the table takes.
- **Changed:** `value_of` returns `std::optional<std::string_view>`, not a pointer:
  cpp-policy's clang-tidy would not let the pointer be tested in an `if` without naming
  its type, and ToneMatcher wrapped it for that.
- `required_value(arguments, name)`: the value of a required option, without a fallback
  nobody means.
- **Changed:** a decimal number of more than 15 digits is refused, with how many it has,
  and no longer read to within a few units of its last place. Every number read is now the
  nearest double, the same on every system, so a program whose arithmetic must be bit for
  bit (ToneMatcher's `--scale`) can use the reader.
- `as_whole_number` and `as_decimal_number`: the number or nothing, for a command that
  writes its own message. Alfar called `whole_number` and threw its message away.
- `Command::listed`: how the program's help lists a command without forms ("export --out
  DIR"). Five of Alfar's commands had a `Form`, with the whole usage by hand and the summary
  twice, only to get that line.
- **Changed:** the usage written from a table has the words after the command's own options
  and before the shared ones ("alfar import FILE --project DIR"), as Alfar's read; and the
  message for a missing word follows it.
- A command's help shows its listed forms with their summaries when it has neither details
  nor a summary, so that a command with forms needs no third sentence.
- `run_command()`: what `run()` does once it has found the command, made public. Alfar's
  window runs one command again, and going through `run()` linked every command into it:
  270,000 bytes.
- The README says that exit code 2 is the library's on every command, and when the link to
  the library is `PUBLIC`. ToneMatcher's adoption found each of them missing.
- The example program's size budgets are 182272 bytes on macOS (was 164864), 184320 on
  Linux (was 166912) and 371712 on Windows (was 348160), approved by the owner. Each is the
  size measured in CI (173312, 174784 and 353792) plus 5%. The program grew by 17216, 16384
  and 22528 bytes for the changes above. By a link map of the macOS build, about half of
  the program is the library and half the example's own code, most of that the standard
  library's printing of decimal numbers, which the library does not use.

## 0.1.0 (2026-10-12)

- Created with cpp-policy's `tools/new-project.sh`.
- The library, in `lib/cli_cpparser/`: a command's table, `parse()`, typed readers of an
  option's value, help and usage written from the table, and a program's table of commands
  with its dispatch and exit codes. Alfar, ToneMatcher, SimpleLocalServer and
  GalleryOrganizer each had a parser of their own; this is their common part, with Alfar's
  rules, so that every program reads its command line the same way and the tests of
  argument reading are in one place.
- An example program in `src/` (`repeat`, `sum`, `pick`) in place of the template's
  greeting: it uses the library as a user would, and its tests check help text, messages
  and exit codes end to end.
- The example program's size budgets are 164864 bytes on macOS (was 109568), 166912 on
  Linux (was 93184) and 348160 on Windows (was 285696), approved by the owner: the old
  numbers were measured for the template's greeting program, which the example replaced.
  Each is the size measured in CI (156096, 158400 and 331264) plus 5%.
- Decimal numbers are read by the library's own code and not by `std::from_chars`: macOS
  has that function for a `double` only from version 26 on, and the first CI run could not
  build the library on GitHub's macOS. A user of the library builds it on systems this
  project never sees, so it uses nothing a system may lack.
