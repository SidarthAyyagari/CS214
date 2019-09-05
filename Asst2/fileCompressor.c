#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "fileCompressor.h"

fnode_t* fhead = NULL;
fnode_t* fptr = NULL;

int is_it_a_directory(const char *path){
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

void listdir(const char *name){

  DIR *dir;
  struct dirent *item;
  dir = opendir(name);

  /* check if it is null if not */
  if (!dir){
    return;
  }

  /* loop thru items in directory */
  while ((item = readdir(dir)) != NULL) {
    // if item is a directory
    if (item->d_type == DT_DIR) {
      char path[1024];
      // each directory has "." which refers to itself and ".." which refers to the parent directory
      // we don't care to do anything with these 2 items.
      if (strcmp(item->d_name, ".") == 0 || strcmp(item->d_name, "..") == 0){
        continue;
      }
    
      // get path of subdirectory
      /*  e.g. current_directory/sub_directory */
      snprintf(path, sizeof(path), "%s/%s", name, item->d_name);
    
      // call list dir on the subdirectory
      listdir(path);
    }
    
    /* (leaf node) means it is not a directory. it is some file */
    else{
      
      char path[1024];
    
      // get path of file
      /*  e.g. current_directory/sub_directory */
      snprintf(path, sizeof(path), "%s/%s", name, item->d_name);
     
      /////////////////////////////////////////                 
      // insert node into global linked list //                            
      /////////////////////////////////////////

      //tbi = to be inserted
      //this is the node we will insert into the global linked list                    
      fnode_t *ftbi = malloc(sizeof(fnode_t));                                       
      //check if malloc was done properly
      if(ftbi == NULL){
        printf("ERROR: in using malloc... Apologies.\n");
        return;
      }      
      
      // put path in node                                   
      ftbi->fullpath = malloc(sizeof(char)*strlen(path)); 
      strcpy(ftbi->fullpath, path);
      ftbi->next = NULL;
                    
      // first insert                                                                             
      if(fhead == NULL){
        fhead = ftbi;
      }
     
      //any following inserts
      else{
        fptr = fhead;
        while(fptr->next != NULL){
          fptr= fptr->next;
        }
        fptr->next = ftbi;
      }
    }
  }
   
  //close directory
  closedir(dir);  
}

/*
      fhead = malloc(sizeof(fnode_t));
      strcpy(fhead->fullpath, path);
      fhead->next = NULL;

*/

//takes in file or path and builds codebook based off of the file or path given.
void build(char* path, char* flag){
 
  int length; 
 
  //path: recursively go through path and tokenize all files
  if(!strcmp(flag,"-R")){
    //check if path given is in fact a directory    
    
    int x = is_it_a_directory(path);

    // print warning message and continue
    if(!x){
      printf("WARNING: -R was passed, but the path given is the path of a file.\n If you choose to pass in '-R' make sure the path given is a path to a directory.\n");

      char* arr[1];
      arr[0] = path;
      length = 1;
      hcbBuilder(arr, length);
      return;
    }

    //actual recursion
    else{
      listdir(path);
      
      fptr = fhead;
      int i = 0;
      while(fptr != NULL){
        fptr = fptr->next;
        ++i;
      }
      
      char*arr[i];
      fptr = fhead;
      i = 0;
      while(fptr != NULL){
        arr[i] = fptr->fullpath;
        fptr = fptr->next;
        ++i;
      }
      length = i;
      hcbBuilder(arr, length);
      return;  
    }
  }
  //individual file 
  else{
    char* arr[1];
    arr[0] = path;
    length = 1;
    hcbBuilder(arr, length);
    return;
  }
  return;
}


void compress(char* path, char* codebook, char* flag){

  //VERIFY THAT CODEBOOK IS REAL

  // recursively compress all files 
  if(!strcmp(flag, "-R")){
 
    //check if path given is really a directory
    int x = is_it_a_directory(path);

    if(!x){
      printf("WARNING: '-R' was passed, but the path provided is not the path of a directory.\nIf you choose to pass in '-R' please make sure that the path provided is a path to a directory.\nThe FILE you provided will be compressed.\n");

      // NOTDONE: MAKE SURE TO verify that the path given is a .txt file
      compressor(path, codebook);
      return;
    }

    //Real Recursion time.    
    else{
      listdir(path);

      fptr = fhead;
      while(fptr!=NULL){
        compressor(fptr->fullpath, codebook);
        fptr = fptr->next;
      }   
      return;
    }

  }

  // just compress one file
  else{
    compressor(path, codebook);
  }

  return;
}

void decompress(char* path, char* codebook, char* flag){

  // recursively decompress
  if(!strcmp(flag, "-R")){
    
    int x = is_it_a_directory(path);
  
    if(!x){

      printf("WARNING: '-R' was passed, but the path provided is not the path of a directory.\nIf you choose to pass in '-R' please make sure that the path provided is a path to a directory.\nThe FILE you provided will be decompressed.\n");
      decompressor(path, codebook);
    }

    else{
      listdir(path);
      
      fptr = fhead;
      while(fptr!=NULL){
        decompressor(fptr->fullpath, codebook);
        fptr = fptr->next;
      }
    }

  }

  // just decompress one file
  else{
    decompressor(path, codebook);
  }
}



int main(int argc, char* argv[]){

  /* determine if input is valid */ 
  /*if(strcmp("./fileCompressor", argv[0]) != 0){
    printf("Error1: INVALID INPUT\n");
    return 0;
  }*/

  if(argc < 3 || argc > 5){
    printf("Error: Invalid Input.\nExample Usage: ./fileCompressor -R -b filename.txt\n");
    return 0;
  }
  

  /* check flags */
  if(!strcmp(argv[1], "-R") || !strcmp(argv[1], "-b") || !strcmp(argv[1], "-c") || !strcmp(argv[1], "-d")){
   
    // if -R comes first (recursive)
    if(!strcmp(argv[1], "-R")){

      // recursive build
      if(!strcmp(argv[2], "-b")){
       
        /* gather arguments and check validity */
        if(argc == 4){
          build(argv[3], argv[1]);
          return 1;  
        }

        // invalid input
        else{
          printf("Invalid amount of arguments. Cannot build\n");
          return 0;
        }
  
      }

      //recursively compress
      else if(!strcmp(argv[2], "-c")){

        if(argc == 5){
          // argv[3] = path 
          // argv[4] = codebook
          // argv[1] = -R (recursive flag)
          compress(argv[3], argv[4], argv[1]);
          return 1;
        } 
      
        // invalid input   
        else{
          printf("Invalid amount of arguments. Cannot compress\n");
          return 0;
        }

      }

      //recursive decompress
      else if(!strcmp(argv[2], "-d")){

        if(argc == 5){
          decompress(argv[3], argv[4], argv[1]);
          return 1;
        } 
      
        // invalid input   
        else{
          printf("Invalid amount of arguments. Cannot decompress\n");
          return 0;
        }
   
      } 

      // invalid input
      else{
        printf("Error: INVALID INPUT\n");
        return 0;
      }

    }

    // build
    else if(!strcmp(argv[1], "-b")){

      // check if recursive flag is passed
      if(!strcmp(argv[2], "-R")){

        /* gather arguments and check validity */
        if(argc == 4){
          build(argv[3], argv[2]);
          return 1;  
        }

        // invalid input
        else{
          printf("Invalid amount of arguments. Cannot build\n");
          return 0;
        } 
      }

      else{
        /* check validity of arguments */
    
        if(argc == 3){
          build(argv[2], "-Y");
          return 1;
        }   

        else{
          printf("Invalid amount of arguments. Cannot build \n");
          return 0;
        }
   
      }
    
    }

    // compress was passed
    else if(!strcmp(argv[1], "-c")){ 
      
      // recursive
      if(!strcmp(argv[2], "-R")){
        /* check validity of following arguments */

        if(argc == 5){
          compress(argv[3], argv[4], argv[2]);
          return 1;
        }   

        else{
          printf("Invalid amount of arguments. Cannot build\n");
          return 0;
        } 
      }
     
      // non-recursive
      else{
        if(argc == 4){
          compress(argv[2], argv[3], "-Y");
          return 1;
        }   

        else{
          printf("Invalid amount of arguments. Cannot build\n");
          return 0;
        } 
      }
    
    }
    else if(!strcmp(argv[1], "-d")){ 
    
      // recursive
      if(!strcmp(argv[2], "-R")){
        /* check validity of following arguments */

        if(argc == 5){
          decompress(argv[3], argv[4], argv[2]);
          return 1;
        }   

        else{
          printf("Invalid amount of arguments. Cannot build\n");
          return 0;
        } 
      }
     
      // non-recursive
      else{
        if(argc == 4){
          decompress(argv[2], argv[3], "-Y");
          return 1;
        }   

        else{
          printf("Invalid amount of arguments. Cannot build\n");
          return 1;
        } 
      }
    }

    else{
      printf("Error in reading flags\n");
      return 0;
    }

  }

  /* bad input */
  else{
    printf("error\n");
    return 0;
  } 

  return 0;
}
