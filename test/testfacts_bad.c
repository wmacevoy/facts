/* REQ1/SX-1, REQ1/SX-2: groups that end a run abnormally.  Each is excluded
   by its declaration, so the check runs exactly one at a time by name. */
#include <signal.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#define sleep(seconds) Sleep((seconds) * 1000)
#else
#include <unistd.h>
#endif
#include "facts.h"

FACTS_EXCLUDE(Crashes) {
  FACT(1,==,1);
#ifdef _WIN32
  /* A real access violation, with the "program has stopped working" box off. */
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
  volatile int *nowhere = 0;
  *nowhere = 1;
#else
  raise(SIGSEGV);
#endif
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
