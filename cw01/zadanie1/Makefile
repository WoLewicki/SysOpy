# you need to export LD_LIBRARY_PATH to make compiler find .so
library:
	gcc -c library.c -library.o
	ar rcs library.a library.o
	make clean
sharedLibrary: library.o
	gcc -fPIC -Wall -shared -o library.so library.o
	make clean
library.o:
	gcc -fPIC -c -Wall library.c -o library.o
clean:
	rm -f *.o
