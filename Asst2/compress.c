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

lnode_t* lroot;
lnode_t* lptr;

void lineParser(char* s){

  /* parse string and get bitencoding and */
   
  char* be = (char*)malloc(sizeof(char)*strlen(s));  //'be' is bit-encoding
  memset(be, '\0', strlen(s));
  char* temp = be; 

  char* asci = (char*)malloc(sizeof(char)*strlen(s)); //'asci value associated w/ bit-encoding'
  memset(asci, '\0', strlen(s));
  char* temp2 = asci;

  int boa = -1; 
  int i;
  int end = strlen(s);
  char* lp = s;

  for(i=0; i<end; i++){
    
    if( (*lp != '\t') && (*lp != '\n') && (boa == -1)){
      *temp = *lp;
      temp++;  
    }
    else if(boa == 1){
      *temp2 = *lp;
      temp2++;
    }
    else{
      //done w/ bitencoding
      if(boa == -1){
        boa = 1;
      }
    }
    lp++;
  }
 
  //insert node into linkedlist
 

  if(lroot == NULL){
    lroot = malloc(sizeof(lnode_t));
    lroot->token = asci;
    lroot->bitencoding = be;
    lroot->next = NULL;
    lptr = lroot;
  }

  else{
    while(lptr->next != NULL){
      lptr = lptr->next;
    }
    lnode_t* tb = malloc(sizeof(lnode_t));
    tb->token = asci;
    tb->bitencoding = be;
    tb->next = NULL;
    lptr->next = tb;
  }
  return;
}

void parseHuffCodebook(char* filename){

    
  struct stat st;
  stat(filename, &st);
  size_t size = st.st_size;

  int fd;

  char *buf = malloc(size);

  ssize_t br;

  fd = open(filename, O_RDONLY);
  br = read(fd, buf, size); 

  char *token = (char*)malloc(sizeof(char)*strlen(buf));
  memset(token, '\0', strlen(buf));
  char* temp = token;
  
  char *lp = buf;
  int i;
  int end = strlen(buf);

  

  int j = 1;  
 

  for(i = 0; i< end; i++){
    // split up by line
    if(*lp != '\n'){
      *temp = *lp;
      temp++;
    }
    else{
      //got a line: now break it up into bitencoding and token-name   
      lineParser(token);        
      //reset token
      token = (char*)malloc(sizeof(char)*strlen(buf));
      memset(token, '\0', strlen(buf));
      temp = token;
      j++;           

    }
    ++lp;
  }

}

void listfreer(lnode_t* p){

  lnode_t* tmp;

  while(p!= NULL){
    tmp = p;
    p = p->next;
    free(tmp);
  }

  return;
}

void compressor(char* filename, char* hcb){

  //////////////////////////
  /* CHECK FILE ARGUMENTS */
  //////////////////////////


  /* check validity of file */
  if(strlen(filename)< 5){
    printf("ERROR: '%s' is an invalid file to compress.\n For a file to be compressed, it should be of the form <filename>.txt\n", filename);
    return;
  }

  int tru = strlen(filename) - 3;
  char* fe = filename + tru;
 
  if(strcmp(fe, "txt")){
    printf("ERROR: '%s' is an invalid file to compress.\n For a file to be compressed, it should be of the form <filename>.txt\n", filename);
    return;
  }

  /* check validity of huffman codebook */
  if(strlen(hcb)<15){ 
    printf("ERROR: '%s' is an invalid HuffmanCodebook.\n", hcb);
    return;
  }

  tru = strlen(hcb) - 15;
  char* huffext = hcb + tru;

  if(strcmp(huffext, "HuffmanCodebook")){ 
    printf("ERROR: '%s' is an invalid HuffmanCodebook.\n", hcb);
    return;
  }


  // this function will build a linked list containing the data provided in the codebook
  parseHuffCodebook(hcb);

  //open up filename (this is the file that we want to compress)  
  struct stat st;
  stat(filename, &st);
  size_t size = st.st_size;

  char *buf = malloc(size);
 
  // read filename
  int fd = open(filename, O_RDONLY);
  ssize_t br = read(fd, buf, size);
  

  // open file to write to
  char* compfilename = (char*)malloc(sizeof(char)*(strlen(filename) + 4));
  memset(compfilename, '\0', strlen(filename) + 4);

  strcpy(compfilename, filename);
  strcat(compfilename, ".hcz");
  int hcz = open(compfilename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  

  // tokenize
  char *token = (char*)malloc(sizeof(char)*strlen(buf));
  memset(token, '\0', strlen(buf));
  char* temp = token;

  
  char *lp = buf;
  int end = strlen(buf);

    
  
  int i;
  //loop thru file u want to compress
  for(i = 0; i<end; i++){
    //delimiter = control character or white space
    if(!(isspace(*lp) || iscntrl(*lp)) ){
      *temp = *lp;
      temp++;
    }

    //reached a token
    else{

      //search for token in linked list
      //int checcer = -1; 
      lptr = lroot;
      while(lptr != NULL){
        if(!strcmp(token,lptr->token)){
          write(hcz, lptr->bitencoding, sizeof(char)*strlen(lptr->bitencoding));
         // checcer = 1;
          break;
        }
        lptr = lptr->next; 
      }      
          

      //reset token
      token = (char*)malloc(sizeof(char)*strlen(buf));
      memset(token, '\0', strlen(buf));
      temp = token;


      /* CATCH DELIMITER */ 
      *token = *lp;

      // search for delimiter token in linked list
      char* jdn;
      if(!strcmp(token, " ")){
        jdn = malloc(sizeof(char)*strlen("<space>"));
        jdn = "<space>";
      }
      else if(!strcmp(token, "\t")){
        jdn = malloc(sizeof(char)*strlen("<tab>"));
        jdn = "<tab>";
      }
      else if(!strcmp(token, "\n")){
        jdn = malloc(sizeof(char)*strlen("<newline>"));
        jdn = "<newline>";
      }

      else{
        //
        printf("Error in compressing... Sorry\n");  
        return;
      }

      
      lptr = lroot;
      while(lptr != NULL){
        if(!strcmp(jdn, lptr->token)){
          write(hcz, lptr->bitencoding, sizeof(char)*strlen(lptr->bitencoding));
          break;
        }
        lptr = lptr->next;
      }

      //reset token
      token = (char*)malloc(sizeof(char)*strlen(buf));
      memset(token, '\0', strlen(buf));
      temp = token;

    }
    lp++; 
  }
  
  write(hcz, "\n", sizeof(char));

  listfreer(lroot); 

  free(buf);
  free(compfilename);
  free(token);

  return;
}
