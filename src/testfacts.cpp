#include "facts.h"

FACTS(Alpha) {
  FACT('c',<=,'b');
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
  std::cerr << "message to cerr." << std::endl;
}

FACTS(LargeErr) {
  for (int i=0; i<100; ++i) {
    std::cerr << "message #" << i << " to cerr." << std::endl;
  }
}

FACTS(Out) {
  std::cout << "message to cout." << std::endl;
}

FACTS_FAST
