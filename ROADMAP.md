# Roadmap

Open work on cli-cpparser: what comes next, and why. Finished work goes to
`CHANGELOG.md`, with its reason, in the commit that finishes it. The layout
follows cpp-policy's POLICY.md, section 11.5.

## Later

- [ ] A reader of whole numbers larger than `int` (`std::int64_t`), when a program has an
      option that needs one (a size in bytes).

## Not planned

- Short options and their bundles, binding results to variables, environment variables,
  configuration files, shell completion, colours: the library is worth having only while
  it stays small. A need for one of them goes to the owner first.
- Rules that belong to one program ("pieces, each once, 1 or more"): they stay in that
  program, built on the readers.
- `version` as a bare word: only `--version` prints it. A program may have a command of
  that name.
- An option that may have an empty value: an empty value is nearly always an unset shell
  variable. A program that wants "none" has a flag for it.
- Rules between options beyond the two the table has (one of several; one only with
  another): no nesting and no conditions. A command checks anything else and returns the
  message.
- A decimal number of more than 15 digits: reading it exactly at any length is several
  hundred lines for input nobody types, and reading it nearly is what 0.1.0 did and
  ToneMatcher could not use.
- Wrapping the help to the terminal's width: the width is the operating system's to say,
  and the library has no such code. A program breaks its long lines by hand.
