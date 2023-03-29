#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS 1
#endif

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 1
#endif

#include <inttypes.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#define FACTS_C 1
#include "facts.h"
static Facts *head = NULL, *tail = NULL;

uint64_t facts_fictions = 0;
uint64_t facts_truths = 0;
int facts_format = 0;

FACTS_EXTERN void FactsFind();
FACTS_EXTERN void FactsRegisterAll();

// Wildcard pattern matcher.
//
// Desiged to work on small systems with possibly no regex library.
//
//
// 5+2*sizeof(void*)+strlen(pattern)/4
//
// bytes of (stack) memory.
FACTS_EXTERN int FactsMatches(const char *str, const char *pattern)
{
  uint8_t np = strlen(pattern);

  // fast exit for match-all pattern
  if (np == 1 && pattern[0] == '*')
    return 1;

  uint8_t ns = strlen(str);

  uint8_t nb = (np + 2) / 8 + ((np + 2) % 8 != 0);
  uint8_t k;

  uint8_t buffer0[nb];
  uint8_t buffer1[nb];

  uint8_t *state0 = buffer0;
  uint8_t *state1 = buffer1;

  memset(state0, 0, nb);
  state0[0] = 1;
  for (k = 1; pattern[k - 1] == '*'; ++k)
    state0[k / 8] |= (1 << (k % 8));

  for (int i = 0; i <= ns; ++i)
  {
    uint8_t c = str[i];

    memset(state1, 0, nb);
    for (int j = 0; j <= np; ++j)
    {
      if (state0[j / 8] & (1 << (j % 8)))
      {
        if (pattern[j] == '*')
        {
          k = j;
          state1[k / 8] |= (1 << (k % 8));
          ++k;
          state1[k / 8] |= (1 << (k % 8));
        }
        else if (pattern[j] == c)
        {
          k = j + 1;
          state1[k / 8] |= (1 << (k % 8));
          while (pattern[k] == '*')
          {
            ++k;
            state1[k / 8] |= (1 << (k % 8));
          }
        }
      }
    }

    uint8_t *tmp = state0;
    state0 = state1;
    state1 = tmp;
  }

  k = np + 1;
  return (state0[k / 8] & (1 << (k % 8))) != 0;
}

// Fiction break point.
//
// This function is called whenever a FACT is actually
// fiction.  It counts the number of calls mostly so
// the optimizer will not remove it.
//
// It is really provided as an easy debug break point when
// tracing a fact check that fails.

FACTS_EXTERN void FactsFiction(const char *file, int line, Facts *facts,
			       const char *a, const char *op, const char *b)
{
  printf("%s(gdb)%s break facts_%s_function\n",FACTS_GREEN,FACTS_RESET,facts->name);
  #ifdef __cplusplus
      FactsTrace::notes();
  #endif
  printf("%s(gdb)%s run --facts_include=%s\n",FACTS_GREEN,FACTS_RESET,facts->name);
}

// Include FACTS to check with wildncard pattern.

FACTS_EXTERN void FactsInclude(const char *pattern)
{
  if (head == NULL)
  {
    FactsRegisterAll();
    for (Facts *facts = head; facts != NULL; facts = facts->next)
    {
      if (facts->status == FACTS_STATE_INCLUDE)
      {
        facts->status = FACTS_STATE_EXCLUDE;
      }
    }
  }
  for (Facts *facts = head; facts != NULL; facts = facts->next)
  {
    if (facts->status == FACTS_STATE_EXCLUDE && FactsMatches(facts->name, pattern))
    {
      facts->status = FACTS_STATE_INCLUDE;
    }
  }
}

// Exclude facts with wildcard pattern.
// Normally all FACTS are checked.
FACTS_EXTERN void FactsExclude(const char *pattern)
{
  if (head == NULL)
  {
    FactsRegisterAll();
  }
  for (Facts *facts = head; facts != NULL; facts = facts->next)
  {
    if (facts->status == FACTS_STATE_INCLUDE && FactsMatches(facts->name, pattern))
    {
      facts->status = FACTS_STATE_EXCLUDE;
    }
  }
}

