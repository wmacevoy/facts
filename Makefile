CSTD?=c11
CXXSTD?=c++11
CDBG?=-g
COPT?=
CFLAGS=$(CDBG) $(COPT) -std=$(CSTD) -Iinclude
CFLAGS_DEBUG=-g -std=$(CSTD) -Iinclude
CXXFLAGS_DEBUG=-g -std=$(CXXSTD) -Iinclude
LDLIBS=

.PHONY: all
all : bin/testfacts bin/sample_facts_c bin/sample_facts_cpp examples

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

bin/sample_facts_c : src/sample_facts.c src/facts.c include/facts.h
	mkdir -p bin
	$(CC) $(CFLAGS_DEBUG) $(LDFLAGS) -o bin/sample_facts_c src/sample_facts.c src/facts.c $(LDLIBS)

bin/sample_facts_cpp : src/sample_facts.cpp src/facts.cpp src/facts.c include/facts.h
	mkdir -p bin
	cp src/sample_facts.cpp bin/sample_facts.cpp
	$(CXX) $(CXXFLAGS_DEBUG) $(LDFLAGS) -o bin/sample_facts_cpp src/sample_facts.cpp src/facts.cpp $(LDLIBS)

examples : examples/hello_c/hello examples/hello_cpp/hello

.PHONY: check
check : all
	bin/testfacts | diff - expected/testfacts.out
	examples/hello_c/hello | diff - expected/hello_c.out
	examples/hello_cpp/hello | diff - expected/hello_cpp.out
	bin/testfacts --facts_junit | diff - expected/testfacts_junit.out
	examples/hello_c/hello --facts_junit | diff - expected/hello_c_junit.out
	examples/hello_cpp/hello --facts_junit | diff - expected/hello_cpp_junit.out
.PHONY: expected
expected: all
	bin/testfacts > expected/testfacts.out || true
	examples/hello_c/hello > expected/hello_c.out || true
	examples/hello_cpp/hello > expected/hello_cpp.out || true
	bin/testfacts --facts_junit > expected/testfacts_junit.out || true
	examples/hello_c/hello --facts_junit > expected/hello_c_junit.out || true
	examples/hello_cpp/hello --facts_junit > expected/hello_cpp_junit.out || true

