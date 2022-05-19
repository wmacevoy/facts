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

  FACTS_WATCH2(x,y);  // get useful info on failure, C++ 20 allows FACTS_WATCH(x,y)

  FACT(g(x),==,y);
}

FACTS_FAST