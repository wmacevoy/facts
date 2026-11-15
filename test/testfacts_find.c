/* REQ1/RG-2, REQ1/RG-3: a program with no FACTS_REGISTER_ALL block, as in the
   README's one-line workflow.  Its groups are known only through --facts_find. */
#include "facts.h"

FACTS(FoundFirst) {
  FACT(1,==,1);
}

FACTS(FoundSecond) {
  FACT(2,==,2);
}

FACTS_MAIN
