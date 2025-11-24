all: server client

server: server.c
	gcc -Wall -Wextra -O2 server.c -o server

client: client.c
	gcc -Wall -Wextra -O2 client.c -o client

clean:
	rm -f server client
