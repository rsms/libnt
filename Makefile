.PHONY: all lib examples test

S_SRC=src/atomic_queue_asmimpl.s
C_SRC=src/util.c \
      src/atomic_queue.c src/freelist.c \
      src/event_base.c \
      src/tcp_socket.c src/tcp_server.c src/tcp_client.c

S_OBJ = ${S_SRC:.s=.o}
C_OBJ = ${C_SRC:.c=.o}

OBJECTS=${S_OBJ} ${C_OBJ}
#SOURCES=${S_SRC} ${C_SRC}

INCDIR = -I/opt/local/include
LIBSDIR = -L/opt/local/lib

CC=gcc
CFLAGS+=-Wall -include src/_prefix.h
CFLAGS+=-std=gnu99
CFLAGS+=-DDEBUG=1
CFLAGS += -I/opt/local/include
TOOL_CFLAGS = -Lbuild/libs -lnt -levent

LD=/usr/bin/libtool
LIBS += -levent -lSystem
#LIBS += -lgcc_s.1
LD_DYLIB_FLAGS = 
DIRS = build build/libs build/examples build/tests

all: lib test

lib: ${DIRS} build/libs/libnt.a build/libs/libnt.dylib

examples: ex_echo

test: test_atomic_queue

ex_echo: lib build/libs/libnt.dylib examples/ex_echo.c
	$(CC) $(CFLAGS) $(TOOL_CFLAGS) $(LIBSDIR) examples/ex_echo.c -o examples/ex_echo

test_atomic_queue: lib build/libs/libnt.dylib tests/atomic_queue.c
	$(CC) $(CFLAGS) $(TOOL_CFLAGS) $(LIBSDIR) tests/atomic_queue.c -o tests/atomic_queue
	./tests/atomic_queue

${DIRS}:
	@mkdir $@

.s.o:
	$(CC) -c $(CFLAGS) $< -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

build/libs/libnt.a: ${OBJECTS}
	$(LD) -static -o build/libs/libnt.a $(LDFLAGS) ${OBJECTS}

build/libs/libnt.dylib: ${OBJECTS}
	$(LD) -dynamic -o build/libs/libnt.dylib $(LDFLAGS) $(LIBSDIR) $(LD_DYLIB_FLAGS) $(LIBS) ${OBJECTS}

clean:
	rm -rf ${DIRS} src/*.o

