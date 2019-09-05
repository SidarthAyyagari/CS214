#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "git.h"



// project builder:
// takes a project name, and a message from server containing all filepaths + their contents
// builds project directory out of them and all subdirectories

int projectBuilder(char* projname, char* message){

  printf("Building '%s' from '%s'\n", projname, message); 
  return 1;

}

int isNumber(char* cand){
 
  int i;
  for(i = 0; i < strlen(cand); i++){
    if(!isdigit(cand[i])){
      return -1;
    }
  }

  return 1;
}

//sme = server manifest entry
//version = server manifest version
int manifestComparer(char* sme, int version){



}


