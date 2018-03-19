test: create
	./mainStatic --elem 1000 --size 100 --alloc 0 --create --add 500
	./mainStatic --elem 1000 --size 100 --alloc 1 --add 500
	./mainDynamic --elem 3000 --size 200 --alloc 1 --add 400
	./mainDynamic --elem 3000 --size 200 --alloc 0 --create --add 400
	./mainShared --elem 1000 --size 100 --alloc 1 --add 500
	./mainStatic --elem 3000 --size 200 --alloc 1 --add 400
	./mainStatic --elem 5000 --size 300 --alloc 0 --create --add 1000
	./mainDynamic --elem 5000 --size 300 --alloc 1 --add 1000
	./mainShared --elem 7000 --size 400 --alloc 0 --create --add 4000
	./mainStatic --elem 7000 --size 400 --alloc 1 --add 4000
	./mainDynamic --elem 9000 --size 500 --alloc 0 --create --add 230
	./mainShared --elem 9000 --size 500 --alloc 1 --add 230
	make clean
create: static dynamic shared

static:
	# skompiluj biblioteke
	gcc -Wall -c library.c -o library.o 
	# stworz statyczna biblioteke.a	
	ar rcs liblibrary.a library.o
	# zlinkuj biblioteke do programu
	gcc -Wall -static main.c -L. -l library -o mainStatic

dynamic:
	#zbudowanie wspoldzielonej biblioteki ladowanej dynamicznie
	gcc -Wall -fPIC -c library.c -o library.o
	gcc -Wall -Wl,-rpath=. -fPIC -shared -o liblibrary.so library.o
	#okreslam flage DLL zeby nie dolaczylo library.h
	gcc -Wall -L. -o mainDynamic main.c -ldl -D DLL

shared:
	gcc -Wall -fPIC -c library.c -o library.o
	gcc -Wall -fPIC -shared -o liblibrary.so library.o
	gcc -Wall -Wl,-rpath=. -L. -o mainShared main.c -llibrary

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f mainStatic
	rm -f mainDynamic
	rm -f mainShared
