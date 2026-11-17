/* REQ4/WR-10: a helper with no Facts parameter checks facts in the running group. */
#include "facts.h"

static void even(int x) {
  Facts *facts = FactsCurrent();
  FACT(x % 2, ==, 0);
}

FACTS(CurrentIsThisGroup) {
  FACT(FactsCurrent() == facts, ==, 1);
  even(4);
}

FACTS(CurrentInHelper) {
  even(3);
}

FACTS_REGISTER_ALL() {
  FACTS_REGISTER(CurrentIsThisGroup);
  FACTS_REGISTER(CurrentInHelper);
}

FACTS_MAIN
