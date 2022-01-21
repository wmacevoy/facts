#include "facts.h"

FACTS(Alpha) {
  FACT_PRINT('c',<=,'b',"%c");
}

FACTS(Integer) {
  int a=1,b=2;
  FACT(a,==,b);
}

FACTS(Double) {
  double a=1.3, b=2.4;
  FACT(a,==,b);
}

// Works if optimizer does not rearrange memory... (uncomment)

// FACTS_FINISHED

// Works even if optimizer rearranges memory...
void FactsRegisterAll() {
  FACTS_REGISTER(Alpha);
  FACTS_REGISTER(Integer);
  FACTS_REGISTER(Double);
}

FACTS_MAIN
