/////////////
//  SERVER  //
//////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include "server.h"

int connectToClients(int port){


  int server_fd, new_socket, valread;

  struct sockaddr_in address;

  int opt = 1;
  int addrlen = sizeof(address);
 
  if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
    printf("ERROR: could not create socket\n");
    return -1;
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
    printf("ERROR: setsockopt()\n");
    return -1;
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  
  //bind
  if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) <0){
    printf("ERROR: bind()\n");
    return -1;
  }

  printf("Server socket created\n");
  // listen  ------ (queue is at max length 3)
  if(listen(server_fd, 3) < 0){
    printf("ERROR: listen()\n");
    return -1;
  }

  printf("listening\n");

  //turn this to while loop
  if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen))<0){
    printf("ERROR: accept\n");
    return -1;
  }

  printf("client accepted\n");


  //send and receive
  int jkl = clientConnector(new_socket);

  if(jkl < 0){
    printf("could not connect\n");
    return;
  }


  //////
  int c = close(new_socket);

  if(c <0){
    printf("new_socket not closed properly");
    return;
  } 

  //close socket
 
  c = close(server_fd);


  return 1;
}


int main(int argc, char *argv[]){

  if(argc != 2){
    printf("INVALID INPUT\n");
    return -1;
  }

  char* portstring = argv[1];

  int i;
  for(i = 0; i <strlen(portstring); i++){
    if(!isdigit(portstring[i])){
      printf("invalid PORT number\n");
      return;
    } 
  }

  int port = atoi(portstring);

  int diditwork = connectToClients(port);


  if(diditwork<0){
    return -1;
  }   


  return 0;

}


