UNAME_S := $(shell uname -s)
ifneq ($(UNAME_S),Darwin)
  LDLIBS_DL = -ldl
  LDFLAGS_EXPORT = -rdynamic
endif

all : hello
hello : hello.cpp facts.cpp facts.c facts.h
	c++ -std=c++11 -g $(LDFLAGS_EXPORT) -o hello hello.cpp facts.cpp $(LDLIBS_DL)
