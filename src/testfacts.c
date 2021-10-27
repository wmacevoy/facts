#include "facts.h"

FACTS(Alpha) {
  FACT_FMT('c',<=,'b',"%c");
}

FACTS(Numeric1) {
  FACT(1,==,2);
}

FACTS(Numeric2) {
  FACT(1.0,==,2.0);
}

FACTS_FINISHED

void main() {
  FactsExclude("*ph*a*");
  FactsCheck();
}
