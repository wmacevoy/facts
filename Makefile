CSTD?=c11
CDBG?=-g
COPT?=-O2
CFLAGS=$(CDBG) $(COPT) -std=$(CSTD) -Iinclude

tmp/facts.o: src/facts.c include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) -o $@ $<

tmp/testfacts.o: src/testfacts.c include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) -o $@ $<

bin/testfacts : tmp/testfacts.o tmp/facts.o
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^

all : bin/testfacts


