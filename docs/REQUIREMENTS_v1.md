# Requirements — v1: facts

What must be observably true, and what is not being built. Changes only with the client; a change
in meaning is a new version (`REQUIREMENTS_v2.md`); a spelling or lint fix is not (SE1/R10).

**Rebuilt from the code, the README, and the git history on 2026-11-14 by an agent (Claude Fable
5.2), at the owner's request (SE1/W12). Everything here is *(inferred)* until the owner confirms
it: nobody stated these as requirements, and a reconstruction describes what the code does, which
is not always what was meant.** Where the code and a requirement below disagree, the queue has a
row.

Client: Warren MacEvoy, author and maintainer. The users are students and other beginners who are
never in the room; the proxy for them is the issue tracker at github.com/wmacevoy/facts and what
the owner sees in class *(inferred)* (SE1/H5) · Date sent: 2026-11-14 · *Tell us what is wrong
with it.*

## Glossary

"Facts", "check", "status", "skip", "fail", and "test" each mean more than one thing in the code
and its output today. Below they are never used bare.

| Term | Area | Meaning |
| ---- | ---- | ------- |
| Fact | writing | One relation the author says is true: `FACT(a,op,b)`. |
| Truth | writing | A fact that held when it was checked. |
| Fiction | writing | A fact that did not hold. The output says "is fiction". |
| Group | writing | A named block of facts: `FACTS(Name) { ... }`. The output calls it "facts check Name"; the README calls it a group. |
| Trace | writing (C++) | A note left by `FACTS_TRACE`, printed if a later fact in its scope is fiction. |
| Watch | writing (C++) | A trace whose note is a gdb breakpoint condition: `FACTS_WATCH(i,j)`. |
| Footnote | writing (C++) | A note given with one fact, `FACT(a op b, note)`, printed only if that fact is fiction. The owner's word, 2026-11-14. |
| Registered | registration | Said of a group the program knows about. Registered groups have an order. |
| Find | registration | Registering the groups a source file declares, by reading the file and looking each name up in the executable: `--facts_find`. |
| Included, excluded | registration | Whether a registered group will be run. A group is excluded by its declaration (`FACTS_EXCLUDE`) or by an option. |
| Run | checking | One pass over the registered groups, in order, running the included ones. The code calls it `FactsCheck`. |
| Evaluated twice | writing | Said of a fiction whose two sides were computed a second time so their values could be printed. |
| Outcome | checking | What a run found for a group it ran: *passed* (no fiction) or *failed* (at least one). A group not run has no outcome. *(new word)* |
| Core | all | Everything that is facts' own: writing, registration, and checking. *(new word)* |
| Edge | all | The one place that spells a model belonging to someone else: JUnit, gdb, the operating system, VS Code. *(new word)* |
| Console report | checking | The lines a run prints for a person: started, ended, ended badly, excluded, then the summary. |
| JUnit report | the JUnit edge | The same run written in JUnit's XML for a CI system: `--facts_junit`. Its words are JUnit's. |
| Safe execution | the process edge | Running the whole run in a child process, so a crash or a hang is reported and does not take the caller down. The default. |
| Abnormal end | the process edge | A run that crashed, timed out, or could not start. Not an outcome: no group passed or failed. *(new word)* |
| Exit code | all | 0: a run ended and found no fiction. 1: it found at least one. 2: abnormal end. |
| Drop-in | all | Using facts by copying its two or three source files beside your own. |

## Mission

- **MISSION-1** *(since v1)* *(judged)* *(inferred)* A beginner can say what should be true of
  their C or C++ code in one line, and when it is not true, the output tells them where to look
  and how to look. Judge: the owner, watching a class use it in its first lab, each term. Veto: a
  student who cannot get from a fiction to the line that caused it without help.

## Fundamental shape

