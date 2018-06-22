export PATH := /home/wojlewy/CLionProjects/LewickiWojciech/cw03/zad1:$(PATH)

main:
	# compile test.c in zad1 to make it not fail.
	gcc -Wall -o test1 test1.c
	gcc -Wall -o main main.c
	./main input.txt 2 1000000000
	make clean
	# you can put other commands in input.txt to test:
	# this would fail because of CPU time: ./test1
	# this would fail too: ls +l
	# and if you want to check DATA fail just cut 1 zero in third argument and the fifth command will fail.
clean:
	rm main
	rm test1
