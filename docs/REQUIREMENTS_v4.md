# Requirements — v4: facts

What must be observably true, and what is not being built. Changes only with the client; a change
in meaning is a new version (`REQUIREMENTS_v5.md`); a spelling or lint fix is not (SE1/R10).

Rebuilt from the code, the README, and the git history on 2026-11-14 by an agent (Claude Fable
5.2), at the owner's request (SE1/W12), and **accepted by the owner on 2026-11-14** ("accept 2 &
8"). Nothing here is marked *(inferred)* any longer. The acceptance was of the document as
written, in one sentence; a line that turns out wrong is corrected in a new version, like any
other.

Client: Warren MacEvoy, author and maintainer. The users are students and other beginners who are
never in the room; the proxy for them is the issue tracker at github.com/wmacevoy/facts and what
the owner sees in class (SE1/H5) · Date sent: 2026-11-14 · *Tell us what is wrong
with it.*

**What changed since v3**, decided by the owner on 2026-11-17, from issue #7 (moving a C++ suite
to facts):

- **WR-8** is new: in C++ a fiction leaves its group by exception, from any depth;
  `FACT_BY_RETURN` keeps the old way. *By return* and *by exception* are new words.
- A C group cannot call C++ that checks facts, and nothing is done for that case (Scope
  decisions).
- **WR-9** is new: in C++ a fiction shows a value of any type, so every comparison compiles.

*Since v2 (in v3)*, all decided by the owner on 2026-11-14:

- The document is accepted, so the *(inferred)* marks are gone, and the four assumptions are
  answered.
- **ED-1** is retired: the VS Code extension is deprecated.
- **ED-2** is new: a VSCodium extension, queued and not started.
- **WR-7** is new: intervals in C++ facts (added 2026-11-15, before v3 was merged).
- **SX-5** is new: stopping the supervisor stops the run (added 2026-11-15).
- A crash under `--facts_junit` leaves the XML unfinished, on purpose (Scope decisions).

*Since v1 (in v2):* CK-3 changed meaning, and CK-5 and SX-4 were added.

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
| Interval | writing (C++) | A range of numbers with each end in or out: `facts::interval`. `x == interval` means x is inside it. `facts::pm(eps)` is the one from -eps to eps. From the owner's darter project, 2026-11-15. |
| Footnote | writing (C++) | A note given with one fact, `FACT(a op b, note)`, printed only if that fact is fiction. The owner's word, 2026-11-14. |
| By return | writing | How a fiction leaves in C, and with `FACT_BY_RETURN`: the function the fact is written in returns. Written in a group, that ends the group; in a helper, only the helper. *(new word, v4)* |
| By exception | writing (C++) | How a fiction leaves in C++, and with `FACT_BY_EXCEPTION`: it throws, and the run catches it where it called the group, so the group ends from any depth. *(new word, v4)* |
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
| Exit code | all | 0: facts were checked and none was fiction. 1: at least one was. 2: no result to trust: an unknown reserved option, a run that checked no fact, or an abnormal end. |
| Reserved | all | Said of a command-line argument beginning `--fact`: it belongs to facts, and must be an option facts knows. |
| Drop-in | all | Using facts by copying its two or three source files beside your own. |

## Mission

- **MISSION-1** *(since v1)* *(judged)* A beginner can say what should be true of
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
  without asking us, and two differ by platform (REQ1/JU-1, REQ1/SX-1, REQ3/ED-2). Refuses:
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
The instructor gets the call, and what the student says is "it doesn't work." *(Read from the
README's "beginner-friendly" and "without build system friction", and from safe execution's
commit message.)*

## People this touches

Students writing their first C or C++ (the users). The instructor who grades or debugs with them.
Anyone reading the JUnit report in a CI system. Packagers for Debian, Alpine, Homebrew, and
Chocolatey. The maintainer. Treated differently: users on Windows, who get the CMake build only
and whose path is tested less (see Not yet answered).

## Requirements

Numbers are the code's.

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
  evaluated for a truth. `FACT(a,op,b)` keeps working in C and C++. *Asked for by the owner, 2026-11-14.*
- **WR-6** *(since v1)* `FACT(a,op,b)` evaluates each side once in C++, and in C wherever the
  compiler can keep a value whose type it was not told (gcc and clang can). With any other C
  compiler a fiction evaluates each side a second time to print it, and when that happened in a
  run, the last line of the console report is a warning saying how many times. A program must
  not need C23, or anything past C11, to use facts. *The owner, 2026-11-14.*
- **WR-7** *(since v3)* In C++, a fact can compare a number with an interval: `x == interval` is
  true when x is inside, `!=` when it is outside. `facts::pm(eps)` is the closed interval from
  -eps to eps; a number plus an interval shifts it, and a number times an interval scales it
  (turning it around for a negative number), so `y + pm(e)` and `y * (1 + pm(e))` state an
  absolute and a relative tolerance. An end can be in or out. A fiction prints the value and the
  interval with enough digits to tell them apart. *Asked for by the owner, 2026-11-15, from
  darter's `interval`.*