Held by: Warren MacEvoy. The holder proposes a change; as client he also makes it, in a new
version. History, all on 2026-11-14: the owner asked for a domain-driven design after the guide's
exemplar; the first draft had three models of our own; the agent then advised that only the edges
and the vocabulary had a force behind them, and the owner took that recommendation. What is
below is the smaller shape: one core, foreign models at edges. The owner accepted it the same day
(verdicts in `docs/DESIGN_v1.md`), so its items are no longer marked *(inferred)*. `docs/DESIGN_v1.md` gives the
forces and what would show it wrong.

- **SHAPE-1** *(since v1)* *(judged)* One core with one vocabulary: each word in the
  glossary means one thing, in the code's names, the options, and the output. Force: "check" is
  one fact, one group's run, the whole run, and a make target; "status" is a selection before the
  run and an outcome after it; "skip" is an option and a JUnit mark; a beginner reads these
  words. Refuses: a new option, macro, or output line that gives a glossary word a second
  meaning; a second model of our own. Judged from the glossary against each diff's new names.
- **SHAPE-2** *(since v1)* *(judged)* What a run tells a report is four named moments:
  run began, group began, group ended, run ended. A report reads the group's name, place, and
  status; it changes nothing. Force: REQ1/CK-1, REQ1/JU-1; a new report format must not be able to
  move an outcome or a count. Refuses: the run asking which format is wanted; a report that sets a
  status. Judged from where a diff lands: a new report format adds a section and edits the
  dispatcher, and the run loop does not change.
- **SHAPE-3** *(since v1)* *(judged)* Some models are someone else's, and each one's
  spelling stays at one edge: JUnit's XML; the operating system's processes and signals; its
  symbol lookup; gdb's command language; and, outside the library, VS Code's test API. Each
  platform's spelling is inside its edge. An edge calls nothing in the core. Force: each changes
  without asking us, and two differ by platform (REQ1/JU-1, REQ1/SX-1, REQ1/ED-1). Refuses:
  their words in the core; a second place that spells them. Judged by searching for those words
  (`test/borders.sh` vetoes for `src/facts.c`) and from the order of the file.
- **Foundation:** SE1.
- **Says no to:** splitting the core into more files (it would end drop-in, REQ1/WR-1); a
  dependency of any kind; finding groups by searching memory or by linker sections (removed in
  2.0, see Scope decisions); a plug-in interface for reports (nobody outside extends this);
  separate models for writing, registration, and checking (tried and withdrawn, see the design).
- **Exceptions:** a group's name, file, and line mean the same to every model, and all share
  them. The FACT macros print a fiction's line themselves and `FACTS_WATCH` spells gdb's words in
  `facts.h`: a macro has to expand where the author wrote it.

## The pain

A student's first C program has no tests, because the test frameworks they find want a build
system, a package manager, or C++ they have not met. When they do write a check and it fails, it
says "assertion failed" and stops, or the program crashes and the terminal shows nothing useful.
The instructor gets the call, and what the student says is "it doesn't work." *(inferred from the
README's "beginner-friendly" and "without build system friction", and from safe execution's
commit message.)*

## People this touches

Students writing their first C or C++ (the users). The instructor who grades or debugs with them.
Anyone reading the JUnit report in a CI system. Packagers for Debian, Alpine, Homebrew, and
Chocolatey. The maintainer. Treated differently: users on Windows, who get the CMake build only
and whose path is tested less (see Not yet answered).

## Requirements

All *(inferred)*; the mark is not repeated on each line. Numbers are the code's.

**Writing**

- **WR-1** *(since v1)* A program uses facts by copying `facts.h` and `facts.c` beside its sources
  (and `facts.cpp` for C++), with a C11 or C++11 compiler and no other library or tool.
- **WR-2** *(since v1)* A fiction prints its file, group, and line, both expressions as written,
  and both values, then ends its group. `FACT_CHECK` prints the same and continues, giving 0.
- **WR-3** *(since v1)* In C++, when a fact is fiction every trace still in scope prints,
  innermost first, and a watch prints a gdb breakpoint line carrying the watched values.
- **WR-4** *(since v1)* `FactsAbsErr` and `FactsRelErr` give the absolute and the symmetric
  relative error of two doubles, so a floating-point fact can state its tolerance.
