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

static int matches(const char *str, const char *pattern) {
  uint8_t np = strlen(pattern);
  uint8_t ns = strlen(str);
  uint8_t nb = (np+2)/8+((np+2)%8 != 0);
  uint8_t k;
  uint8_t buffer[8];

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

void FactsFiction(const char *file, int line, Facts *facts) {
  ++facts_fictions;
}


void FactsInclude(const char *pattern) {
  if (head == NULL) { FactsRegister(); }  
  for (Facts *facts = head; facts != NULL; facts=facts->next) {
    if (matches(facts->name,pattern)) {
      facts->status = 0;
    }
  }
}

void FactsExclude(const char *pattern) {
  if (head == NULL) { FactsRegister(); }  
  for (Facts *facts = head; facts != NULL; facts=facts->next) {
    if (matches(facts->name,pattern)) {
      facts->status = -2;
    }
  }
}

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

double FactsRelErr(double a, double b) {
  double max = (a > b) ? a : b;
  return fabs(a-b)/max;
}

double FactsAbsErr(double a, double b) {
  return fabs(a-b);
}

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

void FactsCheck() {
  if (head == NULL) { FactsRegister(); }
  for (Facts *facts = head; facts != NULL; facts=facts->next) {
    if (facts->status == 0) {
      fprintf(stderr,"facts %s started\n",facts->name);
      facts->function(facts);
      fprintf(stderr,"facts %s ended%s\n",facts->name,(facts->status == -1 ? " (badly)" : ""));
    } else if (facts->status == -2) {
      fprintf(stderr,"facts %s skipped.\n",facts->name);
    }
  }

  fprintf(stderr,"facts summary.\n");    
  for (Facts *facts = head; facts != NULL; facts=facts->next) {
    if (facts->status == 1) {
	fprintf(stderr,"facts %s passed\n",facts->name);
    }
  }
  for (Facts *facts = head; facts != NULL; facts=facts->next) {
    if (facts->status == -1) {
	fprintf(stderr,"facts %s failed\n",facts->name);
    }
  }
  for (Facts *facts = head; facts != NULL; facts=facts->next) {
    if (facts->status == -2) {
	fprintf(stderr,"facts %s skipped\n",facts->name);
    }
  }
  for (Facts *facts = head; facts != NULL; facts=facts->next) {
    if (facts->status < -2 || 1 < facts->status) {
      fprintf(stderr,"facts %s status %d\n",facts->name,facts->status);
    }
  }
}
