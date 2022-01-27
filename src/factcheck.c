#define __STDC_FORMAT_MACROS 1
#define _POSIX_C_SOURCE 1

#include <inttypes.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#define FACT_CHECK_C 1
#include "factcheck.h"

static struct FactCheckStruct *head = NULL, *tail = NULL;

uint64_t fact_fictions = 0;
uint64_t fact_truths = 0;
int fact_format = FACT_CONSOLE;

FACT_EXTERN void FactCheckRegisterAuto();
FACT_EXTERN void FactCheckRegisterAll();

// Wildcard pattern matcher.
//
// Desiged to work on small systems with possibly no regex library.
//
//
// 5+2*sizeof(void*)+strlen(pattern)/4
//
// bytes of (stack) memory.
FACT_EXTERN int FactCheckMatches(const char *str, const char *pattern)
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

FACT_EXTERN void FactIsFiction(const char *file, int line, struct FactCheckStruct *check,
			       const char *a, const char *op, const char *b)
{
  if (strcmp(check->file,file) == 0) {
    printf(FACT_GREEN "Debug fact_check_function_%s on line %d of file %s with a breakpoint on line %d." FACT_RESET "\n",
	   check->name,check->line,check->file,line);
  } else {
    printf(FACT_GREEN "Debug fact_check_function_%s on line %d of file %s with a breakpoint on line %d " FACT_RED " of file %s." FACT_RESET "\n",
	   check->name,check->line,check->file,line,file);
  }

  printf("For example in gdb:\n");
  printf("break fact_check_function_%s\n",check->name);
  printf("run --check_include=%s\n",check->name);
  printf("continue\n");
  if (strcmp(check->file,file)==0) {
    printf("break %d\n",line);
  } else {
    printf("break \"%s\":%d\n",file,line);
  }
  printf("print %s\n",a);
  printf("print %s\n",b);
  printf("print (%s) %s (%s)\n",a,op,b);
  printf("\n");
  
  ++fact_fictions;
}

// Include fact checks with wildncard pattern.

FACT_EXTERN void FactCheckInclude(const char *pattern)
{
  if (head == NULL)
  {
    FactCheckRegisterAll();
    for (struct FactCheckStruct *check = head; check != NULL; check = check->next)
    {
      if (check->status == FACT_STATE_INCLUDE)
      {
        check->status = FACT_STATE_EXCLUDE;
      }
    }
  }
  for (struct FactCheckStruct *check = head; check != NULL; check = check->next)
  {
    if (check->status == FACT_STATE_EXCLUDE && FactCheckMatches(check->name, pattern))
    {
      check->status = FACT_STATE_INCLUDE;
    }
  }
}

// Exclude fact checks with wildcard pattern.
FACT_EXTERN void FactCheckExclude(const char *pattern)
{
  if (head == NULL)
  {
    FactCheckRegisterAll();
  }
  for (struct FactCheckStruct *check = head; check != NULL; check = check->next)
  {
    if (check->status == FACT_STATE_INCLUDE && FactCheckMatches(check->name, pattern))
    {
      check->status = FACT_STATE_EXCLUDE;
    }
  }
}

FACT_EXTERN void FactCheckRegister(struct FactCheckStruct *check)
{
  if (check->prev == NULL && check->next == NULL)
  {
    check->prev = tail;
    check->next = NULL;
    if (tail != NULL)
    {
      tail->next = check;
    }
    if (head == NULL)
    {
      head = check;
    }
    tail = check;
  }
}

// Fact find (internals).
//
// C does not provide a way to initialize a list of
// fact checks.  So we search memory for them by the
// signature (a random byte pattern) each fact check
// creates.

