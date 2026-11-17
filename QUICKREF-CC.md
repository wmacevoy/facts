# facts quick reference: C++

One page for a person or an agent writing facts in C++ (C++11 or later). For C see
[QUICKREF-C.md](QUICKREF-C.md); for the full story, [README.md](README.md).
Every example here was compiled and run with facts 2.2.0; the options and the interval were
checked at 2.3.0, and helpers, `FactsCurrent()`, and the values a fiction shows at 2.4.0.

## Set up

Copy `include/facts.h`, `src/facts.c`, and `src/facts.cpp` beside your sources. Compile
`facts.cpp` only: it includes `facts.c`.

```sh
c++ -std=c++11 -g -o check your.cpp facts.cpp && ./check
```

On Linux add `-rdynamic` if you use `--facts_find`, and `-ldl` if the linker asks for `dlsym`.

## A complete file

```C++
#include <string>
#include "facts.h"

int inc(int x) { return x + 1; }

FACTS(AboutInc) {              // a group: a named block of facts
  FACT(inc(1) == 2);           // a fact: the comparison itself
  FACT(inc(-1) == 0);
}

FACTS(AboutStrings) {
  std::string s = "abc";
  FACT(s == "abc");
  FACT(s.size() == 3);
}

FACTS_REGISTER_ALL() {         // say which groups exist, in order
  FACTS_REGISTER(AboutInc);
  FACTS_REGISTER(AboutStrings);
}

FACTS_MAIN                     // main() that checks the facts
```

## What you can write

| Write | Meaning |
| ----- | ------- |
| `FACTS(Name) { ... }` | A group. `Name` is an identifier. |
| `FACT(a == b)` | The comparison must be true. If not, print both values and leave the group. Any of `== != < <= > >=`. |
| `FACT(a == b, "n=" << n)` | The same, with a footnote printed only if the fact is false. Write it as you would after `std::cout <<`. |
| `FACT_CHECK(a == b)`, `FACT_CHECK(a == b, note)` | The same, but keep going. Its value is 1 for true, 0 for false. |
| `FACT(x == y + facts::pm(1e-9))` | `x` is within `1e-9` of `y`. `y * (1 + facts::pm(1e-9))` is one part in 1e9. `==` an interval means inside, `!=` outside. |
| `facts::interval::open(a,b)`, `::closed(a,b)`, `facts::interval(a,b,true,false)` | An interval with its ends out, in, or one of each. |
| `FACT(a,==,b)` | The form C has. It still works. |
| `FACT_BY_RETURN(a == b)` | Leave only the function it is written in, as `FACT` did through 2.3.0. Takes every form `FACT` takes. |
| `FACTS_TRACE("n=" << n)` | A note for every fact below it in the same block; printed if one of them is false. |
| `FACTS_WATCH(n, m)` | A trace that prints a gdb breakpoint stopping only when `n` and `m` have these values. 1 to 20 variables. |
| `FACTS_EXCLUDE(Name) { ... }` | A group that is registered but not run unless included by an option. |
| `FACTS_MAIN_IF(--facts) { ... }` | Your program keeps its own `main` (the block); the facts run only when `--facts` is given. |

## Helpers

`FACT` works in any function that has a `Facts *facts` in scope. A false fact there ends the
group that called it: the run catches the exception `FACT` throws. A function that cannot take
the parameter (a virtual method, a callback) gets the running group from `FactsCurrent()`.

```C++
static int half(Facts *facts, int x) {   // a helper may return a value
  FACT(x % 2 == 0, "x=" << x);
  return x / 2;
}

FACTS(AboutHalf) {
  FACT(half(facts, 8) == 4);
  half(facts, 3);                          // false: the group ends here
}

int third(int x) {                         // no Facts parameter
  Facts *facts = FactsCurrent();           // the group being run
  FACT(x % 3 == 0);
  return x / 3;
}
```

## Many cases

```C++
FACTS(AboutManyCases) {
  for (int n = 0; n < 100; ++n) {
    FACTS_TRACE("n=" << n);                        // which case, for every fact below
    FACT(inc(n) > n);
    FACT(inc(n) - n == 1, "inc(n)=" << inc(n));    // a note for this fact only
  }
}

FACTS(AboutTable) {
  struct Case { int in, out; };
  Case cases[] = { {0,1}, {1,2}, {41,43} };
  for (const Case &c : cases) {
    FACT_CHECK(inc(c.in) == c.out, "in=" << c.in); // report every bad row, not just the first
  }
}
```

