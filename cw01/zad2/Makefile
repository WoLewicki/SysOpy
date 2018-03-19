test: create
	./main --elem 1000 --size 100 --alloc 0 --create --add 500
	./main --elem 1000 --size 100 --alloc 1 --add 500
	./main --elem 3000 --size 200 --alloc 0 --create --add 400
	./main --elem 3000 --size 200 --alloc 1 --add 400
	./main --elem 5000 --size 300 --alloc 0 --create --add 1000
	./main --elem 5000 --size 300 --alloc 1 --add 1000
	./main --elem 7000 --size 400 --alloc 0 --create --add 4000
	./main --elem 7000 --size 400 --alloc 1 --add 4000
	./main --elem 9000 --size 500 --alloc 0 --create --add 230
	./main --elem 9000 --size 500 --alloc 1 --add 230
	make clean
create:
	# skompiluj biblioteke
	gcc -Wall -c library.c -o library.o 
	# stworz statyczna biblioteke.a	
	ar rcs liblibrary.a library.o
	# zlinkuj biblioteke do programu
	gcc -Wall -static main.c -L. -l library -o main
clean:
	rm main
	rm -f *.o
	rm -f *.a
