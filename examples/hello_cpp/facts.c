// _GNU_SOURCE must come before any includes (needed for RTLD_DEFAULT on glibc)
#if !defined(_WIN32) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

// macOS hides RTLD_DEFAULT when _POSIX_C_SOURCE is set, unless this is too.
// Without it the fallback below is used, and there every symbol lookup fails.
#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS 1
#endif

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define dup    _dup
#define dup2   _dup2
#define close  _close
#define lseek  _lseek
#ifndef fileno
#define fileno _fileno
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#endif
#else
#include <unistd.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/wait.h>
#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT ((void *)0)
#endif
#endif

#define FACTS_C 1
#include "facts.h"
static Facts *head = NULL, *tail = NULL;
static Facts *current = NULL;   // the group FactsCheck is running: FactsCurrent

// These must have C linkage to match the extern declarations in facts.h
// (which are inside extern "C"). Without this, MSVC mangles the names
// differently when facts.c is compiled as C++ via facts.cpp.
#ifdef __cplusplus
extern "C" {
#endif
uint64_t facts_fictions = 0;
uint64_t facts_truths = 0;
uint64_t facts_twice = 0;  // fictions whose sides were evaluated a second time to print them
int facts_format = 0;
#ifdef __cplusplus
}
#endif

// Weak default so the shared library links without user code.
// User-provided strong definition (from FACTS_REGISTER_ALL)
// overrides at link time.
#if defined(__GNUC__) || defined(__clang__)
FACTS_EXTERN __attribute__((weak)) void FactsRegisterAll() {}
#elif defined(_MSC_VER)
FACTS_EXTERN void FactsRegisterAll_default() {}
#pragma comment(linker, "/alternatename:FactsRegisterAll=FactsRegisterAll_default")
FACTS_EXTERN void FactsRegisterAll();
#else
FACTS_EXTERN void FactsRegisterAll();
#endif

// How this file is laid out.
//
// facts.c stays one file so it can be copied beside a program's sources.
// It is divided by lines that say "border": first the four edges, each
// holding the words of a model that is someone else's (gdb, JUnit, the
// dynamic loader, the operating system's processes), then the core.
// An edge calls nothing in the core; the core calls the edges.
// test/borders.sh keeps each edge's words inside its edge.

// ==== border: edge-gdb ====
// gdb's command language is gdb's: the hints printed on a fiction are spelled here.

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
  // --facts_unsafe: otherwise the facts run in a child process, where the
  // debugger's breakpoints are never reached.
  printf("%s(gdb)%s run --facts_unsafe --facts_include=%s\n",FACTS_GREEN,FACTS_RESET,facts->name);
}

// ==== border: edge-junit ====
// JUnit's model is JUnit's: its words (suite, case, skipped, failure) and its
// XML stay in this section, so a change to that format is an edit here only.
// It reads a group's name and status and never changes them.

// Write len bytes to stdout with XML entity escaping.
static void FactsXmlEscape(const char *s, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    switch (s[i]) {
      case '<':  printf("&lt;");   break;
      case '>':  printf("&gt;");   break;
      case '&':  printf("&amp;");  break;
      case '"':  printf("&quot;"); break;
      default:   putchar(s[i]);    break;
    }
  }
}

static void FactsXmlEscapeStr(const char *s) {
  FactsXmlEscape(s, strlen(s));
}
#define FACTS_BLOCKSIZE 1024

// Between "group began" and "group ended" stdout and stderr are redirected to
// temporary files, so whatever the group and the console report print lands
// inside that group's element.
static FILE *junit_tmpout = NULL, *junit_tmperr = NULL;
static int junit_oldout = -1, junit_olderr = -1;

static void FactsJUnitRunBegan(void)
{
  printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  printf("<testsuite name=\"facts\">\n");
}

static void FactsJUnitGroupBegan(Facts *facts)
{
  printf("<testcase name=\"");
  FactsXmlEscapeStr(facts->name);
  printf("\">\n");
  fflush(stdout);
  fflush(stderr);
  junit_tmpout = tmpfile();
  junit_tmperr = tmpfile();
  if (junit_tmpout != NULL && junit_tmperr != NULL) {
    junit_oldout = dup(STDOUT_FILENO);
    junit_olderr = dup(STDERR_FILENO);
    if (junit_oldout >= 0 && junit_olderr >= 0) {
      dup2(fileno(junit_tmpout), STDOUT_FILENO);
      dup2(fileno(junit_tmperr), STDERR_FILENO);
    }
  }
}

