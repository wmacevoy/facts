#include "facts.h"

FACTS(Alpha) {
  FACT_FMT('a',<=,'b',"%c");
}

FACTS(Numeric1) {
  FACT(1,!=,2);
}

FACTS(Numeric2) {
  FACT_FMT(1.0,==,2.0,"%f");
}

FACTS_FINISHED;

void main() {
  FactsInclude("*");
  FactsExclude("*ph*a*");
  FactsRun();
  printf("Alpha@%p\n",&facts_Alpha_data);
  printf("Numeric1@%p\n",&facts_Numeric1_data);
  printf("Numeric2@%p\n",&facts_Numeric2_data);    
}
