# Facts

This is a really simple test framework written in C (C11 standard) and is
compatible with C++.

# Use

Fact check (test) files include the "fact.h" header file and define fact check
functions with

```C
FACTS(GroupName) { ... }
```

This defines a function `facts_GroupName_function` which can be used to fact
check. The `{ ... }` should contain some fact statements or checks like:

```C
FACTS(AboutInts) {
  FACT(0+1,==,1);
  FACT(1+1,==,2);
  FACT(2+1,==,3);
}
```

`FACT(a,op,b)` becomes essentially

```C
if (not (eval(a) op eval(b))) {
  // break point opportunity for debugger
  FactsFiction();
  
  // If the fact-check fails, then a and b are evaluated __TWICE__
  printf(stderr,"str(a) {=eval(a)} str(op) str(b) {=eval(b)} is fiction.");
  fail-test;
  return;
}
```

Notice the __TWICE__.  This means be careful about fact-checks that have side-effects (like `++x` or function calls that change things).  If the check fails, then the print will be different from the check.  There is no easy way to get around this and stick to the C language.

The `FactsFiction()` call makes it easy to set a break point in the debugger to inspect a failure.  Set a break point in FactsFiction() and the first failure will stop the debugger in the FACTS(...) that are being checked.

After describing all the `FACTS`, you __MUST__ mark the end of the facts with

```C
FACTS_FINISHED
```

This creates two marker facts (0000_BEGIN and zzzz_END, two fact groups you cannot use) so the facts framework can find your facts at run time.

You can call the FactsMain(...) as your main (test-only) main with

```C
FACTS_MAIN
```

Here is a simple complete example (sample_facts.c for C or sample_facts.cpp for C++):

```C
#include "limits.h"
#include "facts.h"

FACTS(AboutLogic) {
  FACT(! 0,==,1);
  FACT(! 1,==,0);
}

FACTS(AboutInts) {
  FACT(0+1,>,0);
  FACT(1+1,>,1);
  FACT(2+1,>,2);
  FACT(INT_MAX+1,>,INT_MAX); // fiction
}

FACTS_FINISHED
FACTS_MAIN
```

If the "facts.h" and "facts.c" source files are in the same folder as "sample_facts.c", they can be compiled into one executable with (cc is usally the c compiler, -g is the debug option, and -lm asks for the math library which facts.c uses):
```sh
cc -g -o sample_facts sample_facts.c facts.c -lm
```

Alternatively, if you are writing C++, you can call this file "sample_facts.cpp", and instead compile with:
```sh
c++ -g -o sample_facts sample_facts.cpp facts.c -lm
```

To check all the facts, you can now run the executable:
```sh
./sample_facts
```
Here is the output:
```
sample_facts.c 4: AboutLogic facts check started
sample_facts.c 4: AboutLogic facts check ended
sample_facts.c 9: AboutInts facts check started
sample_facts.c 13: 2147483647+1 {=-2147483648} > 2147483647 {=2147483647} is fiction
sample_facts.c 9: AboutInts facts check ended (badly)
facts summary.
facts check AboutLogic passed
facts check AboutInts failed
5 (83.3%) truths and 1 (16.7%) fictions checked.
```

To check only "AboutLogic", you can run,
```
./sample_facts --facts_exclude="*" --facts_include="AboutLogic"
```

## FactsFiction()

The `FactsFiction()` entry point is to easily set break points in a failed FACTS check.  The sample above has a failed test.  If you use `gdb`, the gnu debugger, you could do the following:

```sh
gdb sample_facts
> b FactsFiction
> r
> up

You are now in the FACTS function call that failed.  Usually you want to extract the specific failure into a seperate FACTS check, so you can set a breakpoint for that  (`b `facts_YOUR_AD_HERE_function`) and follow the steps into the failure there.

Enjoy your fact checking!
