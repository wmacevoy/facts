VERSION_MAJOR = 1
VERSION_MINOR = 3
VERSION_PATCH = 0
VERSION = $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

PREFIX       ?= /usr/local
LIBDIR       ?= $(PREFIX)/lib
INCLUDEDIR   ?= $(PREFIX)/include
PKGCONFIGDIR ?= $(LIBDIR)/pkgconfig

CC       ?= cc
CXX      ?= c++
CSTD     ?= c11
CXXSTD   ?= c++11
CFLAGS   ?= -g -O2
CXXFLAGS ?= -g -O2
CFLAGS   += -std=$(CSTD) -Iinclude
CXXFLAGS += -std=$(CXXSTD) -Iinclude

UNAME_S := $(shell uname -s)

# --- C library (libfacts) ---
STATIC_LIB_C = libfacts.a
ifeq ($(UNAME_S),Darwin)
  SHARED_LIB_C = libfacts.$(VERSION_MAJOR).dylib
  SONAME_FLAG_C = -install_name,$(LIBDIR)/$(SHARED_LIB_C)
  SOLINK_C     = libfacts.dylib
else
  SHARED_LIB_C = libfacts.so.$(VERSION)
  SONAME_C     = libfacts.so.$(VERSION_MAJOR)
  SONAME_FLAG_C = -soname,$(SONAME_C)
  SOLINK_C     = libfacts.so
endif

# --- C++ library (libfacts++) ---
STATIC_LIB_CPP = libfacts++.a
ifeq ($(UNAME_S),Darwin)
  SHARED_LIB_CPP = libfacts++.$(VERSION_MAJOR).dylib
  SONAME_FLAG_CPP = -install_name,$(LIBDIR)/$(SHARED_LIB_CPP)
  SOLINK_CPP     = libfacts++.dylib
else
  SHARED_LIB_CPP = libfacts++.so.$(VERSION)
  SONAME_CPP     = libfacts++.so.$(VERSION_MAJOR)
  SONAME_FLAG_CPP = -soname,$(SONAME_CPP)
  SOLINK_CPP     = libfacts++.so
endif

# Object files (distinct names to avoid collisions)
C_OBJ     = src/facts_c.o
C_PIC     = src/facts_c.pic.o
CPP_OBJ   = src/facts_cpp.o
CPP_PIC   = src/facts_cpp.pic.o

# =============================================================
# Targets
# =============================================================

.PHONY: all
all: $(STATIC_LIB_C) $(SHARED_LIB_C) $(STATIC_LIB_CPP) $(SHARED_LIB_CPP)

.PHONY: libs
libs: $(STATIC_LIB_C) $(SHARED_LIB_C) $(STATIC_LIB_CPP) $(SHARED_LIB_CPP)

# --- C library ---
$(C_OBJ): src/facts.c include/facts.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(C_PIC): src/facts.c include/facts.h
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

$(STATIC_LIB_C): $(C_OBJ)
	$(AR) rcs $@ $<

$(SHARED_LIB_C): $(C_PIC)
	$(CC) -shared -Wl,$(SONAME_FLAG_C) -o $@ $<

# --- C++ library ---
$(CPP_OBJ): src/facts.cpp src/facts.c include/facts.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(CPP_PIC): src/facts.cpp src/facts.c include/facts.h
	$(CXX) $(CXXFLAGS) -fPIC -c -o $@ $<

$(STATIC_LIB_CPP): $(CPP_OBJ)
	$(AR) rcs $@ $<

$(SHARED_LIB_CPP): $(CPP_PIC)
	$(CXX) -shared -Wl,$(SONAME_FLAG_CPP) -o $@ $<

# --- Tests ---
CFLAGS_DEBUG   = -g -std=$(CSTD) -Iinclude
CXXFLAGS_DEBUG = -g -std=$(CXXSTD) -Iinclude

.PHONY: tests
tests: bin/testfacts_c bin/testfacts_cpp bin/testfacts_if_c bin/testfacts_if_cpp

bin/testfacts_c: test/testfacts.c src/facts.c include/facts.h
	mkdir -p bin
	$(CC) $(CFLAGS_DEBUG) $(LDFLAGS) -o $@ test/testfacts.c src/facts.c

