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
  // use a vector to check multiple cases in a loop (case#1 fails)
  // C++ allows use of FACTS_TRACE to report additional info on a fiction.
  vector<tuple<int,int>> expects = {{0,-3}, {1,0}, {2,1}};
  for (int k=0; k<expects.size(); ++k) {
    int x=get<0>(expects[k]);
    int y=get<1>(expects[k]);
    FACTS_TRACE("case #" << k << " expect g(" << x << ")=" << y);
    FACT(g(x),==,y);
  }
}

FACTS_REGISTER_ALL() {
  FACTS_REGISTER(AboutF);
  FACTS_REGISTER(AboutG);
}

FACTS_MAIN
