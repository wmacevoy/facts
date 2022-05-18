#include <stdio.h>
#include <math.h>

// include the facts header file 
#include "facts.h"

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

FACTS_REGISTER_ALL() {
  FACTS_REGISTER(AboutF);
  FACTS_REGISTER(AboutG);
}

FACTS_MAIN
