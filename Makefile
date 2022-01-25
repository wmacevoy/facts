CSTD?=c11
CDBG?=-g
COPT?=
CFLAGS=$(CDBG) $(COPT) -std=$(CSTD) -Iinclude
CFLAGS_DEBUG=-g -std=$(CSTD) -Iinclude
LDLIBS=

examples/hello_c/facts.c : src/facts.c
	cp $< $@

examples/hello_c/facts.h : include/facts.h
	cp $< $@

examples/hello_c/hello : examples/hello_c/hello.c examples/hello_c/facts.c examples/hello_c/facts.h
	$(MAKE) -C examples/hello_c hello

examples/hello_cpp/facts.h : include/facts.h
	cp $< $@

examples/hello_cpp/facts.c : src/facts.c
	cp $< $@

examples/hello_cpp/facts.cpp : src/facts.cpp
	cp $< $@

examples/hello_cpp/hello : examples/hello_cpp/hello.cpp examples/hello_cpp/facts.cpp examples/hello_cpp/facts.c examples/hello_cpp/facts.h
	$(MAKE) -C examples/hello_cpp hello

tmp/facts.o: src/facts.c include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) -o $@ $<

tmp/testfacts.o: src/testfacts.c include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS_DEBUG) -o $@ $<

bin/testfacts : tmp/testfacts.o tmp/facts.o
	mkdir -p bin
	$(CC) $(CFLAGS_DEBUG) $(LDFLAGS) -o $@ $^ $(LDLIBS)

examples : examples/hello_c/hello examples/hello_cpp/hello

.PHONY: all
all : bin/testfacts examples

.PHONY: test
test : all
	bin/testfacts 2>&1 | diff - testfacts.expected
	examples/hello_c/hello 2>&1 | diff - hello_c.expected
	examples/hello_cpp/hello 2>&1 | diff - hello_cpp.expected
