# Facts

Facts is a less-is-more C/C++ test framework.

## Why

Test frameworks have become a barrier to testing.  They are too complicated to build and use.

Facts is a less-is-more C/C++ test framework.  There is intentionally not much here.  With a few things to understand, you can enjoy the benefits of a test framework that is on your side.

## Overview

Here are the things to know:

1. Get files [facts.h](https://raw.githubusercontent.com/wmacevoy/facts/main/include/facts.h), [facts.c](https://raw.githubusercontent.com/wmacevoy/facts/main/src/facts.c), and [facts.cpp](https://raw.githubusercontent.com/wmacevoy/facts/main/src/facts.cpp).  Include the facts header file with `#include "facts.h"` with your other include statements.
    ```C
    #include "facts.h"
    ```

2. Write some code you would like to check some facts about.  For example:
    ```C
    int inc(int x) { return x+1; }
    ```

3. `FACT(a,op,b)` is a fact to check. Here `a` and `b` are simple expressions and `op` is any logical relation, like `==` or `<`.  So `FACT(x,==,3)` is the statement that `x == 3` is a fact to check.  Statements of fact are in FACTS(AboutThing) groups.  For example:
    ```C
    FACTS(AboutInc) {
      int a = 5,b = 6;
      FACT(inc(a),==,b);
    }
    ```

4. End your fact-checking with:
    ```C
    FACTS_FAST
    ```

5. Compile with the `facts.c` (C) or `facts.cpp` (C++) source file.
    ```sh
    cc -g -o check *.c # C program
    c++ -g -o check *.cpp # C++ program
    ```

6. Running the executable (`./check`) will check all the facts.  There are command line options to do other things (`./check --facts_help`).

Below are these steps in more detail.

## Step 1 - Get the facts files

Facts is intentionally written with no dependencies other than what is available in standard C 2011 (std=c11) and after.  You need `facts.h` and `facts.c` to test C programs, and in addition `facts.cpp` for C++ programs.  You can download these files from the repository (type ctrl-s on windows or command-s on macs to save these):

1. https://raw.githubusercontent.com/wmacevoy/facts/main/include/facts.h
1. https://raw.githubusercontent.com/wmacevoy/facts/main/src/facts.c
1. https://raw.githubusercontent.com/wmacevoy/facts/main/src/facts.cpp

At the simplest, they can be in the same directory as your program source code.

Programs that have fact checks should have
```C
#include "facts.h"
```
as a header file they include.  If you the compiler can't find this file, use the `-I <directory>` option (gcc/llvm), as in
```sh
c++ -I<facts-dir> -o check check.cpp <facts-dir>/facts.cpp
```
or
```sh
cc -I<facts-dir> -o check check.c <facts-dir>/facts.c
```


### Step 2. Write code to fact check.

Facts checks code for correctness.  Breaking a problem into smaller parts as functions or objects make each part easier to test.

```C
int inches(int feet) { return 12*feet; }
int fahrenheit(int celcius) { return 9*celcius/5+32; }
```
or maybe (C++):
```C++
struct Duck {
  bool like(string thing) { return thing == "duck"; }
  bool can(string action) { return action == "quack"; }
  bool 
};
```

### Step 3. Write facts in groups about what you think is (or should eventually be) true
```C
FACTS(AboutConversions) {
  int ft = 3, int in = 36;
  FACT(inches(ft),==,in);

  int F = 59, C = 15;
  FACT(fahrenheit(C),==,F);
}
```

```C++
FACTS(AboutDucks) {
  Duck animal;
  FACT(animal.looks("duck"),==,true);
  FACT(animal.can("quack"),==,true);
}
```

This defines two functions `facts_AboutConversions_function` and `facts_AboutDucks_function` which are called to fact check.  

`FACT(a,op,b)` becomes essentially

```C
if (not (eval(a) op eval(b))) {
  printf(stderr,"a {=eval(a)} op b {=eval(b)} is fiction.");
  FactIsFiction();
  fail-test;
  return;
}
```

Notice the eval __TWICE__ for both `a` and `b`.  This means be careful about fact-checks that have side-effects (like `++x` or function calls that change things).  If the check fails, then the print will be different from the check.  There is no reliable way to get around this and stick to the C language.

The `FactsFiction()` call makes it easy to set a break point in the debugger to inspect a failure.  Set a break point in FactsFiction() and the first failure will stop the debugger in the FACTS(...) that are being checked.

`FACTS_FAST` is shorthand for
```
FACTS_REGISTER_ALL() {
  FACTS_REGISTER(AboutThing1); 
  FACTS_REGISTER(AboutThing2);
  FACTS_REGISTER(AboutThing3);
}

int main(int argc, const char *argv[]) {
    return FactsMain(argc,argv);
}
```
You should be explicit about this if you are optimising your tests (-O as a compile flag) which may cause the auto-register to fail because of memory optimizations, or if you want your main to do more than just check your facts.

Here is a simple complete example (sample.cpp since it has a C++ struct):

```C++
#include <string>
#include "facts.h"

using namespace std;

int inches(int feet) { return 12*feet; }
int fahrenheit(int celcius) { return 9*celcius/5+32; }

struct Duck {
  bool like(string thing) { return thing == "duck"; }
  bool can(string action) { return action == "quack"; }
};

FACTS(convert) {
  int ft = 3, int in = 36;
  FACT(inches(ft),==,in);

  int F = 59, C = 15;
  FACT(fahrenheit(C),==,F);
}

FACTS(duck) {
  Duck animal;
  FACT(animal.looks("duck"),==,true);
  FACT(animal.can("quack"),==,true);
}

FACTS_FAST
```

### Step 2 - Compile fact checks into an executable

If the "facts.h" and "facts.c" source files are in the same folder as "sample_facts.c", they can be compiled into one executable with (cc is usally the c compiler, -g is the debug option):
```sh
cc -g -o sample sample_facts.c facts.c
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
run --facts_include=AboutInts
continue
break 13
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

The `FactIsFiction()` entry point is to easily set break points in a failed FACTS check.  The sample above has a failed test.  If you use `gdb`, the gnu debugger, you could do the following:

```sh
gdb sample_facts
> b FactsFiction
> r
> up
> p *facts
```

You are now in the FACTS function call that failed.  Usually you want to extract the specific failure into a seperate FACTS check, so you can set a breakpoint for that  (`b facts_YOUR_AD_HERE_function`) and follow the steps into the failure there.

## Step 4 - Tracing (C++ only)

In a `FACTS(AboutThing) { ... }` fact-check function, you can (only in C++) add `FACTS_TRACE(your << info << here)`.  These are printed in reverse order if a fact-check fails.  This is helpful to include additional information (like the case if looping over cases).  For example:
```
FACTS(...) {
  for (int i=0; i<3; ++i) {
    FACTS_TRACE("i=" << i);
    for (int j=i; j<3; ++j) {
      FACTS_TRACE("j=" << j);
      // FACT -check here
    }
  }
}
```

The `hello.cpp` example uses this to trace multiple cases for the function `g`.

Enjoy your fact checking!
