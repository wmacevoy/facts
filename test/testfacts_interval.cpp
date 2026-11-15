// REQ3/WR-7: facts::interval and facts::pm in C++ facts.
// The first three groups are darter's own interval tests, where the type comes from.
#include <limits>
#include "facts.h"

using facts::interval;
using facts::pm;

static const double eps = std::numeric_limits<double>::epsilon();

FACTS(OpenInterval) {
  auto oi = interval::open(0, 1);

  FACT(0, !=, oi);
  FACT(1, !=, oi);
  FACT(0 + eps, ==, oi);
  FACT(1 - eps, ==, oi);
  FACT(0 - eps, !=, oi);
  FACT(1 + eps, !=, oi);

  double a = 2, b = 5;
  auto ab = a + (b - a) * oi;
  FACT(a * (1 - eps), !=, ab);
  FACT(a, !=, ab);
  FACT(a * (1 + eps), ==, ab);
  FACT(b * (1 - eps), ==, ab);
  FACT(b, !=, ab);
  FACT(b * (1 + eps), !=, ab);

  FACT(-a, !=, -1.0 * ab);
  FACT(-b, !=, -1.0 * ab);
  FACT(-(a + b) / 2, ==, -1.0 * ab);
}

FACTS(ClosedInterval) {
  auto ci = interval::closed(0, 1);

  FACT(0, ==, ci);
  FACT(1, ==, ci);
  FACT(0 - eps, !=, ci);
  FACT(1 + eps, !=, ci);

  double a = 2, b = 5;
  auto ab = a + (b - a) * ci;
  FACT(a * (1 - eps), !=, ab);
  FACT(a, ==, ab);
  FACT(b, ==, ab);
  FACT(b * (1 + eps), !=, ab);
  FACT(-a, ==, -1.0 * ab);
  FACT(-b, ==, -1.0 * ab);
}

// A negative factor turns the interval around, and its ends go with it.
FACTS(ClosedOpenInterval) {
  auto coi = interval(0, 1, true, false);

  FACT(0, ==, coi);
  FACT(1, !=, coi);

  double a = 2, b = 5;
  auto ab = a + (b - a) * coi;
  FACT(a, ==, ab);
  FACT(b, !=, ab);
  FACT(-a, ==, -1.0 * ab);
  FACT(-b, !=, -1.0 * ab);
}

// The expression form, as a test would use it.
FACTS(Tolerance) {
  double x = 0.1 * 7;
  FACT(x == 0.7 + pm(1e-12));           // absolute
  FACT(x == 0.7 * (1 + pm(1e-12)));     // relative
  FACT(x != 0.7 + pm(0));               // 0.1*7 is not exactly 0.7
  FACT(3 == 3 + pm(0));                 // an int on the left
  FACT(-2.0 == -2.0 * (1 + pm(1e-9)));  // a negative center
  FACT(1.0 != 1.0 + pm(-1));            // a negative tolerance holds nothing
}

FACTS(OutsideAbsolute) {
  double x = 2.5;
  FACT(x == 2.0 + pm(0.25));
}

FACTS(OutsideRelative) {
  double x = 1.0 + 1e-6;
  FACT(x == 1.0 * (1 + pm(1e-9)), "a millionth off, a billionth allowed");
}

FACTS(OutsideThreeArguments) {
  double x = 2.5;
  FACT(x, ==, 2.0 + pm(0.25));
}

FACTS(InsideWhenItShouldNotBe) {
  double x = 0.5;
  FACT_CHECK(x != interval::open(0, 1));
  FACT_CHECK(x != interval(0.5, 1, true, false));
  FACT_CHECK(x != interval(0.5, 1, false, false));   // true: 0.5 is not inside (0.5,1)
}

FACTS_REGISTER_ALL() {
  FACTS_REGISTER(OpenInterval);
  FACTS_REGISTER(ClosedInterval);
  FACTS_REGISTER(ClosedOpenInterval);
  FACTS_REGISTER(Tolerance);
  FACTS_REGISTER(OutsideAbsolute);
  FACTS_REGISTER(OutsideRelative);
  FACTS_REGISTER(OutsideThreeArguments);
  FACTS_REGISTER(InsideWhenItShouldNotBe);
}

FACTS_MAIN
