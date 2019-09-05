#ifndef GIT_HEADER_FILE
#define GIT_HEADER_FILE

int connectToServer();

int projectBuilder();

int isNumber(char* cand);

//CLIENT COMMANDS
void configure(char* ip, char* port);

void checkout(char* projname);

void update(char* projname);

void upgrade(char* projname);

void commit(char* projname);

void push(char* projname);

void create(char* projname);

void destroy(char* projname);

void add(char* projname, char* filename);

void remov(char* projname, char* filename);

void currentversion(char* projname);

void history(char* projname);

void rollback(char* projname, char* version);


//void update();

#endif