bin/testfacts_cpp: test/testfacts.cpp src/facts.cpp src/facts.c include/facts.h
	mkdir -p bin
	$(CXX) $(CXXFLAGS_DEBUG) $(LDFLAGS) -o $@ test/testfacts.cpp src/facts.cpp

bin/testfacts_if_c: test/testfacts_if.c src/facts.c include/facts.h
	mkdir -p bin
	$(CC) $(CFLAGS_DEBUG) $(LDFLAGS) -o $@ test/testfacts_if.c src/facts.c

bin/testfacts_if_cpp: test/testfacts_if.cpp src/facts.cpp src/facts.c include/facts.h
	mkdir -p bin
	$(CXX) $(CXXFLAGS_DEBUG) $(LDFLAGS) -o $@ test/testfacts_if.cpp src/facts.cpp

# --- Examples ---
.PHONY: examples
examples: examples/hello_c/hello examples/hello_cpp/hello

examples/hello_c/facts.c: src/facts.c
	cp $< $@

examples/hello_c/facts.h: include/facts.h
	cp $< $@

examples/hello_c/hello: examples/hello_c/hello.c examples/hello_c/facts.c examples/hello_c/facts.h
	$(MAKE) -C examples/hello_c hello

examples/hello_cpp/facts.h: include/facts.h
	cp $< $@

examples/hello_cpp/facts.c: src/facts.c
	cp $< $@

examples/hello_cpp/facts.cpp: src/facts.cpp
	cp $< $@

examples/hello_cpp/hello: examples/hello_cpp/hello.cpp examples/hello_cpp/facts.cpp examples/hello_cpp/facts.c examples/hello_cpp/facts.h
	$(MAKE) -C examples/hello_cpp hello

# --- Check (build tests + examples, diff against expected output) ---
.PHONY: check
check: tests examples
	(bin/testfacts_c 2>/dev/null || true) | diff - expected/testfacts_c.out
	(bin/testfacts_cpp 2>/dev/null || true) | diff - expected/testfacts_cpp.out
	(bin/testfacts_c --facts_plain 2>/dev/null || true) | diff - expected/testfacts_c_plain.out
	(bin/testfacts_cpp --facts_plain 2>/dev/null || true) | diff - expected/testfacts_cpp_plain.out
	(bin/testfacts_c --facts_junit 2>/dev/null || true) | diff - expected/testfacts_c_junit.out
	(bin/testfacts_cpp --facts_junit 2>/dev/null || true) | diff - expected/testfacts_cpp_junit.out
	(bin/testfacts_if_c 2>/dev/null || true) | diff - expected/testfacts_if_c.out
	(bin/testfacts_if_cpp 2>/dev/null || true) | diff - expected/testfacts_if_cpp.out
	(bin/testfacts_if_c --facts 2>/dev/null || true) | diff - expected/testfacts_if_facts_c.out
	(bin/testfacts_if_cpp --facts 2>/dev/null || true) | diff - expected/testfacts_if_facts_cpp.out
	(examples/hello_c/hello 2>/dev/null || true) | diff - expected/hello_c.out
	(examples/hello_cpp/hello 2>/dev/null || true) | diff - expected/hello_cpp.out
	(examples/hello_c/hello --facts_plain 2>/dev/null || true) | diff - expected/hello_c_plain.out
	(examples/hello_cpp/hello --facts_plain 2>/dev/null || true) | diff - expected/hello_cpp_plain.out
	(examples/hello_c/hello --facts_junit 2>/dev/null || true) | diff - expected/hello_c_junit.out
	(examples/hello_cpp/hello --facts_junit 2>/dev/null || true) | diff - expected/hello_cpp_junit.out

.PHONY: expected
expected: tests examples
	bin/testfacts_c 2>/dev/null >expected/testfacts_c.out || true
	bin/testfacts_cpp 2>/dev/null >expected/testfacts_cpp.out || true
	bin/testfacts_c --facts_plain 2>/dev/null >expected/testfacts_c_plain.out || true
	bin/testfacts_cpp --facts_plain 2>/dev/null >expected/testfacts_cpp_plain.out || true
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
	examples/hello_cpp/hello --facts_junit >expected/hello_cpp_junit.out || true

