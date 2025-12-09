CC = gcc

CFLAGS = -Wall -Wextra -g -std=c99

TARGET = servak client_1

all: $(TARGET)

server: server.c
	$(CC) $(CFLAGS) -o servak servak.c

client: client.c
	$(CC) $(CFLAGS) -o client_1 client_1.c

clean:
	rm -f $(TARGET)

.PHONY: all clean