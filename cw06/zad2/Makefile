CFLAGS = -g -O2 -std=gnu99 -static -Wall -Wextra -Isrc -rdynamic -fomit-frame-pointer

main:
	gcc $(CFLAGS) -o server server.c -lrt
	gcc $(CFLAGS) -o client client.c -lrt
	./server &
	./client input.txt
clean:
	rm server
	rm client	
