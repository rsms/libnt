.PHONY: all core

S_SRC=src/atomic_queue.s
C_SRC=src/util.c src/freelist.c src/tcp_socket.c src/event_base.c src/tcp_server.c src/tcp_client.c

S_OBJ_=$(S_SRC:.s=.o)
C_OBJ_=$(C_SRC:.c=.o)

S_OBJ=$(foreach obj,$(S_OBJ_),objs/$(obj))
C_OBJ=$(foreach obj,$(C_OBJ_),objs/$(obj))

OBJECTS=${S_OBJ} ${C_OBJ}
SOURCES=${S_SRC} ${C_SRC}

CC=gcc
CFLAGS+=-Wall -include src/_prefix.h
CFLAGS+=-std=gnu99
CFLAGS+=-DDEBUG=1
CFLAGS += -I/opt/local/include

LIBSDIR += -L/opt/local/lib
LIBS += -levent
LD_DYLIB_FLAGS = -dynamic

DIRS = libs objs/src

all: core

core: ${DIRS} libs/libnt.a libs/libnt.dylib

ex_echo: core libs/libnt.dylib examples/ex_echo.c
	$(CC) $(CFLAGS) $(CPPFLAGS) examples/ex_echo.c -o examples/ex_echo -lnt -Llibs

clean_ex_echo:
	rm -rf ex_echo ex_echo.dSYM

${DIRS}:
	@mkdir $@

$(S_OBJ) : $(S_SRC)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(C_OBJ) : $(C_SRC)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

libs/libnt.a: ${OBJECTS}
	$(LD) $(LDFLAGS) $(LIBSDIR) ${OBJECTS} -static -o libs/libnt.a $(LIBS)

libs/libnt.dylib: ${OBJECTS}
	$(LD) $(LDFLAGS) $(LIBSDIR) ${OBJECTS} -dynamic -o libs/libnt.dylib $(LIBS) -lgcc_s.1

clean:
	rm -rf ${DIRS}

