all:
	gcc -o a server.c -lsocket -lnsl
	gcc -o b client.c -lsocket -lnsl
	gcc -o c client2.c -lsocket -lnsl

run_server:
	./a

run_client:
	./b

run_client2:
	./c