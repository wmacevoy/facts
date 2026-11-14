/* REQ1/WR-6: how many times FACT(a,op,b) evaluates each side.
   Built three ways: as C, as C with -DFACTS_EVAL_TWICE, and as C++. */
#include "facts.h"

static int calls = 0;
static int next(void) { return ++calls; }

FACTS(OnceWhenTruth) {
  calls = 0;
  FACT(next(),==,1);
  FACT(calls,==,1);
}

FACTS(OnceWhenFiction) {
  calls = 0;
  FACT_CHECK(next(),==,5);
  FACT(calls,==,1);
}

FACTS_REGISTER_ALL() {
  FACTS_REGISTER(OnceWhenTruth);
  FACTS_REGISTER(OnceWhenFiction);
}

FACTS_MAIN
