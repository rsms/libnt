SOURCES=ex_echo.c src/util.c src/tcp_socket.c src/event_base.c src/tcp_server.c src/tcp_client.c
EXECUTABLE=build/ex_echo

CFLAGS=-c -Wall -DDEBUG=1 -I/opt/local/include
LDFLAGS=-L/opt/local/lib -levent
OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	mkdir -p build
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o src/*.o $(EXECUTABLE)
