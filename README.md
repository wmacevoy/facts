# Facts

Facts is a less-is-more C/C++ test framework.

## Why

Test frameworks have become a barrier to testing.  They are too complicated to build and use.

Facts is a less-is-more C/C++ test framework.  There is intentionally not much here.  With a few things to understand, you can enjoy the benefits of a test framework that is on your side.

## Overview

Here are the things to know:

1. `FACT(a,op,b)` is a fact check. Here `a` and `b` are simple expressions and `op` is any logical relation, like `==` or `<`.  So `FACT(x,==,3)` is the statement that `x == 3` is a fact.
2. Facts are in a `FACTS(AboutThing) {...} groups, or `FACTS_EXCLUDE(AboutExludedThing)` groups.
3. End your fact-checking with (all your `FACTS`/`FACTS_EXCLUDE`):
```C
FACTS_REGISTER_ALL() {
  FACTS_REGISTER(AboutThing1); 
  FACTS_REGISTER(AboutThing2);
  FACTS_REGISTER(AboutThing3);
}
```
4. If you want facts checking to be the `main()` thing, add `FACTS_MAIN` after `FACTS_ALL() {...}`.

5. Running the executable will check all the (not excluded) facts by default.  There are command line options to do other things (--facts_help).

Below are these steps in more detail.

### Step 1 - Write FACT checks in FACTS groups

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

Notice the __TWICE__.  This means be careful about fact-checks that have side-effects (like `++x` or function calls that change things).  If the check fails, then the print will be different from the check.  There is no reliable way to get around this and stick to the C language.

The `FactsFiction()` call makes it easy to set a break point in the debugger to inspect a failure.  Set a break point in FactsFiction() and the first failure will stop the debugger in the FACTS(...) that are being checked.

After describing all the `FACTS`, you __MUST__ mark the end of the facts with

```C
FACTS_REGISTER_ALL() {
     FACTS_REGISTER(name1);
     FACTS_REGISTER(name2);
     FACTS_REGISTER(name3);
}
```

You can call the FactsMain(...) as your (test-only) main with

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

FACTS_REGISTER_ALL() {
     FACTS_REGISTER(AboutLogic);
     FACTS_REGISTER(AboutInts);
}

FACTS_MAIN
```

### Step 2 - Compile fact checks into an executable

If the "facts.h" and "facts.c" source files are in the same folder as "sample_facts.c", they can be compiled into one executable with (cc is usally the c compiler, -g is the debug option):
```sh
cc -g -o sample_facts sample_facts.c facts.c
```

Alternatively, if you are writing C++, you can call this file "sample_facts.cpp", and instead compile with (if "facts.h", "facts.c", and "facts.cpp" are in the same folder):
```sh
c++ -g -o sample_facts sample_facts.cpp facts.cpp
```

### Step 3 - Run the executable to fact check

To check all the facts, you can now run the executable:
```sh
./sample_facts
```
Here is the output:
```
sample_facts.c 4: AboutLogic facts check started
sample_facts.c 4: AboutLogic facts check ended
sample_facts.c 9: AboutInts facts check started
sample_facts.c/AboutInts 13: 2147483647+1 {=-2147483648} > 2147483647 {=2147483647} is fiction
Debug facts_AboutInts_function on line 9 of file sample_facts.c with a breakpoint on line 13.
For example in gdb:
break facts_AboutInts_function
break "sample_facts.c":13
run --facts_include AboutInts
continue
print 2147483647+1
print 2147483647
print (2147483647+1) > (2147483647)

sample_facts.c 9: AboutInts facts check ended badly
facts summary.
facts check AboutLogic passed
facts check AboutInts failed
5 (83.3%) truths and 1 (16.7%) fictions checked.
```

To check only "AboutLogic", you can run,
```
./sample_facts --facts_include="*Logic"
```

You can combine include/exclude with wildcards to pick facts to check if you name your facts groups nicely.

### Step 3 - Debugging

The `FactsFiction()` entry point is to easily set break points in a failed FACTS check.  The sample above has a failed test.  If you use `gdb`, the gnu debugger, you could do the following:

```sh
gdb sample_facts
> b FactsFiction
> r
> up
> p *facts
```

You are now in the FACTS function call that failed.  Usually you want to extract the specific failure into a seperate FACTS check, so you can set a breakpoint for that  (`b facts_YOUR_AD_HERE_function`) and follow the steps into the failure there.

Enjoy your fact checking!


## Optional Automatic Fact Finding

Note the compiler may rearrange or eliminate `FACTS()` while optimizing code.  If that is true, `FACTS_REGISTER_AUTO` may fail to automatically find all the facts.  Making an explicit `FACTS_REGISTER_ALL` is reliable even if you are building tests with memory optimizations.

Replacing `FACTS_REGISTER_ALL` with `FACTS_REGISTER_AUTO` will ignore the explict registration and scan memory for facts groups.  This is nice when rapidly creating tests.  The `--facts_register_all` option will generate `FACTS_REGISTER_ALL` for portable testing.
