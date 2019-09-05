all: memgrind

memgrind: memgrind.o mymalloc.o
	gcc -o memgrind memgrind.o mymalloc.o

memgrind.o: memgrind.c
	gcc -c -g memgrind.c

mymalloc.o: mymalloc.c
	gcc -c -g mymalloc.c

clean:
	rm -f *.o memgrind

run: memgrind
	./memgrind 2>memgrind.err