FACTS_EXTERN void FactsRegister(Facts *facts)
{
  if (facts->prev == NULL && facts->next == NULL)
  {
    facts->prev = tail;
    facts->next = NULL;
    if (tail != NULL)
    {
      tail->next = facts;
    }
    if (head == NULL)
    {
      head = facts;
    }
    tail = facts;
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

FACTS_EXTERN void FactsFindInMemory(Facts *begin, Facts *end)
{
  if (head != NULL || tail != NULL)
  {
    return;
  }
  unsigned char *sig = &begin->sig[0];
  int delta = sig - (unsigned char *)begin;
  int reversed = 0;
  if (end < begin)
  {
    Facts *tmp = end;
    end = begin;
    begin = tmp;
    reversed = 1;
  }

  unsigned char *b = ((unsigned char *)begin);
  unsigned char *e = ((unsigned char *)end) + sizeof(Facts);

  for (unsigned char *p = b;
       p != NULL && p < e;
       p = (unsigned char *)memchr(p + FACTS_SIG_LEN, sig[0], e - p))
  {
    if (memcmp(p, sig, FACTS_SIG_LEN) == 0)
    {
      Facts *facts = (Facts *)(p - delta);

      if (facts->name != NULL && facts->function != NULL && facts->prev == NULL && facts->next == NULL)
      {
        if (strcmp(facts->name, "0000_BEGIN") == 0 ||
            strcmp(facts->name, "zzzz_END") == 0)
          continue;
        if (reversed)
        {
          facts->next = head;
          facts->prev = NULL;
          if (head != NULL)
          {
            head->prev = facts;
          }
          if (tail == NULL)
          {
            tail = facts;
          }
          head = facts;
        }
        else
        {
          facts->prev = tail;
          facts->next = NULL;
          if (tail != NULL)
          {
            tail->next = facts;
          }
          if (head == NULL)
          {
            head = facts;
          }
          tail = facts;
        }
      }
    }
  }
}

// Symmetric relative error.
FACTS_EXTERN double FactsRelErr(double a, double b)
{
  double abserr = a >= b ? a - b : b - a;
  a = a >= 0 ? a : -a;
  b = b >= 0 ? b : -b;
  double maxabs = a >= b ? a : b;
  return abserr / maxabs;
}

// Absolute error.
FACTS_EXTERN double FactsAbsErr(double a, double b)
{
  double abserr = a >= b ? a - b : b - a;
  return abserr;
}

// Print with dynamic formats.
// format can contain %? patterns.  These
// are replaced with strings in the va-args
// list FIRST.  After the %? substitution,
// the remaing argmuments are processed with
// the modified format string is passed to
// vfprintf.
FACTS_EXTERN void FactsPrint(const char *format, ...)
{
  int i, j;
  int reformatSize = strlen(format) + 1;

  va_list ap;
  va_start(ap, format);
  for (i = 0, j = 0; format[i] != 0;)
  {
    if (format[i] == '%' && format[i + 1] == '?')
    {
      const char *fs = va_arg(ap, const char *);
      reformatSize += strlen(fs);
      i += 2;
    }
    else
    {
      i += 1;
    }
  }

  char reformat[reformatSize];
  va_start(ap, format);
  for (i = 0, j = 0; format[i] != 0;)
  {
    if (format[i] == '%' && format[i + 1] == '?')
    {
      const char *fs = va_arg(ap, const char *);
      strncpy(reformat + j, fs, reformatSize - j);
      j += strlen(fs);
      i += 2;
    }
    else
    {
      reformat[j] = format[i];
      ++j;
      ++i;
    }
  }
  reformat[++j] = 0;
  vprintf(reformat, ap);
  va_end(ap);
}

#define FACTS_BLOCKSIZE 1024
// Execute facts checks.
//
// You can preceed this with FactsInclude and FactsExlude to pick out
// a particular set.
//
//
FACTS_EXTERN void FactsCheck()
{
  if (head == NULL)
  {
    FactsRegisterAll();
  }

  FILE *tmpout = NULL, *tmperr = NULL;
  int oldout = -1, olderr = -1;
  if (FACTS_JUNIT)
  {
    printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    printf("<testsuite name=\"facts\">\n");
  }
  for (Facts *facts = head; facts != NULL; facts = facts->next)
  {
    if (FACTS_JUNIT)
    {
      printf("<testcase name=\"%s\">\n", facts->name);
      printf("<system-out>");      
      fflush(stderr);
      assert((tmperr = tmpfile()) != NULL);
      olderr = dup(STDERR_FILENO);
      assert(dup2(fileno(tmperr), STDERR_FILENO) >= 0);
    }
    if (facts->status == FACTS_STATE_INCLUDE)
    {
      printf("%s %d: %s facts check started\n",
             facts->file, facts->line, facts->name);
#ifdef __cplusplus
      FactsTrace::reset(facts);
#endif
      facts->function(facts);
      if (facts->status == FACTS_STATE_INCLUDE)
      {
        facts->status = FACTS_STATE_PASS;
      }
      if (facts->status == FACTS_STATE_FAIL) {
        printf("%s %d: %s facts check ended %sbadly%s\n",
               facts->file, facts->line, facts->name, FACTS_RED, FACTS_RESET);
      } else {
        printf("%s %d: %s facts check ended\n",
               facts->file, facts->line, facts->name);
      }
    }
    else
    {
      printf("%s %d: %s facts check %sexcluded%s.\n",
             facts->file, facts->line, facts->name, FACTS_RED, FACTS_RESET);
    }
    if (FACTS_JUNIT)
    {
      fflush(stdout);
      fflush(stderr);
      printf("</system-out>\n");      
      int64_t errlen = lseek(STDERR_FILENO, 0, SEEK_CUR);
      dup2(olderr, STDERR_FILENO);
      fseek(tmperr, 0L, SEEK_SET);
      if (errlen > 0) {
	printf("<system-err>");
	for (int64_t p = 0; p < errlen; p += FACTS_BLOCKSIZE)
	  {
	    char data[FACTS_BLOCKSIZE];
	    int n = errlen - p;
	    if (n > FACTS_BLOCKSIZE) {
	      n = FACTS_BLOCKSIZE;
	    }
	    assert(fread(data, n, 1, tmperr)==1);
	    assert(fwrite(data, n, 1, stdout)==1);
	  }
	printf("</system-err>\n");
      }
      if (facts->status == FACTS_STATE_EXCLUDE)
      {
        printf("<skipped />\n");
      }
      if (facts->status == FACTS_STATE_FAIL)
      {
        printf("<failure>See stdout</failure>\n");
      }
      printf("</testcase>\n\n");
      fclose(tmperr);
      close(olderr);
    }
  }

  if (FACTS_CONSOLE)
  {
    printf("facts summary.\n");
    for (Facts *facts = head; facts != NULL; facts = facts->next)
    {
      if (facts->status == FACTS_STATE_PASS)
      {
        printf("facts check %s %spassed%s\n", facts->name, FACTS_GREEN, FACTS_RESET);
      }
    }
    for (Facts *facts = head; facts != NULL; facts = facts->next)
    {
      if (facts->status == FACTS_STATE_FAIL)
      {
        printf("facts check %s %sfailed%s\n", facts->name, FACTS_RED, FACTS_RESET);
      }
    }
    for (Facts *facts = head; facts != NULL; facts = facts->next)
    {
      if (facts->status == FACTS_STATE_EXCLUDE)
      {
        printf("facts check %s %sexcluded%s\n", facts->name, FACTS_RED, FACTS_RESET);
      }
    }
    for (Facts *facts = head; facts != NULL; facts = facts->next)
    {
      if (facts->status != FACTS_STATE_PASS &&
          facts->status != FACTS_STATE_FAIL &&
          facts->status != FACTS_STATE_EXCLUDE)
      {
        printf("facts check %s status %d\n", facts->name, facts->status);
      }
    }
    double checks = ((double)facts_truths) + ((double)facts_fictions);
    double rate = 100.0 / (checks > 0.0 ? checks : 1.0);
    printf("%" PRIu64 " (%1.1f%%) truths and %" PRIu64 " (%1.1f%%) fictions checked.\n",
           facts_truths, facts_truths * rate, facts_fictions, facts_fictions * rate);
  }

  if (FACTS_JUNIT)
  {
    printf("</testsuite>\n");
  }
}

//
// You can call this from your main to process facts
// checks.  Return status 0 is good, it means a fact check was
// called and passed all (at least one) FACT check, 1 means at
// least one FACT failed, and 2 means no facts were checked.
//
//
FACTS_EXTERN int FactsMain(int argc, const char *argv[])
{
  int status = 0;
  int check = 1;
  for (int argi = 1; argi < argc; ++argi)
  {
    const char *arg = (argi < argc) ? argv[argi] : NULL;
    {
      const char *op = "--facts_include=";
      if (strncmp(arg, op, strlen(op)) == 0)
      {
        FactsInclude(arg + strlen(op));
        continue;
      }
    }
    {
      const char *op = "--facts_exclude=";
      if (strncmp(arg, op, strlen(op)) == 0)
      {
        FactsExclude(arg + strlen(op));
        continue;
      }
    }
    {
      const char *op = "--facts_find";
      if (strcmp(arg, op) == 0)
      {
        FactsFind();
        continue;
      }
    }

    {
      const char *op = "--facts_register_all";
      if (strcmp(arg, op) == 0)
      {
        check = 0;
        FactsFind();
        printf("FACTS_REGISTER_ALL() {\n");
        for (Facts *facts = head; facts != NULL; facts = facts->next)
        {
          printf("    FACTS_REGISTER(%s);\n", facts->name);
        }
        printf("}\n");
        continue;
      }
    }
    {
      const char *op = "--facts_skip";
      if (strcmp(arg, op) == 0)
      {
        check = 0;
        continue;
      }
    }
    {
      const char *op = "--facts_junit";
      if (strcmp(arg, op) == 0)
      {
        facts_format |= 1; // FACTS_JUNIT FLAG
        continue;
      }
    }
    {
      const char *op = "--facts_plain";
      if (strcmp(arg, op) == 0)
      {
        facts_format |= 2; // FACTS NO COLORS FLAG
        continue;
      }
    }
    {
      const char *op = "--facts_help";
      if (strcmp(arg, op) == 0)
      {
        check = 0;
        printf("default is to check all registered facts\n");
        printf("    --facts_include=\"*wildcard pattern*\"\n --- include certain facts\n");
        printf("    --facts_exclude=\"*wildcard pattern*\"\n --- exclude certain facts\n");
        printf("    --facts_register_all --- auto* generate FACTS_REGISTER_ALL\n");
        printf("    --facts_find --- auto* find facts\n");
        printf("    --facts_skip --- don't fact check\n");
        printf("    --facts_help --- this help\n");
        printf("    --facts_junit --- use junit format\n");
        printf("    --facts_plain --- no tty colors\n");
        printf("    * Optimized executables may miss auto facts.\n");
        printf("      Use explicit FACTS_REGISTER_ALL() {...} for reliable fact checking.\n");
        continue;
      }
    }
  }

  if (check)
  {
    FactsCheck();
  }

  return (facts_fictions == 0) ? 0 : 1;
}
