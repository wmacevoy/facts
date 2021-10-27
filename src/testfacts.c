#include "facts.h"

FACTS(Alpha) {
  FACT_FMT('c',<=,'b',"%c");
}

FACTS(Integer) {
  int a=1,b=2;
  FACT(a,==,b);
}

FACTS(Double) {
  double a=1.3, b=2.4;
  FACT(a,==,b);
}

FACTS_FINISHED

void main() {
  FactsInclude("*");
  FactsExclude("*ph*a*");
  FactsCheck();
}
