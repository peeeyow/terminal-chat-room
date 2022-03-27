all: client server
server: server.c lib.o 
	gcc server.c lib.o -o server -g -Wall -w -Werror

client: client.c lib.o
	gcc client.c lib.o -o client -g -Wall -w -Werror

lib.o: lib.c lib.h
	gcc -c lib.c -o lib.o -g -Wall -w -Werror
