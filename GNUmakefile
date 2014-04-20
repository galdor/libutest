# Common
prefix= /usr/local
libdir= $(prefix)/lib
incdir= $(prefix)/include

CC=   clang

CFLAGS+= -std=c99
CFLAGS+= -Wall -Wextra -Werror -Wsign-conversion
CFLAGS+= -Wno-unused-parameter -Wno-unused-function

LDFLAGS=

PANDOC_OPTS= -s --toc --email-obfuscation=none

# Platform specific
platform= $(shell uname -s)

ifeq ($(platform), Linux)
	CFLAGS+= -DHTTP_PLATFORM_LINUX
	CFLAGS+= -D_POSIX_C_SOURCE=200809L -D_BSD_SOURCE
endif

# Debug
debug=0
ifeq ($(debug), 1)
	CFLAGS+= -g -ggdb
else
	CFLAGS+= -O2
endif

# Target: libutest
libutest_LIB= libutest.a
libutest_SRC= $(wildcard src/*.c)
libutest_PUBINC= src/utest.h
libutest_INC= $(wildcard src/*.h)
libutest_OBJ= $(subst .c,.o,$(libutest_SRC))

$(libutest_LIB): CFLAGS+=

# Target: tests
tests_SRC= $(wildcard tests/*.c)
tests_OBJ= $(subst .c,.o,$(tests_SRC))
tests_BIN= $(subst .o,,$(tests_OBJ))

$(tests_BIN): CFLAGS+= -Isrc
$(tests_BIN): LDFLAGS+= -L.
$(tests_BIN): LDLIBS+= -lutest

# Target: doc
doc_SRC= $(wildcard doc/*.mkd)
doc_HTML= $(subst .mkd,.html,$(doc_SRC))

# Rules
all: lib $(tests_BIN) $(doc_HTML)

lib: $(libutest_LIB)

$(libutest_OBJ): $(libutest_INC)
$(libutest_LIB): $(libutest_OBJ)
	$(AR) cr $@ $(libutest_OBJ)

$(tests_OBJ): $(libutest_LIB) $(libutest_INC)
tests/%: tests/%.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

doc/%.html: doc/*.mkd
	pandoc $(PANDOC_OPTS) -t html5 -o $@ $<

clean:
	$(RM) $(libutest_LIB) $(wildcard libutest/*.o)
	$(RM) $(tests_BIN) $(wildcard tests/*.o)
	$(RM) -r $(doc_HTML)

install: lib
	mkdir -p $(libdir) $(incdir)
	install -m 644 $(libutest_LIB) $(libdir)
	install -m 644 $(libutest_PUBINC) $(incdir)

uninstall:
	$(RM) $(addprefix $(libdir)/,$(libutest_LIB))
	$(RM) $(addprefix $(incdir)/,$(libutest_PUBINC))

tags:
	$(RM) -f .tags
	ctags -o .tags -a $(wildcard libutest/*.[hc])

.PHONY: all clean install lib uninstall tags