- **WR-8** *(since v4)* In C++, `FACT` leaves by exception: a fiction in a helper, a method, or a
  lambda ends the group that called it, and a helper that returns a value can use `FACT`. The
  next group still runs. `FACT_BY_RETURN` leaves by return, as `FACT` did through 2.3.0, and
  `FACT_BY_EXCEPTION` names the C++ default; each takes the forms `FACT` takes. In C, `FACT` and
  `FACT_BY_RETURN` leave by return, and `FACT_BY_EXCEPTION` does not compile. A C++ build without
  exceptions leaves by return. *The owner, 2026-11-17, from issue #7.*
- **WR-9** *(since v4)* In C++, a fact compiles whatever the types of its sides, as long as the
  comparison does. A fiction shows each value with its own `operator<<` if it has one; otherwise
  a container as its first 20 elements in braces, `{1, 2, ...}`; a pair as `(first, second)`;
  and anything else as `?`. Elements and members are shown the same way. *The owner,
  2026-11-17, from issue #7.*

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
- **CK-3** *(since v2)* The exit code is 0 when facts were checked and none was fiction, and 1
  when any was. When facts are asked to run and no fact is checked (no group registered, no
  group matched, or no `FACT` in the groups that ran), stderr says so and the exit code is 2. It
  is also 2 on an abnormal end. Options that ask for no run (`--facts_skip`, `--facts_help`,
  `--facts_register_all`) exit 0. *The owner, 2026-11-14.*
- **CK-4** *(since v1)* With `FACTS_MAIN_IF(--switch)`, the program runs its own main unless the
  switch is given, and runs the facts when it is.
- **CK-5** *(since v2)* A command-line argument beginning `--fact` is reserved. If it is not an
  option facts knows, stderr names it, nothing runs, and the exit code is 2. The one exception is
  the switch a program gave to `FACTS_MAIN_IF`. Every other argument is the program's own and is
  left alone, with or without `FACTS_MAIN_IF`, because a program may need arguments while its
  facts are checked. *The owner, 2026-11-14.*

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
- **SX-5** *(since v3)* When the process supervising a run is told to stop (SIGINT, SIGTERM, or
  SIGHUP), the run it started is ended too, and the supervisor then ends by that same signal, so
  whoever sent it sees an interrupted process. A hung group never outlives the process that was
  timing it. A signal the caller had set to be ignored stays ignored. POSIX only; on Windows a
  killed supervisor still leaves its child running. *The owner, 2026-11-15: "9 fix."*
- **SX-4** *(since v2)* `--facts_timeout=N` takes a positive whole number of seconds. Anything
  else is refused: stderr says so, nothing runs, and the exit code is 2. *From the agent's
  table, which the owner did not change, 2026-11-14.*

**Packaging and the editor**

- **PK-1** *(since v1)* `make install` and the CMake install each put the header, a static and a
  shared library for C and for C++, and a pkg-config file for each where the platform expects
  them. A minor or patch release replaces the shared library without relinking.
- **PK-2** *(since v1)* The libraries build and the check passes on Linux and macOS; the libraries
  build and the tests run on Windows.
- **ED-1** *(since v3)* *Retired.* It asked that the VS Code extension list groups and show each
  outcome. The extension in `vscode-facts/` is deprecated (the owner, 2026-11-14); it never read
  the console report correctly (finding 2026-11-14). Its content goes to REQ3/ED-2.
- **ED-2** *(since v3)* In VSCodium, an extension lists the groups declared in the workspace's
  sources and, after a run, shows each one's outcome and takes the user to the line of a fiction.
  *The owner asked for the extension, 2026-11-14; the sentence is ED-1's, carried over by the
  agent. Queued, not started.*

## Inputs: absent, late, wrong, lying

No "?" is left: v1's were answered by the owner on 2026-11-14.

| Input | Absent | Late | Wrong | Lying |
| ----- | ------ | ---- | ----- | ----- |
| Registered groups | None registered: no fact is checked, an error (REQ2/CK-3). | — | Registered twice: runs once (REQ1/RG-1). | — |
| A file given to find | Cannot be opened: named on stderr, the rest continue. | — | Declares no groups: nothing registers; the run then checks no fact, an error (REQ2/CK-3). | Declares a group not in the executable: named on stderr (REQ1/RG-2). A name over 255 characters: said on stderr, with the file. |
| A pattern | — | Include or exclude with find, in either order: applies to the groups find registered (REQ1/RG-3). | Matches nothing: no fact is checked, an error (REQ2/CK-3). | — |
| An option | No options: run everything registered. | — | Begins `--fact` and is not known: an error, nothing runs (REQ2/CK-5). Anything else: the program's own, left alone. | `--facts_timeout=abc` or `=0`: refused (REQ2/SX-4). |
| A group's code | — | Hangs: killed at the timeout (REQ1/SX-2). | Crashes: reported (REQ1/SX-1). | A fact with a side effect (`++x`): happens once, except in C on a compiler that cannot keep the value, where a fiction makes it happen twice and the report warns (REQ1/WR-6). A group with no `FACT` in it: runs, and counts for nothing (REQ2/CK-3). |
| The child process | Cannot be started: said on stderr, exit code 2. | Ends early: the parent returns at once. | Exits with a code above 1: passed through. | — |