FACT_EXTERN void FactCheckFindInMemory(struct FactCheckStruct *begin, struct FactCheckStruct *end)
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
    struct FactCheckStruct *tmp = end;
    end = begin;
    begin = tmp;
    reversed = 1;
  }

  unsigned char *b = ((unsigned char *)begin);
  unsigned char *e = ((unsigned char *)end) + sizeof(FactCheck);

  for (unsigned char *p = b;
       p != NULL && p < e;
       p = (unsigned char *)memchr(p + FACT_SIG_LEN, sig[0], e - p))
  {
    if (memcmp(p, sig, FACT_SIG_LEN) == 0)
    {
      struct FactCheckStruct *check = (struct FactCheckStruct *)(p - delta);

      if (check->name != NULL && check->function != NULL && check->prev == NULL && check->next == NULL)
      {
        if (strcmp(check->name, "0000_BEGIN") == 0 ||
            strcmp(check->name, "zzzz_END") == 0)
          continue;
        if (reversed)
        {
          check->next = head;
          check->prev = NULL;
          if (head != NULL)
          {
            head->prev = check;
          }
          if (tail == NULL)
          {
            tail = check;
          }
          head = check;
        }
        else
        {
          check->prev = tail;
          check->next = NULL;
          if (tail != NULL)
          {
            tail->next = check;
          }
          if (head == NULL)
          {
            head = check;
          }
          tail = check;
        }
      }
    }
  }
}

// Symmetric relative error.
FACT_EXTERN double FactRelErr(double a, double b)
{
  double abserr = a >= b ? a - b : b - a;
  a = a >= 0 ? a : -a;
  b = b >= 0 ? b : -b;
  double maxabs = a >= b ? a : b;
  return abserr / maxabs;
}

// Absolute error.
FACT_EXTERN double FactAbsErr(double a, double b)
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
FACT_EXTERN void FactPrint(const char *format, ...)
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

#define FACT_BLOCKSIZE 1024
// Execute fact checks.
//
// You can preceed this with FactCheckInclude and FactCheckExlude to pick out
// a particular set.
FACT_EXTERN void FactCheck()
{
  int fails = 0;
  if (head == NULL)
  {
    FactCheckRegisterAll();
  }

  FILE *tmpout = NULL, *tmperr = NULL;
  int oldout = -1, olderr = -1;
  if (fact_format == FACT_JUNIT)
  {
    printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    printf("<testsuite name=\"fact\">\n");
  }
  for (struct FactCheckStruct *check = head; check != NULL; check = check->next)
  {
    if (fact_format == FACT_JUNIT)
    {
      printf("<testcase name=\"%s\">\n", check->name);
      printf("<system-out>");      
      fflush(stderr);
      assert((tmperr = tmpfile()) != NULL);
      olderr = dup(STDERR_FILENO);
      assert(dup2(fileno(tmperr), STDERR_FILENO) >= 0);
    }
    if (check->status == FACT_STATE_INCLUDE)
    {
      printf("%s %d: fact check %s started\n",
             check->file, check->line, check->name);
      check->function(check);
      if (check->status == FACT_STATE_INCLUDE)
      {
        check->status = FACT_STATE_PASS;
      }
      if (check->status == FACT_STATE_FAIL)
      {
        ++fails;
      }
      printf("%s %d: fact check %s ended%s\n",
             check->file, check->line, check->name,
             (check->status == FACT_STATE_FAIL ? " " FACT_RED "badly" FACT_RESET : ""));
    }
    else
    {
      printf("%s %d: fact check %s " FACT_RED "excluded" FACT_RESET ".\n",
             check->file, check->line, check->name);
    }
    if (fact_format == FACT_JUNIT)
    {
      fflush(stdout);
      fflush(stderr);
      printf("</system-out>\n");      
      int64_t errlen = lseek(STDERR_FILENO, 0, SEEK_CUR);
      dup2(olderr, STDERR_FILENO);
      fseek(tmperr, 0L, SEEK_SET);
      if (errlen > 0) {
	printf("<system-err>");
	for (int64_t p = 0; p < errlen; p += FACT_BLOCKSIZE)
	  {
	    char data[FACT_BLOCKSIZE];
	    int n = errlen - p;
	    if (n > FACT_BLOCKSIZE)
	      n = FACT_BLOCKSIZE;
	    fread(data, n, 1, tmperr);
	    fwrite(data, n, 1, stdout);
	  }
	printf("</system-err>\n");
      }
      if (check->status == FACT_STATE_EXCLUDE)
      {
        printf("<skipped />\n");
      }
      if (check->status == FACT_STATE_FAIL)
      {
        printf("<failure>See stdout</failure>\n");
      }
      printf("</testcase>\n\n");
      fclose(tmperr);
      close(olderr);
    }
  }

  if (fact_format == FACT_CONSOLE)
  {
    printf("Fact check summary.\n");
    for (struct FactCheckStruct *check = head; check != NULL; check = check->next)
    {
      if (check->status == FACT_STATE_PASS)
      {
        printf("Fact check %s " FACT_GREEN "passed" FACT_RESET "\n", check->name);
      }
    }
    for (struct FactCheckStruct *check = head; check != NULL; check = check->next)
    {
      if (check->status == FACT_STATE_FAIL)
      {
        printf("Fact check %s " FACT_RED "failed" FACT_RESET "\n", check->name);
      }
    }
    for (struct FactCheckStruct *check = head; check != NULL; check = check->next)
    {
      if (check->status == FACT_STATE_EXCLUDE)
      {
        printf("Fact check %s " FACT_RED "excluded" FACT_RESET "\n", check->name);
      }
    }
    for (struct FactCheckStruct *check = head; check != NULL; check = check->next)
    {
      if (check->status != FACT_STATE_PASS &&
          check->status != FACT_STATE_FAIL &&
          check->status != FACT_STATE_EXCLUDE)
      {
        printf("Fact check %s " FACT_RED "status %d" FACT_RESET "\n", check->name, check->status);
      }
    }
    double checks = ((double)fact_truths) + ((double)fact_fictions);
    double rate = 100.0 / (checks > 0.0 ? checks : 1.0);
    printf("%" PRIu64 " (%1.1f%%) truths and %" PRIu64 " (%1.1f%%) fictions checked.\n",
           fact_truths, fact_truths * rate, fact_fictions, fact_fictions * rate);
  }

  if (fact_format == FACT_JUNIT)
  {
    printf("</testsuite>\n");
  }
}

