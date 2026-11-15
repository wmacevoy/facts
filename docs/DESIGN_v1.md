# Design — v1: facts

How the code satisfies `REQUIREMENTS_v3.md`, and what was rejected. Every mechanism here is
replaceable without renegotiating a requirement (SE1/R5). Rebuilt from the structure on 2026-11-14
by an agent (Claude Fable 5.2); reasons nobody wrote down are marked *(inferred)* (SE1/W12).

## Why this shape, and how sure

One core, with foreign models at edges, and reports that listen to the run. SE1/D14 asks that a
shape be read off the problem and written so it can be caught wrong, so here are its seven
questions, answered for facts.

| Ask of the problem | Here | Present? |
| ------------------ | ---- | -------- |
| Does anything change state, and who owns it? | A group goes from registered to included or excluded, then to passed or failed. One small record. | Present, small |
| Is the model ours, or someone else's? | Ours, except JUnit's XML, the operating system's processes and symbols, gdb's commands, and VS Code's test API. | Ours, with foreign edges. **Strong** |
| Do many cases differ only in data? | The command-line options, a little. | Weak |
| Are there rules people argue about? | No. The pattern matcher and the scan for `FACTS(Name)` are rules, but nobody disputes them. | **Absent** |
| Does a word change meaning somewhere? | "Check", "status", "facts", "skip", "fail", "test": see the glossary. But inside one small program with one author, not between groups of people. | Present as loose vocabulary, not as a border |
| More than one surface, or source? | Two report formats; two platforms; C and C++; an editor. | Present |
| Is history a requirement, or will outsiders extend it? | No history. Nobody extends it; many read its output. | Absent; the formats are published |

What this says. The foreign models are the strong force, and two of them differ by platform:
that is what an edge is for (REQ1/SHAPE-3). Two report formats over one run call for the run to
tell and the reports to listen (REQ1/SHAPE-2). The words that change meaning call for a glossary
that is kept (REQ1/SHAPE-1), and for nothing more: there is one author and one small model.

**The shape that was tried and withdrawn.** The owner first asked for a domain-driven design, and
the first draft, on 2026-11-14, had three models of our own (writing, registration, checking) with
a border between each. The question that separates that from this is the exemplar's: does a word
change meaning *between groups of people*? In the exemplar three departments argue over "loss".
Here the words are loose inside 800 lines by one author. Three bounded contexts without that
force are accidental complexity (SE1/D8), and it showed: the sections interleaved, and the check
between them could only search for words. What domain-driven design did give is kept: the
glossary, and the edges as anticorruption layers. The nearest well-known name for what is left is
ports and adapters.

