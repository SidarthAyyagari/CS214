#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "git.h"

int main(int argc, char* argv[]){


  if (argc > 4|| argc < 2){
    printf("INVALID AMOUNT OF ARGUMENTS\n");
    return 1;
  }

  if(!strcmp(argv[1], "configure") && argc == 4){

    //check if configure gets a valid ip and port number
    configure(argv[2], argv[3]);  
    
  }
  else if(!strcmp(argv[1], "checkout") && argc == 3){
    
    checkout(argv[2]);
   // printf("Checkout project name\n");
  }

  else if(!strcmp(argv[1], "update") && argc == 3){
    //connect to server
    update(argv[2]);
    //printf("Update '%s'\n", argv[2]);
  }
  
  else if(!strcmp(argv[1], "upgrade") && argc == 3){
    upgrade(argv[2]);
    //printf("Upgrade '%s'\n", argv[2]);
  }

  else if(!strcmp(argv[1], "commit") && argc == 3){
    commit(argv[2]);
    //printf("Commit '%s'\n", argv[2]);
  }

  else if(!strcmp(argv[1], "push") && argc == 3){
    push(argv[2]);
    //printf("push '%s'\n", argv[2]);
  }

  else if(!strcmp(argv[1], "create") && argc == 3){
    create(argv[2]);
    //printf("Create '%s'\n", argv[2]);
  }

  else if(!strcmp(argv[1], "destroy") && argc == 3){
    destroy(argv[2]);
    //printf("destroy '%s'\n", argv[2]);
  }
  
  else if(!strcmp(argv[1], "add") && argc == 4){
    add(argv[2], argv[3]);
    //printf("add '%s'\n", argv[2]);
  }
  
  else if(!strcmp(argv[1], "remove") && argc == 4){
    remov(argv[2], argv[3]);
    //printf("remove '%s'\n", argv[2]);
  }
  
  else if(!strcmp(argv[1], "currentversion") && argc == 3){
    currentversion(argv[2]);
    //printf("currentversion '%s'\n", argv[2]);
  }

  else if(!strcmp(argv[1], "history") && argc == 3){
    history(argv[2]);
    //printf("history '%s'\n", argv[2]);
  }
  
  else if(!strcmp(argv[1], "rollback") && argc == 4){
    rollback(argv[2], argv[3]);
    //printf("Rollback '%s'\n", argv[2]);
  }
  else{
    printf("ERROR: invalid command\n");
  }

  return 1;

}
