test: create
	#
	#
	#testy dla rozmiaru rekordu 4 i liczby rekordow 100
	./main generate dane 100 4
	./main copy dane kopia 40 10 sys
	./main copy dane kopia 40 10 lib
	./main sort dane 100 4 sys
	./main sort kopia 100 4 lib
	#
	#
	#testy dla rozmiaru rekordu 4 i liczby rekordow 500
	./main generate dane 500 4
	./main copy dane kopia 200 10 sys
	./main copy dane kopia 200 10 lib
	./main sort dane 500 4 sys
	./main sort kopia 500 4 lib
	#
	#
	#testy dla rozmiaru rekordu 512 i liczby rekordow 100
	./main generate dane 100 512
	./main copy dane kopia 5120 10 sys
	./main copy dane kopia 5120 10 lib
	./main sort dane 100 512 sys
	./main sort kopia 100 512 lib
	#
	#
	#testy dla rozmiaru rekordu 512 i liczby rekordow 500
	./main generate dane 500 512
	./main copy dane kopia 2560 10 sys
	./main copy dane kopia 2560 10 lib
	./main sort dane 500 512 sys
	./main sort kopia 500 512 lib
	#
	#
	#testy dla rozmiaru rekordu 4096 i liczby rekordow 100
	./main generate dane 100 4096
	./main copy dane kopia 40960 10 sys
	./main copy dane kopia 40960 10 lib
	./main sort dane 100 4096 sys
	./main sort kopia 100 4096 lib
	#
	#
	#testy dla rozmiaru rekordu 4096 i liczby rekordow 500
	./main generate dane 500 4096
	./main copy dane kopia 204800 10 sys
	./main copy dane kopia 204800 10 lib
	./main sort dane 500 4096 sys
	./main sort kopia 500 4096 lib
	#
	#
	#testy dla rozmiaru rekordu 8192 i liczby rekordow 100
	./main generate dane 100 8192
	./main copy dane kopia 81920 10 sys
	./main copy dane kopia 81920 10 lib
	./main sort dane 100 8192 sys
	./main sort kopia 100 8192 lib
	#
	#
	#testy dla rozmiaru rekordu 8192 i liczby rekordow 5000
	./main generate dane 5000 8192
	./main copy dane kopia 4096000 10 sys
	./main copy dane kopia 4096000 10 lib
	./main sort dane 5000 8192 sys
	./main sort kopia 5000 8192 lib
	#
	#
	#testy dla rozmiaru rekordu 8192 i liczby rekordow 50
	./main generate dane 50 8192
	./main copy dane kopia 40960 10 sys
	./main copy dane kopia 4096000 10 lib
	./main sort dane 50 8192 sys
	./main sort kopia 50 8192 lib

	make clean
create:
	# skompiluj biblioteke
	gcc -Wall -c operations.c -o operations.o 
	# stworz statyczna biblioteke.a	
	ar rcs liboperations.a operations.o
	# zlinkuj biblioteke do programu
	gcc -Wall -static main.c -L. -l operations -o main
clean:
	rm main
	rm -f *.o
	rm -f *.a
