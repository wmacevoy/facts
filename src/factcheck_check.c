#include "factcheck.h"

FACT_CHECK(Alpha) {
  FACT_PRINT('c',<=,'b',"%c");
}

FACT_CHECK(Integer) {
  int a=1,b=2;
  FACT(a,==,b);
}

FACT_CHECK(Double) {
  double a=1.3, b=2.4;
  FACT(a,==,b);
}

FACT_CHECK(Err) {
  fprintf(stderr,"message to stderr.\n");
}

FACT_CHECK(LargeErr) {
  for (int i=0; i<100; ++i) {
    fprintf(stderr,"message #%d to stderr.\n",i);
  }
}

FACT_CHECK(Out) {
  fprintf(stdout,"message to stderr.\n");
}

FACT_CHECK_DONE
