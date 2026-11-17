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

#define FACTS_STATE_EXCLUDE -2
#define FACTS_STATE_FAIL -1
#define FACTS_STATE_INCLUDE 0
#define FACTS_STATE_PASS 1

#define FACTS_CONSOLE (((facts_format) & (1<<0)) == 0) 
#define FACTS_JUNIT   (((facts_format) & (1<<0)) != 0) 
#define FACTS_COLOR   (((facts_format) & 3) == 0)

  struct FactsStruct
  {
    const char *file;
    int line;
    const char *name;
    void (*function)(Facts *facts);
    int status;
    Facts *next;
    Facts *prev;
  };

  void FactsPrint(const char *fmt, ...);
  void FactsInclude(const char *pattern);
  void FactsExclude(const char *pattern);
  void FactsRegister(Facts *facts);
  void FactsCheck();
  // The group being run, or NULL outside a run (REQ4/WR-10).  A helper with no
  // Facts parameter writes `Facts *facts = FactsCurrent();` and can use FACT.
  Facts *FactsCurrent(void);
  void FactsFiction(const char *file, int line, Facts *facts,
		    const char *a, const char *op, const char *b);

  double FactsAbsErr(double a, double b);
  double FactsRelErr(double a, double b);

  int FactsMain(int argc, const char *argv[]);
  int FactsMainIf(int argc, const char *argv[], const char *own);

// https://stackoverflow.com/questions/24844970/how-to-print-types-of-unknown-size-like-ino-t
#define FACTS_PRINT_FORMAT(X) _Generic((X),                    \
                                       _Bool                   \
                                       : "%d",                 \
                                         char                  \
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
  extern uint64_t facts_twice;
  extern int facts_format;
#endif

