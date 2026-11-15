/* REQ1/SX-1, REQ1/SX-2: groups that end a run abnormally.  Each is excluded
   by its declaration, so the check runs exactly one at a time by name. */
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "facts.h"

FACTS_EXCLUDE(Crashes) {
  FACT(1,==,1);
  raise(SIGSEGV);
}

FACTS_EXCLUDE(Aborts) {
  FACT(1,==,1);
  abort();
}

/* Hangs while asleep. */
FACTS_EXCLUDE(Sleeps) {
  FACT(1,==,1);
  for (;;) sleep(1000);
}

/* Hangs while busy. */
FACTS_EXCLUDE(Spins) {
  volatile unsigned long n = 0;
  FACT(1,==,1);
  for (;;) ++n;
}

FACTS(Fine) {
  FACT(1,==,1);
}

FACTS_REGISTER_ALL() {
  FACTS_REGISTER(Crashes);
  FACTS_REGISTER(Aborts);
  FACTS_REGISTER(Sleeps);
  FACTS_REGISTER(Spins);
  FACTS_REGISTER(Fine);
}

FACTS_MAIN
