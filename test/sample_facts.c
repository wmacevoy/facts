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

FACTS_FAST
