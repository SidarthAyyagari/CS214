all: WTF WTFserver

WTF: WTFclient.o clientCommands.o clientHelpers.o
	gcc -o WTF -lcrypto -lssl WTFclient.o clientCommands.o clientHelpers.o

WTFclient.o: WTFclient.c
	gcc -c -g -lcrypto -lssl WTFclient.c

clientCommands.o:
	gcc -c -g -lcrypto -lssl clientCommands.c

clientHelpers.o:
	gcc -c -g -lcrypto -lssl clientHelpers.c

WTFserver: WTFserver.o serverHandler.o
	gcc -o WTFserver WTFserver.o

WTFserver.o:
	gcc -c -g WTFserver.c

serverHandler.o:
	gcc -c -g -lcrypto -lpthread serverHandler.c

clean:
	rm -f *.o WTF WTFserver
