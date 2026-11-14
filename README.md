![CI](https://github.com/wmacevoy/facts/actions/workflows/c-cpp.yml/badge.svg)

# Facts

Facts is a tiny, beginner‑friendly C/C++ testing library. It aims to be easy to drop into small programs without build system friction.

## What you get

- **C core**: `FACTS(name)` groups and `FACT(a,op,b)` checks with readable failure messages.
- **C++ extras**: write `FACT(a == b)` directly, add footnotes, and use tracing (`FACTS_TRACE`, `FACTS_WATCH`) to find the failing case among many.
- **No heavy deps**: just compile the provided `.c`/`.cpp` files with your code. An installable shared library + `pkg-config` file is provided too.

## Quick start

1) Add the files beside your sources

- C only: download `include/facts.h` and `src/facts.c`.
- C++: also add `src/facts.cpp` for tracing helpers.

2) Include the header

```C
#include "facts.h"
```

3) Write facts

```C
int inc(int x) { return x + 1; }

FACTS(AboutInc) {
  int a = 5, b = 6;
  FACT(inc(a),==,b);
}

FACTS_MAIN
```

4) Build and run

```sh
# C — build and test in one line
cc -std=c11 -g -o check your.c facts.c && ./check --facts_find your.c

# C++
c++ -std=c++11 -g -o check your.cpp facts.cpp && ./check --facts_find your.cpp
```

`--facts_find` scans source files for `FACTS()` declarations, registers them at runtime via `dlsym`, and runs the tests. No `FACTS_REGISTER_ALL` block needed.

Tip: after `make install`, you can also use `pkg-config`:

```sh
c++ -std=c++11 -g $(pkg-config --cflags facts) -o check your.cpp $(pkg-config --libs facts)
```

## Writing facts

- `FACTS(Name) { ... }` declares a group. Each group compiles to `void facts_Name_function(Facts*)`.
- `FACT(a,op,b)` checks a relation (e.g., `==`, `!=`, `<`, `>=`). On failure it prints the expressions and values and returns from the current `FACTS` group.
- `FACT_CHECK(a,op,b)` behaves like `FACT` but does not early‑return; it evaluates to 1 on success, 0 on failure.

Floating‑point checks: use the helpers for clarity.

```C
FACTS(AboutFloats) {
  double a = 0.1 * 7, b = 0.7;
  FACT(FactsAbsErr(a,b),<,1e-12);
  FACT(FactsRelErr(a,b),<,1e-12);
}
```

Important: on a failure the values of `a` and `b` are printed. On some compilers that means each side is evaluated twice in that case, so avoid side effects (like `++x`) in `FACT(a,op,b)`. With gcc or clang, and in C++ with any compiler, each side is evaluated once. If a run did evaluate twice, the last line of the report is a warning that says so.

### In C++: write the comparison itself

In C++ a fact can be the comparison as you would write it anywhere else:

```C++
FACT(inc(a) == b);            // same as FACT(inc(a),==,b)
FACT(v.size() > 0);
FACT(name == "ada");
```

A fiction still shows both values:

```
your.cpp/AboutInc 7: inc(a) == b {6 == 7} is fiction
```

- Each side is evaluated once.
- `FACT_CHECK(a op b)` works the same way and does not leave the group.
- `FACT(a,op,b)` still works, and is the only form C has.
- A condition joined by `&&` or `||` is checked but prints no values; write two facts instead.
- A shift on the left needs parentheses: `FACT((x << 1) == 4)`. Compare a pointer with `nullptr`, not `0`.
- `FACT(a = b)` does not compile, which is usually what you want.

## Registration and main()

The simplest approach: use `--facts_find` at runtime to discover and run tests from source files. Just add `FACTS_MAIN` to your file — no registration block needed:

```sh
./check --facts_find your.c
```

For baked-in registration (no source files needed at runtime), add a `FACTS_REGISTER_ALL` block:

```C
FACTS_REGISTER_ALL() {
  FACTS_REGISTER(AboutInc);
  FACTS_REGISTER(AnotherGroup);
}

FACTS_MAIN
```

Use `--facts_register_all` to generate this block from whatever is currently registered:

```sh
./check --facts_find src/*.c \; --facts_register_all
```

You can also gate running facts behind a command‑line switch:

```C
FACTS_REGISTER_ALL() { FACTS_REGISTER(MyTest); }

FACTS_MAIN_IF(--facts) {
  /* your program's main */
  return 0;
}
```

## Command‑line options

The options (`--facts_help` prints them):

```
default is to check all registered facts
    --facts_include="*pattern*"   include matching groups
    --facts_exclude="*pattern*"   exclude matching groups
    --facts_find files... [\;]    register facts from source files
    --facts_register_all          print FACTS_REGISTER_ALL block
    --facts_skip                  do not run facts
    --facts_help                  print the options
    --facts_junit                 emit JUnit XML to stdout
    --facts_plain                 disable ANSI colors
    --facts_unsafe                run directly (skip fork/exec wrapper)
    --facts_timeout=N             timeout in seconds (default 10)
```

Exit code: 0 if no `FACT` failed (also if no facts ran), 1 if any did, 2 if the run crashed, timed out, or could not start.

