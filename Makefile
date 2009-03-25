CFLAGS=-c -Wall -DDEBUG=1 -I/opt/local/include
LDFLAGS=-L/opt/local/lib -lev
SOURCES=ex_accept.c ntn/util.c ntn/tcp_socket.c ntn/tcp_server.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=ex_accept

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o ntn/*.o $(EXECUTABLE)
