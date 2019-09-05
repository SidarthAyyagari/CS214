///////////////////////////
//                       //
//    CLIENT COMMANDS    //
//                       //
///////////////////////////



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <openssl/sha.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include "git.h"


int sock; //socket file descriptor





// Attempts to connect to server. If a connection cannot be established, returns -1;
int connectToServer(){

  int fd = open("./.configure", O_RDONLY);

  if(fd < 0){
    printf(".configure file was not written. Please try again to configure.\n");
  }

  ssize_t br;
  char* buf = (char*)malloc(sizeof(char));

  char* token = (char*)malloc(sizeof(char)*20);
  memset(token, '\0', strlen(token));
  char* temp = token;

  char ip[16];  //MAX LENGTH FOR IP_ADDRESS
  memset(ip, '\0', strlen(ip));
  char portstring[6]; //MAX LENGTH FOR PORT NUMBER
  memset(portstring, '\0', strlen(portstring));

  int x = -1;

  while(br = read(fd, buf, sizeof(char))){
    if(*buf != '\n'){
      *temp = *buf;
      ++temp;
    }
    else{

      if(x == -1){
        snprintf(ip, 16, "%s", token);
        x = 1;
      }
      else if(x == 1){
        snprintf(portstring, 6, "%s", token);
      }

      free(token);
      token = (char*)malloc(sizeof(char)*20);
      memset(token, '\0', strlen(token));
      temp = token;

    }
  }

  free(buf);
  free(token);


  //validate port
  int i;
  for(i = 0; i< strlen(portstring); i++){
    if(!isdigit(portstring[i])){
      printf("Invalid port number in .configure file\n");
      return;
    }
  }

  int port = atoi(portstring); 
  
  printf("ip: '%s'\nport: '%d'\n", ip, port);

  /// START TALKING TO SERVER
  // try to connect

  struct sockaddr_in address;
  sock = 0;
  struct sockaddr_in server_address;

  if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    printf("CONNECTION ERROR: Socket creation error.\n");
    return -1;
  }
 
  memset(&server_address, '0', sizeof(server_address));
 
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);

  if(inet_pton(AF_INET,ip, &server_address.sin_addr)<=0){
    printf("CONNECTION ERROR: Invalid ip address/IP address not supported.\n");
    return -1;
  }

  if(connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0){
    printf("ERRNO = '%d'\n", errno);
    printf("CONNECTION ERROR: Could not connect to server address.\n");
    return -1;
  }

  int wr = write(sock, "Hi!", strlen("Hi!"));  

  if(wr < 0){
    printf("CONNECTION ERROR: Could not communicate with server.\n");
    return -1;
  }
 
  printf("message sent to server...\n");

  char *buffer = (char*)malloc(sizeof(char)*strlen("Whatchu need?"));
  memset(buffer, '\0', strlen("Whatchu need?"));
  wr = read(sock, buffer, strlen("Whatchu need?"));


  if(wr < 0){
    printf("CONNECTION ERROR: Could not communicate with server.\n");
    return -1;
  }


  printf("FROM SERVER: '%s'\n", buffer);

  return 1;
}






/////////////////
/// CONFIGURE ///
/////////////////

//(getbyhostname)

/* Validates and writes ip address and port to a file
 * prints error message and returns on fail*/
void configure(char* ip, char* port){

  //get host by name

  if(strlen(ip) > 16){
    printf("invalid IP address\n");
    return;
  }

  int i;
  for(i = 0; i <strlen(ip); i++){
    if(!isdigit(ip[i]) && ip[i] != '.'){
      printf("invalid IP address\n");
      return; 
    }
  }
 

  //maybe this is inaccurate 
  if(strlen(port) > 5){
    printf("invalid PORT number\n");
    return;
  }

  for(i = 0; i <strlen(port); i++){
    if(!isdigit(port[i])){
      printf("invalid PORT number\n");
      return;
    }
  }

  int p = atoi(port);

  if(p < 8000 || p > 64000){
    printf("Invalid PORT number\n");
    return;
  }

  int configfd = open("./.configure", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); 

  if(configfd < 0){
    printf("ERROR: could not create configure file\n");
    return;
  }

  ssize_t wr = write(configfd, ip, sizeof(char)*strlen(ip));

  if(wr < 0){
    printf("ERROR: could not write ip to configure file\n");
    return;
  }  
  
  wr = write(configfd, "\n", sizeof(char));
  
  if(wr < 0){
    printf("ERROR: could not write to configure file\n");
    return;
  }  

  wr = write(configfd, port, sizeof(char)*strlen(port));

  if(wr < 0){
    printf("ERROR: could not write port to configure file\n");
    return;
  } 

  wr = write(configfd, "\n", sizeof(char));

  printf(".configure file has been written!\n");
  return;
}



//////////////////
///  CHECKOUT  ///
//////////////////

