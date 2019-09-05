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

hnode_t* hroot = NULL;
hnode_t* hptr = NULL;
hnode_t* hptr2 = NULL;

void insertHuffNode(char* b, char* a){

 
  // loop through bitencoding 
  char* lp = b;
  int end = strlen(b);
  int i;


  if(hroot == NULL){
    hroot = malloc(sizeof(hnode_t));
    hroot->left = NULL;
    hroot->right = NULL;
    hroot->token = NULL;
    hroot->bitencoding = NULL;
  }

  int lr = -1;
  hptr = hroot;

  for(i = 0; i < end; i++){
    hptr2 = hptr;
    // go left in the tree
    if(*lp == '0'){
      hptr = hptr->left;

      if(hptr == NULL){
        hnode_t *tbi = malloc(sizeof(hnode_t));
     
        tbi->left = NULL;
        tbi->right = NULL;
        tbi->token = NULL;
        tbi->bitencoding = NULL;
        
        hptr = tbi;
        hptr2->left = hptr;
        lr = 0; 
      }

    }
    // go right in the tree
    else if(*lp == '1'){
      hptr = hptr->right;
 
      if(hptr == NULL){
        hnode_t *tbii = malloc(sizeof(hnode_t));
      
        tbii->left = NULL;
        tbii->right = NULL;
        tbii->token = NULL;
        tbii->bitencoding = NULL;
      
        lr = 1;
        hptr = tbii;
        hptr2->right = hptr;
      }
    }
 
    else if(*lp == '\0'){
      break;
    }  
    //something wrong with the file
    else{
      printf("Found this character: '%c' \n\t Error: invalid bitencoding. Huffman Codebook might not be valid\n", *lp);
      return;
    }
    ++lp;
  }

  hnode_t* tbi = malloc(sizeof(hnode_t));
 
  tbi->token = a;
  tbi->bitencoding = b;
  tbi->left = NULL;
  tbi->right = NULL;



  //ptr should be at the position in the huffman tree where we want to insert
 
  hptr = tbi;

  if(hptr != hroot){

    if(lr == 0){
      hptr2->left = hptr;
    }
    else if(lr == 1){
      hptr2->right = hptr;
    }
    else{
      //printf("Error.. could not decompress\n");
    }
  }
  
  return; 
}


void lineBreaker(char* s){

  /* parse string and get bitencoding and */  

  char* be = (char*)malloc(sizeof(char)*strlen(s));  //'be' is bit-encoding
  memset(be, '\0', strlen(s));
  char* temp = be; 
  char* lp = s;

  char* asci = (char*)malloc(sizeof(char)*strlen(s)); //'asci value associated w/ bit-encoding'
  memset(asci, '\0', strlen(s));
  char* temp2 = asci;

  int boa = -1; 
  int i;
  int end = strlen(s);


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

   
   
  
  insertHuffNode(be, asci);
  return;
}

void bookToTree(char* filename){

    
  struct stat st;
  stat(filename, &st);
  off_t size = st.st_size;

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

   
 

  for(i = 0; i< end; i++){
    // split up by line
    if(*lp != '\n'){
      *temp = *lp;
      temp++;
    }
    else{

      //got a line: now break it up into bitencoding and token-name   
      lineBreaker(token);        
          
      //reset token
      token = (char*)malloc(sizeof(char)*strlen(buf));
      memset(token, '\0', strlen(buf));
      temp = token;

    }
    ++lp;
  }
}

//decompress()
void decompressor(char* filename, char* hcb){ 

 /* check validity of file */
 if(strlen(filename)< 5){
   printf("ERROR: '%s' is an invalid file to decompress.\n Decompressed files should be of the form <filename>.hcz\n");
   return;
 }

 int tru = strlen(filename) - 3;
 char* fe = filename + tru;
 
 if(strcmp(fe, "hcz")){
   printf("ERROR: '%s' is an invalid file to decompress.\n Decompressed files should be of the form <filename>.hcz\n");  
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
   printf("ERROR: '%s' is an invalid HuffmanCodebook.\n");
   return;
 }

  // this function will build a huffman tree based off of the given codebook
  bookToTree(hcb);


  //open up compressed file for reading  
  struct stat st;
  stat(filename, &st);
  off_t size = st.st_size;

  char *buf = malloc(size);
 
  int fd = open(filename, O_RDONLY);
  ssize_t br = read(fd, buf, size);
 

  // open file to write to
  
  // make the name of the decompressed file: <filename>.txt
  char* decompfilename = (char*)malloc(sizeof(char)*(strlen(filename)-4));
  memset(decompfilename, '\0', strlen(filename));

  strncpy(decompfilename, filename, strlen(filename)-4);
  int dcz = open(decompfilename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);


  //read from compressed file and writer to decompressed file 
  char *token = (char*)malloc(sizeof(char)*strlen(buf));
  memset(token, '\0', strlen(buf));
  char* temp = token;
  
  char *lp = buf;
  
  int end = strlen(buf);
  hptr = hroot;

  //loop thru whole file
  int i;
  for(i = 0; i<end; i++){

    if(*lp == '0'){
      hptr = hptr->left;
      // is it a leaf
      if((hptr!= NULL) && (hptr->left == NULL) && (hptr->right == NULL)){
        // check if it is a delimiter
        if(!strcmp(hptr->token, "<space>")){
          write(dcz, " ", sizeof(char)*strlen(" "));
        }
        else if(!strcmp(hptr->token, "<tab>")){
          write(dcz, "\t", sizeof(char)*strlen("\t"));
        }    
        else if(!strcmp(hptr->token, "<newline>")){
          write(dcz, "\n", sizeof(char)*strlen("\n"));
        }
        // not a delimiter... business as usual    
        else{
          write(dcz, hptr->token, sizeof(char)*strlen(hptr->token));
        }
        hptr = hroot;
      }
  
    }

    else if(*lp == '1'){   

      hptr = hptr->right;
      
      // is it a leaf
      if((hptr!= NULL) &&(hptr->left == NULL) && (hptr->right == NULL)){
        // check if it is a delimiter   
   
        if(!strcmp(hptr->token, "<space>")){
          write(dcz, " ", sizeof(char)*strlen(" "));
        }
        else if(!strcmp(hptr->token, "<tab>")){
          write(dcz, "\t", sizeof(char)*strlen("\t"));
        }    
        else if(!strcmp(hptr->token, "<newline>")){
          write(dcz, "\n", sizeof(char)*strlen("\n"));
        }
        // not a delimiter... business as usual    
        else{
          write(dcz, hptr->token, sizeof(char)*strlen(hptr->token));
        }
        hptr = hroot;
      }  
    }
    else{
      return;
    }
    lp++;
  }
  write(dcz, "\n", sizeof(char)*strlen("\n"));

  

  return;
}

