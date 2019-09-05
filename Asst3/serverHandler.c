#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include "server.h"


int clientConnector(int sock){

  // The client should first say "hi!"
  char* greeting = (char*)malloc(sizeof(char)*4);
  memset(greeting, '\0', sizeof(char)*strlen(greeting));

  int wr = read(sock, greeting, 4);

  free(greeting);

  if (wr< 0){
    printf("1. COMMUNICATION ERROR\n");
    return -1;
  }

  // We will respond asking what we can do for the client
  wr = write(sock, "Whatchu need?", strlen("Whatchu need?")*sizeof(char));

  if (wr< 0){
    printf("Could not write back to client\n");
    return -1;
  }

  // receive amount of bytes the client is about to send
  char* numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', sizeof(char)*30);

  wr = read(sock, numofbytes, sizeof(char)*30);

  if (wr< 0){
    printf("Could not read what client said\n");
    return -1;
  }

  // make sure it is a number before atoi is called
  int i;
  for(i = 0; i < strlen(numofbytes); i++){
    if(!isdigit(numofbytes[i])){
      printf("COMMUNICAATION ERROR\n");
      return -1;
    }
  }

  int x = atoi(numofbytes);

  //got the number, now we know how much to read.
  wr = write(sock, "lemme have it", strlen("lemme have it")*sizeof(char));

  if(wr<0){
    printf("Error writing back to client\n");
    return -1;
  }


  // here is the request
  char* req = (char*)malloc(sizeof(char)*x);
  memset(req, '\0', sizeof(char)*x);

  wr = read(sock, req, x);

  if(wr<0){
    printf("Error writing back to client\n");
    return -1;
  }

  free(numofbytes);

  //at this point, the client's request has been saved in the char* "req"
  //now we should interpret this request and try to do what it is asking us to do

  x = requestParser(sock, req);

  if(x < 0){
    printf("ERROR: client's request could not be completed. Sorry :(\n");
    return -1;
  } 

  free(req);
  return 1;
}

//goes through client's request message and figures out what to do based off of the message 
int requestParser(int socc, char* r){

  int x;
  int i;


  if(r[0] == 'c'){

    //checkout
    if(r[1] == 'h'){
 
      i = strlen("checkout:");
      char* pn = (char*)malloc(sizeof(char)*(strlen(r) - i));  //project noame = 'pn'
      strncpy(pn, r + i, strlen(r) - i);    

      //checkout: retrieve manifest + all files
      x = checkout(socc, pn);
      if(x<0){
        free(pn);
        return -1;
      }
      free(pn);
      return 1;
    }

    else if(r[1] == 'o'){
      //commit
      i = strlen("commit:");
      char* pn = (char*)malloc(sizeof(char)*(strlen(r) - i));  //project noame = 'pn'
      strncpy(pn, r + i, strlen(r) - i);    

      //checkout: retrieve manifest + all files
      x = checkout(socc, pn);
      if(x<0){
        free(pn);
        return -1;
      }
      free(pn);
      return 1;
    }

    else if(r[1] == 'r'){
      //create
    }

    else if(r[1] == 'u'){
      //current version
    }
    else{
      //error
      printf("Internal Server error\n");
      return -1;
    }

  }

  else if(r[0] == 'u' && r[1] == 'p'){

    if(r[2] == 'd'){
    
      //isolate project name
      i = strlen("update:");
      char* pn = (char*)malloc(sizeof(char)*(strlen(r) - i));  //project noame = 'pn'
      strncpy(pn, r + i, strlen(r) - i);    

      //checkout: retrieve manifest + all files
      x = update(socc, pn);
      if(x<0){
        return -1;
      }
        
    }

    else if(r[2] == 'g'){
      //upgrade
    }
   
    else{
      //error
    }

  }

  else if(r[0] == 'p'){
    //push
  }

  else if(r[0] == 'd'){
    //destroy
  }

  else if(r[0] == 'h'){
    //history
  }
  
  else if(r[0] == 'r'){
    //rollback
    //request should look like "rollback:<lengthofprojectname>:<projectname>:<versionnumber>

     //skip over rollback
     r = r + 9;

     char* token = (char*)malloc(sizeof(char)*strlen(r));
     memset(token, '\0', strlen(token));
     char* temp = token;


     //project name length digit by digit
     while(isdigit(*r)){
       *temp = *r;
       temp++;
       r++;
     }

     //project name length
     int pnl = atoi(token);

     free(token);
     token = (char*)malloc(sizeof(char)*strlen(r));
     memset(token, '\0', strlen(token));
     temp =token;
     r++; // skip over semicolon

     // get the project name
     int i;
     for(i = 0; i < pnl; i++){
       *temp = *r;
       temp++;
       r++;
     }

     // project name = pn
     char* pn = (char*)malloc(sizeof(char)*pnl);
     memset(pn, '\0', strlen(pn));
     sprintf(pn, "%s", token);

     free(token);
     token = (char*)malloc(sizeof(char)*strlen(r));
     memset(token, '\0', strlen(token));
     temp =token;
     r++; // skip over semicolon

     
     //version number digit by digit
     while(isdigit(*r)){
       *temp = *r;
       temp++;
       r++;
     }

     // v = version
     int v = atoi(token);
     free(token);

      //checkout: retrieve manifest + all files
      x = rollback(socc, pn, v);
      if(x<0){
        return -1;
      }
  }

  else{
    //error
  }

  return -1;
}