void checkout(char* projname){

  int wr;
  //printf("checkout: '%s'\n", projname);

  //first check if project exists on client side
 
  DIR *p = opendir(projname);
  if(p != NULL){
    printf("Project already exists on client side. Cannot checkout!\n");
    return;
  }

  int diditwork = connectToServer();

  if(diditwork == -1){
    printf("Cannot checkout '%s' ... Failed to connect to server\n", projname);
    return;
  }


  //send how many bytes the message is, so the server knows how much to read
  int x = strlen(projname)+9;

  //int to string
  char* numofbytes;
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30);
  sprintf(numofbytes, "%d", x);
  
  // write how many bytes u are about to send to the server
  wr = write(sock, numofbytes, sizeof(char)*strlen(numofbytes));

  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  free(numofbytes);

  // see if the server is ready to catch the real message ur about to give it
  char* servmess = (char*)malloc(sizeof(char)*strlen("lemme have it"));
  wr = read(sock, servmess, sizeof(char)*strlen("lemme have it"));
 
  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 

  if(strcmp(servmess, "lemme have it")){
    printf("Error in communicating w/ server.. Cannot checkout '%s'\n", projname);
    return; 
  }

  free(servmess);

  //write down the message that we (the client) want to send to the server to request a project
  char* message = (char*)malloc(sizeof(char)*x);
  memset(message, '\0', strlen(message));
  snprintf(message, sizeof(message), "checkout:%s",projname);
 

  //send checkout request message
  wr = write(sock, message, sizeof(char)*strlen(message));

  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  free(message);
  // read number of bytes to read for real message
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30); 
  wr = read(sock, numofbytes, sizeof(char)*30);

  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 
 
  int i; 
  for(i = 0; i <strlen(numofbytes); i++){
    if(!isdigit(numofbytes[i])){
      printf("Error in reading bytes from server...\n");
      return;
    }
  } 
 
  int size = atoi(numofbytes); 

  // we are going to read the server's checkout message in this 
  char* checkoutmess = (char*)malloc(sizeof(char)*size);
  memset(checkoutmess, '\0', strlen(checkoutmess));

  wr = write(sock, "gimme", sizeof(char)*strlen("gimme"));
  
  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  wr = read(sock, checkoutmess, sizeof(char)*size);

  // if server sends back "project does not exist"
  if(!strcmp(checkoutmess, "project does not exist")){
    printf("Project: '%s' not found on the server. Sorry!\n", projname);
    return;
  }

  //otherwise you have all of the filepaths and their contents in "checkoutmess"
  // ORRRRRR...... you have the tarred file containing the whole directory structure, just untar it.

  char* tgzf = (char*)malloc(sizeof(char)*(strlen(projname)+7));
  memset(tgzf, '\0', strlen(tgzf));
 
  snprintf(tgzf, sizeof(tgzf), "./%s.tgz", projname);

  int nptfd = open(tgzf, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  wr = write(nptfd, checkoutmess,  size);

  //now untar proj file

  char* syscal = (char*)malloc(sizeof(char)*(strlen(tgzf)+10));
  memset(syscal, '\0', strlen(syscal));
  snprintf(syscal, sizeof(syscal), "tar -xzvf %s", tgzf); 

  system(syscal);

  remove(tgzf);
 
  printf("Project '%s' checked out successfully!!\n", projname);

  //close socket, free stuff etc.
  free(syscal);
  free(tgzf);
  free(checkoutmess);  
  free(numofbytes);
  close(nptfd);
  close(sock);

  return;
}

//////////////
/// UPDATE ///
//////////////

