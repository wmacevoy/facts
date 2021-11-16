#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include <stdint.h>

#define FACTS_C 1
#include "facts.h"
static Facts *head = NULL, *tail=NULL;

int facts_fictions = 0;
int facts_truths = 0;
void FactsRegister();

// Wildcard pattern match.
//
// Desiged to work on small
// systems with possibly no regex library using
//
// 5+2*sizeof(void*)+strlen(pattern)/4
//
// bytes of (stack) memory.  
static int matches(const char *str, const char *pattern) {
  uint8_t np = strlen(pattern);

  // fast exit for match-all pattern
  if (np == 1 && pattern[0] == '*') return 1;
  
  uint8_t ns = strlen(str);
  
  uint8_t nb = (np+2)/8+((np+2)%8 != 0);
  uint8_t k;

  uint8_t buffer0[nb];
  uint8_t buffer1[nb];

  uint8_t *state0=buffer0;
  uint8_t *state1=buffer1;

  memset(state0,0,nb);
  state0[0]=1;
  for (k=1; pattern[k-1] == '*'; ++k) state0[k/8] |= (1 << (k%8));

  for (int i=0; i<=ns; ++i) {
    uint8_t c=str[i];

    memset(state1,0,nb);
    for (int j=0; j<=np; ++j) {
      if (state0[j/8] & (1 << (j%8))) {
        if (pattern[j] == '*') {
          k=j;
          state1[k/8] |= (1 << (k%8));
          ++k;
          state1[k/8] |= (1 << (k%8));
        } else if (pattern[j] == c) {
          k=j+1;
          state1[k/8] |= (1 << (k%8));
          while (pattern[k] == '*') {
            ++k; 
            state1[k/8] |= (1 << (k%8));
          }
        }
      }
    }

    uint8_t *tmp = state0;
    state0 = state1;
    state1 = tmp;
  }

  k=np+1;
  return (state0[k/8]&(1<<(k%8))) != 0;
}

// Fiction break point.
//
// This function is called whenever a FACT is actually
// fiction.  It counts the number of calls mostly so
// the optimizer will not remove it.
//
// It is really provided as an easy debug break point when
// tracing a fact check that fails.

void FactsFiction(const char *file, int line, Facts *facts) {
  ++facts_fictions;
}


// Include FACTS to check with wildcard pattern.
//
// By default all facts are checked, so this is
// useful only if some were excluded.
void FactsInclude(const char *pattern) {
  if (head == NULL) { FactsRegister(); }  
  for (Facts *facts = head; facts != NULL; facts=facts->next) {
    if (matches(facts->name,pattern)) {
      facts->status = 0;
    }
  }
}

// Exclude facts with wildcard pattern.
// Normally all FACTS are checked,
// but
void FactsExclude(const char *pattern) {
  if (head == NULL) { FactsRegister(); }  
  for (Facts *facts = head; facts != NULL; facts=facts->next) {
    if (matches(facts->name,pattern)) {
      facts->status = -2;
    }
  }
}

// Fact find (internals).
//
// C does not provide a way to initialize a list of
// FACTS checks.  So we search memory for them by the
// signature (a random byte pattern) each FACTS check
// creates.
//
// The test declarations are bracketed in source by
// the #include "facts.h" to begin and FACTS_FINISHED
// at the end.  FACTS_FINISHED creates a function that
// calls this with two book-end tests that are ignored.
//
void FactsFindInMemory(unsigned char *begin, unsigned char *end) {
  int reversed = 0;
  if (end < begin) {
    void *tmp = end;
    end = begin;
    begin = tmp;
    reversed = 1;
  }
  end = end - 16;
  
  unsigned char *p = (unsigned char*) begin;
  for (unsigned char *p = begin; p != NULL && p < end; p = (unsigned char*) memchr(p+16,0xe3,end-p)) {
    if (p[ 0]==0xe3 && p[ 1]==0xb0 && p[ 2]==0xc4 && p[ 3]==0x42 &&
	p[ 4]==0x98 && p[ 5]==0xfc && p[ 6]==0x1c && p[ 7]==0x14 &&
	p[ 8]==0x9a && p[ 9]==0xfb && p[10]==0xf4 && p[11]==0x89 &&
	p[12]==0x96 && p[13]==0xfb && p[14]==0x92 && p[15]==0x00) {

      Facts *facts = (Facts*)p;

      if (facts->name != NULL && facts->function != NULL && facts->prev == NULL && facts->next == NULL) {
	if (strcmp(facts->name,"0000_BEGIN")==0) continue;
	if (strcmp(facts->name,"zzzz_END")==0) continue;
	if (reversed) {
	  facts->next = head;
	  facts->prev = NULL;
	  if (head != NULL) { head->prev = facts; }
	  if (tail == NULL) { tail = facts; }
	  head = facts;
	} else {
	  facts->prev = tail;
	  facts->next = NULL;
	  if (tail != NULL) { tail->next = facts; }
	  if (head == NULL) { head = facts; }
	  tail = facts;
	}
      }
    }
  }
}