## Things that bite

- **Values print as well as they can.** A value with an `operator<<` prints with it. Without
  one, a container prints its first 20 elements, `{1, 2, ...}`; a pair, `(first, second)`;
  anything else, `?`. Give a type an `operator<<` to see more than `?`.
- **C strings are pointers.** `FACT(p == "abc")` with a `const char *` compares addresses. Make
  one side a `std::string`.
- **Floating point is rarely equal.** `FACT(0.1 * 7 == 0.7)` is false. Use an interval:
  `FACT(x == 0.7 + facts::pm(1e-12))`. A value compared with an interval prints with 17 digits.
- **`&&` and `||` print no values.** `FACT(a == 1 && b == 2)` is checked as a whole. Write two
  facts: `FACT(p != nullptr); FACT(p->x == 2);` still never reaches the second when the first is
  false.
- **A comma inside a template argument splits the macro's arguments:**
  `FACT(f<A, B>() == 1)` does not compile. Parenthesize it: `FACT((f<A, B>()) == 1)`.
- **A footnote is streamed,** so one with a comparison in it needs parentheses:
  `FACT(x == 2, (a < b))`.
- **`--facts_timeout=N` is for the whole run** (10 seconds by default). Raise it for a long one.
- **Parenthesize a shift on the left:** `FACT((x << 1) == 4)`.
- **Compare pointers with `nullptr`,** not `0`.
- **`FACT(a = b)` does not compile.** That is deliberate.
- **`FACT` leaves the group,** from any depth, by throwing `FactsLeave`. In a loop it stops at the
  first bad case; use `FACT_CHECK` to see them all. A `catch (...)` of your own around a `FACT`
  catches it and the group goes on, and a `FACT` in a destructor or `noexcept` function ends the
  program: use `FACT_CHECK` or `FACT_BY_RETURN` there. Built without exceptions
  (`-fno-exceptions`), `FACT` leaves by return.
- **A group you forget to register does not run**, and nothing says so. `--facts_register_all`
  prints the block for what is registered now; `--facts_find` avoids the block altogether.
- Each side of a fact is evaluated once, and a note is only built when the fact is false.

## Run it

```sh
./check                                  # everything registered
./check --facts_include='About*'         # only matching groups (* is a wildcard)
./check --facts_exclude='*Slow'          # all but matching groups
./check --facts_find your.cpp other.cpp \;   # register the groups those files declare
./check --facts_plain                    # no color codes
./check --facts_junit                    # JUnit XML on stdout
./check --facts_timeout=60               # default is 10 seconds for the whole run
./check --facts_unsafe                   # run in this process: needed under a debugger
./check --facts_help
```

Arguments beginning `--fact` are reserved: a misspelled one is an error and nothing runs. Other
arguments are your program's and are left alone.

## Read the result

```
your.cpp/AboutTable 30: inc(c.in) == c.out {42 == 43} is fiction: in=41
(gdb) break facts_AboutTable_function
(gdb) run --facts_unsafe --facts_include=AboutTable
...
206 (99.5%) truths and 1 (0.5%) fictions checked.
```

The first line is file, group, line, the comparison as written, its two values in braces, and
the footnote. Trace notes follow it, innermost first. The `(gdb)` lines can be pasted into
`gdb ./check`; with a `FACTS_WATCH` there is one more, a `break ... if n==41` line that stops in
the failing case.

| Exit code | Meaning |
| --------- | ------- |
| 0 | Facts were checked and all were true. |
| 1 | At least one fact was false. |
| 2 | No result to trust: a crash, a timeout, a wrong option, or no fact was checked at all. |

## For an agent

- Build and run with the two commands above; decide by the exit code, and treat 2 as "not
  checked", never as a pass.
- Use `--facts_plain` when you will read the output, and `--facts_junit` when a tool will.
- To check one group while working: `./check --facts_plain --facts_include=Name`.
- A fact that fails is reported by the line `file/Group line: ... is fiction`; grep for
  `is fiction`.
- Put the case in a `FACTS_TRACE` or a footnote whenever a fact is inside a loop, so a failure
  names its input.
- After adding a group, add it to `FACTS_REGISTER_ALL`, or run with `--facts_find` on the files.
- `--facts_include` and `--facts_exclude` apply to what `--facts_find` registered, in either
  order (from 2.3.0; in 2.2.0 an include after find narrowed nothing).