**The rival** *(not yet written by someone who has not seen this; queue #2)*. Both shapes so far
came from one agent in one session, which is a single reading (SE1/D14).

**It stops fitting if** a second report format is never wanted and the Windows path is dropped
(the edges lose their force: then it is one plain file), or if the code grows fixtures, parallel
runs, and plug-ins (one core is then too few).

## Verdicts

The shape is *(judged)*: its holder passes it, and no test can (SE1/W11). One line each.

- REQ1/SHAPE-1 verdict: pass by Warren MacEvoy, 2026-11-14 ("accept shape", in session).
- REQ1/SHAPE-2 verdict: pass by Warren MacEvoy, 2026-11-14 ("accept shape", in session).
- REQ1/SHAPE-3 verdict: pass by Warren MacEvoy, 2026-11-14 ("accept shape", in session).

Given before a rival shape was written by a reader who had not seen this one (queue #2); a rival
that differs is new information and reopens them (SE1/H10). REQ1/MISSION-1 has no verdict yet:
its occasion is a class using facts.

## Components and the dependency rule

One library, one translation unit. Its parts are sections of `src/facts.c`, each opened by a line
`// ==== border: NAME ====`, because REQ1/WR-1 forbids more files. The four edges come first in
the file and the core last.

| Component | Where | Kind | Contract (one line) | Holds state? |
| --------- | ----- | ---- | ------------------- | ------------ |
| gdb edge | border `edge-gdb`; `FACTS_WATCH` in `facts.h` | edge | Spells the debugger commands printed on a fiction. | No |
| JUnit edge | border `edge-junit` | edge | Writes the run as JUnit XML; captures a group's output between *began* and *ended*. | Two temporary files, for one group at a time |
| Symbol edge | border `edge-os-symbols` | edge | Gives the group named N from the executable's symbols, or nothing. | No |
| Process edge | border `edge-os-process` | edge | Runs this program again as a child with `--facts_unsafe`; gives its exit code, or 2 for a crash, a timeout, or a failure to start. | No |
| Core: writing | `include/facts.h`; `FactsPrint` and the error helpers in `core`; `FactsTrace` in `facts.cpp` | core | The macros an author types. A fiction prints itself, marks its group failed, and counts. | The counts of truths and fictions; the trace stack (C++) |
| Core: registration | border `core` | core | Keeps the registered groups in order; includes and excludes by pattern; finds groups from source files. | The list of groups, and each one's selection |
| Core: checking | border `core` | core | `FactsCheck` runs each included group once and sets its outcome. The `FactsReport...` functions pass the four moments to the console report and, when asked for, the JUnit edge. | Each group's outcome |
| Core: command line | border `core` | core | `FactsMain` reads the options and calls the rest in the order given. | The report format |
| Editor extension | `vscode-facts/` | edge, separate program, **deprecated 2026-11-14** | Was to list groups from source, run the executable, and show outcomes in VS Code's test view. A VSCodium extension is to replace it (REQ3/ED-2). | A copy of the scan rule |

The rule: the core calls the edges, and an edge calls nothing in the core. Two things enforce
part of it (SE1/D10). The order of the file: C needs a function declared before it is used, so an
edge, coming first, cannot call a core function that is not in `facts.h`. And `test/borders.sh`:
JUnit's element names only in `edge-junit`, process calls only in `edge-os-process`, symbol
lookup only in `edge-os-symbols`, `(gdb)` only in `edge-gdb`, and each section opened once. A
word search cannot see everything; the holder judges the rest from the diff. Within the core,
the run loop does not test the report format (REQ1/SHAPE-2); nothing checks that but a reader.

## State

| State | Owner (authoritative) | Copies | Max staleness | While wrong, the user sees |
| ----- | --------------------- | ------ | ------------- | -------------------------- |
| The registered groups and their order | Core: registration (`head`, `tail`) | A `FACTS_REGISTER_ALL` block pasted from `--facts_register_all` | Until someone re-pastes it | A new group silently not run. Nothing says so. |
| A group's selection | Core: registration (`Facts.status`: include, exclude) | None | — | — |
| A group's outcome | Core: checking (`Facts.status`: pass, fail) | The same field as the selection | — | A group run twice in one process would be reported excluded the second time |
| Truths, fictions, and fictions evaluated twice | Core: writing (`facts_truths`, `facts_fictions`, `facts_twice`) | None; the exit code is computed from them | — | — |
| Report format | Core: command line (`facts_format`) | None | — | — |
| The captured output of one group | JUnit edge | None | — | Lost if the group crashes: the child dies with the capture, and the XML is never closed |
| Which source lines declare a group | The C preprocessor | Find's scan in `facts.c`; the extension's scan in `discovery.ts` | Never refreshed: two hand-written rules | A group in a macro, a comment, or a string is found or missed differently by each |
| The core's source | `include/facts.h`, `src/facts.c`, `src/facts.cpp` | `examples/*/` copies, made by `make examples` and committed | Until the next `make examples` | An example that behaves like an older release |

## Essential complexity

- Registration is a list the program builds, because C has no way to enumerate functions —
  REQ1/RG-1, REQ1/RG-2.
- The values of a fiction are printed with a format picked from the type by `_Generic` in C and by
  `operator<<` in C++, so the author never writes a format — REQ1/WR-2.
- `FACT(a,op,b)` has to print values whose types it was never told. In C that takes
  `__typeof__` and a block inside an expression, which gcc and clang have in every `-std` mode
  and the standard gained only in C23; `facts.h` uses them when they are there and falls back
  when they are not. In C++ a lambda holds both sides by reference — REQ1/WR-6, REQ1/WR-1.
- `FACT(a op b)` in C++ is taken apart by operator precedence: the macro writes
  `FactsDecompose() << a op b`, the shift keeps a reference to `a`, and the comparison keeps `b`
  and the answer. That is how both values are printed with one evaluation; C has nothing like it,
  so C keeps `FACT(a,op,b)` — REQ1/WR-5.
- `facts.cpp` includes `facts.c`, so C++ users compile one file and the C core has C linkage —
  REQ1/WR-1, REQ1/PK-1.
- Safe execution runs the program again, not just `fork`, because Windows has no fork and a fresh
  process is the same on both — REQ1/SX-1, REQ1/SX-2.
- The JUnit report redirects the process's own stdout and stderr to files, because the group's
  output must appear inside its case — REQ1/JU-1.
- `FactsRegisterAll` is a weak symbol, so the shared library links with no user code and a
  program's own block replaces it — REQ1/PK-1.

## Seams and how they fail

- **Reports.** A report that fails to open its temporary files writes the case with no captured
  output and says nothing.
- **Process edge.** Timeout 10 seconds, or `--facts_timeout=N`. On POSIX the parent sleeps until
  the child ends or an alarm rings, with both signals blocked except while it sleeps, so it
  neither polls nor misses one. A crash or a timeout loses everything the child had not
  flushed, and leaves a JUnit report unfinished (a scope decision).
- **Symbol edge.** Needs the executable to export its symbols (`-rdynamic` on Linux). Without it
  every lookup fails and each group is named as not found.
- **Editor extension.** Deprecated. It looks for lines that begin `PASS` or `FAIL`, which the
  console report has never printed (finding 2026-11-14). Its replacement reads a real report
  from `expected/` in its first test (queue #18).

## Five likely changes, and where each should land

Written before any of them, so the shape can be caught wrong (SE1/D14). After each real change,
tag its edits *essential*, *shape*, or *incidental* in the commit.

1. **A third report format (TAP, or JSON for the extension).** A new edge section and a line in
   each `FactsReport...` function. If `FactsCheck` changes, REQ1/SHAPE-2 is not holding.
2. **JUnit's failure element should carry the fiction's text, not "See stdout".** Awkwardly. The
   fiction's line is printed by a macro in `facts.h`, and no report is told about it: the moments
   are per group, not per fact. This needs a fifth moment, *fact was fiction*, and the macros in
   `facts.h` (published) would call it. The shape's price, named in its Exceptions.
3. **Per-group timeout or per-group isolation.** The process edge and `FactsMain`; and a scope
   decision is reversed (one child for the whole run), so a new requirements version first.
4. **Windows gains a faster or different way to wait for the child.** The process edge only. An
   edit anywhere else says REQ1/SHAPE-3 is not holding.
5. **Separate "excluded" from "passed/failed" in the group's record.** The core and `facts.h`: a published struct, so a major version. The reports change only where they read
   the field.

## One-way doors

Each of these is published, so each needs the owner before it changes (SE1/D11). None has had a
review by someone who did not share the author's assumptions.

- `facts.h`: the macros and the `Facts` struct. The shared library's soname carries the major
  version.
- The command-line options and the exit codes.
- The console report, byte for byte (`expected/`), since the editor extension and people's
  scripts read it.
- The JUnit report's element layout.
- The symbol names `facts_<Name>_function` and `facts_<Name>_data`, which find and the gdb hints
  both spell.

## Decisions

- **One core and four edges, not three models of our own** — REQ1/SHAPE-1, REQ1/SHAPE-3 — see
  *Why this shape*. Rejected: bounded contexts for writing, registration, and checking; no group
  of people stands behind each. Source: the owner, 2026-11-14, on the agent's recommendation.
- **Sections in one file, edges first, with a border check** — REQ1/WR-1, REQ1/SHAPE-3 — more
  files would end the two-file drop-in. Rejected: split sources with a generated single file (a
  build step and a generated file to keep honest), until the file is several times this size.
  Source: this adoption, 2026-11-14.
- **The run tells; reports listen** — REQ1/SHAPE-2 — before 2026-11-14 the run loop tested the
  format on every step and the JUnit capture was written through it. Rejected: a table of report
  function pointers (an interface with two implementations and no third in sight, SE1/D8).
  Source: this adoption; the golden files did not change.
- **The console's per-group lines print under JUnit too** — REQ1/JU-1 — they are captured into
  the case, which is how the published XML already looks. *(inferred)*
- **The expression form splits on `<<`, and on `<=` with clang** — REQ1/WR-5 — each compiler
  warns about the other spelling at every `FACT` under its default or `-Wall` warnings (gcc:
  parentheses around a comparison; clang: `<<` against a comparison), and neither can be
  silenced from inside a macro: gcc 12 refuses a pragma inside an expression, and clang's pragma
  does not reach a warning located in the macro's argument (both tried, 2026-11-14). The price:
  `FACT(x << 1 == 4)` compiles with clang and not with gcc, so the README says to parenthesize.
  Rejected: marking `facts.h` a system header (hides every warning in it). Source: this session,
  and the macOS CI run that showed the first attempt failing.
- **The footnote is written without a stream name**, `FACT(a == b, "i=" << i)` — REQ1/WR-5 — the
  same as `FACTS_TRACE` here and as ArduinoUnit, the owner's sibling project. Source: this
  session; the owner has not yet said which he meant.
- **A fiction in the expression form prints `a == b {1 == 2}`** — REQ1/WR-5 — the macro has the
  expression as one text and does not split it, so the values follow it in one pair of braces.
  The three-argument form keeps `a {=1} == b {=2}`. Source: the owner, 2026-11-14, who found
  `{=1 == 2}` odd.
- **The printed `(gdb) run` line carries `--facts_unsafe`** — REQ1/WR-3, REQ1/SX-3 — without it
  the facts run in a child and no breakpoint is reached (finding 2026-11-14). A change to a
  published line, in every golden file. Source: the owner, 2026-11-14.
- **Detect the compiler; do not require C23** — REQ1/WR-6, REQ1/WR-1 — C23 is too new to ask of
  a classroom machine. Rejected: requiring it; leaving the double evaluation everywhere. The cost
  is a rule that differs by compiler, which is why the run itself says when it applied. Source:
  the owner, 2026-11-14.
- **The fallback counts in a new global, `facts_twice`** — REQ1/WR-6 — a symbol added to the
  library, so a program built with this header needs this library; it is used only by the
  fallback. Rejected: a spare bit of `facts_format` (a second meaning for a word, REQ1/SHAPE-1).
  `FACTS_EVAL_TWICE` forces the fallback so the check can run it on gcc. The JUnit report does
  not carry the warning. MSVC is treated as unable, untried. Source: this session.
- **`FactsMainIf` carries the program's switch** — REQ2/CK-5 — `--fact*` is reserved, and the
  switch given to `FACTS_MAIN_IF` (`--facts` in the README) begins that way, so facts has to be
  told which one argument to let through. A second new symbol after `facts_twice`. The price: a
  program built with an older `facts.h` calls `FactsMain`, so if it is linked to this shared
  library its own `--facts` switch is refused until it is rebuilt. Rejected: reserving only
  `--facts_` (the owner said `--fact*`). Source: the owner, 2026-11-14.
- **Exit code 2 means "no result to trust"** — REQ2/CK-3, REQ2/CK-5, REQ2/SX-4 — an unknown
  reserved option, a refused timeout, and a run that checked no fact join crash and timeout,
  so a CI job with a typo no longer passes. The count of facts decides, not the count of
  groups: a group with no `FACT` in it checks nothing. Source: the owner, 2026-11-14; the code's
  own comment before 2.0 said "2 means no facts were checked".
- **A torn JUnit report stays torn** — REQ1/JU-1, REQ1/SX-1 — after a crash or a timeout the
  child is gone and the XML has no closing elements. Rejected: the parent closing or replacing
  the report (it would have to know the format, which is the JUnit edge's, in the process
  edge). Exit code 2 and stderr carry the news. Source: the owner, 2026-11-14.
- **Find is read before the other options** — REQ1/RG-3 — so an include or exclude applies to
  what find registered wherever it is written; and "the first include" is remembered, not read
  off an empty list. Source: queue #5.
- **The parent waits on signals, not on a clock** — REQ1/SX-2 — it used to look once a second, so
  every run cost a second. Rejected: looking every 10 ms (still polling; the owner asked for
  the clean version); having the child set its own alarm (a group that uses `alarm` would
  cancel its own timeout). The parent installs handlers for `SIGCHLD` and `SIGALRM` only while
  it waits, and it is the wrapper process, which runs nothing else. Windows already waited on
  the process handle. Source: the owner, 2026-11-14, queue #3.
- **Two test lists, one per build system** — REQ1/PK-2 — `test/check.sh` for `make check` and
  `facts_expect` cases in `CMakeLists.txt` for ctest, which is all Windows has. The second is
  the smaller, and checks an exit code and one line where the first compares whole reports: the
  golden files hold POSIX paths. Rejected: one list (it would need a shell on Windows, or CMake
  on every Unix). The cost is two places to add a case. Source: queue #10.
- **Explicit registration and find, no memory search** — REQ1/RG-2 — linkers reorder data.
  Source: commit `177d015`.
- **Safe execution by re-running the program, on by default** — REQ1/SX-1 — source: commit
  `ece1a23`.
- **One status field for selection and outcome** — kept, though it gives "status" two meanings
  (REQ1/SHAPE-1) — the struct is published. *(inferred)*; see the requirements' Deferred.

## Deferred

- Not yet: a footnote on the three-argument form, or any footnote in C — C has no lambda; the
  trigger is someone asking.
- Not yet: a *fact was fiction* moment (change 2) — nobody has asked for it; the trigger is a
  JUnit or editor user who needs the text.
- Not yet: one scan rule shared by find and the extension — two languages; the trigger is the
  first bug report where they disagree, or a table of cases both are tested against.
