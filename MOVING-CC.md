# Moving hand-built C++ tests to facts

A guide for people and agents moving a C++ test suite with its own checks (a `check(what, ok)`
function, a `CHECK(x)` macro, a `failures` counter, a `main` that returns 1) to facts, without
losing or weakening a check. For writing new facts see [QUICKREF-CC.md](QUICKREF-CC.md).

It comes from moving 37 hand-built tests (about 1,100 checks) in a C++ networking project to
facts, on 2026-11-17 (issue #6). Every example here was compiled and run with facts 2.4.0, gcc 12.
Through 2.3.0 several of these steps were harder; where that matters, it says so.

## Build

Use the C++ build of facts: compile `facts.cpp` (it includes `facts.c`) or link `facts++`.
The C library alone cannot catch a fiction that leaves by exception.

```sh
c++ -std=c++11 -g -o test_queue test_queue.cpp facts.cpp && ./test_queue
c++ -std=c++11 -g $(pkg-config --cflags facts++) -o test_queue test_queue.cpp $(pkg-config --libs facts++)
```

## The map

| Before | After |
| ------ | ----- |
| `check("what", ok)`: print and count, go on | `FACT_CHECK(ok, "what")`: the note prints only when false, as the `FAIL` line did |
| `check(what, ok, got)` | `FACT_CHECK(ok, (what) << ": " << (got))` |
| `CHECK(x)` that returned or exited | `FACT(x)`: ends the group |
| `CHECK(x)` that went on | `FACT_CHECK(x)` |
| `void test_foo()` called from `main` | `FACTS(Foo) { ... }`, registered in the same order in `FACTS_REGISTER_ALL` |
| a `main` that was the whole test | one `FACTS(Name)` group |
| the tail that counted failures, printed PASS or FAIL, and returned 1 | delete it: the exit code is 0, 1, or 2 (no result to trust) |

Keep each check's kind. A check that went on becomes `FACT_CHECK`, even where `FACT` would read
better: changing which checks run after a failure is a different change, made later on purpose.

## An example

Before:

```C++
static int failures = 0;

static void check(const char *what, bool ok) {
  if (!ok) { std::printf("FAIL: %s\n", what); ++failures; }
}

#define CHECK(x) do { if (!(x)) { std::printf("FAIL: %s\n", #x); return; } } while (0)

static void test_push() {
  Queue q;
  for (int i = 0; i < 4; ++i) check("push fits", q.push(i));
  check("full refuses", !q.push(9));
  check("size is MAX", q.items.size() == Queue::MAX);
}

static void test_pop() {
  Queue q;
  q.push(1); q.push(2);
  CHECK(q.pop() == 1);
  CHECK(q.pop() == 2 && q.items.empty());
}

int main() {
  test_push();
  test_pop();
  if (failures) { std::printf("FAIL: %d\n", failures); return 1; }
  std::printf("PASS\n");
  return 0;
}
```

After:

```C++
#include "facts.h"

FACTS(Push) {
  Queue q;
  for (int i = 0; i < 4; ++i) FACT_CHECK(q.push(i), "push fits, i=" << i);
  FACT_CHECK(!q.push(9), "full refuses");
  FACT_CHECK(q.items.size() == Queue::MAX, "size is MAX");
}

FACTS(Pop) {
  Queue q;
  q.push(1); q.push(2);
  FACT(q.pop() == 1);
  FACT(q.pop() == 2);
  FACT(q.items.empty());
}

FACTS_REGISTER_ALL() {
  FACTS_REGISTER(Push);
  FACTS_REGISTER(Pop);
}

FACTS_MAIN
```

`Queue::MAX` is a `static const size_t` declared in the class and never defined outside it.
Through 2.3.0 a fact comparing it compiled and then failed to link; from 2.4.0 it links.

## Where a false `FACT` goes

`FACT` ends the group from any depth: in a helper, a method, a lambda, or a helper that returns a
value. The run catches it where it called the group, and the next group runs. `FACT_CHECK` goes on.

- A `catch (...)` of your own around a `FACT` catches the fiction too, and the group goes on. It
  is still counted and printed.
- A `FACT` in a destructor or a `noexcept` function ends the program: gcc warns "'throw' will
  always call 'terminate'". Use `FACT_CHECK` there.
- `FACT_BY_RETURN` leaves only the function it is in, as `FACT` did through 2.3.0.

## Checks in helpers and methods

A function with a `Facts *facts` parameter can use any fact. Pass the group's `facts`:

```C++
static int popExpecting(Facts *facts, Queue &q, int want) {
  FACT(!q.items.empty(), "popping " << want << " from an empty queue");
  int got = q.pop();
  FACT(got == want);
  return got;
}
```

A function that cannot take the parameter, a virtual override or a callback, asks for the group
being run:

```C++
struct ExpectsSmall : Listener {
  void received(int n) override {
    Facts *facts = FactsCurrent();
    FACT(n < 10, "received " << n);
  }
};
```

Do not keep a file-wide pointer of your own named `facts`: that is the name of facts' namespace
(`facts::pm`), and `static Facts *facts;` does not compile.

## What a fiction shows

Every comparison compiles. A value prints with its own `operator<<`; without one, a container
prints its first 20 elements and a pair its two members, and anything else prints `?`:

```text
got == want {{1, 2, 3} == {1, 2, 4}} is fiction
range == want {(0, 9) == (0, 10)} is fiction
```

Through 2.3.0 such a comparison did not compile, and the move put it in parentheses,
`FACT_CHECK((a == b), note)`, which compiles but prints only `{0}`. Take those parentheses out.

## Commas and notes

- **A comma in a template argument splits the macro's arguments.** `std::make_pair<int, int>(0, 10)`
  inside a fact does not compile. Parentheses around the whole condition fix it, but then it
  prints only `{0}`. Better, name the value first:
  `auto want = std::make_pair<int, int>(0, 10); FACT_CHECK(range == want);`
- **A note is streamed,** so one with a comparison or `?:` in it needs parentheses:
  `FACT_CHECK(how == 0, (how == 1 ? "fast" : "slow"))`.

## `&&` conditions

`FACT(a == 1 && b == 2)` works, but a fiction prints only `{0}`. Split it so the fiction says
which part was false:

- With `FACT` the split is always safe: the first fiction ends the group, so the second half
  never runs where `&&` would have skipped it.
- With `FACT_CHECK` both halves run. Split a check that went on only where its second half is
  safe to run, or make the guard a `FACT`:

```C++
FACT(p != nullptr);       // false: the next line never runs
FACT_CHECK(*p == 2);
```

## Show that nothing was lost

Passing proves little about a rewrite of tests. Do both:

1. **Compare the conditions.** List the old checks' conditions and the new facts', in order, and
   compare them, ignoring added parentheses. Each old check should appear once, with the same
   condition, and the same kind (goes on, or stops).
2. **Make one fact false in each file** (`FACT(!(q.pop() == 1))`) and see a fiction and exit code
   1. Do it once inside a helper too, and check that a line after the helper's call does not run.
   A fact that only runs on a failure path (`FACT_CHECK(false, "never settled")`) will not go red
   that way: pick another.

## Time

`--facts_timeout=N` is for the whole run, 10 seconds unless you say otherwise; there is no timeout
per group. A suite with real sockets or long loops may need more: `./test_queue --facts_timeout=60`.

## Not yet

The mechanical part (blanking comments and string literals, finding functions by their braces,
splitting arguments at top-level commas) was done by a script of about 250 lines of Python. It
is not part of facts; issue #6 says a tested version would make the map above a command.
