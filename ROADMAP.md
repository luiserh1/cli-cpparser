# Roadmap

Open work on cli-cpparser: what comes next, and why. Finished work goes to
`CHANGELOG.md`, with its reason, in the commit that finishes it. The layout
follows cpp-policy's POLICY.md, section 11.5.

## 0.3.0 (in progress)

**Goal:** a decimal number is read exactly whatever its length, so that a number another
program printed in full (17 digits) is not refused and ToneMatcher can drop its own reader.

### Scope

- [x] Upgrade to cpp-policy v0.36.0, when the owner says it is released. From v0.34.0 every
      Mac build is for macOS 26.0, which has `std::from_chars` for a `double`; v0.33.5 built
      for the machine's own system, and GitHub's macOS 15 lacks it.
- [ ] `decimal_number` and `as_decimal_number` read with `std::from_chars`, keeping the
      strict check of what a number is written with. The limit of 15 digits and the
      constant `most_decimal_digits` go.

### Out of scope

- Any other change to the interface: 0.2.0 was its one change from adoption.

### Validation

- [ ] The gate passes in CI on every system in `.github/workflows/ci.yml`.
- [ ] The release commit is tagged `v0.3.0` once its run is green.

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