// Copy the first len bytes of a capture file to stdout inside <tag>.
static void FactsJUnitCaptured(const char *tag, FILE *tmp, int64_t len)
{
  if (len <= 0) return;
  fseek(tmp, 0L, SEEK_SET);
  printf("<%s>", tag);
  for (int64_t p = 0; p < len; p += FACTS_BLOCKSIZE)
  {
    char data[FACTS_BLOCKSIZE];
    int n = (int)(len - p);
    if (n > FACTS_BLOCKSIZE) n = FACTS_BLOCKSIZE;
    if (fread(data, n, 1, tmp) == 1)
      FactsXmlEscape(data, n);
  }
  printf("</%s>\n", tag);
}

static void FactsJUnitGroupEnded(Facts *facts)
{
  fflush(stdout);
  fflush(stderr);
  if (junit_tmpout != NULL && junit_tmperr != NULL && junit_oldout >= 0 && junit_olderr >= 0) {
    int64_t outlen = lseek(STDOUT_FILENO, 0, SEEK_CUR);
    int64_t errlen = lseek(STDERR_FILENO, 0, SEEK_CUR);
    dup2(junit_oldout, STDOUT_FILENO);
    dup2(junit_olderr, STDERR_FILENO);
    close(junit_oldout);
    close(junit_olderr);
    junit_oldout = junit_olderr = -1;
    FactsJUnitCaptured("system-out", junit_tmpout, outlen);
    FactsJUnitCaptured("system-err", junit_tmperr, errlen);
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
  if (junit_tmpout != NULL) fclose(junit_tmpout);
  if (junit_tmperr != NULL) fclose(junit_tmperr);
  junit_tmpout = junit_tmperr = NULL;
}

static void FactsJUnitRunEnded(void)
{
  printf("</testsuite>\n");
}

// ==== border: edge-os-symbols ====
// The dynamic loader's model is the platform's: symbol lookup is spelled here.

// Look up a Facts struct by name using dynamic symbol lookup.
// Returns NULL if not found (e.g. not compiled into this binary).
static Facts *FactsLookup(const char *name) {
  char sym[256];
  snprintf(sym, sizeof(sym), "facts_%s_data", name);
#ifdef _WIN32
  return (Facts *)GetProcAddress(GetModuleHandle(NULL), sym);
#else
  return (Facts *)dlsym(RTLD_DEFAULT, sym);
#endif
}

// ==== border: edge-os-process ====
// The operating system's model (process, signal, exit status) stays here, once for Windows and once for POSIX.

#ifndef _WIN32
// What the parent is told while it waits for the child.  SIGCHLD only has to
// wake the wait; the alarm and an interrupt each leave a note.
static volatile sig_atomic_t facts_alarm_rang = 0;
static volatile sig_atomic_t facts_interrupted = 0;   // the signal, or 0
static void FactsChildSignal(int sig) {
  if (sig == SIGALRM) facts_alarm_rang = 1;
  else if (sig != SIGCHLD) facts_interrupted = sig;
}

// SIGCHLD and SIGALRM, then the signals that ask this process to stop.
static const int facts_watched[] = {SIGCHLD, SIGALRM, SIGINT, SIGTERM, SIGHUP};
#define FACTS_WATCHED ((int)(sizeof(facts_watched) / sizeof(facts_watched[0])))
#endif

// Re-exec self as a child process with --facts_unsafe, monitoring for
// crashes and timeouts.  Returns the exit code to propagate.
static int FactsExecSafe(int argc, const char *argv[], int timeout) {
  // Build new argv: argv[0] --facts_unsafe [rest of argv...]
  const char **child_argv = (const char **)malloc((argc + 2) * sizeof(const char *));
  if (!child_argv) return 2;
  child_argv[0] = argv[0];
  child_argv[1] = "--facts_unsafe";
  for (int i = 1; i < argc; ++i)
    child_argv[i + 1] = argv[i];
  child_argv[argc + 1] = NULL;

#ifdef _WIN32
  // Build command line string for CreateProcess
  char cmdline[32768];
  int pos = 0;
  for (int i = 0; child_argv[i] && pos < (int)sizeof(cmdline) - 2; ++i) {
    if (i > 0) cmdline[pos++] = ' ';
    // Quote each argument
    cmdline[pos++] = '"';
    for (const char *p = child_argv[i]; *p && pos < (int)sizeof(cmdline) - 3; ++p) {
      if (*p == '"') cmdline[pos++] = '\\';
      cmdline[pos++] = *p;
    }
    cmdline[pos++] = '"';
  }
  cmdline[pos] = '\0';

  STARTUPINFOA si = {0};
  PROCESS_INFORMATION pi = {0};
  si.cb = sizeof(si);

  if (!CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
    fprintf(stderr, "facts: CreateProcess failed (%lu)\n", GetLastError());
    free(child_argv);
    return 2;
  }
  free(child_argv);

  DWORD wait = WaitForSingleObject(pi.hProcess, timeout * 1000);
  if (wait == WAIT_TIMEOUT) {
    TerminateProcess(pi.hProcess, 1);
    WaitForSingleObject(pi.hProcess, 1000);
    fprintf(stderr, "\nfacts: TIMEOUT after %d seconds\n", timeout);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 2;
  }

  DWORD exit_code = 0;
  GetExitCodeProcess(pi.hProcess, &exit_code);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  // An unhandled exception ends a process with its exception code, and all of
  // those are 0xC0000000 or above: that is a crash, not a choice of exit code.
  if (exit_code >= 0xC0000000UL) {
    fprintf(stderr, "\nfacts: child crashed (exception 0x%08lX)\n", (unsigned long)exit_code);
    return 2;
  }
  if (exit_code > 2) {
    fprintf(stderr, "\nfacts: child exited with code %lu\n", (unsigned long)exit_code);
  }
  return (int)exit_code;

#else // POSIX
  // Wait for the child with no polling: SIGCHLD says it ended, SIGALRM says
  // the timeout passed, and SIGINT, SIGTERM or SIGHUP says someone wants this
  // run stopped.  All are blocked except while sigsuspend waits, so none can
  // arrive in the gap between looking and waiting and be missed.
  sigset_t block, before, waiting;
  sigemptyset(&block);
  for (int w = 0; w < FACTS_WATCHED; ++w) sigaddset(&block, facts_watched[w]);
  sigprocmask(SIG_BLOCK, &block, &before);
  waiting = before;
  for (int w = 0; w < FACTS_WATCHED; ++w) sigdelset(&waiting, facts_watched[w]);

  struct sigaction note, old[FACTS_WATCHED];
  memset(&note, 0, sizeof(note));
  note.sa_handler = FactsChildSignal;
  sigemptyset(&note.sa_mask);
  facts_alarm_rang = 0;
  facts_interrupted = 0;
  for (int w = 0; w < FACTS_WATCHED; ++w) {
    sigaction(facts_watched[w], NULL, &old[w]);
    // A signal the caller chose to ignore (nohup, a shell's background job)
    // stays ignored, here and in the child.
    if (w >= 2 && old[w].sa_handler == SIG_IGN) continue;
    sigaction(facts_watched[w], &note, NULL);
  }

  pid_t pid = fork();
  if (pid < 0) {
    perror("facts: fork");
    for (int w = 0; w < FACTS_WATCHED; ++w) sigaction(facts_watched[w], &old[w], NULL);
    sigprocmask(SIG_SETMASK, &before, NULL);
    free(child_argv);
    return 2;
  }

  if (pid == 0) {
    // Child: the signal mask survives exec, so put it back first.
    sigprocmask(SIG_SETMASK, &before, NULL);
    // Lower priority and exec
    nice(10);
    execv(argv[0], (char *const *)child_argv);
    // If exec fails, try /proc/self/exe (Linux)
    execv("/proc/self/exe", (char *const *)child_argv);
    perror("facts: exec");
    _exit(2);
  }

  // Parent
  free(child_argv);

  int wstatus = 0;
  int timed_out = 0;
  int interrupted = 0;
  alarm((unsigned)timeout);
  for (;;) {
    pid_t ended = waitpid(pid, &wstatus, WNOHANG);
    if (ended != 0) break;          // it ended (or waitpid failed)
    if (facts_alarm_rang || facts_interrupted) {
      // The child may be stuck or ignoring signals, so it is not asked.
      kill(pid, SIGKILL);
      waitpid(pid, &wstatus, 0);
      interrupted = facts_interrupted;
      timed_out = !interrupted;
      break;
    }
    sigsuspend(&waiting);           // sleep until one of the watched signals
  }
  alarm(0);
  for (int w = 0; w < FACTS_WATCHED; ++w) sigaction(facts_watched[w], &old[w], NULL);
  sigprocmask(SIG_SETMASK, &before, NULL);

  if (interrupted) {
    // The run is over and its child is gone.  Now end the way the signal
    // meant, so whoever sent it sees an interrupted process and not exit 2.
    raise(interrupted);
    return 2;                       // only if the caller handles that signal itself
  }

  if (timed_out) {
    fprintf(stderr, "\nfacts: TIMEOUT after %d seconds\n", timeout);
    return 2;
  }

  if (WIFSIGNALED(wstatus)) {
    int sig = WTERMSIG(wstatus);
    const char *signame = "unknown";
    if (sig == SIGSEGV) signame = "SIGSEGV";
    else if (sig == SIGABRT) signame = "SIGABRT";
    else if (sig == SIGFPE) signame = "SIGFPE";
    else if (sig == SIGBUS) signame = "SIGBUS";
    fprintf(stderr, "\nfacts: child killed by signal %d (%s)\n", sig, signame);
    return 2;
  }

  return WIFEXITED(wstatus) ? WEXITSTATUS(wstatus) : 2;
#endif
}

// ==== border: core ====
// Everything that is facts' own: which groups are registered and included,
// the helpers the FACT macros call, the run, the console report, and the
// command line.

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
  size_t np = strlen(pattern);

  // fast exit for match-all pattern
  if (np == 1 && pattern[0] == '*')
    return 1;

  size_t ns = strlen(str);

  size_t nb = (np + 2) / 8 + ((np + 2) % 8 != 0);
  size_t k;

  uint8_t *state0 = (uint8_t *)calloc(nb, 1);
  uint8_t *state1 = (uint8_t *)calloc(nb, 1);

  if (!state0 || !state1) { free(state0); free(state1); return 0; }
  state0[0] = 1;
  for (k = 1; pattern[k - 1] == '*'; ++k)
    state0[k / 8] |= (1 << (k % 8));

  for (size_t i = 0; i <= ns; ++i)
  {
    unsigned char c = str[i];

    memset(state1, 0, nb);
    for (size_t j = 0; j <= np; ++j)
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
  int result = (state0[k / 8] & (1 << (k % 8))) != 0;
  free(state0);
  free(state1);
  return result;
}

// Include FACTS to check with wildncard pattern.

FACTS_EXTERN void FactsInclude(const char *pattern)
{
  // The first include starts from nothing; later ones add to it.  "First" is
  // remembered here and not read off the list being empty, because find may
  // have registered groups before any include is seen.
  static int narrowed = 0;
  if (head == NULL)
  {
    FactsRegisterAll();
  }
  if (!narrowed)
  {
    narrowed = 1;
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
  if (facts->prev == NULL && facts->next == NULL && facts != head)
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

// Scan a source file for FACTS(Name) declarations.
// If register_mode, look up and register each test found.
// Otherwise, print FACTS_REGISTER(Name) lines to stdout.
static void FactsScanFile(const char *filename, int register_mode) {
  FILE *f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "facts: cannot open %s\n", filename);
    return;
  }

  char line[4096];
  while (fgets(line, sizeof(line), f)) {
    // skip preprocessor directives (#define, #include, etc.)
    const char *trimmed = line;
    while (*trimmed == ' ' || *trimmed == '\t') ++trimmed;
    if (*trimmed == '#') continue;

    const char *p = line;
    while ((p = strstr(p, "FACTS(")) != NULL) {
      // word boundary: previous char must not be identifier char
      if (p > line) {
        char c = p[-1];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '_') {
          p += 6;
          continue;
        }
      }

      // extract identifier
      const char *start = p + 6;
      const char *end = start;
      while ((*end >= 'A' && *end <= 'Z') || (*end >= 'a' && *end <= 'z') ||
             (*end >= '0' && *end <= '9') || *end == '_')
        ++end;

      if (*end == ')' && end > start) {
        int len = (int)(end - start);
        if (register_mode) {
          char name[256];
          if (len < (int)sizeof(name)) {
            memcpy(name, start, len);
            name[len] = '\0';
            Facts *facts = FactsLookup(name);
            if (facts) {
              FactsRegister(facts);
            } else {
              fprintf(stderr, "facts: %s not found in executable\n", name);
            }
          } else {
            fprintf(stderr, "facts: a group name in %s is over %d characters\n", filename, (int)sizeof(name) - 1);
          }
        } else {
          printf("    FACTS_REGISTER(%.*s);\n", len, start);
        }
      }

      p = (end > p) ? end : p + 1;
    }
  }

  fclose(f);
}