int checkout(int socket, char* projname){
  
  int wr;
  char* numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', strlen(numofbytes)); 

  char* gimme = (char*)malloc(sizeof(char)*5);
  memset(gimme, '\0', strlen(gimme)); 

  //try to open directory
  char* projpath = (char*)malloc(sizeof(char)*(strlen(projname)+strlen("./repos/")));
  snprintf(projpath, sizeof(projpath), "./repos/%s", projname); 
  DIR *p = opendir(projpath);


  // project directory does not exist
  if(p == NULL){
    sprintf(numofbytes, "%d", strlen("project does not exist"));
    wr = write(socket, numofbytes, sizeof(char)*strlen(numofbytes)); 

    free(numofbytes);
  
    if(wr<0){
      printf("Error writing numofbytes to client\n");
      return -1;
    }

    wr = read(socket, gimme, sizeof(char)*strlen(gimme));
    
    if(wr<0){
      printf("Error reading gimme from client\n");
      return -1;
    }

    //CHECK GIMME

    //write back fail message
    wr = write(socket, "project does not exist", strlen("project does not exist"));

    if(wr<0){
      printf("Error writing fail message to client\n");
      return -1;
    }

    return 0;
  }

  //project directory does exist
  else{

    // (zlib/tar the whole directory and send it back)   
    
    

  }
 
  return;   

}



int update(int socket, char* projname){

  /////////////////////////////////////////////////////
  // send back current manifest of specified project // 
  /////////////////////////////////////////////////////

  //try to open directory
 
  int wr; 
  
  char* projpath = (char*)malloc(sizeof(char)*(strlen("./repos/") + strlen(projpath)));
  memset(projpath, '\0', sizeof(char)*strlen(projpath));
  snprintf(projpath, sizeof(projpath), "./repos/%s", projname); 

  

  DIR *p = opendir(projpath);

  char* numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', strlen(numofbytes));

  char* lhi = (char*)malloc(sizeof(char)*14);
  memset(lhi, '\0', strlen(lhi));

  char* gimme = (char*)malloc(sizeof(char)*6);

  // project directory does not exist
  if(p == NULL){
    
    sprintf(numofbytes, "%d", strlen("project does not exist"));
    wr = write(socket, numofbytes, sizeof(char)*strlen(numofbytes)); 

    free(numofbytes);
  
    if(wr<0){
      printf("Error writing numofbytes to client\n");
      return -1;
    }

    

    wr = read(socket, gimme, sizeof(char)*strlen(gimme));
    
    if(wr<0){
      printf("Error reading gimme from client\n");
      return -1;
    }

    //CHECK GIMME

    //write back fail message
    wr = write(socket, "project does not exist", strlen("project does not exist"));

    if(wr<0){
      printf("Error writing fail message to client\n");
      return -1;
    }

    return 0;
  }

  //project exists and you need to retrieve the most current .manifest file
  else{

    return -1;
  }

  return -1;

}

