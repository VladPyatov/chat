main: server.c client.c
	gcc -g -Wall server.c -o server
	gcc -g -Wall client.c -o client