// Symmetric relative error.
double FactsRelErr(double a, double b) {
  return fabs(b-a)/fmax(fabs(a),fabs(b));
}

// Absolute error.
double FactsAbsErr(double a, double b) {
  return fabs(a-b);
}

// Print with dynamic formats.
// format can contain %? patterns.  These
// are replaced with strings in the va-args
// list FIRST.  After the %? substitution,
// the remaing argmuments are processed with
// the modified format string is passed to
// vfprintf.
void FactsPrint(const char *format,...) {
  int i=0,j=0;
  char reformat[80];
  va_list ap;
  va_start(ap, format);
  for (i=0; format[i] != 0;) {
    if (format[i] == '%' && format[i+1] == '?') {
      const char *fs=va_arg(ap, const char *);
      strncpy(reformat+j,fs,80-j);
      j += strlen(fs);
      i += 2;
    } else {
      reformat[j]=format[i];
      ++j; ++i;
    }
    if (j >= sizeof(reformat)) break;
  }
  assert(j < sizeof(reformat)-1);
  reformat[++j]=0;
  vfprintf(stderr,reformat,ap);
  va_end(ap);
}

// Execute facts checks.
//
// You can preceed this with FactsInclude and FactsExlude to pick out
// a particular set.
//
//
void FactsCheck() {
  int fails = 0;
  if (head == NULL) { FactsRegister(); }
  for (Facts *facts = head; facts != NULL; facts=facts->next) {
    if (facts->status == 0) {
      fprintf(stderr,"%s %d: %s facts check started\n",
	      facts->file,facts->line,facts->name);
      facts->function(facts);
      if (facts->status == 0) { facts->status=1; }
      if (facts->status == -1) ++fails;
      fprintf(stderr,"%s %d: %s facts check ended%s\n",
	      facts->file,facts->line,facts->name,
	      (facts->status == -1 ? " (badly)" : ""));
    } else if (facts->status == -2) {
      fprintf(stderr,"%s %d: %s facts check skipped.\n",
	      facts->file,facts->line,facts->name);
    }
  }

  fprintf(stderr,"facts summary.\n");
  for (Facts *facts = head; facts != NULL; facts=facts->next) {
    if (facts->status == 1) {
    	fprintf(stderr,"facts check %s passed\n",facts->name);
    }
  }
  for (Facts *facts = head; facts != NULL; facts=facts->next) {
    if (facts->status == -1) {
    	fprintf(stderr,"facts check %s failed\n",facts->name);
    }
  }
  for (Facts *facts = head; facts != NULL; facts=facts->next) {
    if (facts->status == -2) {
    	fprintf(stderr,"facts check %s skipped\n",facts->name);
    }
  }
  for (Facts *facts = head; facts != NULL; facts=facts->next) {
    if (facts->status != 1 && facts->status != -1 && facts->status != -2) {
      fprintf(stderr,"facts check %s status %d\n",facts->name,facts->status);
    } 
  }
  double rate=100.0/fmax(facts_truths+facts_fictions,1);
  fprintf(stderr,"%d (%1.1f%%) truths and %d (%1.1f%%) fictions checked.\n",
	  facts_truths,facts_truths*rate,facts_fictions,facts_fictions*rate);
}

//
// You can call this from your main to process facts
// checks.  Return status 0 is good, it means a fact check was
// called and passed all (at least one) FACT check, 1 means at
// least one FACT failed, and 2 means no facts were checked.
//
//
int FactsMain(int argc, const char *argv[]) {
  int status = 2;
  int checked = 0;
  for (int argi=1; argi <= argc; ++argi) {
    const char *arg=(argi < argc) ? argv[argi] : NULL;
    {
      const char *op="--facts_include=";
      if (arg != NULL && strncmp(arg,op,strlen(op))==0) {
	FactsInclude(arg+strlen(op));
	continue;
      }
    }
    {
      const char *op="--facts_exclude=";
      if (arg != NULL && strncmp(arg,op,strlen(op))==0) {
	FactsExclude(arg+strlen(op));
	continue;
      }
    }
    {
      const char *op="--facts_check";
      if ((arg != NULL && strcmp(arg,op)==0) || (arg == NULL && !checked)) {
	FactsCheck();
	checked = 1;
	if (facts_fictions > 0) {
	  status = 1;
	} if (facts_truths > 0 && status == 2) {
	  status = 0;
	}
	continue;
      }
    }
  }
  return status;
}