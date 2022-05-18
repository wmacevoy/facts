// include the facts header file 
#include "facts.h"

#include <vector>

using namespace std;

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
  int x = 0;
  int y = 3;

  // TRACE is C++ 11 and above only
  // traces are printed in reverse order if a fact is fiction.
  
  FACTS_WATCH(x,y);
  // TRACE shorthand is WATCH<N>, like FACTS_WATCH2(x,y);
  FACT(g(x),==,y);
}

FACTS_REGISTER_ALL() {
  FACTS_REGISTER(AboutF);
  FACTS_REGISTER(AboutG);
}

FACTS_MAIN