//
// You can call this from your main to process fact checks
// checks.  Return status 0 is means no checked fact failed.
FACT_EXTERN int FactCheckMain(int argc, const char *argv[])
{
  int status = 0;
  int check = 1;
  for (int argi = 1; argi < argc; ++argi)
  {
    const char *arg = (argi < argc) ? argv[argi] : NULL;
    {
      const char *op = "--check_include=";
      if (strncmp(arg, op, strlen(op)) == 0)
      {
        FactCheckInclude(arg + strlen(op));
        continue;
      }
    }
    {
      const char *op = "--check_exclude=";
      if (strncmp(arg, op, strlen(op)) == 0)
      {
        FactCheckExclude(arg + strlen(op));
        continue;
      }
    }

    {
      const char *op = "--check_register_all";
      if (strcmp(arg, op) == 0)
      {
        check = 0;
        FactCheckRegisterAuto();
        printf("FACT_CHECK_REGISTER_ALL() {\n");
        for (struct FactCheckStruct *check = head; check != NULL; check = check->next)
        {
          printf("    FACT_CHECK_REGISTER(%s);\n", check->name);
        }
        printf("}\n");
        continue;
      }
    }
    {
      const char *op = "--check_junit";
      if (strcmp(arg, op) == 0)
      {
        fact_format = FACT_JUNIT;
        continue;
      }
    }
    {
      const char *op = "--check_help";
      if (strcmp(arg, op) == 0)
      {
        check = 0;
        printf("default is to run all fact checks\n");
        printf("    --check_include=\"*wildcard pattern*\"\n --- include certain fact checks\n");
        printf("    --check_exclude=\"*wildcard pattern*\"\n --- exclude certain fact checks\n");
        printf("    --check_register_all --- auto* generate FACT_CHECK_REGISTER_ALL\n");
        printf("    --check_help --- this help\n");
        printf("    --check_junit --- use junit format\n");
        printf("    * Optimized executables may miss auto fact checks.\n");
        printf("      Use explicit FACT_CHECK_REGISTER_ALL() {...} for reliable fact checking.\n");
        continue;
      }
    }
  }

  if (check)
  {
    FactCheck();
  }

  return (fact_fictions == 0) ? 0 : 1;
}
