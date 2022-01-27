#pragma once

#ifdef __cplusplus
#include <iostream>
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>

  struct FactsStruct;
  typedef struct FactsStruct Facts;

#define FACTS_GREEN "\033[1;32m"
#define FACTS_RED "\033[1;31m"
#define FACTS_RESET "\033[0m"

#define FACTS_SIG               \
  {                             \
    0xae, 0x1e, 0xe5, 0xab,     \
        0xdf, 0x2c, 0x8f, 0xfd, \
        0x9d, 0x1e, 0xa7, 0x37, \
        0xc6, 0xf3, 0xe0, 0xe8, \
        0xb7, 0xdc, 0x56, 0x93, \
        0x08, 0xd8, 0xe3, 0x13, \
        0xe2, 0xe4, 0x43, 0x2d, \
        0x91, 0x4a, 0x32, 0x55  \
  }

#define FACTS_SIG_LEN 32

#define FACTS_STATE_EXCLUDE -2
#define FACTS_STATE_FAIL -1
#define FACTS_STATE_INCLUDE 0
#define FACTS_STATE_PASS 1

#define FACTS_CONSOLE 0
#define FACTS_JUNIT 1

  struct FactsStruct
  {
    unsigned char sig[FACTS_SIG_LEN];
    const char *file;
    int line;
    const char *name;
    void (*function)(Facts *facts);
    int status;
    Facts *next;
    Facts *prev;
  };

  void FactsPrint(const char *fmt, ...);
  void FactsFindInMemory(Facts *begin, Facts *end);
  void FactsInclude(const char *pattern);
  void FactsExclude(const char *pattern);
  void FactsRegister(Facts *facts);
  void FactsCheck();
  void FactsFiction(const char *file, int line, Facts *facts,
		    const char *a, const char *op, const char *b);

  double FactsAbsErr(double a, double b);
  double FactsRelErr(double a, double b);

  int FactsMain(int argc, const char *argv[]);

// https://stackoverflow.com/questions/24844970/how-to-print-types-of-unknown-size-like-ino-t
#define FACTS_PRINT_FORMAT(X) _Generic((X),                    \
                                       char                    \
                                       : "%c",                 \
                                         unsigned char         \
                                       : "%hhu",               \
                                         unsigned short        \
                                       : "%hu",                \
                                         unsigned int          \
                                       : "%u",                 \
                                         unsigned long         \
                                       : "%lu",                \
                                         unsigned long long    \
                                       : "%llu",               \
                                         signed char           \
                                       : "%hhd",               \
                                         short                 \
                                       : "%hd",                \
                                         int                   \
                                       : "%d",                 \
                                         long                  \
                                       : "%ld",                \
                                         long long             \
                                       : "%lld",               \
                                         float                 \
                                       : "%g",                 \
                                         double                \
                                       : "%g",                 \
                                         long double           \
                                       : "%Lg",                \
                                         const char *          \
                                       : "%s",                 \
                                         const unsigned char * \
                                       : "%s",                 \
                                         const void *          \
                                       : "%p",                 \
                                         char *                \
                                       : "%s",                 \
                                         unsigned char *       \
                                       : "%s",                 \
                                         void *                \
                                       : "%p")

#ifndef FACTS_C
  extern uint64_t facts_fictions;
  extern uint64_t facts_truths;
  extern int facts_format;
#endif

#define FACT_CHECK_PRINT(a, op, b, fmt) (((a)op(b)) ? (++facts_truths, 1) : (FactsPrint(FACTS_RED "%s/%s %d: %s {=%?} " #op " %s {=%?} is fiction" FACTS_RESET "\n", fmt, fmt, __FILE__, facts->name, __LINE__, #a, (a), #b, (b)), FactsFiction(__FILE__, __LINE__, facts, #a, #op, #b), facts->status = -1, 0))
#define FACT_PRINT(a, op, b, fmt)  \
  if (!FACT_CHECK_PRINT(a, op, b, fmt)) \
  return

#define FACT_CHECK_CERR(a, op, b) (((a)op(b)) ? (++facts_truths, 1) : (std::cout << FACTS_RED << __FILE__ << "/" << facts->name << " " << __LINE__ << ": " << #a << " {=" << (a) << "} " << #op << " " << #b << " {=" << (b) << "} is fiction" FACTS_RESET << std::endl, FactsFiction(__FILE__, __LINE__, facts, #a, #op, #b), facts->status = -1, 0))
#define FACT_CERR(a, op, b)  \
  if (!FACT_CHECK_CERR(a, op, b)) \
  return

#ifdef __cplusplus
#define FACT(a, op, b) FACT_CERR(a, op, b)
#define FACT_CHECK(a, op, b) FACT_CHECK_CERR(a, op, b)
#define FACTS_EXTERN extern "C"
#else
#define FACT(a, op, b) FACT_PRINT(a, op, b, FACTS_PRINT_FORMAT(a))
#define FACT_CHECK(a, op, b) FACT_CHECK_PRINT(a, op, b, FACTS_PRINT_FORMAT(a))
#define FACTS_EXTERN
#endif

#define FACTS_DECLARE(name, state)                                                                                 \
  void facts_##name##_function(Facts *facts);                                                                      \
  Facts facts_##name##_data = {FACTS_SIG, __FILE__, __LINE__, #name, &facts_##name##_function, state, NULL, NULL}; \
  void facts_##name##_function(Facts *facts)

#define FACTS_INCLUDE(name) FACTS_DECLARE(name, FACTS_STATE_INCLUDE)
#define FACTS_EXCLUDE(name) FACTS_DECLARE(name, FACTS_STATE_EXCLUDE)
#define FACTS(name) FACTS_INCLUDE(name)

#ifndef FACTS_C
  FACTS(0000_BEGIN)
  {
  }
#endif

#define FACTS_REGISTER(name) FactsRegister(&facts_##name##_data)

#define FACTS_REGISTER_ALL                                           \
  FACTS(zzzz_END){};                                                 \
  FACTS_EXTERN void FactsFind()                                      \
  {                                                                  \
    FactsFindInMemory(&facts_0000_BEGIN_data, &facts_zzzz_END_data); \
  }                                                                  \
  FACTS_EXTERN void FactsRegisterAll

#define FACTS_REGISTER_AUTO             \
  FACTS_REGISTER_ALL() { FactsFind(); } \
  void FactRegisterIgnored

#define FACTS_MAIN \
  int main(int argc, const char *argv[]) { return FactsMain(argc, argv); }

#define FACTS_DONE	  \
  FACTS_REGISTER_AUTO(); {}			\
  FACTS_MAIN

#ifdef __cplusplus
} // extern "C"
#endif