// Helpers the FACT macros in facts.h call from the author's code.

// Symmetric relative error.
FACTS_EXTERN double FactsRelErr(double a, double b)
{
  double abserr = a >= b ? a - b : b - a;
  a = a >= 0 ? a : -a;
  b = b >= 0 ? b : -b;
  double maxabs = a >= b ? a : b;
  if (maxabs == 0.0) return abserr;
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

  char *reformat = (char *)malloc(reformatSize);
  if (!reformat) { va_end(ap); return; }
  va_end(ap);
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
  reformat[j] = 0;
  vprintf(reformat, ap);
  va_end(ap);
  free(reformat);
}

// The console report.

static void FactsConsoleGroupBegan(Facts *facts)
{
  printf("%s %d: %s facts check started\n",
         facts->file, facts->line, facts->name);
}

// ran is 0 for a group that was not included when the run reached it.
static void FactsConsoleGroupEnded(Facts *facts, int ran)
{
  if (!ran)
  {
    printf("%s %d: %s facts check %sexcluded%s.\n",
           facts->file, facts->line, facts->name, FACTS_RED, FACTS_RESET);
  }
  else if (facts->status == FACTS_STATE_FAIL)
  {
    printf("%s %d: %s facts check ended %sbadly%s\n",
           facts->file, facts->line, facts->name, FACTS_RED, FACTS_RESET);
  }
  else
  {
    printf("%s %d: %s facts check ended\n",
           facts->file, facts->line, facts->name);
  }
}