void update(char* projname){

  // 1. connect to server
  // 2. send how many bytes request message is: "update:<projname>"
  // 3. read back "lemme have it"
  // 4. send "update:<projname>"
  // 5. read how many bytes their message will be
  // 6. save it and send "gimme"
  // 7. read servmess (this is the.Manifest file)
  // 8. compute hashes for all files we have and write it to it's own.Manifest
  // 9. for each file in the client.Manifest, search through server.Manifest and try to find it
  // 10a. if u cant find it, then compare.Manifest versions
  // 	- if client and server both have same.Manifest version, write U with the filepath, file version, and hash
  //	- if client and server have different.Manifest versions, 
  // 9. for each file we have, search through.Manifest and see if you can find it
  // 10. if u do then compare the hashes
  // 11. go through the 
  //

  int wr;
  
  int diditwork = connectToServer();

  if(diditwork == -1){
   printf("Cannot update '%s' ... Failed to connect to server\n", projname);
   return;
  }
  
  //connected
  
  //send how many bytes the message is, so the server knows how much to read
  int x = strlen(projname)+7; 

  //int to string
  char* numofbytes;
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30);
  sprintf(numofbytes, "%d", x);
  
  // write how many bytes u are about to send to the server
  wr = write(sock, numofbytes, sizeof(char)*strlen(numofbytes));
 
  if(wr<0){
    printf("Error writing to server\n");
    free(numofbytes);
    return;
  } 

  // see if the server is ready to catch the real message ur about to give it
  char* servmess = (char*)malloc(sizeof(char)*strlen("lemme have it"));
  wr = read(sock, servmess, sizeof(char)*strlen("lemme have it"));
 
  if(wr<0){
    printf("Error reading from server\n");
    free(servmess);
    return;
  } 

  if(strcmp(servmess, "lemme have it")){
    printf("Error in communicating w/ server.. Cannot update '%s'\n", projname);
    free(servmess);
    return; 
  }

  free(servmess);

  //write down the message that we (the client) want to send to the server to request a project
  char* message = (char*)malloc(sizeof(char)*x);
  memset(message, '\0', strlen(message));

  snprintf(message, sizeof(message), "update:%s", projname);

  wr = write(sock, message, sizeof(char)*strlen(message));
 
  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  // read how many bytes the message is going to be 
  memset(numofbytes, '\0', sizeof(char)*30);
  wr = read(sock, numofbytes, sizeof(char)*strlen(numofbytes));

    
  if(wr<0){
    printf("Error reading from server\n");
    free(numofbytes);
    return;
  }
  
  // 
  int i; 
  for(i = 0; i <strlen(numofbytes); i++){
    if(!isdigit(numofbytes[i])){
      printf("Error in reading bytes from server...\n");
      return;
    }
  } 
   
  x = atoi(numofbytes); //potential bug
  servmess = (char*)malloc(sizeof(char)*x); 
  memset(servmess, '\0', sizeof(char)*strlen(servmess));

  wr = write(sock,"gimme", strlen("gimme")*sizeof(char));
 
  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  wr = read(sock, servmess, x*sizeof(char)); 


  if(strcmp(servmess, "lemme have it")){
    printf("Error in communicating w/ server.. Cannot checkout '%s'\n", projname);
    return; 
  }

  //servmess looks like .Manifest:<filecontents>"

  if(!(servmess[0] == 'm' && servmess[1] == 'a' && servmess[2] == 'n' && servmess[3] == 'i'&& servmess[4] == 'f' && servmess[5] == 'e' && servmess[6] == 's' && servmess[7] == 't')){
    printf("Could not retrieve project.Manifest from server\n");
    return;
  }


  // open up client.Manifest file

  char* manipath = (char*)malloc(sizeof(char)*(strlen(projname)+13));
  memset(manipath, '\0', strlen(manipath));
  snprintf(manipath, sizeof(manipath), "./%s/.Manifest", projname);

  int cmfd = open(manipath, O_RDONLY);
  free(manipath); 

  if(cmfd < 0){
    printf("Could not open up '%s'.\n", manipath);
    close(cmfd);
    return;
  }

  //strip.Manifest tag from server's message containing.Manifest file
  char* servcontents = servmess + 9;

  // check server.Manifest version
  char* version = (char*)malloc(sizeof(char)*30);
  memset(version, '\0', 30);

  char* lp = servcontents;

  i = 0;
  while(isdigit(*lp)){
    version[i] = *lp;
    lp++;
    i++;
  }

  //version number of server.Manifest
  int v = atoi(version);
  free(version);


  // Loop thru the server's.Manifest file line by line, and compare with client.Manifest
  servcontents = servcontents + i +1;   // skip over.Manifest version number at the top
  char* lp2 = servcontents;

  char* token = (char*)malloc(sizeof(char)*1024);
  memset(token, '\0', strlen(token));
  char* temp = token; 

  for(i = 0; i <strlen(servcontents); i++){
    if(*lp2 != '\n'){
      *temp = *lp2;
      temp++;
    }
    else{

      //x = ManifestCompare(token, v);
      x = -1;
     
      if(x < 0){
        printf("Error in comparing.Manifest files\n");
    
        free(token);
        free(servmess);     
        return;
      }      

      free(token);
      token = (char*)malloc(sizeof(char)*strlen(servcontents));
      memset(token, '\0', strlen(servcontents));
      temp = token;
    }
    lp2++;
  }
  
  free(servmess);
  printf("Update performed successfully!\n"); 
  return;
} 



void upgrade(char* projname){

  //first check if the .Update file is present

  DIR *p = opendir(projname);

  if(p == NULL){
    printf("Project: '%s' does not exist. Cannot perform upgrade.\n", projname);
    return;
  }  

  char* ufp = (char*)malloc(sizeof(char)*(strlen(projname)+11));
  memset(ufp, '\0', strlen(ufp));
  snprintf(ufp, sizeof(ufp), "./%s/.Update", projname);
   
  int ufd = open(ufp, O_RDONLY);

  if(ufd < 0){
    printf(".Update file does not exist in project: '%s' directory. Cannot perform upgrade.\n");
    free(ufp);
    return;
  } 

int wr;

  // connect to server 
  int diditwork = connectToServer();
 

  if(diditwork == -1){
    printf("Cannot perform upgrade on '%s' ... Failed to connect to server\n", projname);
    //close socket
    return;
  }


  //send how many bytes the message is, so the server knows how much to read
  int x = strlen(projname)+9;

  //int to string
  char* numofbytes;
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30);
  sprintf(numofbytes, "%d", x);
  
  // write how many bytes u are about to send to the server
  wr = write(sock, numofbytes, sizeof(char)*strlen(numofbytes));

  if(wr<0){
    free(numofbytes);
    printf("Error writing to server\n");
    return;
  } 

  free(numofbytes);

  // see if the server is ready to catch the real message ur about to give it
  char* servmess = (char*)malloc(sizeof(char)*strlen("lemme have it"));
  wr = read(sock, servmess, sizeof(char)*strlen("lemme have it"));
 
  if(wr<0){
    free(servmess);
    printf("Error reading from server\n");
    return;
  } 

  if(strcmp(servmess, "lemme have it")){
    free(servmess);
    printf("Error in communicating w/ server.. Cannot upgrade '%s'\n", projname);
    return; 
  }

  free(servmess);

  //write down the message that we (the client) want to send to the server to request a project
  char* message = (char*)malloc(sizeof(char)*x);
  memset(message, '\0', strlen(message));
  snprintf(message, sizeof(message), "upgrade:%s",projname);
 

  //send upgrade request message
  wr = write(sock, message, sizeof(char)*strlen(message));

  if(wr<0){
    free(message);
    printf("Error writing to server\n");
    return;
  } 

  free(message);
  // read number of bytes to read for real message
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30); 
  wr = read(sock, numofbytes, sizeof(char)*30);

  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 
 
  int i; 
  for(i = 0; i <strlen(numofbytes); i++){
    if(!isdigit(numofbytes[i])){
      free(numofbytes);
      printf("Error in reading bytes from server...\n");
      return;
    }
  } 
 
  int y = atoi(numofbytes); 

  // we are going to read the server's upgrademessage in this 
  char* upgmess = (char*)malloc(sizeof(char)*y);
  memset(upgmess, '\0', strlen(upgmess));

  wr = write(sock, "gimme", sizeof(char)*strlen("gimme"));
  
  if(wr<0){
    free(upgmess);
    printf("Error writing to server\n");
    return;
  } 


  if(!strcmp(upgmess, "project does not exist")){
    free(upgmess);
    printf("Project: '%s' not found on the server. Sorry!\n");
    return;
  }

 
  //upgmess says go for it
  

  //applyUpdate(); 

  return;
}

