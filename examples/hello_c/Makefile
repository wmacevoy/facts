UNAME_S := $(shell uname -s)
ifneq ($(UNAME_S),Darwin)
  LDLIBS_DL = -ldl
  LDFLAGS_EXPORT = -rdynamic
endif

all : hello
hello : hello.c facts.c facts.h
	cc -std=c11 -g $(LDFLAGS_EXPORT) -o hello hello.c facts.c $(LDLIBS_DL)
