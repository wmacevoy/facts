CSTD?=c11
CXXSTD?=c++11
CDBG?=-g
COPT?=
CFLAGS=$(CDBG) $(COPT) -std=$(CSTD) -Iinclude
CFLAGS_DEBUG=-g -std=$(CSTD) -Iinclude
CXXFLAGS_DEBUG=-g -std=$(CXXSTD) -Iinclude
LDLIBS=

.PHONY: all
all : bin/factcheck_check bin/sample_check_c bin/sample_check_cpp examples

examples/hello_c/factcheck.c : src/factcheck.c
	cp $< $@

examples/hello_c/factcheck.h : include/factcheck.h
	cp $< $@

examples/hello_c/hello : examples/hello_c/hello.c examples/hello_c/factcheck.c examples/hello_c/factcheck.h
	$(MAKE) -C examples/hello_c hello

examples/hello_cpp/factcheck.h : include/factcheck.h
	cp $< $@

examples/hello_cpp/factcheck.c : src/factcheck.c
	cp $< $@

examples/hello_cpp/factcheck.cpp : src/factcheck.cpp
	cp $< $@

examples/hello_cpp/hello : examples/hello_cpp/hello.cpp examples/hello_cpp/factcheck.cpp examples/hello_cpp/factcheck.c examples/hello_cpp/factcheck.h
	$(MAKE) -C examples/hello_cpp hello

bin/factcheck_check : src/factcheck_check.c src/factcheck.c include/factcheck.h
	mkdir -p bin
	$(CC) $(CFLAGS_DEBUG) $(LDFLAGS) -o $@ src/factcheck_check.c src/factcheck.c $(LDLIBS)

bin/sample_check_c : src/sample_check.c src/factcheck.c include/factcheck.h
	mkdir -p bin
	cp $^ bin
	cd bin && $(CC) -g -o sample_check_c sample_check.c factcheck.c

bin/sample_check_cpp : src/sample_check.cpp src/factcheck.cpp src/factcheck.c include/factcheck.h
	mkdir -p bin
	cp $^ bin
	cd bin && $(CXX) -g -o sample_check_cpp sample_check.cpp factcheck.cpp

examples : examples/hello_c/hello examples/hello_cpp/hello

.PHONY: check
check : all
	bin/factcheck_factcheck | diff - expected/factcheck_factcheck.out
	examples/hello_c/hello | diff - expected/hello_c.out
	examples/hello_cpp/hello | diff - expected/hello_cpp.out
	bin/factcheck_factcheck --check_junit | diff - expected/factcheck_factcheck_junit.out
	examples/hello_c/hello --check_junit | diff - expected/hello_c_junit.out
	examples/hello_cpp/hello --check_junit | diff - expected/hello_cpp_junit.out
.PHONY: expected
expected: all
	bin/testfacts > expected/testfacts.out || true
	examples/hello_c/hello > expected/hello_c.out || true
	examples/hello_cpp/hello > expected/hello_cpp.out || true
	bin/factcheck_factcheck --check_junit > expected/factcheck_factcheck_junit.out || true
	examples/hello_c/hello --check_junit > expected/hello_c_junit.out || true
	examples/hello_cpp/hello --check_junit > expected/hello_cpp_junit.out || true