// fail if project name does not exist on the server
// fail if server could not be contacted
// fail if client cannot fetch server's.Manifest file
// .Update file is not empty
// 
//
//
//
// 1. check .Update
// 2. connect to server 
// 3. fetch server's.Manifest
// 4. client should check if both the.Manifest versions are the same
// 5. if they match, the client should run thru its own.Manifest and compute hashcodes for each file
// 6. if a hashcode is different, then write it to .Commit and increment version number
// *7. files that are in both.Manifests: if the server's version is higher than the client's version issa fail
// 8. if it goes through, then client should end up with a commit

void commit(char* projname){
  
  //check if update file exists or is empty

  char* updatepath = (char*)malloc(sizeof(char)*(strlen(projname)+11));
  snprintf(updatepath, sizeof(updatepath), "./%s/.Update");

  int updfd = open(updatepath, O_RDONLY);    

  int wr;
  // .Update file exists
  if(!(updfd < 0)){
     
    struct stat st;
    stat(updatepath, &st);
    int size = st.st_size;

    char* buf = (char*)malloc(size);

    wr = read(updfd , buf,size);

    // .Update file is non-empty
    if(strcmp(buf, "")){
 
      printf(".Update file is not empty. Please apply all updates before requesting a commit.\n");
      free(buf);
      free(updatepath);
      return;
    }

    free(buf);

  }


  //connect to server

  int diditwork = connectToServer();

  if(diditwork == -1){
   
    printf("Cannot commit '%s' ... Failed to connect to server\n", projname);
    return;
  }


  //send how many bytes the message is, so the server knows how much to read
  int x = strlen(projname)+9;

  //int to string
  char* numofbytes;
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30);
  sprintf(numofbytes, "%d", x);
  
  // write how many bytes u are about to send to the server
  wr = write(sock, numofbytes, sizeof(char)*strlen(numofbytes));

  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  free(numofbytes);

  // see if the server is ready to catch the real message ur about to give it
  char* servmess = (char*)malloc(sizeof(char)*strlen("lemme have it"));
  wr = read(sock, servmess, sizeof(char)*strlen("lemme have it"));
 
  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 

  if(strcmp(servmess, "lemme have it")){
    printf("Error in communicating w/ server.. Cannot perform commit on '%s'\n", projname);
    return; 
  }

  free(servmess);

  //write down the message that we (the client) want to send to the server to request a project
  char* message = (char*)malloc(sizeof(char)*x);
  memset(message, '\0', strlen(message));
  snprintf(message, sizeof(message), "commit:%s",projname);
 

  //send checkout request message
  wr = write(sock, message, sizeof(char)*strlen(message));

  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  free(message);
  // read number of bytes to read for real message
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30); 
  wr = read(sock, numofbytes, sizeof(char)*30);

  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 
 
  int i; 
  for(i = 0; i <strlen(numofbytes); i++){
    if(!isdigit(numofbytes[i])){
      printf("Error in reading bytes from server...\n");
      return;
    }
  } 
 
  int y = atoi(numofbytes); 

  // we are going to read the server's message in this 
  char* commitmess = (char*)malloc(sizeof(char)*y);
  memset(commitmess, '\0', strlen(commitmess));

  wr = write(sock, "gimme", sizeof(char)*strlen("gimme"));
  
  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  wr = read(sock, commitmess, sizeof(char)*strlen(commitmess));

  if(wr < 0){

    printf("Error reading from server.\n");
    free(commitmess);
    return;

  }

  if(!strcmp(commitmess, "project does not exist")){

    printf("Project: '%s' does not exist in server's repository.\n", projname);
    free(commitmess);
    return; 
  }


  return;
}


////////////
////////////
/// PUSH ///
////////////
////////////


// 1. check .Update 
// connect to server
// 2. client sends request: "create:<projectname>"
// 3. server sees if project exists
// 4. if it does, then server sends back "project alredy exists".
// 5. if not, then server does it's thing and sends back a blank.Manifest file for it. 
// 6. client disconnects
// 7. client makes a directory <projname> and puts the.Manifest in it

