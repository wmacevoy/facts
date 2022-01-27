#include "limits.h"
#include "factcheck.h"

FACT_CHECK(AboutLogic) {
  FACT(! 0,==,1);
  FACT(! 1,==,0);
}

FACT_CHECK(AboutInts) {
  FACT(0+1,>,0);
  FACT(1+1,>,1);
  FACT(2+1,>,2);
  FACT(INT_MAX+1,>,INT_MAX); // fiction
}

FACT_CHECK_DONE
