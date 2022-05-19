#include <stdio.h>

// from include/facts.h in https://github.com/wmacevoy/facts/releases)
// you will also need to compile/link against src/facts.c, 
//
// cc -g -std=c11 -o hello hello.c lib/facts/src/facts.c
//
// 
#include "facts.h"

// something to fact-check

int f(int x) {
  int y = 2*x+3;
  return y;
}


FACTS(AboutF) {
  FACT(3,==,f(0));
  FACT(23,==,f(10));
}

int g(int x) {
  int y=2*x-3;
  return y;
}

FACTS(AboutG) {
  int x = 3, y = 4;
  FACT(-3,==,g(0));
  FACT(23,==,g(10)); // fails
}

FACTS_FAST
