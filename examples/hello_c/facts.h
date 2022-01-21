#pragma once


#ifdef __cplusplus
#include <iostream>
#endif

#include <stdio.h>
#include <string.h>


struct FactsStruct;
typedef struct FactsStruct Facts;

#define FACTS_SIG {							\
    0xae, 0x1e, 0xe5, 0xab,						\
      0xdf, 0x2c, 0x8f, 0xfd,						\
      0x9d, 0x1e, 0xa7, 0x37,						\
      0xc6, 0xf3, 0xe0, 0xe8,						\
      0xb7, 0xdc, 0x56, 0x93,						\
      0x08, 0xd8, 0xe3, 0x13,						\
      0xe2, 0xe4, 0x43, 0x2d,						\
      0x91, 0x4a, 0x32, 0x55						\
      }

#define FACTS_SIG_LEN 32

#define FACTS_STATE_EXCLUDE -2
#define FACTS_STATE_FAIL    -1
#define FACTS_STATE_INCLUDE  0
#define FACTS_STATE_PASS     1



struct FactsStruct {
  unsigned char sig[FACTS_SIG_LEN];
  const char *file;
  int line;
  const char *name;
  void (*function)(Facts *facts);
  int status;
  Facts *next;
  Facts *prev;
};

void FactsPrint(const char *fmt,...);
void FactsFindInMemory(Facts *begin,Facts *end);
void FactsInclude(const char *pattern);
void FactsExclude(const char *pattern);
void FactsRegister(Facts *facts);
void FactsCheck();
void FactsFiction(const char *file, int line,Facts *facts);

double FactsAbsErr(double a, double b);
double FactsRelErr(double a, double b);

int FactsMain(int argc, const char *argv[]);

// https://stackoverflow.com/questions/24844970/how-to-print-types-of-unknown-size-like-ino-t
#define FACTS_PRINT_FORMAT(X) _Generic((X),				\
				       char: "%c",			\
				       unsigned char: "%hhu",		\
				       unsigned short: "%hu",		\
				       unsigned int: "%u",		\
				       unsigned long: "%lu",		\
				       unsigned long long: "%llu",	\
				       signed char: "%hhd",		\
				       short: "%hd",			\
				       int: "%d",			\
				       long: "%ld",			\
				       long long: "%lld",		\
				       float: "%g",			\
				       double: "%g",			\
				       long double: "%Lg",		\
				       const char *: "%s",		\
				       const unsigned char *: "%s",	\
				       const void *: "%p",		\
				       char *: "%s",			\
				       unsigned char *: "%s",		\
				       void *: "%p")
  
#ifndef FACTS_C
extern int facts_fictions;
extern int facts_truths;
#endif

#define CHECK_PRINT(a,op,b,fmt) (((a) op (b)) ? (++facts_truths,1) : (FactsFiction(__FILE__,__LINE__,facts),FactsPrint("%s %d: %s {=%?} " #op " %s {=%?} is fiction\n",fmt,fmt,__FILE__,__LINE__,#a,(a),#b,(b)), facts->status=-1,0))
#define FACT_PRINT(a,op,b,fmt) if (!CHECK_PRINT(a,op,b,fmt)) return

#define CHECK_CERR(a,op,b) (((a) op (b)) ? (++facts_truths,1) : (FactsFiction(__FILE__,__LINE__,facts),std::cerr << __FILE__ << " " << __LINE__ << ": " << #a << "{=" << (a) << "} " << #op << #b << " {=" << (b) << "} is fiction" << std::endl, facts->status=-1,0))
#define FACT_CERR(a,op,b) if (!CHECK_CERR(a,op,b)) return


#ifdef __cplusplus
#define FACT(a,op,b) FACT_CERR(a,op,b)
#define CHECK(a,op,b) CHECK_CERR(a,op,b)
#else
#define CHECK(a,op,b) CHECK_PRINT(a,op,b,FACTS_PRINT_FORMAT(a))
#define FACT(a,op,b) FACT_PRINT(a,op,b,FACTS_PRINT_FORMAT(a))
#endif

#define FACTS_DECLARE(name,state) void facts_ ##name## _function (Facts *facts); Facts facts_ ##name## _data = { FACTS_SIG, __FILE__, __LINE__, #name, &facts_ ##name## _function, state, NULL, NULL }; void facts_ ##name## _function(Facts *facts)

#define FACTS(name) FACTS_DECLARE(name,FACTS_STATE_INCLUDE)
#define FACTS_EXCLUDE(name) FACTS_DECLARE(name,FACTS_STATE_EXCLUDE)

#ifndef FACTS_C
FACTS(0000_BEGIN) {}
#endif

#define FACTS_FINISHED FACTS(zzzz_END) {}; void FactsRegisterAll() { FactsFindInMemory(&facts_0000_BEGIN_data, &facts_zzzz_END_data); }

#define FACTS_REGISTER(name) FactsRegister(&facts_ ##name## _data)

#define FACTS_MAIN int main(int argc, const char *argv[]) { return FactsMain(argc, argv); }