## Safe execution

By default, facts fork/exec's itself as a child process before running tests. This provides:

- **Crash isolation** — segfaults, aborts, and other signals in tests are caught and reported cleanly instead of crashing your terminal.
- **Timeout** — tests that hang are killed after 10 seconds (configurable with `--facts_timeout=N`).
- **Niceness** — the child process runs at lower priority (`nice 10`) so runaway tests don't starve your system.

Pass `--facts_unsafe` to skip the wrapper and run tests directly in the current process. A debugger needs it, and the `(gdb) run` line printed with a fiction includes it.

## Testing many cases (C++)

One fact about one value is rarely enough. Put the facts in a loop and check hundreds of cases:

```C++
int isqrt(int n);   // integer square root: the largest r with r*r <= n

FACTS(AboutIsqrt) {
  for (int n = 0; n <= 200; ++n) {
    int r = isqrt(n);
    FACT(r * r <= n);
    FACT(n < (r + 1) * (r + 1));
  }
}
```

When this fails you see the two sides of the comparison, but not which case you were in:

```
isqrt.cpp/AboutIsqrt 13: n < (r + 1) * (r + 1) {100 < 100} is fiction
```

Here you can guess `n`, but usually you cannot. There are three ways to say which case it was. The text of a note is only built when a fact is fiction, so notes are cheap while the facts are true.

### A footnote: one fact, one note

Add a second argument to `FACT`. Write it as you would after `std::cout <<`:

```C++
    FACT(n < (r + 1) * (r + 1), "n=" << n << " r=" << r);
```

```
isqrt.cpp/AboutIsqrt 13: n < (r + 1) * (r + 1) {100 < 100} is fiction: n=100 r=9
```

### A trace: one note for every fact below it

`FACTS_TRACE(note)` covers every fact that follows it in the same block, so you write the case once at the top of the loop instead of on each fact:

```C++
FACTS(AboutIsqrt) {
  for (int n = 0; n <= 200; ++n) {
    FACTS_TRACE("n=" << n);
    int r = isqrt(n);
    FACT(r * r <= n);
    FACT(n < (r + 1) * (r + 1));
  }
}
```

```
isqrt.cpp/AboutIsqrt 22: n < (r + 1) * (r + 1) {100 < 100} is fiction
(gdb) break facts_AboutIsqrt_function
isqrt.cpp/AboutIsqrt:19 n=100
(gdb) run --facts_unsafe --facts_include=AboutIsqrt
```

The trace line gives the file, group, and line of the `FACTS_TRACE`, then your note. Traces nest: with one in an outer loop and one in an inner loop, a fiction prints the inner note first, then the outer. A trace ends where its block ends. It captures the variables in its note by value at the moment it is reached, so the note shows the case as it was at the top of the loop.

### A watch: stop the debugger at the failing case

`FACTS_WATCH(n)` is a trace whose note is a gdb breakpoint that stops only in the case that failed. It takes 1 to 20 variables:

```C++
  for (int n = 0; n <= 200; ++n) {
    FACTS_WATCH(n);
    ...
```

```
isqrt.cpp/AboutIsqrt 31: n < (r + 1) * (r + 1) {100 < 100} is fiction
(gdb) break facts_AboutIsqrt_function
isqrt.cpp/AboutIsqrt:28 (gdb) break facts_AboutIsqrt_function:facts_trace_1 if n==100
(gdb) run --facts_unsafe --facts_include=AboutIsqrt
```

To use it, start `gdb ./check` and paste the two `(gdb)` lines that follow the fiction: the `break ... if n==100` line, then the `run` line. `--facts_unsafe` in the `run` line makes the facts run inside the debugger and not in a child process.

gdb stops at the watch in the 101st trip around the loop, just before the fact that fails, and you can step from there.

### See every failing case, not just the first

`FACT` leaves the group at the first fiction, so a loop stops at the first bad case. `FACT_CHECK` reports and keeps going, which suits a table of cases:

```C++
struct Case { int n; int root; };

FACTS(AboutIsqrtTable) {
  Case cases[] = { {0,0}, {1,1}, {15,3}, {16,4}, {99,9}, {100,10}, {144,12} };
  for (const Case &c : cases) {
    FACT_CHECK(isqrt(c.n) == c.root, "n=" << c.n);
  }
}
```

```
isqrt.cpp/AboutIsqrtTable 40: isqrt(c.n) == c.root {9 == 10} is fiction: n=100
isqrt.cpp/AboutIsqrtTable 40: isqrt(c.n) == c.root {9 == 12} is fiction: n=144
```

The summary counts every fact checked, so a loop of 201 cases with two facts each adds 402 to the truths.

### Which to use

- One fact needs explaining: a footnote.
- Several facts share a case, or loops are nested: a trace at the top of each loop.
- You want to step through the failing case: a watch.
- You want all the failing cases at once: `FACT_CHECK`.

## Examples

- `examples/hello_c/` shows a pure‑C setup (`facts.c` is required).
- `examples/hello_cpp/` shows C++ with tracing (`facts.cpp` plus `facts.c`).

## Notes and limitations

- ANSI colors are used by default; pass `--facts_plain` to disable.

Enjoy your fact checking!
