#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include "fileCompressor.h"

/* Global Variables */
int fdfd;


tnode_t *thead = NULL;
tnode_t *tptr = NULL;
tnode_t *tprev = NULL;

void fileTokenizer(char* filename){
 
  //CHECK IF FILE IS .txt
    

  struct stat st;
  stat(filename, &st);
  size_t size = st.st_size;

  int fd;
  char* buf = (char*)malloc(size);
  
  ssize_t br;

  //mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

  fd = open(filename, O_RDONLY);
  br = read(fd, buf, size);    

  // buf has the whole file saved as a string
 
  char *token = (char*)malloc(sizeof(char)*strlen(buf));
  memset(token, '\0', strlen(buf));
  char* temp = token;  

  char *lp = buf;
  int i;
  int end = strlen(buf);
  
  int predicatef = 0;
  //loop thru whole file
  for(i = 0; i<end; i++){
    //delimiter = control character or white space
    if(!(isspace(*lp) || iscntrl(*lp)) ){ 
      *temp = *lp;
      temp++;
    }

    else{
      
      // CATCH THE TOKEN! 
      // u hit a whitespace/control character. Save the token 
      // and the whitespace/ctrl char as a token
      if(token[0] != '\0'){

        /* CATCH ACTUAL TOKEN */
         
        // first token
        if(thead == NULL){
          thead = malloc(sizeof(tnode_t));
          thead->name = token;
          thead->count = 1;
          thead->next = NULL;
          tprev = thead;
        }    

        // rest of tokens
        else{
          tptr = thead;
          // see if we are dealing with a predicate felon token :)
          while(tptr != NULL){
            if(!strcmp(tptr->name, token)){
              tptr->count = 1 + tptr->count;
              predicatef = 1;
              break;
            }
            tptr = tptr->next;
          }

          // not a predicate felon.. add it to the end of the linked list
          if(!predicatef == 1){
            tnode_t *ttbi = malloc(sizeof(tnode_t));
            ttbi->name = token;
            ttbi->count = 1;
            tprev->next = ttbi;
            ttbi->next = NULL;
            tprev = ttbi;
          }
        }
   
        //reset token
        token = (char*)malloc(sizeof(char)*strlen(buf));
        memset(token, '\0', strlen(buf));
        temp = token;
        predicatef = 0;


        /* CATCH DELIMITER */
        *token = *lp;
        
        // first token
        if(thead == NULL){
          thead = malloc(sizeof(tnode_t));
          thead->name = token;
          thead->count = 1;
          thead->next = NULL;
          tprev = thead;
        }    

        // rest of tokens
        else{
          tptr = thead;
          // see if we are dealing with a predicate felon token :)
          while(tptr != NULL){
            if(!strcmp(tptr->name, token)){
              tptr->count = 1 + tptr->count;
              predicatef = 1;
              break;
            }
            tptr = tptr->next;
          }

          // not a predicate felon.. add it to the end of the linked list
          if(!predicatef == 1){
            tnode_t *ttbi = malloc(sizeof(tnode_t));
            ttbi->name = token;
            ttbi->count = 1;
            tprev->next = ttbi;
            ttbi->next = NULL;
            tprev = ttbi;
          }
        } 
        //reset token
        token = (char*)malloc(sizeof(char)*strlen(buf));
        memset(token, '\0', strlen(buf));
        temp = token;
        predicatef = 0;
      }
    }
    lp++; 
  }


  //last token, IF NECCESSARY:
  if(token[0] != '\0'){
    tptr = thead;
  
    while(tptr!=NULL){
      if(!strcmp(tptr->name, token)){
        tptr->count = 1 + tptr->count;
        predicatef = 1;
        break;
      }
    }
    if(!predicatef == 1){
      tnode_t *ttba =  malloc(sizeof(tnode_t));
      ttba->name = token;
      ttba->count = 1;
      ttba->next = NULL;
      if (tprev!= NULL){
        tprev->next = ttba;
        tprev = ttba;
      }
      else{
        thead = ttba;
      }
    }
  }

  free(buf);
  free(token);
}

/* -------------------------------- */