- **WR-5** *(since v1)* In C++, a fact can be written as the comparison itself, `FACT(a op b)`: a
  fiction prints the expression and both values, and each side is evaluated once whether it is a
  truth or a fiction. A footnote, `FACT(a op b, note)`, is printed with the fiction and is not
  evaluated for a truth. `FACT(a,op,b)` keeps working in C and C++. *Not inferred: asked for by the owner on 2026-11-14 ("a feature, maybe"); a draft
  until he has used it.*
- **WR-6** *(since v1)* `FACT(a,op,b)` evaluates each side once in C++, and in C wherever the
  compiler can keep a value whose type it was not told (gcc and clang can). With any other C
  compiler a fiction evaluates each side a second time to print it, and when that happened in a
  run, the last line of the console report is a warning saying how many times. A program must
  not need C23, or anything past C11, to use facts. *Not inferred: the owner, 2026-11-14.*

**Registration**

- **RG-1** *(since v1)* Groups run in the order they were registered, and a group registered twice
  runs once.
- **RG-2** *(since v1)* `--facts_find FILE... [;]` registers every group those files declare that
  is in the executable, in the files' order. A declared group that is not in the executable is
  named on stderr, and the rest still register.
- **RG-3** *(since v1)* `--facts_include=PATTERN` runs only the groups whose names match, adding
  to any earlier include; `--facts_exclude=PATTERN` removes matching groups. `*` in a pattern
  matches any run of characters. An excluded group is reported as excluded, never silently
  dropped.
- **RG-4** *(since v1)* `--facts_register_all` prints the registered groups as a
  `FACTS_REGISTER_ALL` block to paste into a source file, and runs nothing.

**Checking**

- **CK-1** *(since v1)* The console report says for each registered group, in order, that it
  started and ended, ended badly, or was excluded; then lists the groups passed, failed, and
  excluded; then the counts of truths and fictions, each with its percentage; then the warning of
  REQ1/WR-6, if it applies.
- **CK-2** *(since v1)* `--facts_plain` prints the same report with no color codes.
- **CK-3** *(since v1)* The exit code is 1 when any fact was fiction, 0 when none was (also when
  no group ran), and 2 on an abnormal end.
- **CK-4** *(since v1)* With `FACTS_MAIN_IF(--switch)`, the program runs its own main unless the
  switch is given, and runs the facts when it is.

**The JUnit edge**

- **JU-1** *(since v1)* `--facts_junit` writes one suite with one case per registered group.
  Whatever the group printed to stdout and to stderr is inside its case, escaped. An excluded
  group is marked skipped, and a failed one is marked a failure.

**The process edge (safe execution)**

- **SX-1** *(since v1)* When a group crashes, the caller's shell survives; stderr names the signal
  and the exit code is 2.
- **SX-2** *(since v1)* A run still going after 10 seconds, or after N with `--facts_timeout=N`,
  is killed; stderr says it timed out and the exit code is 2.
- **SX-3** *(since v1)* `--facts_unsafe` runs in the calling process, for use under a debugger,
  and reports the same.

**Packaging and the editor**

- **PK-1** *(since v1)* `make install` and the CMake install each put the header, a static and a
  shared library for C and for C++, and a pkg-config file for each where the platform expects
  them. A minor or patch release replaces the shared library without relinking.
- **PK-2** *(since v1)* The libraries build and the check passes on Linux and macOS; the libraries
  build and the tests run on Windows.
- **ED-1** *(since v1)* In VS Code, the extension lists the groups declared in the workspace's
  sources and, after a run, shows each one's outcome and takes the user to the line of a fiction.

## Inputs: absent, late, wrong, lying

What the code does today. "?" marks behavior nobody seems to have chosen; each has a queue row.

