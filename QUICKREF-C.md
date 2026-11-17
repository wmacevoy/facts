# facts quick reference: C

One page for a person or an agent writing facts in C (C11). For C++ see
[QUICKREF-CC.md](QUICKREF-CC.md); for the full story, [README.md](README.md).
Every example here was compiled and run with facts 2.2.0, and the options rechecked at 2.3.0.

## Set up

Copy `include/facts.h` and `src/facts.c` beside your sources. Nothing else is needed.

```sh
cc -std=c11 -g -o check your.c facts.c && ./check
```

On Linux add `-rdynamic` if you use `--facts_find`, and `-ldl` if the linker asks for `dlsym`.

## A complete file

```C
#include <string.h>
#include "facts.h"

int inc(int x) { return x + 1; }

FACTS(AboutInc) {              /* a group: a named block of facts */
  FACT(inc(1),==,2);           /* a fact: left, operator, right   */
  FACT(inc(-1),==,0);
}

FACTS_REGISTER_ALL() {         /* say which groups exist, in order */
  FACTS_REGISTER(AboutInc);
}

FACTS_MAIN                     /* main() that checks the facts */
```

## What you can write

| Write | Meaning |
| ----- | ------- |
| `FACTS(Name) { ... }` | A group. `Name` is an identifier. |
| `FACT(a,op,b)` | `a op b` must be true. If not, print both values and leave the group. |
| `FACT_CHECK(a,op,b)` | The same, but keep going. Its value is 1 for true, 0 for false. |
| `FACT_PRINT(a,op,b,"%x")` | `FACT` with your own `printf` format for both values. |
| `FACTS_EXCLUDE(Name) { ... }` | A group that is registered but not run unless included by an option. |
| `FactsAbsErr(a,b)`, `FactsRelErr(a,b)` | Absolute error, and error relative to the larger magnitude (0 when both are 0), of two doubles: `FACT(FactsRelErr(a,b),<,1e-12)`. |
| `FACTS_MAIN_IF(--facts) { ... }` | Your program keeps its own `main` (the block); the facts run only when `--facts` is given. |

## Things that bite

- **Strings are pointers.** `FACT(s,==,"abc")` compares addresses. Write `FACT(strcmp(s,"abc"),==,0)`.
- **Both values are printed with the left side's type.** Give the two sides the same type:
  `FACT(strlen(s),==,(size_t)3)`.
- **Supported types** are the integer and floating types, `char`, `_Bool`, `char *` (printed as a
  string), and `void *`. For anything else compare a member, or use `FACT_PRINT`.
- **Side effects.** With gcc or clang each side is evaluated once. With other C compilers a false
  fact evaluates each side twice, and the report ends with a warning if that happened. Keep `++x`
  out of facts.
- **`FACT` returns from the function it is in.** In a group that ends the group; in a helper,
  only the helper. A helper with no `Facts *` parameter writes `Facts *facts = FactsCurrent();`. Free what you allocated before the first `FACT` that can fail, or use
  `FACT_CHECK`.
- **A group you forget to register does not run**, and nothing says so. `--facts_register_all`
  prints the block for what is registered now; `--facts_find` avoids the block altogether.

## Run it

```sh
./check                                  # everything registered
./check --facts_include='About*'         # only matching groups (* is a wildcard)
./check --facts_exclude='*Slow'          # all but matching groups
./check --facts_find your.c other.c \;   # register the groups those files declare
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
your.c/AboutHex 24: mask & 0xFF {=0xf0} == 0xF1u {=0xf1} is fiction
(gdb) break facts_AboutHex_function
(gdb) run --facts_unsafe --facts_include=AboutHex
...
5 (83.3%) truths and 1 (16.7%) fictions checked.
```

The first line is file, group, line, then each side as written with its value. The `(gdb)` lines
can be pasted into `gdb ./check` to stop in that group.

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
- After adding a group, add it to `FACTS_REGISTER_ALL`, or run with `--facts_find` on the files.
- `--facts_include` and `--facts_exclude` apply to what `--facts_find` registered, in either
  order (from 2.3.0; in 2.2.0 an include after find narrowed nothing).
