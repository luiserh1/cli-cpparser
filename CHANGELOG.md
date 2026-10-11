# Changelog

Each entry says what changed and why. The version is defined once, in
`project()` in `CMakeLists.txt`; each release is an annotated tag `v<version>`.

Policy: cpp-policy v0.33.5.

## Unreleased

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
- The example program's size budget on macOS is 164864 bytes (was 109568), approved by the
  owner: the old number was measured for the template's greeting program, which the example
  replaced. Linux and Windows follow once CI has measured them.