| Input | Absent | Late | Wrong | Lying |
| ----- | ------ | ---- | ----- | ----- |
| Registered groups | None registered: the run reports nothing, exit code 0. ? | — | Registered twice: runs once (REQ1/RG-1). | — |
| A file given to find | Cannot be opened: named on stderr, the rest continue. | — | Declares no groups: nothing registers, nothing said. | Declares a group not in the executable: named on stderr (REQ1/RG-2). A name over 255 characters is dropped silently. ? |
| A pattern | — | Include given after find: narrows nothing, every group runs. ? | Matches nothing: every group is excluded, exit code 0. ? | — |
| An option | No options: run everything registered. | — | Misspelled `--facts_` option: ignored silently. ? | `--facts_timeout=abc`: read as 0, and the run is killed at once. ? |
| A group's code | — | Hangs: killed at the timeout (REQ1/SX-2). | Crashes: reported (REQ1/SX-1). | A fact with a side effect (`++x`): happens once, except in C on a compiler that cannot keep the value, where a fiction makes it happen twice and the report warns (REQ1/WR-6). |
| The child process | Cannot be started: said on stderr, exit code 2. | Ends in under a second: the parent still waits out the second. ? | Exits with a code above 1: passed through. | — |

## Operating envelope

- **Scale:** one program, tens of groups, hundreds of facts. Not designed for thousands of groups
  or for running them in parallel.
- **Speed:** *not stated.* Measured: a safe run takes at least one second whatever it checks,
  because the parent looks at the child once a second (queue #3).
- **Recovery:** nothing is kept between runs, so there is nothing to recover.
- **Resources:** no allocation while a group runs, except in the report of a fiction. The pattern
  matcher uses memory in proportion to the pattern's length, not the name's.
- **Interruption:** not examined: what Ctrl-C does to the parent and the child (queue #9).

## Security, law, and ethics

Facts runs the author's own code with the author's own rights, and adds no network, no file
writes beyond temporary files for the JUnit report, and no data about people. Find reads only the
files named on the command line and looks up only symbols named `facts_<Name>_data`. The JUnit
report escapes `<`, `>`, `&`, and `"`. MIT license; no code from another project is known to be
in it, and the two links in the source cite Stack Overflow answers for an idea, not copied code
*(inferred)*.

## Assumptions (K4s), read back to the client

- "We're assuming the two-file drop-in is the property everything else gives way to. True?" — not
  yet asked
- "We're assuming `Facts.status` holding both the selection and the outcome is history, not
  intent, and that it stays as it is because the struct is published. True?" — not yet asked
- "We're assuming the console format in `expected/` is published: people and tools read it, so a
  change is a decision. True?" — not yet asked
- "We're assuming the VS Code extension is meant to work today, not a sketch. True?" — not yet
  asked

## Scope decisions

- Groups are registered explicitly, or found from named source files. Not by searching memory:
  linkers reorder data and groups were silently missed — commit `177d015`, 2026-03-05.
- Safe execution is the default and `--facts_unsafe` is the exception — commit `ece1a23`; a
  beginner's crash should be a message *(reason inferred)*.
- The whole run is one child process, not one per group — the code; *(reason inferred)*: one
  fork, and output order stays the program's.
- A fiction ends its group but not the run — the code and README.
- The C++ library contains the C core (`facts.cpp` includes `facts.c`); a program links one,
  never both — `PACKAGING.md`.
- The summary counts facts, and the report names groups — the code.

## Non-goals

All *(inferred)* from their absence.

- Not fixtures, setup and teardown, or mocks — a beginner's first tests do not need them.
- Not parallel or per-group isolated runs — see Scope decisions.
- Not a matcher or assertion vocabulary beyond `FACT(a,op,b)` — one form to learn.
- Not a report plug-in interface — two formats, both ours to write.
- Not test discovery without naming source files — removed in 2.0 for cause.

## Not yet answered

- Is exit code 0 right when no group ran, or matched? The README says so in passing; a CI job
  with a misspelled include passes. — the owner, before 2.1
- Is safe execution on Windows tested anywhere? CI builds and runs the tests there under
  `WILL_FAIL`, which also passes if they crash. — the owner
- Should an unknown `--facts_` option be an error? — the owner
- Who uses the JUnit report, and with which CI? Its failure text is "See stdout". — the owner

## Deferred

- Not yet: splitting `Facts.status` into a selection and an outcome — it changes a published
  struct; the trigger is the next major version.
