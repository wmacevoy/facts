CSTD?=c11
CXXSTD?=c++11
CDBG?=-g
COPT?=
CFLAGS=$(CDBG) $(COPT) -std=$(CSTD) -Iinclude
CFLAGS_DEBUG=-g -std=$(CSTD) -Iinclude
CXXFLAGS_DEBUG=-g -std=$(CXXSTD) -Iinclude
LDLIBS=

.PHONY: all
all : bin/testfacts_c bin/testfacts_cpp bin/testfacts_if_c bin/testfacts_if_cpp examples

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

bin/testfacts_c : test/testfacts.c src/facts.c include/facts.h
	mkdir -p bin
	$(CC) $(CFLAGS_DEBUG) $(LDFLAGS) -o $@ test/testfacts.c src/facts.c $(LDLIBS)

bin/testfacts_cpp : test/testfacts.cpp src/facts.cpp src/facts.c include/facts.h
	mkdir -p bin
	$(CXX) $(CXXFLAGS_DEBUG) $(LDFLAGS) -o $@ test/testfacts.cpp src/facts.cpp src/facts.c $(LDLIBS)

bin/testfacts_if_c : test/testfacts_if.c src/facts.c include/facts.h
	mkdir -p bin
	$(CC) $(CFLAGS_DEBUG) $(LDFLAGS) -o $@ test/testfacts_if.c src/facts.c $(LDLIBS)

bin/testfacts_if_cpp : test/testfacts_if.cpp src/facts.cpp src/facts.c include/facts.h
	mkdir -p bin
	$(CXX) $(CXXFLAGS_DEBUG) $(LDFLAGS) -o $@ test/testfacts_if.cpp src/facts.cpp src/facts.c $(LDLIBS)

examples : examples/hello_c/hello examples/hello_cpp/hello

.PHONY: test
test : all
	bin/testfacts_c 2>/dev/null | diff - expected/testfacts_c.out
	bin/testfacts_cpp 2>/dev/null | diff - expected/testfacts_cpp.out
	bin/testfacts_c --facts_plain 2>/dev/null | diff - expected/testfacts_c_plain.out
	bin/testfacts_cpp --facts_plain 2>/dev/null | diff - expected/testfacts_cpp_plain.out
	bin/testfacts_c --facts_junit | diff - expected/testfacts_c_junit.out
	bin/testfacts_cpp --facts_junit | diff - expected/testfacts_cpp_junit.out
	bin/testfacts_if_c 2>/dev/null | diff - expected/testfacts_if_c.out
	bin/testfacts_if_cpp 2>/dev/null | diff - expected/testfacts_if_cpp.out
	bin/testfacts_if_c --facts 2>/dev/null | diff - expected/testfacts_if_facts_c.out
	bin/testfacts_if_cpp --facts 2>/dev/null | diff - expected/testfacts_if_facts_cpp.out
	examples/hello_c/hello 2>/dev/null | diff - expected/hello_c.out
	examples/hello_cpp/hello 2>/dev/null | diff - expected/hello_cpp.out
	examples/hello_c/hello --facts_plain 2>/dev/null | diff - expected/hello_c_plain.out
	examples/hello_cpp/hello --facts_plain 2>/dev/null | diff - expected/hello_cpp_plain.out
	examples/hello_c/hello --facts_junit | diff - expected/hello_c_junit.out
	examples/hello_cpp/hello --facts_junit | diff - expected/hello_cpp_junit.out

.PHONY: expected
expected: all
	bin/testfacts_c 2>/dev/null >expected/testfacts_c.out || true
	bin/testfacts_cpp 2>/dev/null >expected/testfacts_cpp.out || true
	bin/testfacts_c 2>/dev/null --facts_plain >expected/testfacts_c_plain.out || true
	bin/testfacts_cpp 2>/dev/null --facts_plain >expected/testfacts_cpp_plain.out || true
	bin/testfacts_c --facts_junit >expected/testfacts_c_junit.out || true
	bin/testfacts_cpp --facts_junit >expected/testfacts_cpp_junit.out || true
	bin/testfacts_if_c 2>/dev/null >expected/testfacts_if_c.out || true
	bin/testfacts_if_cpp 2>/dev/null >expected/testfacts_if_cpp.out || true
	bin/testfacts_if_c --facts 2>/dev/null >expected/testfacts_if_facts_c.out || true
	bin/testfacts_if_cpp --facts 2>/dev/null >expected/testfacts_if_facts_cpp.out || true
	examples/hello_c/hello 2>/dev/null >expected/hello_c.out || true
	examples/hello_cpp/hello 2>/dev/null >expected/hello_cpp.out || true
	examples/hello_c/hello --facts_plain 2>/dev/null >expected/hello_c_plain.out || true
	examples/hello_cpp/hello --facts_plain 2>/dev/null >expected/hello_cpp_plain.out || true
	examples/hello_c/hello --facts_junit >expected/hello_c_junit.out || true
	examples/hello_cpp/hello  --facts_junit >expected/hello_cpp_junit.out || true

.PHONY: clean
clean:
	/bin/rm -rf bin/* tmp/* build/`uname -s`-`uname -m` examples/hello_c/hello examples/hello_cpp/hello

.PHONY: install
install:
	mkdir -p build/`uname -s`-`uname -m`
	cd build/`uname -s`-`uname -m` && cmake ../.. && $(MAKE) && $(MAKE) install && ldconfig

