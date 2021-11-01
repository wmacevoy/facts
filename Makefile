CSTD?=c11
CDBG?=-g
COPT?=-O2
CFLAGS=$(CDBG) $(COPT) -std=$(CSTD) -Iinclude
LDLIBS=-lm

examples/hello/facts.c : src/facts.c
	cp $< $@

examples/hello/facts.h : include/facts.h
	cp $< $@

examples/hello/hello : examples/hello/hello.c examples/hello/facts.c examples/hello/facts.h
	$(MAKE) -C examples/hello

tmp/facts.o: src/facts.c include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) -o $@ $<

tmp/testfacts.o: src/testfacts.c include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) -o $@ $<

bin/testfacts : tmp/testfacts.o tmp/facts.o
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

examples : examples/hello/facts.c examples/hello/facts.h examples/hello/hello

all : bin/testfacts examples