static void FactsConsoleRunEnded(void)
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
  if (facts_twice > 0)
  {
    // Only a build that could not keep a FACT's values counts these (facts.h).
    printf("warning: %" PRIu64 " of those fictions evaluated each side twice to print it; "
           "with this compiler, a side effect such as ++x in a FACT happens twice.\n",
           facts_twice);
  }
}

// The four moments a run tells its reports about.  This is the only place
// that knows which reports are listening; the run below never asks.
// The console's lines for a group are printed in both formats: under JUnit
// they are captured into that group's element.

static void FactsReportRunBegan(void)
{
  if (FACTS_JUNIT) FactsJUnitRunBegan();
}

static void FactsReportGroupBegan(Facts *facts, int ran)
{
  if (FACTS_JUNIT) FactsJUnitGroupBegan(facts);
  if (ran) FactsConsoleGroupBegan(facts);
}

static void FactsReportGroupEnded(Facts *facts, int ran)
{
  FactsConsoleGroupEnded(facts, ran);
  if (FACTS_JUNIT) FactsJUnitGroupEnded(facts);
}

static void FactsReportRunEnded(void)
{
  if (FACTS_CONSOLE) FactsConsoleRunEnded();
  if (FACTS_JUNIT) FactsJUnitRunEnded();
}