# --- pkg-config ---
facts.pc: facts.pc.in
	sed -e 's|@PREFIX@|$(PREFIX)|g' \
	    -e 's|@LIBDIR@|$(LIBDIR)|g' \
	    -e 's|@INCLUDEDIR@|$(INCLUDEDIR)|g' \
	    -e 's|@PROJECT_VERSION@|$(VERSION)|g' \
	    $< >$@

facts++.pc: facts++.pc.in
	sed -e 's|@PREFIX@|$(PREFIX)|g' \
	    -e 's|@LIBDIR@|$(LIBDIR)|g' \
	    -e 's|@INCLUDEDIR@|$(INCLUDEDIR)|g' \
	    -e 's|@PROJECT_VERSION@|$(VERSION)|g' \
	    $< >$@

# --- Install ---
.PHONY: install
install: $(STATIC_LIB_C) $(SHARED_LIB_C) $(STATIC_LIB_CPP) $(SHARED_LIB_CPP) facts.pc facts++.pc
	install -d $(DESTDIR)$(INCLUDEDIR)
	install -d $(DESTDIR)$(LIBDIR)
	install -d $(DESTDIR)$(PKGCONFIGDIR)
	install -m 644 include/facts.h     $(DESTDIR)$(INCLUDEDIR)/facts.h
	install -m 644 $(STATIC_LIB_C)     $(DESTDIR)$(LIBDIR)/$(STATIC_LIB_C)
	install -m 755 $(SHARED_LIB_C)     $(DESTDIR)$(LIBDIR)/$(SHARED_LIB_C)
	install -m 644 $(STATIC_LIB_CPP)   $(DESTDIR)$(LIBDIR)/$(STATIC_LIB_CPP)
	install -m 755 $(SHARED_LIB_CPP)   $(DESTDIR)$(LIBDIR)/$(SHARED_LIB_CPP)
ifneq ($(UNAME_S),Darwin)
	ln -sf $(SHARED_LIB_C)   $(DESTDIR)$(LIBDIR)/$(SONAME_C)
	ln -sf $(SHARED_LIB_CPP) $(DESTDIR)$(LIBDIR)/$(SONAME_CPP)
endif
	ln -sf $(SHARED_LIB_C)   $(DESTDIR)$(LIBDIR)/$(SOLINK_C)
	ln -sf $(SHARED_LIB_CPP) $(DESTDIR)$(LIBDIR)/$(SOLINK_CPP)
	install -m 644 facts.pc   $(DESTDIR)$(PKGCONFIGDIR)/facts.pc
	install -m 644 facts++.pc $(DESTDIR)$(PKGCONFIGDIR)/facts++.pc

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(INCLUDEDIR)/facts.h
	rm -f $(DESTDIR)$(LIBDIR)/$(STATIC_LIB_C)
	rm -f $(DESTDIR)$(LIBDIR)/$(SHARED_LIB_C)
	rm -f $(DESTDIR)$(LIBDIR)/$(STATIC_LIB_CPP)
	rm -f $(DESTDIR)$(LIBDIR)/$(SHARED_LIB_CPP)
ifneq ($(UNAME_S),Darwin)
	rm -f $(DESTDIR)$(LIBDIR)/$(SONAME_C)
	rm -f $(DESTDIR)$(LIBDIR)/$(SONAME_CPP)
endif
	rm -f $(DESTDIR)$(LIBDIR)/$(SOLINK_C)
	rm -f $(DESTDIR)$(LIBDIR)/$(SOLINK_CPP)
	rm -f $(DESTDIR)$(PKGCONFIGDIR)/facts.pc
	rm -f $(DESTDIR)$(PKGCONFIGDIR)/facts++.pc

# --- Cosmopolitan Libc (Actually Portable Executable) — C library only ---
.PHONY: cosmo
cosmo:
	$(MAKE) CC=cosmocc AR=cosmoar $(STATIC_LIB_C)

.PHONY: cosmo-check
cosmo-check:
	$(MAKE) CC=cosmocc AR=cosmoar CXX=cosmoc++ check

# --- Clean ---
.PHONY: clean
clean:
	rm -f src/*.o $(STATIC_LIB_C) $(SHARED_LIB_C) $(STATIC_LIB_CPP) $(SHARED_LIB_CPP)
	rm -f libfacts.so* libfacts*.dylib libfacts++.so* libfacts++*.dylib
	rm -f facts.pc facts++.pc
	rm -rf bin
	rm -f examples/hello_c/hello examples/hello_cpp/hello