// FACT(a, op, b) prints the values of a and b when it is fiction.
//
// Where the compiler can keep a value whose type it was not told (__typeof__
// and a block inside an expression: gcc, clang), each side is evaluated once.
// Elsewhere a fiction evaluates each side a second time to print it, counts
// that in facts_twice, and the console report says so in its last line.
// In C++ each side is always evaluated once.
// Define FACTS_EVAL_TWICE to get the second behavior anywhere (the check does).
#if !defined(FACTS_EVAL_TWICE) && (defined(__GNUC__) || defined(__clang__))
// "0 ? (a) : (a)" has the type a value of a has: an array becomes a pointer,
// a bit-field an int.  __typeof__ does not evaluate it.
#define FACT_CHECK_PRINT(a, op, b, fmt) __extension__ ({ \
    __typeof__(0 ? (a) : (a)) facts_a_ = (a); \
    __typeof__(0 ? (b) : (b)) facts_b_ = (b); \
    (facts_a_ op facts_b_) ? (++facts_truths, 1) : (FactsPrint("%s/%s %d: %s%s {=%?} " #op " %s {=%?} is fiction%s\n", fmt, fmt, __FILE__, facts->name, __LINE__, FACTS_RED, #a, facts_a_, #b, facts_b_, FACTS_RESET), FactsFiction(__FILE__, __LINE__, facts, #a, #op, #b), facts->status = -1, ++facts_fictions, 0); })
#else
#define FACT_CHECK_PRINT(a, op, b, fmt) (((a)op(b)) ? (++facts_truths, 1) : (FactsPrint("%s/%s %d: %s%s {=%?} " #op " %s {=%?} is fiction%s\n", fmt, fmt, __FILE__, facts->name, __LINE__, FACTS_RED, #a, (a), #b, (b), FACTS_RESET), FactsFiction(__FILE__, __LINE__, facts, #a, #op, #b), facts->status = -1, ++facts_fictions, ++facts_twice, 0))
#endif
#define FACT_PRINT(a, op, b, fmt)  \
  if (!FACT_CHECK_PRINT(a, op, b, fmt)) \
  return

// C++: the lambda is there to hold each side while it is compared and printed
// (a number by value, anything else by reference: FactsHeld).
#define FACT_CHECK_CERR(a, op, b) ([&]() -> int { typename FactsHeld<decltype((a))>::type facts_a_ = (a); typename FactsHeld<decltype((b))>::type facts_b_ = (b); return (facts_a_ op facts_b_) ? (++facts_truths, 1) : (std::cout  << __FILE__ << "/" << facts->name << " " << __LINE__ << ": " << FACTS_RED << #a << " {=" << FactsShow(facts_a_) << "} " << #op << " " << #b << " {=" << FactsShow(facts_b_) << "} is fiction" << FACTS_RESET << std::endl, FactsFiction(__FILE__, __LINE__, facts, #a, #op, #b), facts->status = -1, ++facts_fictions, 0); }())
#define FACT_CERR(a, op, b)  \
  if (!FACT_CHECK_CERR(a, op, b)) \
  return

#ifdef __cplusplus
// C++ takes three forms, picked by the number of arguments:
//   FACT(a, op, b)      the form C has
//   FACT(a op b)        the comparison as you would write it; the values are still printed
//   FACT(a op b, note)  note is streamed (as in FACTS_TRACE) only if the fact is fiction
// FACT_CHECK takes the same three.
#define FACTS_EXPAND(x) x
#define FACTS_FACT_PICK(_1, _2, _3, name, ...) name
#define FACT_CHECK(...) FACTS_EXPAND(FACTS_FACT_PICK(__VA_ARGS__, FACT_CHECK_CERR, FACT_CHECK_NOTE, FACT_CHECK_EXPR, )(__VA_ARGS__))
// The operator that takes the expression apart is the one each compiler is
// quiet about: gcc suggests parentheses at every "<= a == b", clang at
// "<< a + 1 == b", and neither warning can be switched off from inside a macro.
#if defined(__clang__)
#define FACTS_DECOMPOSE(expr) FactsDecompose() <= expr
#else
#define FACTS_DECOMPOSE(expr) FactsDecompose() << expr
#endif
#define FACT_CHECK_EXPR(expr) FactsExprCheck(facts, __FILE__, __LINE__, #expr, FACTS_DECOMPOSE(expr))
#define FACT_CHECK_NOTE(expr, note) FactsExprCheck(facts, __FILE__, __LINE__, #expr, FACTS_DECOMPOSE(expr), [&](std::ostream &foot) { foot << note; })
#define FACT_EXPR(expr)  \
  if (!FACT_CHECK_EXPR(expr)) \
  return
#define FACT_NOTE(expr, note)  \
  if (!FACT_CHECK_NOTE(expr, note)) \
  return
// How a fiction leaves (REQ4/WR-8).  By return: the function the fact is
// written in returns, which ends the group only when that is the group.  By
// exception: FactsLeave is thrown and the group's FACTS_DECLARE catches it, so a fiction in a
// helper, method, or lambda ends the group, and a helper that returns a value
// can use FACT.  FACT leaves by exception wherever the compiler has them.
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
#define FACTS_EXCEPTIONS 1
#endif
struct FactsLeave {};
#define FACT_BY_RETURN(...) if (!FACT_CHECK(__VA_ARGS__)) return
#ifdef FACTS_EXCEPTIONS
#define FACT_BY_EXCEPTION(...) if (!FACT_CHECK(__VA_ARGS__)) throw FactsLeave()
#define FACT(...) FACT_BY_EXCEPTION(__VA_ARGS__)
#else
#define FACT(...) FACT_BY_RETURN(__VA_ARGS__)
#endif
#define FACTS_EXTERN extern "C"
#else
#define FACT(a, op, b) FACT_PRINT(a, op, b, FACTS_PRINT_FORMAT(a))
#define FACT_CHECK(a, op, b) FACT_CHECK_PRINT(a, op, b, FACTS_PRINT_FORMAT(a))
#define FACT_BY_RETURN(a, op, b) FACT(a, op, b)
#define FACT_BY_EXCEPTION(a, op, b) _Static_assert(0, "FACT_BY_EXCEPTION is C++ only; C leaves by return")
#define FACTS_EXTERN
#endif

#if defined(__cplusplus) && defined(FACTS_EXCEPTIONS)
// In C++ the run calls facts_<name>_caught, which catches the fiction FACT
// throws (REQ4/WR-8).  The catch is here, in the program's own code, so it
// works whichever library is linked, the C one included.  The group's body
// keeps the name facts_<name>_function, which the gdb hints break in.
#define FACTS_DECLARE(name, state)                                                                    \
  void facts_##name##_function(Facts *facts);                                                           \
  static void facts_##name##_caught(Facts *facts)                                                       \
  {                                                                                                     \
    try { facts_##name##_function(facts); } catch (FactsLeave &) {}                                     \
  }                                                                                                     \
  Facts facts_##name##_data = {__FILE__, __LINE__, #name, &facts_##name##_caught, state, NULL, NULL};   \
  void facts_##name##_function(Facts *facts)
#else
#define FACTS_DECLARE(name, state)                                                                    \
  void facts_##name##_function(Facts *facts);                                                           \
  Facts facts_##name##_data = {__FILE__, __LINE__, #name, &facts_##name##_function, state, NULL, NULL}; \
  void facts_##name##_function(Facts *facts)
#endif

#define FACTS_INCLUDE(name) FACTS_DECLARE(name, FACTS_STATE_INCLUDE)
#define FACTS_EXCLUDE(name) FACTS_DECLARE(name, FACTS_STATE_EXCLUDE)
#define FACTS(name) FACTS_INCLUDE(name)

#define FACTS_REGISTER(name) FactsRegister(&facts_##name##_data)

#define FACTS_REGISTER_ALL                \
  FACTS_EXTERN void FactsRegisterAll

#define FACTS_MAIN 						\
  int main(int argc, const char *argv[]) { return FactsMain(argc, argv); }

#define FACTS_MAIN_IF(check)						\
  int FactsMainElse(int argc, const char *argv[]);			\
  int main(int argc, const char *argv[]) {				\
    const char *facts=#check;						\
    for (int argi=1; argi<argc; ++argi) {				\
      const char *arg=argv[argi];					\
      for (int c=0; arg[c] == facts[c]; ++c) {				\
        if (arg[c] == 0) return FactsMainIf(argc, argv, facts);		\
      }									\
    }									\
    return FactsMainElse(argc,argv);					\
  }									\
  int FactsMainElse(int argc, const char *argv[])
    
#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
#include <functional>
#include <iostream>
#include <type_traits>
#include <iterator>
#include <utility>

  // How a fact holds a side while it compares and prints it.  A number, enum,
  // or pointer is copied: a reference to `static const size_t MAX = 64;`,
  // declared in a class and never defined outside it, fails to link.  Anything
  // else is held by reference, so it need not be copyable.
  template <class T> struct FactsHeld
  {
    typedef typename std::remove_cv<typename std::remove_reference<T>::type>::type bare;
    typedef typename std::conditional<std::is_scalar<bare>::value, bare, const bare &>::type type;
  };
 
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

  // ---- FACT(a op b): the expression form ----
  //
  // "FactsDecompose() << a == b" parses as "(FactsDecompose() << a) == b",
  // because << binds tighter than every comparison and looser than arithmetic.
  // (With clang it is <=, which binds no looser than the other comparisons.)
  // The first step keeps a reference to a, the comparison keeps a reference to
  // b and the answer, so a fiction can print both values without evaluating
  // either side again.  An expression joined by && or || is checked as a whole
  // and prints no values.  A shift on the left needs its own parentheses to
  // compile everywhere: FACT((x << 1) == 4).

  extern "C" {
    extern uint64_t facts_fictions;
    extern uint64_t facts_truths;
    extern int facts_format;
  }

  // ---- facts::interval: a value within a tolerance ----
  //
  //   FACT(x == y + facts::pm(1e-9));          // within 1e-9 of y
  //   FACT(x == y * (1 + facts::pm(1e-9)));    // within one part in 1e9 of y
  //   FACT(x != facts::interval::open(0, 1));  // not strictly between 0 and 1
  //
  // "x == interval" asks whether x is in it.  pm(eps) is [-eps,eps]; adding a
  // number shifts an interval and multiplying by one scales it, so the two
  // lines above read as written.  From the darter project, by the same author.
  // Inside a group `facts` is also the name of a parameter; `facts::pm` still
  // means this namespace, because a name before :: is never a variable.
  namespace facts
  {
    struct interval
    {
      double a;
      double b;
      bool ainc;   // is a itself inside?
      bool binc;   // is b itself inside?
      interval(double _a, double _b, bool _ainc = true, bool _binc = true)
          : a(_a), b(_b), ainc(_ainc), binc(_binc) {}
      static interval open(double a, double b) { return interval(a, b, false, false); }
      static interval closed(double a, double b) { return interval(a, b, true, true); }
    };

    inline interval pm(double eps) { return interval::closed(-eps, eps); }

    inline interval operator+(double x, const interval &i)
    {
      return interval(i.a + x, i.b + x, i.ainc, i.binc);
    }

    // A negative factor turns the interval around, ends and all.
    inline interval operator*(double x, const interval &i)
    {
      return (x >= 0) ? interval(x * i.a, x * i.b, i.ainc, i.binc)
                      : interval(x * i.b, x * i.a, i.binc, i.ainc);
    }

    inline bool operator==(double x, const interval &i)
    {
      if (i.b < i.a) return false;
      if (i.a < x && x < i.b) return true;
      if (i.ainc && i.a == x) return true;
      if (i.binc && i.b == x) return true;
      return false;
    }

    inline bool operator!=(double x, const interval &i) { return !(x == i); }

    // 17 digits: a tolerance of 1e-9 printed with the usual 6 would show
    // [1,1], which says nothing about why the fact was fiction.
    inline std::ostream &operator<<(std::ostream &out, const interval &i)
    {
      std::streamsize digits = out.precision(17);
      out << (i.ainc ? "[" : "(") << i.a << "," << i.b << (i.binc ? "]" : ")");
      out.precision(digits);
      return out;
    }
  }

  template <class L, class R> struct FactsBinary
  {
    typename FactsHeld<L>::type lhs;
    const char *op;
    typename FactsHeld<R>::type rhs;
    bool truth;
    explicit operator bool() const { return truth; }
  };

// FACT(v.size() == 3) compares a size_t with an int: the literal's type is all
// the template sees, so it would warn where the plain expression does not.
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif
  // Each comparison twice: a number taken by value, anything else by reference
  // (FactsHeld).  An array is taken by value too, as the pointer it decays to;
  // otherwise "abc" would match both.
#define FACTS_LHS_OP(OP) \
    template <class R> typename std::enable_if<std::is_scalar<R>::value, FactsBinary<L, R> >::type operator OP(R rhs) const { return FactsBinary<L, R>{lhs, #OP, rhs, static_cast<bool>(lhs OP rhs)}; } \
    template <class R> typename std::enable_if<!std::is_scalar<R>::value && !std::is_array<R>::value, FactsBinary<L, R> >::type operator OP(const R &rhs) const { return FactsBinary<L, R>{lhs, #OP, rhs, static_cast<bool>(lhs OP rhs)}; }
  template <class L> struct FactsLhs
  {
    typename FactsHeld<L>::type lhs;
    explicit operator bool() const { return static_cast<bool>(lhs); }
    FACTS_LHS_OP(==)
    FACTS_LHS_OP(!=)
    FACTS_LHS_OP(<)
    FACTS_LHS_OP(<=)
    FACTS_LHS_OP(>)
    FACTS_LHS_OP(>=)
  };
#undef FACTS_LHS_OP
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

  struct FactsDecompose
  {
    template <class L> typename std::enable_if<std::is_scalar<L>::value, FactsLhs<L> >::type operator<<(L lhs) const { return FactsLhs<L>{lhs}; }
    template <class L> typename std::enable_if<!std::is_scalar<L>::value && !std::is_array<L>::value, FactsLhs<L> >::type operator<<(const L &lhs) const { return FactsLhs<L>{lhs}; }
    template <class L> typename std::enable_if<std::is_scalar<L>::value, FactsLhs<L> >::type operator<=(L lhs) const { return FactsLhs<L>{lhs}; }
    template <class L> typename std::enable_if<!std::is_scalar<L>::value && !std::is_array<L>::value, FactsLhs<L> >::type operator<=(const L &lhs) const { return FactsLhs<L>{lhs}; }
  };

  // ---- How a fiction shows a value (REQ4/WR-9) ----
  //
  // With its own operator<< if it has one; else a container as its first 20
  // elements in braces, a pair as (first, second); else "?".  So every
  // comparison compiles, whatever its sides.  The ranks pick the first that
  // compiles: a call with FactsRank2 prefers the overload taking FactsRank2.
  struct FactsRank0 {};
  struct FactsRank1 : FactsRank0 {};
  struct FactsRank2 : FactsRank1 {};

  template <class T> void FactsValue(std::ostream &out, const T &value);

  template <class T> auto FactsValueRanked(std::ostream &out, const T &value, FactsRank2) -> decltype((void)(out << value))
  {
    out << value;
  }

  template <class T> auto FactsValueRanked(std::ostream &out, const T &value, FactsRank1) -> decltype((void)std::begin(value), (void)std::end(value))
  {
    const int most = 20;
    int count = 0;
    out << "{";
    for (auto at = std::begin(value); at != std::end(value); ++at, ++count)
    {
      if (count == most) { out << ", ..."; break; }
      if (count > 0) out << ", ";
      FactsValue(out, *at);
    }
    out << "}";
  }

  template <class A, class B> void FactsValueRanked(std::ostream &out, const std::pair<A, B> &value, FactsRank1)
  {
    out << "(";
    FactsValue(out, value.first);
    out << ", ";
    FactsValue(out, value.second);
    out << ")";
  }

  template <class T> void FactsValueRanked(std::ostream &out, const T &, FactsRank0) { out << "?"; }

  template <class T> void FactsValue(std::ostream &out, const T &value) { FactsValueRanked(out, value, FactsRank2()); }

  // `out << FactsShow(x)` writes x as FactsValue does, inside a chain of <<.
  template <class T> struct FactsShowing { const T &value; };
  template <class T> FactsShowing<T> FactsShow(const T &value) { return FactsShowing<T>{value}; }
  template <class T> std::ostream &operator<<(std::ostream &out, const FactsShowing<T> &shown)
  {
    FactsValue(out, shown.value);
    return out;
  }

  // Is it true, and what values does a fiction show: one pair of overloads
  // for each thing the expression can come out as.
  inline bool FactsExprTruth(bool whole) { return whole; }
  template <class L> bool FactsExprTruth(const FactsLhs<L> &one) { return static_cast<bool>(one); }
  template <class L, class R> bool FactsExprTruth(const FactsBinary<L, R> &two) { return two.truth; }

  inline void FactsExprValues(std::ostream &, bool) {}
  template <class L> void FactsExprValues(std::ostream &out, const FactsLhs<L> &one) { out << " {" << FactsShow(one.lhs) << "}"; }
  template <class L, class R> void FactsExprValues(std::ostream &out, const FactsBinary<L, R> &two) { out << " {" << FactsShow(two.lhs) << " " << two.op << " " << FactsShow(two.rhs) << "}"; }

  // Against an interval, the value is printed with the interval's 17 digits.
  template <class L> void FactsExprValues(std::ostream &out, const FactsBinary<L, facts::interval> &two)
  {
    std::streamsize digits = out.precision(17);
    out << " {" << FactsShow(two.lhs) << " " << two.op << " ";
    out.precision(digits);
    out << two.rhs << "}";
  }

  struct FactsNoFootnote {};
  inline void FactsExprFootnote(std::ostream &, const FactsNoFootnote &) {}
  template <class F> void FactsExprFootnote(std::ostream &out, const F &footnote) { out << ": "; footnote(out); }

  // Returns 1 for a truth.  For a fiction, prints it, marks the group failed, and returns 0.
  template <class V, class F>
  bool FactsExprCheck(Facts *facts, const char *file, int line, const char *expr, const V &value, const F &footnote)
  {
    if (FactsExprTruth(value))
    {
      ++facts_truths;
      return true;
    }
    std::cout << file << "/" << facts->name << " " << line << ": " << FACTS_RED << expr;
    FactsExprValues(std::cout, value);
    std::cout << " is fiction";
    FactsExprFootnote(std::cout, footnote);
    std::cout << FACTS_RESET << std::endl;
    FactsFiction(file, line, facts, expr, "", "");
    facts->status = FACTS_STATE_FAIL;
    ++facts_fictions;
    return false;
  }

  template <class V>
  bool FactsExprCheck(Facts *facts, const char *file, int line, const char *expr, const V &value)
  {
    return FactsExprCheck(facts, file, line, expr, value, FactsNoFootnote());
  }

  // The label is only there for a debugger to break at (FACTS_WATCH), so
  // nothing in the program jumps to it.
  #if defined(__GNUC__) || defined(__clang__)
  #define FACTS_UNUSED_LABEL __attribute__((unused))
  #else
  #define FACTS_UNUSED_LABEL
  #endif

  #define FACTS_TRACE_SUFFIX(note,suffix) \
    FactsTrace facts_trace##suffix([=](Facts *facts, std::ostream&out) { out << __FILE__ << "/" << facts->name << ":" << __LINE__ << " " << note << std::endl; }); \
    facts_trace_##suffix: FACTS_UNUSED_LABEL
  #define FACTS_TRACE_COUNT(note,counter) FACTS_TRACE_SUFFIX(note,counter)
  #define FACTS_TRACE(note) FACTS_TRACE_COUNT(note,__COUNTER__)

  #define FACTS_WATCH_SUFFIX(note,suffix) FACTS_TRACE_SUFFIX(FACTS_GREEN << "(gdb)" << FACTS_RESET << " break facts_" << facts->name << "_function:facts_trace_" << suffix << " if " << note, suffix)
  #define FACTS_WATCH_COUNT(note,counter) FACTS_WATCH_SUFFIX(note,counter)
  #define FACTS_WATCH_IF(watch) FACTS_WATCH_COUNT(watch,__COUNTER__)

  // internal: ostream chain — do not call directly
  #define FACTS_OSTREAM1(x)       #x << "==" << (x)
  #define FACTS_OSTREAM2(x, ...)  #x << "==" << (x) << " && " << FACTS_OSTREAM1(__VA_ARGS__)
  #define FACTS_OSTREAM3(x, ...)  #x << "==" << (x) << " && " << FACTS_OSTREAM2(__VA_ARGS__)
  #define FACTS_OSTREAM4(x, ...)  #x << "==" << (x) << " && " << FACTS_OSTREAM3(__VA_ARGS__)
  #define FACTS_OSTREAM5(x, ...)  #x << "==" << (x) << " && " << FACTS_OSTREAM4(__VA_ARGS__)
  #define FACTS_OSTREAM6(x, ...)  #x << "==" << (x) << " && " << FACTS_OSTREAM5(__VA_ARGS__)
  #define FACTS_OSTREAM7(x, ...)  #x << "==" << (x) << " && " << FACTS_OSTREAM6(__VA_ARGS__)
  #define FACTS_OSTREAM8(x, ...)  #x << "==" << (x) << " && " << FACTS_OSTREAM7(__VA_ARGS__)
  #define FACTS_OSTREAM9(x, ...)  #x << "==" << (x) << " && " << FACTS_OSTREAM8(__VA_ARGS__)
  #define FACTS_OSTREAM10(x, ...) #x << "==" << (x) << " && " << FACTS_OSTREAM9(__VA_ARGS__)
  #define FACTS_OSTREAM11(x, ...) #x << "==" << (x) << " && " << FACTS_OSTREAM10(__VA_ARGS__)
  #define FACTS_OSTREAM12(x, ...) #x << "==" << (x) << " && " << FACTS_OSTREAM11(__VA_ARGS__)
  #define FACTS_OSTREAM13(x, ...) #x << "==" << (x) << " && " << FACTS_OSTREAM12(__VA_ARGS__)
  #define FACTS_OSTREAM14(x, ...) #x << "==" << (x) << " && " << FACTS_OSTREAM13(__VA_ARGS__)
  #define FACTS_OSTREAM15(x, ...) #x << "==" << (x) << " && " << FACTS_OSTREAM14(__VA_ARGS__)
  #define FACTS_OSTREAM16(x, ...) #x << "==" << (x) << " && " << FACTS_OSTREAM15(__VA_ARGS__)
  #define FACTS_OSTREAM17(x, ...) #x << "==" << (x) << " && " << FACTS_OSTREAM16(__VA_ARGS__)
  #define FACTS_OSTREAM18(x, ...) #x << "==" << (x) << " && " << FACTS_OSTREAM17(__VA_ARGS__)
  #define FACTS_OSTREAM19(x, ...) #x << "==" << (x) << " && " << FACTS_OSTREAM18(__VA_ARGS__)
  #define FACTS_OSTREAM20(x, ...) #x << "==" << (x) << " && " << FACTS_OSTREAM19(__VA_ARGS__)

  // internal: argument count dispatch
  #define FACTS_ARGC_(                                                   \
    _1,_2,_3,_4,_5,_6,_7,_8,_9,_10,                                     \
    _11,_12,_13,_14,_15,_16,_17,_18,_19,_20,N,...) N
  #define FACTS_ARGC(...) FACTS_ARGC_(__VA_ARGS__,                       \
    20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)
  #define FACTS_PASTE_(a,b) a##b
  #define FACTS_PASTE(a,b)  FACTS_PASTE_(a,b)

  // Conditional gdb breakpoint hint — accepts 1 to 20 variables.
  #define FACTS_WATCH(...)                                               \
    FACTS_WATCH_IF(FACTS_PASTE(FACTS_OSTREAM,FACTS_ARGC(__VA_ARGS__))(__VA_ARGS__))

#endif
