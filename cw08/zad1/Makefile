main:
	gcc -Wall -o main main.c -lpthread -lm
	./main 1 input1 filter output1 >> Times.txt
	./main 2 input1 filter output1 >> Times.txt
	./main 4 input1 filter output1 >> Times.txt
	./main 8 input1 filter output1 >> Times.txt
	./main 1 input1 filter25 output1 >> Times.txt
	./main 2 input1 filter25 output1 >> Times.txt
	./main 4 input1 filter25 output1 >> Times.txt
	./main 8 input1 filter25 output1 >> Times.txt
	./main 1 input1 filter39 output1 >> Times.txt
	./main 2 input1 filter39 output1 >> Times.txt
	./main 4 input1 filter39 output1 >> Times.txt
	./main 8 input1 filter39 output1 >> Times.txt
	./main 1 input2 filter output2 >> Times.txt
	./main 2 input2 filter output2 >> Times.txt
	./main 4 input2 filter output2 >> Times.txt
	./main 8 input2 filter output2 >> Times.txt
	./main 1 input2 filter25 output2 >> Times.txt
	./main 2 input2 filter25 output2 >> Times.txt
	./main 4 input2 filter25 output2 >> Times.txt
	./main 8 input2 filter25 output2 >> Times.txt
	./main 1 input2 filter39 output2 >> Times.txt
	./main 2 input2 filter39 output2 >> Times.txt
	./main 4 input2 filter39 output2 >> Times.txt
	./main 8 input2 filter39 output2 >> Times.txt
	make clean
clean:
	rm main
