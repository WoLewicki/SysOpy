CC = gcc
CFLAGS = -g -O2 -std=gnu99 -static -Wall -Wextra -Isrc -rdynamic -fomit-frame-pointer

all: clean client server
	./server &
	./client input.txt &
	./client input.txt &
	./client input.txt &
	./client input.txt &
	./client input.txt &
	./client input.txt &
	./client endinginput.txt

client:
	$(CC) $(CFLAGS) client.c -o client -lrt

server:
	$(CC) $(CFLAGS) server.c -o server -lrt


clean:
	rm -f client
	rm -f server
