// REQ1/WR-5: in C++, FACT(a op b) and FACT(a op b, note).
#include <string>
#include <vector>
#include "facts.h"

static int calls = 0;
static int once() { ++calls; return calls; }

FACTS(ExprTruths) {
  int x=2, y=2;
  std::string s="abc";
  std::vector<int> v(3);
  FACT(x == y);
  FACT(x != 3);
  FACT(x < 3);
  FACT(x <= y);
  FACT(x + 1 > y);
  FACT(x >= y);
  FACT(s == "abc");
  FACT(v.size() == 3);
  FACT(true);
  FACT(x == 2 && y == 2);
  FACT(x,==,y);
}

FACTS(ExprEqual) {
  int x=1, y=2;
  FACT(x == y);
}

FACTS(ExprArithmetic) {
  int x=1, y=2;
  FACT(x + 1 < y);
}

FACTS(ExprString) {
  std::string s="abc";
  FACT(s == "abd");
}

FACTS(ExprBool) {
  bool ok=false;
  FACT(ok);
}

FACTS(ExprAnd) {
  int x=1, y=2;
  FACT(x == 1 && y == 3);
}

FACTS(ExprNote) {
  for (int i=0; i<5; ++i) {
    FACT(i*i < 4, "i=" << i);
  }
}

// A footnote is not evaluated for a truth.
FACTS(ExprNoteLazy) {
  int notes=0;
  FACT(1 == 1, "n=" << ++notes);
  FACT(notes == 0);
}

// Each side is evaluated once, also when the fact is fiction.
FACTS(ExprOnce) {
  calls=0;
  FACT_CHECK(once() == 5);
  FACT(calls == 1);
}

FACTS(ExprCheck) {
  int truths = FACT_CHECK(1 == 2) + FACT_CHECK(2 == 2);
  FACT_CHECK(truths == 1, "and the group goes on");
  FACT_CHECK(truths == 0, "truths=" << truths);
}

// REQ1/WR-5: a constant declared in a class and never defined outside it
// links: each side that is a number is held by value.
struct Limits { static const size_t MAX = 64; static const int LOW = -1; enum { ONE = 1 }; };

FACTS(ExprStaticConst) {
  size_t n = 64;
  FACT(n == Limits::MAX);
  FACT(Limits::LOW < Limits::ONE);
  FACT(n, ==, Limits::MAX);
  FACT_CHECK(Limits::MAX == n, "n=" << n);
}

FACTS_REGISTER_ALL() {
  FACTS_REGISTER(ExprTruths);
  FACTS_REGISTER(ExprEqual);
  FACTS_REGISTER(ExprArithmetic);
  FACTS_REGISTER(ExprString);
  FACTS_REGISTER(ExprBool);
  FACTS_REGISTER(ExprAnd);
  FACTS_REGISTER(ExprNote);
  FACTS_REGISTER(ExprNoteLazy);
  FACTS_REGISTER(ExprOnce);
  FACTS_REGISTER(ExprCheck);
  FACTS_REGISTER(ExprStaticConst);
}

FACTS_MAIN