## Operating envelope

- **Scale:** one program, tens of groups, hundreds of facts. Not designed for thousands of groups
  or for running them in parallel.
- **Speed:** safe execution adds nothing a person would notice: measured 3 ms for a short run
  (2026-11-14, Linux), where it was a full second through 2.2.0. The check asserts twenty short
  runs in under 5 seconds.
- **Recovery:** nothing is kept between runs, so there is nothing to recover.
- **Resources:** no allocation while a group runs, except in the report of a fiction. The pattern
  matcher uses memory in proportion to the pattern's length, not the name's.
- **Interruption:** Ctrl-C and a closed terminal reach both processes, and both end. A signal to
  the supervising process alone (`kill`, a stop button, a cancelled CI job) ends the run too
  (REQ3/SX-5); through 2.2.0 it left the child running, with nothing left to time it out.
  Tried on Linux, 2026-11-15, each of the four ways. Not covered: SIGKILL of the supervisor,
  which cannot be caught, and Windows.

## Security, law, and ethics

Facts runs the author's own code with the author's own rights, and adds no network, no file
writes beyond temporary files for the JUnit report, and no data about people. Find reads only the
files named on the command line and looks up only symbols named `facts_<Name>_data`. The JUnit
report escapes `<`, `>`, `&`, and `"`. MIT license; no code from another project is known to be
in it, and the two links in the source cite Stack Overflow answers for an idea, not copied code.


## Assumptions (K4s), read back to the client

Each was accepted with the document on 2026-11-14, not asked one by one.

- "We're assuming the two-file drop-in is the property everything else gives way to. True?" —
  accepted, 2026-11-14
- "We're assuming `Facts.status` holding both the selection and the outcome is history, not
  intent, and that it stays as it is because the struct is published. True?" — accepted,
  2026-11-14
- "We're assuming the console format in `expected/` is published: people and tools read it, so a
  change is a decision. True?" — accepted, 2026-11-14
- "We're assuming the VS Code extension is meant to work today, not a sketch. True?" — no: it is
  deprecated, 2026-11-14

## Scope decisions

- Groups are registered explicitly, or found from named source files. Not by searching memory:
  linkers reorder data and groups were silently missed — commit `177d015`, 2026-03-05.
- Safe execution is the default and `--facts_unsafe` is the exception — commit `ece1a23`; a
  beginner's crash should be a message *(the agent's reading of the reason)*.
- The whole run is one child process, not one per group — the code; *(the agent's reading of the reason)*: one
  fork, and output order stays the program's.
- A fiction ends its group but not the run — the code and README. Through 2.3.0 a fiction in a C++
  helper ended only the helper; from v4 it ends the group (REQ4/WR-8).
- A C group cannot call C++ that checks facts: an exception thrown through C may not unwind, and
  nothing is done about it — the owner, 2026-11-17: "C cannot test c++, that seems like a
  constraint of the language."
- The C++ library contains the C core (`facts.cpp` includes `facts.c`); a program links one,
  never both — `PACKAGING.md`.
- The summary counts facts, and the report names groups — the code.
- A run that checks no fact is an error, not a pass — the owner, 2026-11-14: "if facts are run,
  but there are no facts, then that is an error."
- A crash or timeout under `--facts_junit` leaves the XML unfinished: the report is torn, and
  stays torn. The exit code 2 and the line on stderr say what happened — the owner, 2026-11-14:
  "leave torn."
- The VS Code extension is deprecated and gets no fixes; an extension for VSCodium replaces it
  — the owner, 2026-11-14.
- Arguments beginning `--fact` are reserved and must match; all others are the program's — the
  owner, 2026-11-14: "a program might need command line args even for tests."

## Non-goals

- Not fixtures, setup and teardown, or mocks — a beginner's first tests do not need them.
- Not parallel or per-group isolated runs — see Scope decisions.
- Not a matcher or assertion vocabulary beyond `FACT(a,op,b)` — one form to learn.
- Not a report plug-in interface — two formats, both ours to write.
- Not test discovery without naming source files — removed in 2.0 for cause.

## Not yet answered

- Is safe execution on Windows tested anywhere? CI builds and runs the tests there under
  `WILL_FAIL`, which also passes if they crash. — the owner
- Who uses the JUnit report, and with which CI? Its failure text is "See stdout". Not blocking: the torn-report question it was holding up is decided. — the owner

## Deferred

- Not yet: splitting `Facts.status` into a selection and an outcome — it changes a published
  struct; the trigger is the next major version.
