#ifndef FILECOMPRESSOR_HEADER_FILE
#define FILECOMPRESSOR_HEADER_FILE

//////////////////////
/* filecompressor.c */
//////////////////////

typedef struct fileNode{
  
  char* fullpath;
  struct fileNode* next;
  
}fnode_t;

int is_it_a_directory(const char* path);

void listdir(const char* name);

void build(char* path, char* flag);

void compress(char* path, char* codebookpath, char* flag);

void decompress(char* path, char* codebookpath, char* flag);



/////////////
/* build.c */
/////////////

//int fdfd;

typedef struct tokenNode{
  char* name;
  int count;
  char* bitencoding;
  struct tokenNode *next;
}tnode_t;

//huffman tree
typedef struct treenode{
  int count;
  char* name;
  struct treenode* left;
  struct treenode* right;
}btnode_t;


typedef struct minheap{
  int size;
  int capacity;
  btnode_t** arr;
}minheap_t;

//extern tnode_t* thead;
//extern tnode_t *tptr;
//extern tnode_t *tprev;


void fileTokenizer(char* filename);

//merge sort
tnode_t* sortedMerge(tnode_t *a, tnode_t *b);
void fbsplit(tnode_t* source, tnode_t** front, tnode_t** back);
void mergesort(tnode_t** headRef);


btnode_t* newNode(char* d, int f);
minheap_t* createMinHeap(int c);
void swapmhnode(btnode_t** a, btnode_t** b);
void heapify(minheap_t* mh, int x);
int issizeone(minheap_t* mh);
btnode_t* extractmin(minheap_t* mh);
void insertMinHeap(minheap_t* mh, btnode_t* mhn);
void buildMinHeap(minheap_t* mh);
int isLeaf(btnode_t* r);
minheap_t* createAndBuildMinHeap(char* data[], int freq[], int size);
btnode_t* buildHuffmanTree(char* data[], int freq[], int size);
void writeCodes(btnode_t* r, int arr[], int top);
void deallocate(btnode_t* h);
void HuffmanCodes(char* data[], int freq[], int size);
int is_it_txt_file(char* filenaam);
void hcbBuilder(char* filenames[], int fnl);



////////////////
/* compress.c */
////////////////

typedef struct llNode{
  char* token;
  char* bitencoding;
  struct llNode *next;
}lnode_t;

void lineParser(char* s);
void parseHuffCodebook(char* filename);
void compressor(char* filename, char* hcb);



//////////////////
/* decompress.c */
//////////////////

typedef struct huffNode{
  char* token;
  char* bitencoding;
  struct huffNode *left;
  struct huffNode *right;
}hnode_t;

void insertHuffNode(char* b, char* a);
void lineBreaker(char* s);
void bookToTree(char* filename);
void decompressor(char* filename, char* hcb);



#endif