void push(char* projname){
  int wr;
  printf("UNFINISHED: push!!\n");
  int diditwork = connectToServer();

  if(diditwork == -1){
    printf("Cannot checkout '%s' ... Failed to connect to server\n", projname);
    return;
  }


  //send how many bytes the message is, so the server knows how much to read
  int x = strlen(projname)+9;

  //int to string
  char* numofbytes;
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30);
  sprintf(numofbytes, "%d", x);
  
  // write how many bytes u are about to send to the server
  wr = write(sock, numofbytes, sizeof(char)*strlen(numofbytes));

  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  free(numofbytes);

  // see if the server is ready to catch the real message ur about to give it
  char* servmess = (char*)malloc(sizeof(char)*strlen("lemme have it"));
  wr = read(sock, servmess, sizeof(char)*strlen("lemme have it"));
 
  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 

  if(strcmp(servmess, "lemme have it")){
    printf("Error in communicating w/ server.. Cannot checkout '%s'\n", projname);
    return; 
  }

  free(servmess);

  //write down the message that we (the client) want to send to the server to request a project
  char* message = (char*)malloc(sizeof(char)*x);
  memset(message, '\0', strlen(message));
  snprintf(message, sizeof(message), "checkout:%s",projname);
 

  //send checkout request message
  wr = write(sock, message, sizeof(char)*strlen(message));

  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  free(message);
  // read number of bytes to read for real message
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30); 
  wr = read(sock, numofbytes, sizeof(char)*30);

  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 
 
  int i; 
  for(i = 0; i <strlen(numofbytes); i++){
    if(!isdigit(numofbytes[i])){
      printf("Error in reading bytes from server...\n");
      return;
    }
  } 
 
  int y = atoi(numofbytes); 

  // we are going to read the server's checkout message in this 
  char* checkoutmess = (char*)malloc(sizeof(char)*y);
  memset(checkoutmess, '\0', strlen(checkoutmess));

  wr = write(sock, "gimme", sizeof(char)*strlen("gimme"));
  
  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  return;
}



//////////////
//////////////
/// CREATE ///
//////////////
//////////////


// 1. connect to server
// 2. client sends request: "create:<projectname>"
// 3. server sees if project exists
// 4. if it does, then server sends back "project alredy exists".
// 5. if not, then server does it's thing and sends back a blank.Manifest file for it. 
// 6. client disconnects
// 7. client makes a directory <projname> and puts the.Manifest in it

void create(char* projname){
//  printf("UNFINISHED: create!!\n");

  int wr;
  int diditwork = connectToServer();

  if(diditwork < 0){
    printf("ERROR: cannot connect to server... failed to create project: '%s'\n", projname);
    return;
  }


  //connected

  int x = strlen(projname)+strlen("create:"); 

  //int to string
  char* numofbytes;
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30);
  sprintf(numofbytes, "%d", x);
  
  // write how many bytes u are about to send to the server
  wr = write(sock, numofbytes, sizeof(char)*strlen(numofbytes));
  
  free(numofbytes);
 
  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  // see if the server is ready to catch the real message ur about to give it
  char* servmess = (char*)malloc(sizeof(char)*strlen("lemme have it"));
  wr = read(sock, servmess, sizeof(char)*strlen("lemme have it"));
 
  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 

  if(strcmp(servmess, "lemme have it")){
    printf("Error in communicating w/ server.. Cannot update '%s'\n", projname);
    return; 
  }

  free(servmess);

  //write down the message that we (the client) want to send to the server to request that they create a project
  char* message = (char*)malloc(sizeof(char)*x);
  memset(message, '\0', strlen(message));
  snprintf(message, sizeof(message), "create:%s",projname);
 

  //send checkout request message
  wr = write(sock, message, sizeof(char)*strlen(message));

  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  free(message);

  // read number of bytes to read for real message
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30); 
  wr = read(sock, numofbytes, sizeof(char)*30);

  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 

  //before using atoi 
  int i; 
  for(i = 0; i <strlen(numofbytes); i++){
    if(!isdigit(numofbytes[i])){
      printf("Error in reading bytes from server...\n");
      return;
    }
  } 
 
  int y = atoi(numofbytes); 

  // we are going to read the server's checkout message in this 
  char* createmess = (char*)malloc(sizeof(char)*y);
  memset(createmess, '\0', strlen(createmess));

  wr = write(sock, "gimme", sizeof(char)*strlen("gimme"));

  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 

  wr = read(sock, createmess, strlen(createmess)*sizeof(char));


  //FAIL CASE
  if(!strcmp(createmess, "project already exists")){

    printf("Project: '%s' already exists on server.\n");
    return; 

  }

  //error
  else if(!strcmp(createmess, "error")){

    printf("ERROR: we dont know what happened\n");
    return;

  }

  // success
  else{
 
    //check if project already exists on client side

    //

    //create project folder
    //char* dirpath = malloc(sizeof(char)*(strlen(projname)+2)
    //int newdirfd = mkdir
    return;

    //read tar file containing.Manifest
    

    //untar file and put it in the new project folder


  }

  return;
}



///////////////
///////////////
/// DESTROY ///
///////////////
///////////////


// 1. connect to server
// 2. client sends request: "destroy:<projectname>"
// 3. server sees if project exists
// 4. if not, then it sends back to client "project does not exist". 
// 5. if it does exist in the server's repos, then the server does it's thing
// 6. server should send back a success message
// 7. client closes socket, free's memory and returns

