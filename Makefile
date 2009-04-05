.PHONY: all lib

S_SRC=src/atomic_queue.s
C_SRC=src/util.c \
      src/atomic_queue_e.c src/freelist.c \
      src/event_base.c \
      src/tcp_socket.c src/tcp_server.c src/tcp_client.c

S_OBJ = ${S_SRC:.s=.o}
C_OBJ = ${C_SRC:.c=.o}

#OBJECTS = $(foreach fn,$(S_OBJ_) $(C_OBJ_),objs/$(fn))
OBJECTS=${S_OBJ} ${C_OBJ}
SOURCES=${S_SRC} ${C_SRC}

CC=gcc
CFLAGS+=-Wall -include src/_prefix.h
CFLAGS+=-std=gnu99
CFLAGS+=-DDEBUG=1
CFLAGS += -I/opt/local/include

LD=/usr/bin/libtool
LIBS += -levent -lSystem
#LIBS += -lgcc_s.1
LD_DYLIB_FLAGS=-L/opt/local/lib
DIRS = libs objs/src

all: lib

lib: ${DIRS} libs/libnt.a libs/libnt.dylib

ex_echo: lib libs/libnt.dylib examples/ex_echo.c
	$(CC) $(CFLAGS) $(CPPFLAGS) examples/ex_echo.c -o examples/ex_echo -L/opt/local/lib -Llibs -lnt -levent

clean_ex_echo:
	rm -rf ex_echo ex_echo.dSYM

${DIRS}:
	@mkdir $@

.s.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

libs/libnt.a: ${OBJECTS}
	$(LD) -static -o libs/libnt.a $(LDFLAGS) $(LIBSDIR) ${OBJECTS}

libs/libnt.dylib: ${OBJECTS}
	$(LD) -dynamic -o libs/libnt.dylib $(LDFLAGS) $(LIBSDIR) $(LD_DYLIB_FLAGS) $(LIBS) ${OBJECTS}

clean:
	rm -rf ${DIRS} src/*.o

