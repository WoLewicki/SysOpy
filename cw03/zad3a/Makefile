export PATH := /home/wojlewy/CLionProjects/LewickiWojciech/cw03/zad1:$(PATH)

main:
	# compile test.c in zad1 to make it not fail.
	gcc -Wall -o test1 test1.c
	gcc -Wall -o main main.c
	./main input.txt 2 20
	make clean
clean:
	rm main
	rm test1
