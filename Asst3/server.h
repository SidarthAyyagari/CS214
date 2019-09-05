#ifndef GIT_HEADER_FILE
#define GIT_HEADER_FILE

int connectToClients(int port);

int clientConnector(int sock);

int requestParser(int socc, char* r);

int checkout(int socket, char* projname);

int update(int socket, char* projname);

int upgrade(int socket, char* projname;

int commit(int socket, char* projname);

int push(int socket, char* projname);

int create(int socket, char* projname);

int destroy(int socket, char* projname);

int currentversion(int socket, char* projname);

int history(int socket, char* projname);

int rollback(int socket, char* projname, char* version);

#endif