/* merge sort helper function */
tnode_t* sortedMerge(tnode_t *a, tnode_t *b){

  tnode_t* result = NULL;

  /* base cases */

  if(a == NULL){
    return b;
  }

  else if(b == NULL){
    return a;
  }


  if (a->count <= b->count){
    result = a;
    result->next = sortedMerge(a->next, b);
  }
  else{
    result = b;
    result->next = sortedMerge(a, b->next);
  }

  return result;
}


/* split linked list into two */
void fbsplit(tnode_t* source, tnode_t** front, tnode_t** back){

  tnode_t* fast;
  tnode_t* slow;

  slow = source;
  fast = source->next;


  while(fast != NULL){

    fast = fast->next;

    if(fast != NULL){
      slow = slow->next;
      fast = fast->next;
    }
  }

  *front = source;
  *back = slow->next;
  slow->next = NULL;

  return;
}


/* merge sort function */
void mergesort(tnode_t** headRef){
  tnode_t* head = *headRef;
  tnode_t* a;
  tnode_t* b;


  /* if linked list length = 0 or 1 */
  if((head == NULL) || (head->next == NULL)){
    return;
  }

  /* Split head into 'a' and 'b' sublists */
  fbsplit(head, &a, &b);

  /* Recursively sort the sublists */
  mergesort(&a);
  mergesort(&b);

  /* Put it back together*/
  *headRef = sortedMerge(a,b);
  
  return;
}
/* -------------------------------- */

btnode_t* newNode(char* d, int f){

  btnode_t* temp = malloc(sizeof(btnode_t));//(btnode_t*)malloc(sizeof(btnode_t))
  temp->left = NULL;
  temp->right = NULL;
  temp->name = d;
  temp->count = f;
  
  return temp;
}

minheap_t* createMinHeap(int c){

  minheap_t* mh = malloc(sizeof(minheap_t));
  mh->size = 0;
  mh->capacity = c ;
  
  mh->arr = malloc(sizeof(btnode_t)*c);
  return mh;
}

void swapmhnode (btnode_t** a, btnode_t** b){

  btnode_t* temp = *a;
  *a = *b;
  *b = temp;

}

void heapify(minheap_t* mh, int x){

  int smallest = x;
  int left = 2 * x +1;
  int right = 2 * x +2;

  if (left < mh->size && mh->arr[left]->count < mh->arr[smallest]->count){
    smallest = left;
  }

  if (right < mh->size && mh->arr[right]->count < mh->arr[smallest]->count){
    smallest = right;
  }

  if (smallest != x) {
    swapmhnode(&mh->arr[smallest],&mh->arr[x]);
    heapify(mh, smallest);
  }
}

int issizeone(minheap_t* mh){
  return (mh->size == 1);
}

btnode_t* extractmin(minheap_t* mh){
  btnode_t* temp = mh->arr[0];
  mh->arr[0] = mh->arr[mh->size-1];
  
  --mh->size;
  heapify(mh, 0);

  return temp; 
}

void insertMinHeap(minheap_t* mh, btnode_t* mhn){
  
  ++mh->size;
  int i = mh->size-1;

  while(i && mhn->count < mh->arr[(i-1)/2]->count){
    mh->arr[i] = mh->arr[(i-1)/2];
    i = (i-1)/2;
  }

  mh->arr[i] = mhn;
}

void buildMinHeap(minheap_t* mh){

  int n = mh->size - 1;
  int i;
 
  for(i = (n-1)/2;i >=0; --i){
    heapify(mh, i);
  }
}


int isLeaf(btnode_t* r){
  return !(r->left) && !(r->right);
}

minheap_t* createAndBuildMinHeap(char* data[], int freq[], int size){

        minheap_t* mh = createMinHeap(size);
        int i;
        for (i = 0; i < size; ++i)
                mh->arr[i] = newNode(data[i], freq[i]);

        mh->size = size;
        buildMinHeap(mh);

        return mh;
}

