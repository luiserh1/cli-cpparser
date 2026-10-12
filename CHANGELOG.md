# Changelog

Each entry says what changed and why. The version is defined once, in
`project()` in `CMakeLists.txt`; each release is an annotated tag `v<version>`.

Policy: cpp-policy v0.33.5.

## Unreleased

- The README says that exit code 2 is the library's on every command, and when the link to
  the library is `PUBLIC`; `Form`'s comment says that a form with only its usage is the
  usual case. ToneMatcher's adoption found each of them missing.

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
