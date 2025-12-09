CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = server client1 client2

all: $(TARGETS)

server: server.c
	$(CC) $(CFLAGS) server.c -o server

client1: client_1.c
	$(CC) $(CFLAGS) client_1.c -o client1

client2: client_2.c
	$(CC) $(CFLAGS) client_2.c -o client2

clean:
	rm -f $(TARGETS)

.PHONY: all clean