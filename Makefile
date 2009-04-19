.PHONY: all lib examples tests

LIB_S_SRCS =  src/atomic_queue_asmimpl.s
LIB_C_SRCS =  src/util.c src/machine.c \
              src/buffer.c src/array.c \
              src/mpool.c \
              src/atomic_queue.c \
              src/runloop.c \
              src/sockaddr.c src/sockutil.c \
              src/sockserv.c src/sockconn.c
LIB_S_OBJS = ${LIB_S_SRCS:.s=.o}
LIB_C_OBJS = ${LIB_C_SRCS:.c=.o}
LIB_OBJS=${LIB_S_OBJS} ${LIB_C_OBJS}

TESTS = refcount test_mpool atomic_queue test_buffer test_array
TEST_SRCS = $(foreach n,$(TESTS),tests/$(n).c)
TEST_OBJS = ${TEST_SRCS:.c=.o}

EXAMPLES = echo_server
EXAMPLE_SRCS = $(foreach n,$(EXAMPLES),examples/$(n).c)
EXAMPLE_OBJS = $(foreach n,$(EXAMPLES),examples/$(n).o)
#EXAMPLE_OBJS = ${EXAMPLES_SRCS:.c=.o}

INCDIR = -I/opt/local/include
LIBSDIR = -L/opt/local/lib

CC=gcc
SFLAGS=
CFLAGS += -Wall -include src/_prefix.h
CFLAGS += -std=c99
CFLAGS += -DNT_DEBUG=1
#CFLAGS += -DNT_LOG_TRACE=1
CFLAGS += -DNT_POOL_ENABLE_LOGGING=1
CFLAGS += -I/opt/local/include
TOOL_CFLAGS = -Lbuild/libs -lnt -levent

LD=/usr/bin/libtool
LIBS += -levent -lSystem
#LIBS += -lgcc_s.1
LD_DYLIB_FLAGS = 
DIRS = build build/libs build/tests build/examples

all: lib tests

lib: ${DIRS} build/libs/libnt.a build/libs/libnt.dylib

tests: ${TESTS}

examples: ${EXAMPLES}

${TESTS}: lib ${TEST_OBJS}
	@echo running test ./build/tests/$@
	@$(CC) $(LDFLAGS) -Lbuild/libs -lnt tests/$@.o -o build/tests/$@
	@./build/tests/$@ > /dev/null

${EXAMPLES}: lib ${EXAMPLE_OBJS}
	@echo ${EXAMPLE_OBJS}
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBSDIR) -Lbuild/libs -lnt -levent examples/$@.o -o build/examples/$@

${DIRS}:
	@mkdir $@

.s.o:
	$(CC) -c $(SFLAGS) $< -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

build/libs/libnt.a: ${LIB_OBJS}
	$(LD) -static -o build/libs/libnt.a $(LDFLAGS) ${LIB_OBJS}

build/libs/libnt.dylib: ${LIB_OBJS}
	$(LD) -dynamic -o build/libs/libnt.dylib $(LDFLAGS) $(LIBSDIR) $(LD_DYLIB_FLAGS) $(LIBS) ${LIB_OBJS}

clean:
	rm -rf ${DIRS} src/*.o tests/*.o