void destroy(char* projname){
  printf("UNFINISHED: destroy!!\n");
  int diditwork = connectToServer();

  int wr;

  if(diditwork == -1){
    printf("Cannot checkout '%s' ... Failed to connect to server\n", projname);
    return;
  }


  //send how many bytes the message is, so the server knows how much to read
  int x = strlen(projname)+9;

  //int to string
  char* numofbytes;
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30);
  sprintf(numofbytes, "%d", x);
  
  // write how many bytes u are about to send to the server
  wr = write(sock, numofbytes, sizeof(char)*strlen(numofbytes));

  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  free(numofbytes);

  // see if the server is ready to catch the real message ur about to give it
  char* servmess = (char*)malloc(sizeof(char)*strlen("lemme have it"));
  wr = read(sock, servmess, sizeof(char)*strlen("lemme have it"));
 
  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 

  if(strcmp(servmess, "lemme have it")){
    printf("Error in communicating w/ server.. Cannot checkout '%s'\n", projname);
    return; 
  }

  free(servmess);

  //write down the message that we (the client) want to send to the server to request a project
  char* message = (char*)malloc(sizeof(char)*x);
  memset(message, '\0', strlen(message));
  snprintf(message, sizeof(message), "checkout:%s",projname);
 

  //send checkout request message
  wr = write(sock, message, sizeof(char)*strlen(message));

  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  free(message);
  // read number of bytes to read for real message
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30); 
  wr = read(sock, numofbytes, sizeof(char)*30);

  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 
 
  int i; 
  for(i = 0; i <strlen(numofbytes); i++){
    if(!isdigit(numofbytes[i])){
      printf("Error in reading bytes from server...\n");
      return;
    }
  } 
 
  int y = atoi(numofbytes); 

  // we are going to read the server's checkout message in this 
  char* checkoutmess = (char*)malloc(sizeof(char)*y);
  memset(checkoutmess, '\0', strlen(checkoutmess));

  wr = write(sock, "gimme", sizeof(char)*strlen("gimme"));
  
  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  return;
}



/////////////
/////////////
///  ADD  ///
/////////////
/////////////


// 1. check if project exists
// 2. see if file exists
// 3. compute hash for file
// 4. look in.Manifest if the file entry is there
// 5. if it is then replace hash with new hash (if they are the same, makes no dif)
// 6. if not, then add it to the end
// 7. print success message  

void add(char* projname, char* filepath){


  DIR *p = opendir(projname);
  
  if(p == NULL){
    printf("Project: '%s' does not exist. Cannot add file '%s'!\n", projname, filepath);
    return;
  }

  closedir(p);

  int nufilefd = open(filepath, O_RDONLY);

  // if file does not exist or if they did not give path
  if(nufilefd < 0){
    printf("File '%s' not found. Please provide full path of file\n");
    return;
  }

  //compute hash of file

  struct stat st;
  stat(filepath, &st);
  int size = st.st_size;

  char* buf = (char*)malloc(size);

  int wr = read(nufilefd, buf, size);

  //now whole file's contents is in buf

  close(nufilefd);

  unsigned char dirtyhash[SHA_DIGEST_LENGTH];

  SHA1(buf, size, dirtyhash);

  free(buf); 

  char hash[SHA_DIGEST_LENGTH*2];

  int i;
  for(i = 0; i<SHA_DIGEST_LENGTH; i++){
    sprintf((char*)&(hash[i*2]), "%02x", dirtyhash[i]);
  }
 
  // write hash to.Manifest file
  // 1.  check if.Manifest file already exists, or if this is the first add
 
  char* manipath = (char*)malloc(sizeof(char)*(strlen(projname)+13));
  memset(manipath, '\0', strlen(manipath));
  snprintf(manipath, sizeof(manipath), "./%s/.Manifest", projname);
  
  int manifd = open(manipath, O_APPEND | O_RDWR, S_IRUSR | S_IWUSR);
  free(manipath);
 
  //.Manifest does not exist
  if(manifd < 0){

    close(manifd);
    printf(".Manifest file created!\n");
    manifd = open(manipath, O_CREAT |O_RDWR, S_IRUSR | S_IWUSR);
    wr = write(manifd, "1\n", sizeof(char)*strlen("1\n"));
    wr = write(manifd, filepath, sizeof(char)*strlen(filepath));
    wr = write(manifd, "\t", sizeof(char)*strlen("\t"));
    wr = write(manifd, hash, strlen(hash)*sizeof(char));
    wr = write(manifd, "\t", sizeof(char)*strlen("\t"));
    wr = write(manifd, "1\n", sizeof(char));
    close(manifd);
    //free(manipath);
    return;
  }  
  
  //.Manifest does exist
  // 	search through the file to see if <filepath> was already added 

  close(manifd);
  manifd = open(manipath, O_RDWR);

  int br;
  buf = malloc(sizeof(char));
  char* token = (char*)malloc(sizeof(char)*1024);
  memset(token, '\0', strlen(token));
  char* temp = token;

  while(br = read(manifd, buf, sizeof(char))){

    if(*buf != '\n' && *buf != '\t'){
      *temp = *buf;
      ++temp;
    }

    else{

      if(!strcmp(token, filepath)){
        write(manifd, hash, 40);
        printf("File already added to .Manifest. Hash has been updated\n");
        close(manifd);
        free(buf);
        free(token);
        return;     
      }
      
      free(token);
      token = (char*)malloc(sizeof(char)*1024);
      memset(token, '\0', 1024);
      temp = token;
    }
     
  }

  //if it makes it here, then this is a valid add.
  write(manifd, filepath, strlen(filepath)*sizeof(char));
  write(manifd, "\t", sizeof(char));
  write(manifd, hash, 40); 
  write(manifd, "\t1\n", strlen("\t1\n")*sizeof(char));  
  

  return;


}



