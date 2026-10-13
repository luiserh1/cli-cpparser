# Roadmap

Open work on cli-cpparser: what comes next, and why. Finished work goes to
`CHANGELOG.md`, with its reason, in the commit that finishes it. The layout
follows cpp-policy's POLICY.md, section 11.5.

## 0.4.0 (in progress)

**Goal:** Alfar's tables hold what its commands still check or write by hand. Alfar adopted
0.2.0 on the owner's condition that no message a user reads changes, and so it could not
use `Choice` and `Needs`, and two commands keep a `Form` only for one usage line.

### Scope

- [x] The program's own text on a rule between options, as `Option::missing` is for a
      required option: `Needs::message`, and `Choice::several` and `Choice::missing`. Empty:
      the library's sentence. Seven of Alfar's rules are checks inside four commands.
- [x] Two options that each need the other are written in one pair of brackets in the usage
      written from the table: `[--tonematcher PROGRAM --result OUT]`. The second is written
      beside the first. Alfar's `export` keeps a `Form` for that line.
- [x] `Words::shown`: how the usage writes a word, when the messages call it otherwise
      (`FILE|FOLDER` beside `FILE`). Empty: `Words::name`. Alfar's `import` keeps a `Form`
      for that line.

### Out of scope

- `NEEDS_POLICY` in `cpp_policy_library()` (cpp-policy v0.38.0): writing it requires v0.38.0
  of every user, where the library needs v0.34.0. It waits until it costs no project an
  upgrade.
- A release that grows a program much: on macOS `alfar` has 40,368 bytes of its size budget
  left and each release of the library has cost it 17,000 or more. The example's size is
  measured after each item.

### Validation

- [ ] The gate passes in CI on every system in `.github/workflows/ci.yml`.
- [ ] The release commit is tagged `v0.4.0` once its run is green.

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
- Wrapping the help to the terminal's width: the width is the operating system's to say,
  and the library has no such code. A program breaks its long lines by hand.
