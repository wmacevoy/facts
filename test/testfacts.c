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

FACTS(Err) {
  fprintf(stderr,"message to stderr.\n");
}

FACTS(LargeErr) {
  for (int i=0; i<100; ++i) {
    fprintf(stderr,"message #%d to stderr.\n",i);
  }
}

FACTS(Out) {
  fprintf(stdout,"message to stderr.\n");
}

FACTS_FAST
