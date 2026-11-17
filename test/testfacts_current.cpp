// REQ4/WR-10: a method whose signature cannot take a Facts* checks facts in the running group.
#include <iostream>
#include "facts.h"

struct Shape {
  virtual ~Shape() {}
  virtual int sides() const = 0;
};

struct Square : Shape {
  int sides() const override {
    Facts *facts = FactsCurrent();
    FACT(sides_ == 4, "a square with " << sides_ << " sides");
    return sides_;
  }
  int sides_;
  explicit Square(int n) : sides_(n) {}
};

FACTS(CurrentInMethod) {
  Square good(4), bad(5);
  const Shape &shape = good;
  FACT(shape.sides() == 4);
  bad.sides();
  std::cout << "not reached: CurrentInMethod went on" << std::endl;
}

FACTS_REGISTER_ALL() {
  FACTS_REGISTER(CurrentInMethod);
}

FACTS_MAIN
