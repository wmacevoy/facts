#pragma once

#ifdef __cplusplus
#include <iostream>
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>

  struct FactCheckStruct;

#define FACT_GREEN "\033[1;32m"
#define FACT_RED "\033[1;31m"
#define FACT_RESET "\033[0m"

#define FACT_SIG               \
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

#define FACT_SIG_LEN 32

#define FACT_STATE_EXCLUDE -2
#define FACT_STATE_FAIL -1
#define FACT_STATE_INCLUDE 0
#define FACT_STATE_PASS 1

#define FACT_CONSOLE 0
#define FACT_JUNIT 1

  struct FactCheckStruct
  {
    unsigned char sig[FACT_SIG_LEN];
    const char *file;
    int line;
    const char *name;
    void (*function)(struct FactCheckStruct *check);
    int status;
    struct FactCheckStruct *next;
    struct FactCheckStruct *prev;
  };

  void FactPrint(const char *fmt, ...);
  void FactCheckFindInMemory(struct FactCheckStruct *begin, struct FactCheckStruct *end);
  void FactCheckInclude(const char *pattern);
  void FactCheckExclude(const char *pattern);
  void FactCheckRegister(struct FactCheckStruct *check);
  void FactCheck();
  void FactIsFiction(const char *file, int line, struct FactCheckStruct *check,
		    const char *a, const char *op, const char *b);

  double FactAbsErr(double a, double b);
  double FactRelErr(double a, double b);

  int FactCheckMain(int argc, const char *argv[]);

// https://stackoverflow.com/questions/24844970/how-to-print-types-of-unknown-size-like-ino-t
#define FACT_PRINT_FORMAT(X) _Generic((X),                    \
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

#ifndef FACT_CHECK_C
  extern uint64_t fact_fictions;
  extern uint64_t fact_truths;
  extern int fact_format;
#endif

#define FACT_OR_FICTION_PRINT(a, op, b, fmt) (((a)op(b)) ? (++fact_truths, 1) : (FactPrint(FACT_RED "%s/%s %d: %s {=%?} " #op " %s {=%?} is fiction" FACT_RESET "\n", fmt, fmt, __FILE__, check->name, __LINE__, #a, (a), #b, (b)), FactIsFiction(__FILE__, __LINE__, check, #a, #op, #b), check->status = -1, 0))
#define FACT_PRINT(a, op, b, fmt)  \
  if (!FACT_OR_FICTION_PRINT(a, op, b, fmt)) \
  return

#define FACT_OR_FICTION_COUT(a, op, b) (((a)op(b)) ? (++facts_truths, 1) : (std::cout << FACTS_RED << __FILE__ << "/" << facts->name << " " << __LINE__ << ": " << #a << " {=" << (a) << "} " << #op << " " << #b << " {=" << (b) << "} is fiction" FACTS_RESET << std::endl, FactsFiction(__FILE__, __LINE__, facts, #a, #op, #b), facts->status = -1, 0))
#define FACT_COUT(a, op, b)  \
  if (!FACT_OR_FICTION_COUT(a, op, b)) \
  return

#ifdef __cplusplus
#define FACT(a, op, b) FACT_COUT(a, op, b)
#define FACT_OR_FICTION(a, op, b) FACT_OR_FICTION_COUT(a, op, b)
#define FACT_EXTERN extern "C"
#else
#define FACT(a, op, b) FACT_PRINT(a, op, b, FACT_PRINT_FORMAT(a))
#define FACT_OR_FICTION(a, op, b) FACT_OR_FICTION_PRINT(a, op, b, FACT_PRINT_FORMAT(a))
#define FACT_EXTERN
#endif

#define FACT_CHECK_DEFINE(name, state)                                                                                 \
  void fact_check_function_##name(struct FactCheckStruct *check);                                                                      \
  struct FactCheckStruct fact_check_struct_##name = {FACT_SIG, __FILE__, __LINE__, #name, &fact_check_function_##name, state, NULL, NULL}; \
  void fact_check_function_##name(struct FactCheckStruct *check)

#define FACT_CHECK_INCLUDE(name) FACT_CHECK_DEFINE(name, FACT_STATE_INCLUDE)
#define FACT_CHECK_EXCLUDE(name) FACT_CHECK_DEFINE(name, FACT_STATE_EXCLUDE)
#define FACT_CHECK(name) FACT_CHECK_INCLUDE(name)

#ifndef FACT_CHECK_C
  FACT_CHECK(0000_BEGIN)
  {
  }
#endif

#define FACT_CHECK_REGISTER(name) FactCheckRegister(&fact_check_struct_##name##)

#define FACT_CHECK_REGISTER_ALL                                           \
  FACT_CHECK(zzzz_END){};                                                 \
  FACT_EXTERN void FactCheckRegisterAuto()                                      \
  {                                                                  \
    FactCheckFindInMemory(&fact_check_struct_0000_BEGIN, &fact_check_struct_zzzz_END); \
  }                                                                  \
  FACT_EXTERN void FactCheckRegisterAll

#define FACT_CHECK_REGISTER_AUTO             \
  FACT_CHECK_REGISTER_ALL() { FactCheckRegisterAuto(); } \
  FACT_EXTERN void FactCheckRegisterIgnore

#define FACT_CHECK_IF(arg) { int argi; for (argi=1; argi < argc; ++argi) { if (strcmp(argv[argi],#arg)==0) { return FactCheckMain(argc,argv); }}}}
#define FACT_CHECK_MAIN \
  int main(int argc, const char *argv[]) { return FactCheckMain(argc,argv); }

#define FACT_CHECK_DONE	  \
  FACT_CHECK_REGISTER_AUTO() {}			\
  FACT_CHECK_MAIN

#ifdef __cplusplus
} // extern "C"
#endif
