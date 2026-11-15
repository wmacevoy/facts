# Contributing

How to work on this repository, for a new teammate and for an agent, who both arrive knowing
nothing (SE1/A3). Keep it short and true: a change that makes a line here untrue updates it in the
same change (SE1/W11). Principle IDs such as `SE1/A3` are defined in `vendor/se1/`; a grep for
`SE1/A3.` there lands on the definition.

## Run it

- **Prerequisites:** a C11 compiler, a C++11 compiler, `make`, and a POSIX `sh` with `awk` and
  `diff`. Linux or macOS for the Makefile; Windows builds with CMake (3.14 or later). Nothing else.
- **Build:** `make` builds `libfacts` and `libfacts++`, static and shared.
- **Run:** `make examples && examples/hello_c/hello`. You should see two groups checked, one
  fiction printed in red with two `(gdb)` hints, and `3 (75.0%) truths and 1 (25.0%) fictions
  checked.`
- **Reset to a known state:** `make clean`. It removes objects, libraries, `bin/`, and the example
  programs; under a second. It does not undo edits to `expected/`: use `git checkout expected/`.

## Check it

- **The full check:** `make check`, the same one CI runs (SE1/O3). It builds the test programs and
  the examples, then runs `test/check.sh`.
- **Success prints:** `facts: all checks passed`
- **Failure prints:** one `FAIL:` line per broken case, then `facts: CHECK FAILED`, and exits 1.
- **What it runs:** the golden files in `expected/` (the console and JUnit formats, byte for byte),
  exit codes, include and exclude, find, and `test/borders.sh`. Each case in `test/check.sh` names
  its requirement.
- **Seeing one fire:** change `facts check started` in `src/facts.c` and run it; add the word
  `testcase` to a comment outside the `edge-junit` section and run it (SE1/T3).
- **Time:** about 6 seconds; two of them are the timeout cases waiting out their one second each.
- **Not in the check:** install layout, Debian, Alpine, Homebrew, Windows, and the VS Code
  extension build run only in CI (`.github/workflows/c-cpp.yml`).
- **On Windows** CI runs `ctest`, not this script. Its cases are in `CMakeLists.txt`, each an
  exact exit code and a piece of output checked by `test/expect.cmake`. To run them anywhere:
  `cmake -B build -DFACTS_BUILD_TESTS=ON && cmake --build build && ctest --test-dir build`.

## Rules for this repository

Constraints the code cannot show.

- **The core is two files a user copies:** `include/facts.h` and `src/facts.c`, plus
  `src/facts.cpp` for C++ (REQ1/WR-1). Do not split them, and do not add a dependency.
- **`src/facts.c` is divided by borders.** A line `// ==== border: NAME ====` opens a section:
  four edges first (`edge-gdb`, `edge-junit`, `edge-os-symbols`, `edge-os-process`), then `core`.
  Code that spells someone else's model goes in that model's edge, and an edge calls nothing in
  the core. `test/borders.sh` vetoes the crossings a word search can see (REQ1/SHAPE-3).
- **`examples/*/facts.h`, `facts.c`, `facts.cpp` are copies** made by `make examples`. Edit the
  originals in `include/` and `src/`, never the copies. They are committed so an example can be
  downloaded alone.
- **`expected/` is the published output format.** `make expected` rewrites it from whatever the
  code prints now, so it can bless a mistake. Run it only for an intended format change, and read
  the diff before keeping it.
- **`facts.h`, the command-line options, the console and JUnit formats, and the exit codes are
  published.** A change to any of them is a stop-and-ask (`docs/DESIGN_v1.md`, one-way doors).
- **The version number lives in six files.** `PACKAGING.md` lists them under *Versioning*.
- **`vendor/se1/` and `AGENTS.md` are copies of the guide; do not edit them.** Local rules go
  here, and this file wins where they disagree.
- **A change in a requirement's meaning is a new version** (`docs/REQUIREMENTS_v4.md` next). Cite
  a requirement by the version that last changed it: `REQ1/WR-2`, `REQ2/CK-3`.

## Where things are

| AGENTS.md says                 | In this repository                                    |
| ------------------------------ | ----------------------------------------------------- |
| the repository guide           | `CONTRIBUTING.md` (this file)                         |
| the requirements               | `docs/REQUIREMENTS_v3.md`                             |
| the design                     | `docs/DESIGN_v1.md`                                   |
| the queue                      | `docs/QUEUE.md`                                       |
| findings                       | `docs/FINDINGS.md`                                    |
| the check command              | `make check`; success prints `facts: all checks passed` |
| the handoff check              | `./se --post`, run by a `SessionStart` hook in `.claude/settings.json` at startup, resume, and compaction |
| `Non-goals`, `Scope decisions` | sections of `docs/REQUIREMENTS_v3.md`                 |
| the glossary                   | the first section of `docs/REQUIREMENTS_v3.md`        |

- **The guide:** `vendor/se1/`, copied on 2026-11-14 from a local checkout at `v1.7-4-g98c4756`:
  release v1.7 plus four unreleased commits, which add the draft of SE1/D14 that the fundamental
  shape here follows. `AGENTS.md` is v1.7's text unchanged. Re-copy at the next release (ADOPT.md
  step 1, in the guide's repository). The exemplars the shape was weighed against are in that
  repository under `exemplars/shapes/` and are not copied here.
- **`./se --doctor`** says what the adoption still lacks. `se`, `se.ps1`, and `se.py` are the
  guide's, unedited.
- **For users of facts:** `README.md`, and one page each in `QUICKREF-C.md` and `QUICKREF-CC.md`, for
  people and for agents. Their examples were compiled and run; a change to a macro, an option, an
  exit code, or the report updates them in the same change.
- **Packaging:** `PACKAGING.md`. **The editor extension:** `vscode-facts/`, deprecated; leave it alone.
- **Done** means: the item's *done* sentence is met, `make check` printed its success line, a test
  naming the requirement was seen to fail without the change, what the change made untrue in
  `docs/` and here is updated, and a human has read the diff. Only a human marks a queue row done.
