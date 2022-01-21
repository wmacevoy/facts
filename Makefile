CSTD?=c11
CDBG?=-g
COPT?=
CFLAGS=$(CDBG) $(COPT) -std=$(CSTD) -Iinclude
LDLIBS=

examples/hello_c/facts.c : src/facts.c
	cp $< $@

examples/hello_c/facts.h : include/facts.h
	cp $< $@

examples/hello_c/hello : examples/hello_c/hello.c examples/hello_c/facts.c examples/hello_c/facts.h
	$(MAKE) -C examples/hello_c

examples/hello_cpp/facts.h : include/facts.h
	cp $< $@

examples/hello_cpp/facts.c : src/facts.c
	cp $< $@

examples/hello_cpp/hello : examples/hello_cpp/hello.cpp examples/hello_cpp/facts.c examples/hello_cpp/facts.h
	$(MAKE) -C examples/hello_cpp

tmp/facts.o: src/facts.c include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) -o $@ $<

tmp/testfacts.o: src/testfacts.c include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) -o $@ $<

bin/testfacts : tmp/testfacts.o tmp/facts.o
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

examples : examples/hello_c/hello examples/hello_cpp/hello

all : bin/testfacts examples