/*
  //insert (star slash)
    char* manipath = (char*)malloc(sizeof(char)*(strlen("./repos/") + strlen(projpath)+ strlen("/current/")+ strlen(".Manifest")));
    memset(manipath, '\0', sizeof(char)*strlen(manipath));
    snprintf(manipath, sizeof(manipath), "./repos/%s/current/.Manifest", projname); 
    
    int manifd = open(manipath, O_RDONLY);
   
    if(manifd < 0){

      //could not open up manifest blah blah blah
 
    }


 
    struct stat st;
    stat(manipath, &st);
    size_t size = st.st_size;
  
    //char* manifestbuffer = (char*)malloc(sizeof(char)*(size+strlen("manifest:
     


    struct dirent *item;

    //DIR *curr = ;
    int counter;
    while( (item = readdir(p)) != NULL){

      //check each version
      if(item->d_type == DT_DIR){

        if(strcmp(item->d_name, ".") == 0 || strcmp(item->d_name, "..") == 0){
          continue;
        }        

        //check the name
      }

    }

  }

}

*/
/////////////
// UPGRADE //
/////////////

// 1. check if project name exists
// 2. send message saying go for it

int upgrade(int socket, char* projname){

  

}



////////////
// COMMIT //
////////////

// 1. check if project name exists
// 2. send current manifest to client
// 3. receive .commit from client
// 4. go through .commit's and make this .commitX
// 5. send success message to client


int commit(int socket, char* projname){

  

}

//////////
// PUSH //
//////////

// 1. check if project name exists
// 2. lock repos
// 3. run through each .commit1, .commit2, .commit3, etc. and see if 
// 4. if you find one, delete all other commits
// 5. cp current -> vX 
// 6. modify current based off of .commit
// 7. update current .manifest
// 8. unlock repos
// 9. send success message back to client


int push(int socket, char* projname){

  

}


////////////
// CREATE //
////////////

// 1. check if project name exists
// 2. if not, create directory <projname>
// 3. create directory in it current
// 4. place .manifest in repos/projname/current/


