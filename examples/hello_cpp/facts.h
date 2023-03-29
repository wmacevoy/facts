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

#define FACTS_GREEN ((FACTS_COLOR) ? "\033[1;32m" : "" )
#define FACTS_RED ((FACTS_COLOR) ? "\033[1;31m" : "")
#define FACTS_RESET ((FACTS_COLOR) ? "\033[0m" : "")

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

#define FACTS_CONSOLE (((facts_format) & (1<<0)) == 0) 
#define FACTS_JUNIT   (((facts_format) & (1<<0)) != 0) 
#define FACTS_COLOR   (((facts_format) & 3) == 0)

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

#if !(defined(FACTS_C) || defined(FACTS_CPP))
  extern uint64_t facts_fictions;
  extern uint64_t facts_truths;
  extern int facts_format;
#endif

#define FACT_CHECK_PRINT(a, op, b, fmt) (((a)op(b)) ? (++facts_truths, 1) : (FactsPrint("%s/%s %d: %s%s {=%?} " #op " %s {=%?} is fiction%s\n", fmt, fmt, __FILE__, facts->name, __LINE__, FACTS_RED, #a, (a), #b, (b), FACTS_RESET), FactsFiction(__FILE__, __LINE__, facts, #a, #op, #b), facts->status = -1, ++facts_fictions, 0))
#define FACT_PRINT(a, op, b, fmt)  \
  if (!FACT_CHECK_PRINT(a, op, b, fmt)) \
  return

#define FACT_CHECK_CERR(a, op, b) (((a)op(b)) ? (++facts_truths, 1) : (std::cout  << __FILE__ << "/" << facts->name << " " << __LINE__ << ": " << FACTS_RED << #a << " {=" << (a) << "} " << #op << " " << #b << " {=" << (b) << "} is fiction" << FACTS_RESET << std::endl, FactsFiction(__FILE__, __LINE__, facts, #a, #op, #b), facts->status = -1, ++facts_fictions, 0))
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

#if !(defined(FACTS_C) || defined(FACTS_CPP))
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

#define FACTS_MAIN 						\
  int main(int argc, const char *argv[]) { return FactsMain(argc, argv); }

#define FACTS_MAIN_IF(check)						\
  int FactsMainElse(int argc, const char *argv[]);			\
  int main(int argc, const char *argv[]) {				\
    const char *facts=#check;						\
    for (int argi=1; argi<argc; ++argi) {				\
      const char *arg=argv[argi];					\
      for (int c=0; arg[c] == facts[c]; ++c) {				\
        if (arg[c] == 0) return FactsMain(argc, argv);			\
      }									\
    }									\
    return FactsMainElse(argc,argv);					\
  }									\
  int FactsMainElse(int argc, const char *argv[])
  
#define FACTS_FAST				\
  FACTS_REGISTER_AUTO() {}			\
  FACTS_MAIN
  
#define FACTS_FAST_IF(arg)			\
  FACTS_REGISTER_AUTO() {}			\
  FACTS_MAIN_IF(arg)
    
#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
#include <functional>
#include <iostream>
 
  class FactsTrace
  {
    private: static FactsTrace *top;
    private: static Facts *facts;
    private: FactsTrace *up;
    private: std::function<void(Facts *facts, std::ostream&)> note;

    public: static void reset(Facts *_facts);
    public: static void notes();

    FactsTrace(std::function<void(Facts *facts, std::ostream&)> _note);
    ~FactsTrace();
  };

  #define FACTS_TRACE_SUFFIX(note,suffix) \
    FactsTrace facts_trace##suffix([=](Facts *facts, std::ostream&out) { out << __FILE__ << "/" << facts->name << ":" << __LINE__ << " " << note << std::endl; }); \
    facts_trace_##suffix:
  #define FACTS_TRACE_COUNT(note,counter) FACTS_TRACE_SUFFIX(note,counter)
  #define FACTS_TRACE(note) FACTS_TRACE_COUNT(note,__COUNTER__)

  #define FACTS_WATCH_SUFFIX(note,suffix) FACTS_TRACE_SUFFIX(FACTS_GREEN << "(gdb)" << FACTS_RESET << " break facts_" << facts->name << "_function:facts_trace_" << suffix << " if " << note, suffix)
  #define FACTS_WATCH_COUNT(note,counter) FACTS_WATCH_SUFFIX(note,counter)
  #define FACTS_WATCH_IF(watch) FACTS_WATCH_COUNT(watch,__COUNTER__)

  #define FACTS_OSTREAM0(x,...) "!!! too many watch values, break-up on or before " << #x << " !!!"
  #define FACTS_OSTREAM1(x,...) #x << "==" << (x) __VA_OPT__(<< " && " <<  FACTS_OSTREAM0(__VA_ARGS__))
  #define FACTS_OSTREAM2(x,...) #x << "==" << (x) __VA_OPT__(<< " && " <<  FACTS_OSTREAM1(__VA_ARGS__))
  #define FACTS_OSTREAM3(x,...) #x << "==" << (x) __VA_OPT__(<< " && " <<  FACTS_OSTREAM2(__VA_ARGS__))
  #define FACTS_OSTREAM4(x,...) #x << "==" << (x) __VA_OPT__(<< " && " <<  FACTS_OSTREAM3(__VA_ARGS__))
  #define FACTS_OSTREAM5(x,...) #x << "==" << (x) __VA_OPT__(<< " && " <<  FACTS_OSTREAM4(__VA_ARGS__))
  #define FACTS_OSTREAM6(x,...) #x << "==" << (x) __VA_OPT__(<< " && " <<  FACTS_OSTREAM5(__VA_ARGS__))
  #define FACTS_OSTREAM7(x,...) #x << "==" << (x) __VA_OPT__(<< " && " <<  FACTS_OSTREAM6(__VA_ARGS__))
  #define FACTS_OSTREAM8(x,...) #x << "==" << (x) __VA_OPT__(<< " && " <<  FACTS_OSTREAM7(__VA_ARGS__))
  #define FACTS_OSTREAM9(x,...) #x << "==" << (x) __VA_OPT__(<< " && " <<  FACTS_OSTREAM8(__VA_ARGS__))
  #define FACTS_WATCH(...) FACTS_WATCH_IF(FACTS_OSTREAM9(__VA_ARGS__))

  #define FACTS_WATCH1(...) FACTS_WATCH_IF(FACTS_OSTREAM1(__VA_ARGS__))
  #define FACTS_WATCH2(...) FACTS_WATCH_IF(FACTS_OSTREAM2(__VA_ARGS__))
  #define FACTS_WATCH3(...) FACTS_WATCH_IF(FACTS_OSTREAM3(__VA_ARGS__))
  #define FACTS_WATCH4(...) FACTS_WATCH_IF(FACTS_OSTREAM4(__VA_ARGS__))
  #define FACTS_WATCH5(...) FACTS_WATCH_IF(FACTS_OSTREAM5(__VA_ARGS__))
  #define FACTS_WATCH6(...) FACTS_WATCH_IF(FACTS_OSTREAM6(__VA_ARGS__))
  #define FACTS_WATCH7(...) FACTS_WATCH_IF(FACTS_OSTREAM7(__VA_ARGS__))
  #define FACTS_WATCH8(...) FACTS_WATCH_IF(FACTS_OSTREAM8(__VA_ARGS__))
  #define FACTS_WATCH9(...) FACTS_WATCH_IF(FACTS_OSTREAM9(__VA_ARGS__))

#endif