btnode_t * buildHuffmanTree(char* data[], int freq[], int size){

  btnode_t *left, *right, *top;
  minheap_t *mh = createAndBuildMinHeap(data, freq, size);

  while(!issizeone(mh)){
    left = extractmin(mh);
    right = extractmin(mh);
    top = newNode("", left->count + right->count);

    top->left = left;
    top->right = right;

    insertMinHeap(mh, top);
  }

  return extractmin(mh);
}

void writeCodes(btnode_t* r, int arr[], int top){

  if (r->left){
    arr[top] = 0;
    writeCodes(r->left, arr, top+1);
  }

  if (r->right){
    arr[top] = 1;
    writeCodes(r->right, arr, top+1);
  }
 
  if(isLeaf(r)){
    int i;
    for(i = 0; i <top; ++i){
      if(arr[i] == 0){
        write(fdfd, "0", sizeof(char));
      }
      else if(arr[i] == 1){
        write(fdfd, "1", sizeof(char));
      }

      // should never get here
      else{
        write(fdfd, "$", sizeof(char));
      }
    }

    write(fdfd, "\t", sizeof(char));
    
    if(!strcmp(r->name, "\n")){
      write(fdfd, "<newline>", sizeof(char)*strlen("<newline>"));
    }
    else if(!strcmp(r->name, "\t")){
      write(fdfd, "<tab>", sizeof(char)*strlen("<tab>"));
    }
    else if(!strcmp(r->name, " ")){
      write(fdfd, "<space>", sizeof(char)*strlen("<space>")); 
    }
    else{
      write(fdfd, r->name, sizeof(char)*strlen(r->name));
    }
  
    write(fdfd, "\n", sizeof(char));
  }
}
  //  printArr(arr,top);
/*  void printArr(int arr[], int n)

        int i;
        for (i = 0; i < n; ++i)
                printf("%d", arr[i]);

        printf("\n");
*/

void deallocate(btnode_t* h){

  if(h == NULL){
    return;
  }
  
  deallocate(h->left);
  deallocate(h->right);

  free(h);
 
}


void HuffmanCodes(char* data[], int freq[], int size){
  
  btnode_t* r = buildHuffmanTree(data, freq, size);

  int arr[100], top = 0;

  writeCodes(r, arr, top);
  deallocate(r);
}

// checks if file is good or not
int is_it_txt_file(char* filenaam){

  // not long enough for a .txt file
  if(strlen(filenaam) < 5){
    return 0;
  }

  int tru = strlen(filenaam) - 3;
  char* fe = filenaam + tru;

  if(strcmp(fe, "txt")){
    return 0;
  }

  return 1;
}

/* -------------------------------- */


/* BUILD: takes in a fileNode that should point to the head of the linked list of files.
 *        Loops through linked list of files, and tokenizes each, building a linked list 
 *        of tokens with frequencies. Then mergesort is called, which sorts the token
 *        linked list in ascending order of frequency. This new list of tokens is then
 *        turned into a min heap, then a binary tree, which is then used to generate 
 *        huffman codes for every token. During the building of the huffman tree, each
 *        token is written to a file along with its bitencoding. The file is called 
 *        HuffmanCodebook and it will be present in the current working directory.  
 * */
void hcbBuilder(char* filenames[], int fnl){

/*
  fptr = fhead;
  while(fptr!= NULL){
    fileTokenizer(fptr->fullpath);
  }
*/
  
  int i;
  for(i = 0; i < fnl; ++i){
    if(is_it_txt_file(filenames[i])){
      fileTokenizer(filenames[i]);
    }
    else{
      printf("'%s' is not a .txt file. This file will not be used to build codebook\n", filenames[i]);
    }
  }


  if(thead == NULL){
    return;
  }
  
  mergesort(&thead);
  tptr = thead; 

  i = 0;
  while(tptr != NULL){
    tptr = tptr->next;
    ++i; 
  }
 
  tptr = thead;
  
  char* tokennames[i];
  int tokencounts[i];

  int abc = 0;
  while(tptr != NULL){
    tokennames[abc] =tptr->name;
    tokencounts[abc] = tptr->count;
    abc++;
    tptr = tptr->next; 
  }

  //free linked list
  
  fdfd = open("./HuffmanCodebook", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
 
  HuffmanCodes(tokennames, tokencounts, i);

  return;
}