FACTS_EXTERN Facts *FactsCurrent(void)
{
  return current;
}

// Execute facts checks.
//
// You can preceed this with FactsInclude and FactsExlude to pick out
// a particular set.
FACTS_EXTERN void FactsCheck()
{
  if (head == NULL)
  {
    FactsRegisterAll();
  }

  FactsReportRunBegan();
  for (Facts *facts = head; facts != NULL; facts = facts->next)
  {
    int ran = (facts->status == FACTS_STATE_INCLUDE);
    FactsReportGroupBegan(facts, ran);
    if (ran)
    {
#ifdef __cplusplus
      FactsTrace::reset(facts);
#endif
      current = facts;
      facts->function(facts);
      current = NULL;
      if (facts->status == FACTS_STATE_INCLUDE)
      {
        facts->status = FACTS_STATE_PASS;
      }
    }
    FactsReportGroupEnded(facts, ran);
  }
  FactsReportRunEnded();
}

// You can call this from your main to process facts checks.
// Exit code: 0 when facts were checked and none was fiction (or the options
// asked for no run), 1 when at least one was fiction, 2 when no result can
// be trusted: an unknown option, a run that checked no fact, or a crash,
// timeout, or failure to start seen by safe execution.
//
// Arguments beginning "--fact" are reserved: each must be an option facts
// knows.  Any other argument is the program's own and is left alone, since a
// program may need arguments even while its facts are checked.  own is the
// one reserved argument that is the program's: the switch FACTS_MAIN_IF was
// given (NULL when there is none).
static int FactsMainWith(int argc, const char *argv[], const char *own)
{
  // Check for --facts_unsafe and --facts_timeout before anything else.
  // If --facts_unsafe is absent, re-exec under a safe wrapper.
  int unsafe = 0;
  int timeout = 10;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--facts_unsafe") == 0) unsafe = 1;
    else if (strncmp(argv[i], "--facts_timeout=", 16) == 0) {
      const char *text = argv[i] + 16;
      char *end = NULL;
      long seconds = strtol(text, &end, 10);
      // The upper limit keeps seconds * 1000 inside an int for Windows.
      if (end == text || *end != 0 || seconds <= 0 || seconds > 2000000) {
        fprintf(stderr, "facts: --facts_timeout needs a positive number of seconds, not '%s'\n", text);
        return 2;
      }
      timeout = (int)seconds;
    }
  }

  if (!unsafe) {
    return FactsExecSafe(argc, argv, timeout);
  }

  // Register what --facts_find names before reading the other options, so
  // an include or exclude applies to those groups wherever it is written.
  for (int argi = 1; argi < argc; ++argi)
  {
    if (strcmp(argv[argi], "--facts_find") == 0)
    {
      for (++argi; argi < argc; ++argi) {
        if (strcmp(argv[argi], ";") == 0) break;
        FactsScanFile(argv[argi], 1);
      }
    }
  }

  int status = 0;
  int check = 1;
  int unknown = 0;
  for (int argi = 1; argi < argc; ++argi)
  {
    const char *arg = (argi < argc) ? argv[argi] : NULL;
    if (strcmp(arg, "--facts_unsafe") == 0 ||
        strncmp(arg, "--facts_timeout=", 16) == 0)
      continue;
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
        // Already registered above; step over its files.
        for (++argi; argi < argc; ++argi) {
          if (strcmp(argv[argi], ";") == 0) break;
        }
        continue;
      }
    }
    {
      const char *op = "--facts_register_all";
      if (strcmp(arg, op) == 0)
      {
        check = 0;
        if (head == NULL) FactsRegisterAll();
        printf("FACTS_REGISTER_ALL() {\n");
        for (Facts *f = head; f != NULL; f = f->next)
          printf("    FACTS_REGISTER(%s);\n", f->name);
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
        printf("    --facts_include=\"*pattern*\" --- include certain facts\n");
        printf("    --facts_exclude=\"*pattern*\" --- exclude certain facts\n");
        printf("    --facts_find files... [\\;] --- register facts from source\n");
        printf("    --facts_register_all --- print FACTS_REGISTER_ALL block\n");
        printf("    --facts_skip --- don't fact check\n");
        printf("    --facts_help --- this help\n");
        printf("    --facts_junit --- use junit format\n");
        printf("    --facts_plain --- no tty colors\n");
        printf("    --facts_unsafe --- run directly (skip fork/exec wrapper)\n");
        printf("    --facts_timeout=N --- timeout in seconds (default 10)\n");
        continue;
      }
    }
    if (strncmp(arg, "--fact", 6) == 0 && (own == NULL || strcmp(arg, own) != 0))
    {
      fprintf(stderr, "facts: unknown option %s (--facts_help lists the options)\n", arg);
      unknown = 1;
    }
  }

  if (unknown)
  {
    return 2;
  }

  if (check)
  {
    FactsCheck();
    if (facts_truths + facts_fictions == 0)
    {
      // Asked to check and checked nothing: a pattern that matched no group,
      // no group registered, or groups with no FACT in them.  Not a pass.
      fflush(stdout);
      fprintf(stderr, "facts: no facts were checked\n");
      return 2;
    }
  }

  return (facts_fictions == 0) ? 0 : 1;
}

FACTS_EXTERN int FactsMain(int argc, const char *argv[])
{
  return FactsMainWith(argc, argv, NULL);
}

// For FACTS_MAIN_IF: own is the switch that asked for the facts.
FACTS_EXTERN int FactsMainIf(int argc, const char *argv[], const char *own)
{
  return FactsMainWith(argc, argv, own);
}
