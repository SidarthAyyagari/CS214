#ifndef MYMALLOC_HEADER_FILE
#define MYMALLOC_HEADER_FILE

#define malloc( x ) mymalloc( x, __FILE__, __LINE__ )
#define free( x ) myfree( x, __FILE__, __LINE__ )

void* mymalloc(size_t size, char* file, int line);
void myfree(void* ptr, char* file, int line);

void defragment();

static char myblock[4096];

#endif
