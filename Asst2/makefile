all: fileCompressor

fileCompressor: fileCompressor.o build.o compress.o decompress.o
	gcc -o fileCompressor fileCompressor.o build.o compress.o decompress.o

fileCompressor.o: fileCompressor.c
	gcc -c -g fileCompressor.c

build.o:
	gcc -c -g build.c

compress.o:
	gcc -c -g compress.c

decompress.o:
	gcc -c -g decompress.c

clean:
	rm -f *.o fileCompressor
