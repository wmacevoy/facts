/* REQ1/WR-4: FactsAbsErr and FactsRelErr.  Every fact here is a truth. */
#include "facts.h"

FACTS(AbsErr) {
  FACT(FactsAbsErr(1.0, 3.0),==,2.0);
  FACT(FactsAbsErr(3.0, 1.0),==,2.0);     /* either order */
  FACT(FactsAbsErr(-1.0, 1.0),==,2.0);
  FACT(FactsAbsErr(2.5, 2.5),==,0.0);
}

/* The error relative to the larger of the two magnitudes. */
FACTS(RelErr) {
  FACT(FactsRelErr(1.0, 2.0),==,0.5);
  FACT(FactsRelErr(2.0, 1.0),==,0.5);     /* either order */
  FACT(FactsRelErr(-1.0, -2.0),==,0.5);   /* sign of the pair does not matter */
  FACT(FactsRelErr(-1.0, 1.0),==,2.0);    /* opposite signs: as far apart as it gets */
  FACT(FactsRelErr(0.0, 4.0),==,1.0);
  FACT(FactsRelErr(4.0, 4.0),==,0.0);
  FACT(FactsRelErr(0.0, 0.0),==,0.0);     /* no division by zero */
  FACT(FactsRelErr(100.0, 101.0),<,0.01);
  FACT(FactsRelErr(100.0, 102.0),>,0.01);
}

/* As the README uses them. */
FACTS(Tolerances) {
  double a = 0.1 * 7, b = 0.7;
  FACT(a,!=,b);
  FACT(FactsAbsErr(a,b),<,1e-12);
  FACT(FactsRelErr(a,b),<,1e-12);
  FACT(FactsRelErr(1e300, 1.0000001e300),<,1e-6);   /* large values do not overflow it */
  FACT(FactsAbsErr(1e-300, 2e-300),<,1e-12);
  FACT(FactsRelErr(1e-300, 2e-300),==,0.5);         /* small values keep their relative error */
}

FACTS_REGISTER_ALL() {
  FACTS_REGISTER(AbsErr);
  FACTS_REGISTER(RelErr);
  FACTS_REGISTER(Tolerances);
}

FACTS_MAIN
