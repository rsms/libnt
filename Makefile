SOURCES=ex_echo.c src/util.c src/tcp_socket.c src/tcp_server.c
CFLAGS=-c -Wall -DDEBUG=1 -I/opt/local/include
LDFLAGS=-L/opt/local/lib -levent
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=ex_echo

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o ntn/*.o $(EXECUTABLE)
