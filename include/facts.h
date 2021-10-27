#pragma once

#include <stdio.h>
#include <string.h>


struct FactsStruct;
typedef struct FactsStruct Facts;

#define FACTS_SIG {0xe3,0xb0,0xc4,0x42, 0x98,0xfc,0x1c,0x14, 0x9a,0xfb,0xf4,0x89, 0x96,0xfb,0x92,0x00}

struct FactsStruct {
  unsigned char sig[16];
  const char *name;
  void (*function)(Facts *facts);
  int status;
  Facts *next;
  Facts *prev;
};

void FactsPrint(const char *fmt,...);
void FactsFindInMemory(unsigned char *begin,unsigned char *end);
void FactsInclude(const char *pattern);
void FactsExclude(const char *pattern);
void FactsCheck();
void FactsFiction(const char *file, int line,Facts *facts);

double FactsAbsErr(double a, double b);
double FactsRelErr(double a, double b);

#define PASS facts->status =  1
#define FAIL facts->status = -1

#define FACTS_FMT(X) _Generic((X),		    \
			      char: "%c",	    \
			      int: "%d",	    \
			      long: "%ld",	    \
			      float: "%lg",	    \
			      double: "%lg",	    \
			      const char *: "%s" )

#ifndef FACTS_C
extern int facts_fictions;
extern int facts_truths;
#endif

#define CHECK_FMT(a,op,b,fmt) (((a) op (b)) ? (++facts_truths,1) : (FactsPrint("%s %d: %s (=%?) " #op " %s (=%?) is fiction\n",fmt,fmt,__FILE__,__LINE__,#a,(a),#b,(b)), facts->status=-1, FactsFiction(__FILE__,__LINE__,facts),0))
#define FACT_FMT(a,op,b,fmt) if (!CHECK_FMT(a,op,b,fmt)) return
#define CHECK(a,op,b) CHECK_FMT(a,op,b,FACTS_FMT(a))
#define FACT(a,op,b) FACT_FMT(a,op,b,FACTS_FMT(a))

#define FACTS(name) void facts_ ##name## _function (Facts *facts); Facts facts_ ##name## _data = { FACTS_SIG, #name, &facts_ ##name## _function, 0, NULL, NULL }; void facts_ ##name## _function(Facts *facts)

#ifndef FACTS_C
FACTS(0000_BEGIN) {}
#endif

#define FACTS_FINISHED FACTS(zzzz_END) {}; void FactsRegister() { FactsFindInMemory((unsigned char*)&facts_0000_BEGIN_data, (unsigned char *)&facts_zzzz_END_data); }
