# Findings

What we learned the expensive way: surprises, dead ends, measurements. The only document reality
writes (SE1/W10). Blameless: say what would have found it, not who should have asked (SE1/H12).

## 2026-11-14 — The editor extension reads a format the library never printed

- **What happened:** `vscode-facts/src/runner.ts` looks for lines beginning `PASS` or `FAIL`. The
  console report prints `facts check Out passed`. So every group comes back "Test did not produce
  output". Read from the code, not run in VS Code.
- **Kind:** K3 (the extension builds and packages in CI, which read as "works").
- **Cost:** unknown; the extension has shipped since 2026-03-05 in this state as far as the code
  shows.
- **Findable by:** one test that feeds the parser a real golden file from `expected/`.
- **Changed:** queue #1; `docs/DESIGN_v1.md` names the console report a one-way door with the
  extension as a reader.

## 2026-11-14 — Every safe run takes one second

- **What happened:** `make check` took 33 seconds for about sixty short runs. The parent in safe
  execution asks whether the child has ended, then sleeps a whole second before asking again:
  measured 1.001 s with safe execution and 0.002 s with `--facts_unsafe` for the same group.
- **Kind:** K2.
- **Cost:** a second per run for every user since 2.0; half a minute per check here.
- **Findable by:** a speed line in the operating envelope, and timing one run.
- **Changed:** queue #3; the envelope records the measurement.

## 2026-11-14 — A new test that could not fail

- **What happened:** the first version of the find case in `test/check.sh` compared the program's
  output with itself (`diff - -`). It passed, and it still passed with symbol lookup broken.
- **Kind:** K3, an hour old.
- **Cost:** minutes, because the code was broken on purpose before the test was trusted.
- **Findable by:** seeing the test red first (SE1/T3), which is what found it.
- **Changed:** the case now compares against the expected text and was seen to fail. Seen red by
  breaking the code: the exit code, the pattern matcher, symbol lookup, both border rules, a
  console line, and JUnit escaping. Not seen red: the `--facts_unsafe` case.

## 2026-11-14 — Include after find narrows nothing

- **What happened:** `--facts_find test/testfacts.c \; --facts_include=Out` runs all six groups.
  Include empties the selection first only when nothing is registered yet, and find has already
  registered.
- **Kind:** K2.
- **Cost:** none known; the README's one-line workflow uses find, so a user narrowing a run that
  way gets every group and no warning.
- **Findable by:** a case combining the two options.
- **Changed:** queue #5; the requirements' inputs table.

## 2026-11-14 — A fluent shape, written and withdrawn the same day

- **What happened:** asked for a domain-driven design, the agent wrote three bounded contexts for
  an 800-line library with one author, with a border check between them, and everything passed.
  Asked afterwards whether it helped, the agent answered that only the edges, the listening
  reports, and the glossary had a force behind them. The owner took that; the shape is now one
  core and four edges.
- **Kind:** K3, caught in hours: the shape arrived confident and nothing checked it (SE1/D14).
- **Cost:** one extra pass over `src/facts.c`, the border check, and three documents.
- **Findable by:** asking of each shape item, before writing code, "which fact about the problem
  calls for this?" The design's own table said the force was weak; the build went ahead anyway.
- **Changed:** REQ1/SHAPE-1 to SHAPE-3 rewritten in place (v1 was an unconfirmed draft);
  `docs/DESIGN_v1.md` records the withdrawn shape and why; `test/borders.sh` checks edges only,
  and that each section opens once. Seen red on broken copies: a JUnit word in the core, a
  section opened twice, an edge's marker removed. Queue #2 still asks for a rival nobody here
  wrote.

## 2026-11-14 — The printed debugger hint had not worked since safe execution

- **What happened:** a fiction prints three `(gdb)` lines to paste. With the `run` line as printed
  since 2.0, gdb 13.1 set the breakpoint, the program exited with code 1, and the breakpoint was
  never reached: the facts ran in the child that safe execution starts. With `--facts_unsafe`
  added, gdb stopped at the watch with `n == 100`.
- **Kind:** K3: the hint is in every golden file, so every check confirmed its text and none its
  effect.
- **Cost:** unknown; any user who pasted the hint since 2026-03-05 got no stop and no reason.
- **Findable by:** pasting the hint into a debugger once (SE1/T11: gdb is the only judge of its
  own commands). No debugger was installed on the machine the check ran on.
- **Changed:** the hint now prints `run --facts_unsafe --facts_include=Name`; `expected/` was
  regenerated and every changed line is that one. No test runs gdb: the check still confirms the
  text only (queue #13 could add one where gdb exists).
