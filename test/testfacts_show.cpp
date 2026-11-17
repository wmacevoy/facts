// REQ4/WR-9: a fiction shows a value whatever its type, and every comparison compiles.
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "facts.h"

struct Opaque {
  int id;
  bool operator==(const Opaque &other) const { return id == other.id; }
};

struct Named {
  int id;
  bool operator==(const Named &other) const { return id == other.id; }
};
std::ostream &operator<<(std::ostream &out, const Named &named) { return out << "Named#" << named.id; }

FACTS(ShowContainers) {
  std::vector<int> a = {1, 2}, b = {1, 3};
  FACT_CHECK(a == b);
  FACT_CHECK(a, ==, b);
  std::vector<std::vector<int> > nested = {{1}, {2, 3}}, other = {{1}};
  FACT_CHECK(nested == other);
  std::map<std::string, int> m = {{"one", 1}, {"two", 2}}, empty;
  FACT_CHECK(m == empty);
  std::vector<int> many(25, 7), fewer(3, 7);
  FACT_CHECK(many == fewer);
}

FACTS(ShowPairs) {
  std::pair<int, std::string> p(1, "x"), q(2, "y");
  FACT_CHECK(p == q);
  FACT_CHECK(p < q);
  FACT_CHECK(p, >, q);
}

FACTS(ShowOthers) {
  Opaque x = {1}, y = {2};
  FACT_CHECK(x == y);
  FACT_CHECK(x, ==, y);
  Named n = {1}, o = {2};
  FACT_CHECK(n == o);
  std::string s = "abc";
  FACT_CHECK(s == "abd");
}

FACTS_REGISTER_ALL() {
  FACTS_REGISTER(ShowContainers);
  FACTS_REGISTER(ShowPairs);
  FACTS_REGISTER(ShowOthers);
}

FACTS_MAIN