//////////////
//////////////
/// REMOVE ///
//////////////
//////////////


// 1. create blank n.Manifest 
// 2. go through.Manifest and try to find the entry the user wants to remove
// 3. If u find it, just don't write it to the new file
// 4. at the end print success message

void remov(char* projname, char* filename){

  char* manipath = (char*)malloc(sizeof(char)*(strlen(projname)+14));
  memset(manipath,'\0', strlen(manipath));
  snprintf(manipath, sizeof(manipath),"./%s/.Manifest", projname);

  int fd = open(manipath, O_RDWR);

  if(fd< 0){
    printf("Could not find.Manifest file... Cannot remove '%s' from '%s'\n", filename, projname);
    return;
  }  

  struct stat st;
  stat(manipath, &st);
  int size = st.st_size;

  char* buf = (char*)malloc(sizeof(char));
  char* token = (char*)malloc(sizeof(char)*1024);
  memset(token, '\0', 1024);
  char* temp = token;

  int foundit = -1;
  int founditcount = -1;
  int realfoundit = -1;

  int br;

  
  char* numanipath = (char*)malloc(sizeof(char)*(strlen(projname)+17));
  memset(numanipath,'\0', strlen(numanipath));
  snprintf(numanipath, sizeof(numanipath),"./%s/.nu.Manifest", projname);

  int nufd = open(numanipath, O_CREAT| O_RDWR, S_IRUSR | S_IWUSR);

  while(br = read(fd, buf, sizeof(char))){

    if(*buf != '\n' && *buf != '\t'){
      *temp = *buf;
      ++temp;
    }
    else{

      printf("%s\n", token);
      if(!strcmp(token, filename)){
        foundit = 1;
        founditcount = 0;
         realfoundit = 1;
      }
      if(founditcount != -1){
        founditcount++;
        if(founditcount == 4){
          foundit = -1;
          founditcount = -1;
        }

      }
      if(foundit == -1){
        write(nufd, token, sizeof(char)*strlen(token));
        write(nufd, buf, sizeof(char));
      }


      free(token);
      token = (char*)malloc(sizeof(char)*1024);
      memset(token, '\0', 1024);
      temp = token;
   }

  }


  close(fd);
  close(nufd);

  remove(manipath);
  rename(numanipath, manipath);

  if(realfoundit<0){
    printf("File '%s' not found in project '%s'. Nothing to remove..\n");
    return;
  }

  printf("File '%s' removed from project '%s' .Manifest file.\n");

  return;
}


////////////////////////
////////////////////////
/// CURRENT VERSION  ///
////////////////////////
////////////////////////


// 1. connect to server
// 2. client request message: "currentversion:<projname>"
// 3. server sends back file containing all files in project with version number
// 4. client disconnects
// 5. client prints file

void currentversion(char* projname){
  int wr;
  printf("UNFINISHED: currentversion!!\n");
  int diditwork = connectToServer();

  if(diditwork == -1){
    printf("Cannot checkout '%s' ... Failed to connect to server\n", projname);
    return;
  }


  //send how many bytes the message is, so the server knows how much to read
  int x = strlen(projname)+9;

  //int to string
  char* numofbytes;
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30);
  sprintf(numofbytes, "%d", x);
  
  // write how many bytes u are about to send to the server
  wr = write(sock, numofbytes, sizeof(char)*strlen(numofbytes));

  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  free(numofbytes);

  // see if the server is ready to catch the real message ur about to give it
  char* servmess = (char*)malloc(sizeof(char)*strlen("lemme have it"));
  wr = read(sock, servmess, sizeof(char)*strlen("lemme have it"));
 
  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 

  if(strcmp(servmess, "lemme have it")){
    printf("Error in communicating w/ server.. Cannot checkout '%s'\n", projname);
    return; 
  }

  free(servmess);

  //write down the message that we (the client) want to send to the server to request a project
  char* message = (char*)malloc(sizeof(char)*x);
  memset(message, '\0', strlen(message));
  snprintf(message, sizeof(message), "checkout:%s",projname);
 

  //send checkout request message
  wr = write(sock, message, sizeof(char)*strlen(message));

  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  free(message);
  // read number of bytes to read for real message
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30); 
  wr = read(sock, numofbytes, sizeof(char)*30);

  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 
 
  int i; 
  for(i = 0; i <strlen(numofbytes); i++){
    if(!isdigit(numofbytes[i])){
      printf("Error in reading bytes from server...\n");
      return;
    }
  } 
 
  int y = atoi(numofbytes); 

  // we are going to read the server's checkout message in this 
  char* checkoutmess = (char*)malloc(sizeof(char)*y);
  memset(checkoutmess, '\0', strlen(checkoutmess));

  wr = write(sock, "gimme", sizeof(char)*strlen("gimme"));
  
  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  return;
}



////////////////
////////////////
/// HISTORY  ///
////////////////
////////////////


// 1. connect to server
// 2. client request message: "history:<projname>"
// 3. server sends back history file plus contents
// 4. client disconnects
// 4. client prints out history (output similar to update)

