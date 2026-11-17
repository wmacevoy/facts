// REQ4/WR-8: in C++ a fiction leaves its group by exception, from any depth;
// FACT_BY_RETURN leaves only the function it is written in.
// Linked with test/testfacts_leave.c, whose C groups leave by return.
// A line printing "not reached" must never print; test/check.sh looks.
#include <iostream>
#include "facts.h"

// A helper that returns a value could not use FACT through 2.3.0.
static int half(Facts *facts, int x) {
  FACT(x % 2 == 0);
  return x / 2;
}

static void even(Facts *facts, int x) {
  FACT(x % 2, ==, 0);
}

static void evenByReturn(Facts *facts, int x) {
  FACT_BY_RETURN(x % 2 == 0);
  std::cout << "not reached: evenByReturn went on" << std::endl;
}

FACTS(LeaveValueHelper) {
  FACT(half(facts, 4) == 2);
  half(facts, 3);
  std::cout << "not reached: LeaveValueHelper went on" << std::endl;
}

FACTS(LeaveVoidHelper) {
  even(facts, 3);
  std::cout << "not reached: LeaveVoidHelper went on" << std::endl;
}

FACTS(LeaveLambda) {
  auto positive = [&](int x) { FACT(x > 0, "x=" << x); };
  positive(1);
  positive(-1);
  std::cout << "not reached: LeaveLambda went on" << std::endl;
}

FACTS(LeaveNamed) {
  FACT_BY_EXCEPTION(1, ==, 1);
  FACT_BY_EXCEPTION(2 == 2, "never printed");
  FACT_BY_EXCEPTION(1 == 2);
  std::cout << "not reached: LeaveNamed went on" << std::endl;
}

FACTS(ReturnHelperGoesOn) {
  evenByReturn(facts, 3);
  std::cout << "reached: ReturnHelperGoesOn went on after its helper returned" << std::endl;
}

FACTS(ReturnForms) {
  FACT_BY_RETURN(1, ==, 1);
  FACT_BY_RETURN(2 == 2, "never printed");
  FACT_BY_RETURN(1, ==, 2);
  std::cout << "not reached: ReturnForms went on" << std::endl;
}

extern "C" Facts facts_CGroupReturns_data;
extern "C" Facts facts_CGroupHelperReturns_data;

FACTS_REGISTER_ALL() {
  FACTS_REGISTER(LeaveValueHelper);
  FACTS_REGISTER(LeaveVoidHelper);
  FACTS_REGISTER(LeaveLambda);
  FACTS_REGISTER(LeaveNamed);
  FACTS_REGISTER(ReturnHelperGoesOn);
  FACTS_REGISTER(ReturnForms);
  FACTS_REGISTER(CGroupReturns);
  FACTS_REGISTER(CGroupHelperReturns);
}

FACTS_MAIN
