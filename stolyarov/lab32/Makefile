CC = gcc

CFLAGS = -Wall -Wextra -g -std=c99

TARGET = server client

all: $(TARGET)

server: servak.c
	$(CC) $(CFLAGS) -o serv servak.c

client: client_1.c
	$(CC) $(CFLAGS) -o cli1 client_1.c

client: client_2.c
	$(CC) $(CFLAGS) -o cli2 client_2.c

clean:
	rm -f $(TARGET)

.PHONY: all clean