void history(char* projname){
  int wr;
  printf("UNFINISHED: history!!\n");
  int diditwork = connectToServer();

  if(diditwork == -1){
    printf("Cannot checkout '%s' ... Failed to connect to server\n", projname);
    return;
  }


  //send how many bytes the message is, so the server knows how much to read
  int x = strlen(projname)+9;

  //int to string
  char* numofbytes;
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30);
  sprintf(numofbytes, "%d", x);
  
  // write how many bytes u are about to send to the server
  wr = write(sock, numofbytes, sizeof(char)*strlen(numofbytes));

  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  free(numofbytes);

  // see if the server is ready to catch the real message ur about to give it
  char* servmess = (char*)malloc(sizeof(char)*strlen("lemme have it"));
  wr = read(sock, servmess, sizeof(char)*strlen("lemme have it"));
 
  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 

  if(strcmp(servmess, "lemme have it")){
    printf("Error in communicating w/ server.. Cannot checkout '%s'\n", projname);
    return; 
  }

  free(servmess);

  //write down the message that we (the client) want to send to the server to request a project
  char* message = (char*)malloc(sizeof(char)*x);
  memset(message, '\0', strlen(message));
  snprintf(message, sizeof(message), "checkout:%s",projname);
 

  //send checkout request message
  wr = write(sock, message, sizeof(char)*strlen(message));

  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  free(message);
  // read number of bytes to read for real message
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30); 
  wr = read(sock, numofbytes, sizeof(char)*30);

  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 
 
  int i; 
  for(i = 0; i <strlen(numofbytes); i++){
    if(!isdigit(numofbytes[i])){
      printf("Error in reading bytes from server...\n");
      return;
    }
  } 
 
  int y = atoi(numofbytes); 

  // we are going to read the server's checkout message in this 
  char* checkoutmess = (char*)malloc(sizeof(char)*y);
  memset(checkoutmess, '\0', strlen(checkoutmess));

  wr = write(sock, "gimme", sizeof(char)*strlen("gimme"));
  
  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  return;
}

//////////////
//////////////
// ROLLBACK //
//////////////
//////////////


// 1. connect to server
// 2. client request message: "rollback:<projname>:<versionnumber>
// 3. server sends back success message
// 4. client disconnects


void rollback(char* projname, char* versio){
//  printf("UNFINISHED: rollback!!\n");
  int wr;
  // if version number

  int jkl = 0;

  for(jkl = 0; jkl< strlen(versio); jkl++){

    if(!isdigit(versio[jkl])){
      printf("Invalid version number\n");
      return;
    }

  }

  int version = atoi(versio);

  if(version < 0){
    printf("Invalid version number. Please give version number greater than or equal to 0.\n");
    return;
  }


  int diditwork = connectToServer();

  if(diditwork == -1){
    printf("Cannot checkout '%s' ... Failed to connect to server\n", projname);
    return;
  }


  //send how many bytes the message is, so the server knows how much to read
  int x = strlen(projname)+9;

  //int to string
  char* numofbytes;
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30);
  sprintf(numofbytes, "%d", x);
  
  // write how many bytes u are about to send to the server
  wr = write(sock, numofbytes, sizeof(char)*strlen(numofbytes));

  if(wr<0){
    printf("Error writing to server\n");
    return;
  }

  free(numofbytes);

  // see if the server is ready to catch the real message ur about to give it
  char* servmess = (char*)malloc(sizeof(char)*strlen("lemme have it"));
  wr = read(sock, servmess, sizeof(char)*strlen("lemme have it"));
 
  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 

  if(strcmp(servmess, "lemme have it")){
    printf("Error in communicating w/ server.. Cannot rollback '%s'\n", projname);
    return; 
  }

  free(servmess);

  //write down the message that we (the client) want to send to the server to request a project
  char* message = (char*)malloc(sizeof(char)*x);
  memset(message, '\0', strlen(message));
  snprintf(message, sizeof(message), "rollback:%s",projname);
 

  //send checkout request message
  wr = write(sock, message, sizeof(char)*strlen(message));

  if(wr<0){
    printf("Error writing to server\n");
    return;
  } 

  free(message);

}
 


/*



  // read number of bytes to read for real message
  numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30); 
  wr = read(sock, numofbytes, sizeof(char)*30);

  if(wr<0){
    printf("Error reading from server\n");
    return;
  } 
 
  int i; 
  for(i = 0; i <strlen(numofbytes); i++){
    if(!isdigit(numofbytes[i])){
      printf("Error in reading bytes from server...\n");
      return;
    }
  } 

  // 1(b). attempt to parse configure 
  

  if(config == -1){
  }  
  if(config == -2){
    printf("Configure was not run properly. Please try to run configure again, and pay atention to error messages.\n");
    return;
  }

  // 2. client can't connect to server
  char projpath[1024];

  //should all projects on client side be in the working directory??
  snprintf(projpath, sizeof(projpath), "./%s", projname);

  DIR* isithere =  opendir(projpath);

  if(isithere != NULL){
    printf("Directory already exists. Cannot checkout.\n");
    return;
  }

  // 4. check if project exists on server side

  message = malloc(sizeof(char)*(strlen(projpath)+13));

  snprintf(message, sizeof(message), "gimme:project:%s",projpath);

  // ask server
  write(sock, message, sizeof(char)*strlen(message);
  free(message);
  
  //if server replies
 // read(sock, )


  return;
}
//update
//void update(){

//}

//upgrade
//commit
//push
//create
//destroy
//add
//remove
//currentversion
//history
//rollback

*/
