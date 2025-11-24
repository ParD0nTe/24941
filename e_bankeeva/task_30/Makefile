all: server client

server: server.c
	gcc -Wall -Wextra -O2 server.c -o server -lsocket -lnsl

client: client.c
	gcc -Wall -Wextra -O2 client.c -o client -lsocket -lnsl

clean:
	rm -f server client