int create(int socket, char* projname){

  int wr;

  char* projpath = (char*)malloc(sizeof(char)*(strlen(projname)+9));
  memset(projpath, '\0', strlen(projpath));

  snprintf(projpath, sizeof(projpath), "./repos/%s", projname);
  

  DIR* p = opendir(projpath);

    
  char* numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', strlen(numofbytes));

  char* lhi = (char*)malloc(sizeof(char)*14);
  memset(lhi, '\0', strlen(lhi));

  //project does exist in server repos
  if(p != NULL){

    //write to socket that project does exist

    sprintf(numofbytes, "%d", strlen("project already exists"));

    wr = write(socket, numofbytes, 30);
    
    if(wr < 0){
      printf("Error writing numofbytes to client\n");
      free(numofbytes);
      free(projpath);
      closedir(p);
      close(socket);
      return -1;
    }
    
    
    wr = read(socket, lhi, strlen("lemme have it")*sizeof(char));
    
    if(wr < 0){
      printf("Error reading confirmation from client\n");
      free(lhi);
      free(numofbytes);
      free(projpath);
      closedir(p);
      close(socket);
      return -1;
    }

    free(lhi);
    wr = write(socket, "project already exists", strlen("project already exists")*sizeof(char));
    
    if(wr < 0){
      printf("Error writing numofbytes to client\n"); 
      free(numofbytes);
      free(projpath);
      closedir(p);
      close(socket);
      return -1;
    }

     
    free(projpath);
    closedir(p);
    close(socket);
    return -1;    
  }

  //project does not exist
  
  // create project
  int status = mkdir(projpath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

  if( status < 0){

    printf("Error could not make directory\n"); 
    free(numofbytes);
    free(projpath);
    closedir(p);
    close(socket);
    return -1;

  }

  char* currpath = (char*)malloc(sizeof(char)*(strlen(projpath)+17));
  memset(currpath, '\0', strlen(currpath));

  snprintf(currpath, 1024, "./repos/%s/current", projname);

  status = mkdir(currpath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

  char* manipath = (char*)malloc(sizeof(char)*(strlen(projpath)+10));
  memset(manipath, '\0', strlen(manipath));
  snprintf(manipath, 1024, "%s/.Manifest", projpath);

  snprintf(manipath, 1024,"%s/.Manifest", projpath);

  int fd = open(manipath, O_CREAT |O_RDWR, S_IRUSR | S_IWUSR);
   
  if(fd < 0 ){

    printf("Error could not make directory\n"); 
    free(manipath);
    free(numofbytes);
    free(projpath);
    closedir(p);
    close(socket);
    return -1;
  } 
 
  wr = write(fd, "1\n", sizeof(char)*strlen("1\n"));

   
  if(wr < 0 ){

    printf("Error could not make directory\n"); 
    free(manipath);
    free(numofbytes);
    free(projpath);
    closedir(p);
    close(socket);
    return -1;
  }

  //send manifest to client

 
    
}



/////////////
// DESTROY //
/////////////

// 1. check if project name exists
// 2. lock repos delete entire project directory


int destroy(int socket, char* projname){

  int wr;

  char* projpath = (char*)malloc(sizeof(char)*(strlen(projname)+9));
  memset(projpath, '\0', strlen(projpath));

  snprintf(projpath, sizeof(projpath), "./repos/%s", projname);
  

  DIR* p = opendir(projpath);

    
  char* numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', strlen(numofbytes));

  char* lhi = (char*)malloc(sizeof(char)*14);
  memset(lhi, '\0', strlen(lhi));

  //project does not exist in server repos
  if(p == NULL){

    //write to socket that project does not exist

    sprintf(numofbytes, "%d", strlen("project does not exist"));

    wr = write(socket, numofbytes, 30);
    
    if(wr < 0){
      printf("Error writing numofbytes to client\n");
      free(numofbytes);
      free(projpath);
      closedir(p);
      close(socket);
      return -1;
    }
    
    
    wr = read(socket, lhi, strlen("lemme have it")*sizeof(char));
    
    if(wr < 0){
      printf("Error reading confirmation from client\n");
      free(lhi);
      free(numofbytes);
      free(projpath);
      closedir(p);
      close(socket);
      return -1;
    }

    free(lhi);
    wr = write(socket, "project does not exist", strlen("project does not exist")*sizeof(char));
    
    if(wr < 0){
      printf("Error writing numofbytes to client\n");
      free(numofbytes);
      free(projpath);
      closedir(p);
      close(socket);
      return -1;
    }

     
    free(projpath);
    closedir(p);
    close(socket);
    return -1;    
  }
 
  //project does exist
  //lock repos
  
  char* sc = (char*)malloc(sizeof(char)*(strlen(projpath)+ 8));  
  memset(sc, '\0', strlen(sc));
  snprintf(sc, sizeof(sc), "rm -rf %s", projpath);


  system(sc);
 
  wr = write(socket, numofbytes, 30);
    
  if(wr < 0){
    printf("Error writing numofbytes to client\n");
    free(numofbytes);
    free(projpath);
    closedir(p);
    close(socket);
    return -1;
  }
    
  wr = read(socket, lhi, strlen("lemme have it")*sizeof(char));
    
  if(wr < 0){
    printf("Error reading confirmation from client\n");
    free(lhi);
    free(numofbytes);
    free(projpath);
    closedir(p);
    close(socket);
    return -1;
  }

  free(lhi);
  wr = write(socket, "project destroyed succesfully!", strlen("project destroyed succesfully!")*sizeof(char));
    
  if(wr < 0){
    printf("Error writing numofbytes to client\n");
    free(numofbytes);
    free(projpath);
    closedir(p);
    close(socket);
    return -1;
  }

  //unlock repos
     
  free(projpath);
  closedir(p);
  close(socket);

  return 1;

}



/////////////////////
// CURRENT VERSION //
/////////////////////

// 1. check if project name exists
// 2. if it does, go to <projname>/current and send .manifest back 

int currentversion(int socket, char* projname){

  int wr;

  char* projpath = (char*)malloc(sizeof(char)*(strlen(projname)+9));
  memset(projpath, '\0', strlen(projpath));

  snprintf(projpath, sizeof(projpath), "./repos/%s", projname);
  

  DIR* p = opendir(projpath);

  

  //project does not exist in server repos
  if(p == NULL){

    //write to socket that project does not exist

    wr = write(socket, "project does not exist", strlen("project does not exist")*sizeof(char));
    free(projpath);
    closedir(p);
    close(socket);
    return -1;    
  }
 

  //project does exist in server's repos
   
  char* filepath = (char*)malloc(sizeof(char)*(strlen(projname)+27));
  memset(filepath, '\0', strlen(filepath));

  snprintf(filepath, sizeof(filepath), "./repos/%s/current/.Manifest", projname);
  

  // try to open .manifest
  int fd = open(filepath, O_RDONLY); 

  if(fd < 0){

    printf(".Manifest file not found.\n");
    write(socket, "file not found", strlen("file not found"));
    free(projpath);
    free(filepath);
    closedir(p);
    close(socket);
    return -1;
  }

  //we have a handle

  struct stat st;
  stat(filepath, &st);
  int size = st.st_size;

  char* buffer = (char*)malloc(size);

  wr = read(fd, buffer, size);
 
  if(wr < 0){
    printf("Error writing .manifest back to client\n");
    free(buffer);
    free(projpath);
    free(filepath);
    closedir(p);
    close(socket);
    return -1;
  }

  char* numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', strlen(numofbytes));
  sprintf(numofbytes, "%d", size);


  //write to client the size of the message
  wr = write(socket, numofbytes, sizeof(char)*strlen(numofbytes));

  if(wr < 0){
    printf("Error writing .history back to client\n");
    free(numofbytes);
    free(buffer);
    free(projpath);
    free(filepath);
    closedir(p);
    close(socket);
    return -1;
  }

  // client is ready to receive
  char* lhi = (char*)malloc(sizeof(char)*14);
  wr = read(socket, lhi, strlen("lemme have it")*sizeof(char));


  if(wr < 0){
    printf("Error writing .Manifest back to client\n");
    free(lhi);
    free(numofbytes);
    free(buffer);
    free(projpath);
    free(filepath);
    closedir(p);
    close(socket);
    return -1;
  }

  if(strcmp(lhi, "lemme have it")){
    printf("client is not ready\n"); 
    free(lhi);
    free(numofbytes);
    free(buffer);
    free(projpath);
    free(filepath);
    closedir(p);
    close(socket);
    return -1;
  }

  free(lhi);

  //send history file to client
  wr = write(socket, buffer, size);

  if(wr < 0){
    printf("Error writing .Manifest back to client\n");
    free(numofbytes);
    free(buffer);
    free(projpath);
    free(filepath);
    closedir(p);
    close(socket);
    return -1;
  }



  free(numofbytes);
  free(buffer);
  free(projpath);
  free(filepath);
  closedir(p);
  close(socket);
  
  return 1;

} 




/////////////
// HISTORY //
/////////////

// 1. check if project name exists
// 2. open .history, put it into a buffer, and write it to client

int history(int socket, char* projname){

  int wr;

  char* projpath = (char*)malloc(sizeof(char)*(strlen(projname)+9));
  memset(projpath, '\0', strlen(projpath));

  snprintf(projpath, sizeof(projpath), "./repos/%s", projname);
  

  DIR* p = opendir(projpath);

  

  //project does not exist in server repos
  if(p == NULL){

    //write to socket that project does not exist

    wr = write(socket, "project does not exist", strlen("project does not exist")*sizeof(char));
    free(projpath);
    closedir(p);
    close(socket);
    return -1;    
  }
 

  //project does exist in server's repos

   
  char* filepath = (char*)malloc(sizeof(char)*(strlen(projname)+18));
  memset(filepath, '\0', strlen(filepath));

  snprintf(filepath, sizeof(filepath), "./repos/%s/.history", projname);
  

  // try to open .history
  int fd = open(filepath, O_RDONLY); 

  if(fd < 0){

    printf(".history file not found.\n");
    write(socket, "file not found", strlen("file not found"));
    free(projpath);
    free(filepath);
    closedir(p);
    close(socket);
    return -1;
  }

  //we have a handle

  struct stat st;
  stat(filepath, &st);
  int size = st.st_size;

  char* buffer = (char*)malloc(size);

  wr = read(fd, buffer, size);
 
  if(wr < 0){
    printf("Error writing .history back to client\n");
    free(buffer);
    free(projpath);
    free(filepath);
    closedir(p);
    close(socket);
    return -1;
  }

  char* numofbytes = (char*)malloc(sizeof(char)*30);
  memset(numofbytes, '\0', strlen(numofbytes));
  sprintf(numofbytes, "%d", size);


  //write to client the size of the message
  wr = write(socket, numofbytes, sizeof(char)*strlen(numofbytes));

  if(wr < 0){
    printf("Error writing .history back to client\n");
    free(numofbytes);
    free(buffer);
    free(projpath);
    free(filepath);
    closedir(p);
    close(socket);
    return -1;
  }

  // client is ready to receive
  char* lhi = (char*)malloc(sizeof(char)*14);
  wr = read(socket, lhi, strlen("lemme have it")*sizeof(char));

  if(wr < 0){
    printf("Error writing .history back to client\n");
    free(lhi);
    free(numofbytes);
    free(buffer);
    free(projpath);
    free(filepath);
    closedir(p);
    close(socket);
    return -1;
  }

  if(strcmp(lhi, "lemme have it")){
    printf("client is not ready\n"); 
    free(lhi);
    free(numofbytes);
    free(buffer);
    free(projpath);
    free(filepath);
    closedir(p);
    close(socket);
    return -1;
  }

  free(lhi);


  //send history file to client
  wr = write(socket, buffer, size);

  if(wr < 0){
    printf("Error writing .history back to client\n");
    free(numofbytes);
    free(buffer);
    free(projpath);
    free(filepath);
    closedir(p);
    close(socket);
    return -1;
  }



  free(numofbytes);
  free(buffer);
  free(projpath);
  free(filepath);
  closedir(p);
  close(socket);
  
  return 1;
 
} 


//////////////
// ROLLBACK //
//////////////

// 1. check if project name exists
// 2. check if project version exists (if it doesn't, send fail message_
// 3. mv current v!, and mv specified version to current
// 4. delete all directories' greater than specified version
// 5. send back success message


/* UNFINISHED */
int rollback(int socket, char* projname, int version){

  int wr;

  char* projpath = (char*)malloc(sizeof(char)*(strlen(projname)+9));
  memset(projpath, '\0', strlen(projpath));

  snprintf(projpath, sizeof(projpath), "./repos/%s", projname);
  

  DIR* p = opendir(projpath);

  //project does not exist in server repos
  if(p == NULL){

    //write to socket that project does not exist

    wr = write(socket, "project does not exist", strlen("project does not exist")*sizeof(char));
    free(projpath);
    closedir(p);
    close(socket);
    return -1;    
  }
 

  //see if project version exists
  char* versionpath = (char*)malloc(sizeof(char)*(31+strlen(projpath)));
  memset(versionpath, '\0', 31+strlen(projpath));

  //turn version # to a string
  char* verz = (char*)malloc(sizeof(char)*30);
  memset(verz, '\0', 30);
  sprintf(verz, "%d", version);

  //version path has the path of the directory 
  snprintf(versionpath, sizeof(versionpath), "./repos/%s/v%s",projname, verz);
  
  //check if this version exists

  p = opendir(versionpath);

  // project version does not exist
  if(p != NULL){

    wr = write(socket, "version not valid", strlen("version not valid")*sizeof(char));
    free(projpath);
    free(verz);
    free(versionpath);
    closedir(p);
    close(socket);
    return -1;    

  }

  //project version does exist
  char* cp = (char*)malloc(sizeof(char)*(strlen(projname)+ 17));
  memset(cp, '\0', strlen(projname)+17);

  snprintf(cp, sizeof(cp), "./repos/%s/current", projname);

  // remove current
  remove(cp);
  free(cp); 
  
  //set versionpath to current
  char* sc = (char*)malloc(sizeof(char)*(strlen(projname)+strlen(versionpath)+21));
  snprintf(sc, sizeof(sc), "mv %s ./repos/%s/current", versionpath, projname); 
  system(sc);

  int i = version;
  //rm -rf all directories greater than v0  
  while(p != NULL){
    
    closedir(p);
    memset(versionpath, '\0', 30);
    memset(verz, '\0', 30);
    i++;

    sprintf(verz, "%d", i);

    snprintf(versionpath, 1024, "v%s", verz);
    p = opendir(versionpath);

    //printf("versionpath: '%s'\n", versionpath);
    if(p!=NULL){

       memset(sc, '\0', strlen(sc));
       snprintf(sc, 1024, "rm -rf %s", versionpath);
       system(sc);
    }
  }

   free(verz);
   free(versionpath);
   free(sc);

   write(socket, "project rollback successful!", strlen("project rollback successful")*sizeof(char)); 

   return 1;
   //close socket

} 


