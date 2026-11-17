/* REQ4/WR-8: C groups in a C++ program still leave by return.
   Linked with test/testfacts_leave.cpp. */
#include "facts.h"

static void odd(Facts *facts, int x) {
  FACT(x % 2, ==, 1);
  printf("not reached: odd went on\n");
}

FACTS(CGroupReturns) {
  FACT_BY_RETURN(1, ==, 2);
  printf("not reached: CGroupReturns went on\n");
}

FACTS(CGroupHelperReturns) {
  odd(facts, 2);
  printf("reached: CGroupHelperReturns went on after its helper returned\n");
}
