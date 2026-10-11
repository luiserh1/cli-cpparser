# Roadmap

Open work on cli-cpparser: what comes next, and why. Finished work goes to
`CHANGELOG.md`, with its reason, in the commit that finishes it. The layout
follows cpp-policy's POLICY.md, section 11.5.

## 0.1.0 (in progress)

**Goal:** Alfar and ToneMatcher can replace their own argument parsers with the library.

### Scope

- [x] A command's table as a constant: its options, the words it takes, its forms.
- [x] One parse function, with Alfar's strict rules and messages that say what to write.
- [x] Required options, with the default message or the option's own.
- [x] Lists of options shared between commands, joined at compile time.
- [x] Help and usage written from the table; forms by hand where a table can't say them.
- [x] A program's table of commands: dispatch, `help`, `-h`, `--help`, `--version`,
      unknown command, and three exit codes.
- [x] Typed readers: a whole number and a decimal number in a range, a list with commas,
      one word of a set.
- [x] An example program in `src/`, tested end to end.

### Out of scope

- Short options and their bundles, binding results to variables, environment variables,
  configuration files, shell completion, colours: the library is worth having only while
  it stays small. A need for one of them goes to the owner first.
- Rules that belong to one program ("pieces, each once, 1 or more"): they stay in that
  program, built on the readers.
- Adoption in Alfar and ToneMatcher: it is those projects' work.

### Validation

- [ ] The gate passes in CI on every system in `.github/workflows/ci.yml`.
- [x] The owner approves the example program's size budgets, measured in CI.
- [ ] The release commit is tagged `v0.1.0` once its run is green.

## Later

- [ ] A reader of whole numbers larger than `int` (`std::int64_t`), when a program has an
      option that needs one (a size in bytes).
- [ ] A text appended to every hand-written usage line (`usage_tail`), if writing the
      shared options out in each line proves a burden in Alfar.

## Not planned

- `version` as a bare word: only `--version` prints it. A program may have a command of
  that name.
- An option that may have an empty value: an empty value is nearly always an unset shell
  variable. A program that wants "none" has a flag for it.
- Groups of options in the table ("one of `--piece` and `--all`"): the command checks them
  and writes its forms by hand.
