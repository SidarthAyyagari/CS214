#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mymalloc.h"

/* Each meta data block is 5 bytes. 
 * - 1 byte for in-use flag
 * - 2 bytes to hold size of user data
 * - 2 bytes for magic number (to verify that this is data and not just random memory) */

void* mymalloc(size_t size, char* file, int line){
  
  /* INVALID INPUT (size_t is unsigned, so no need to check for negative) */
  if(size > 4091){
    fprintf(stderr, "ERROR at File: %s, Line: %d\nYou asked for too much memory. Please request no more than 4091 bytes\n", file, line);
    return NULL;
  }  

  /* corner cases:  */

  void *ptr;


  /////////////////
  /////////////////  
  /*    setup    */
  /////////////////
  /////////////////
  
  /* these two variables will be used to see if this is the first malloc */
  unsigned short magic = 20497;
  unsigned short isitmagic = myblock[3] | myblock[4] << 8;

  int count = 0;

  //////////////////
  /* FIRST MALLOC */
  //////////////////

  if(isitmagic != magic){
    
    /* First metadata */
 
    // We set the in-use flag to 'Y'
    myblock[count] = 'Y';
    ++count;   
    
    // set the size block to the size requested (int) 
    unsigned short *sr = (unsigned short*)(&myblock[count]);
    *sr = size;
    count += 2;

    // a short is 2 bytes so it splits it 20497 into two separate bytes
    // and it puts the least significant byte in myblock[1] and the 
    // most signiicant byte in myblock[2]
    unsigned short *mn = (unsigned short*)(&myblock[count]);
    *mn = magic;
    count += 2;

    //catch the ptr
    ptr = &myblock[count];
 
    /* allocate the next metadata */
   
    //in use 
    count += size;
    myblock[count] = 'N';
    count += 1;
    
    //size (allocate 4096 - 5 - size - 5) !!!!!!!!!!!!!WHAT THE -
    unsigned short *ns = (unsigned short*)(&myblock[count]); 
    *ns = 4096 - (10+size);
    count += 2;

    //magic number
    unsigned short *mnone = (unsigned short*)(&myblock[count]); 
    *mnone = magic;
    
    return ptr;
  }

  //////////////////////////
  /* NOT THE FIRST MALLOC */
  //////////////////////////

  else{
    unsigned short usersize = 0;
   
    count = 0; 
    while(count < 4096){
     
      /* already occupied*/
      if(myblock[count] == 'Y'){
        ++count;
        //printf("already occupied\n");
        usersize = myblock[count] | myblock[count + 1] << 8;
        count += (4 + usersize);
        continue;
      }

      /* Not occupied.. */
      else if(myblock[count] == 'N'){
        
        usersize =  myblock[count + 1] | myblock[count + 2] << 8;

        /* 3 cases */
        
        // HIT! perfect fit 
        if(size == usersize){ 
	  
          myblock[count] = 'Y';
	  count += 5;
	  ptr = &myblock[count];
          return ptr;
        }
       
        // If the avaiable size < requested size
        // MISS.. Not enough space here, keep looking
        else if(size > usersize){
          count += (5 + usersize);
          continue;
	}
  
        // If the available size > requested size
        // HIT!
        else{
          int diff = usersize - size;
	  
          // There is enough space here to allocate for the request and set metadata for the next one
          if(diff >= 5){
            myblock[count] = 'Y';
            ++count; 
            unsigned short *nns = (unsigned short*)(&myblock[count]); 
            *nns = size;
            count += 4;  // magic number is already set, so we can skip over it 
           
            ptr = &myblock[count];
            
            /* Allocate next metadata */
            count += size;
            
            // set in use to 'N'
            myblock[count] = 'N';

            // set usersize  
            ++count;
            unsigned short *nnns = (unsigned short*)(&myblock[count]); 
            *nnns = diff - 5;
            count += 2;

            // set magic number
            unsigned short *mntwo = (unsigned short*)(&myblock[count]);
            *mntwo = magic;
            return ptr; 
	  }
          // INTERNAL FRAGMENTATION. We don't have enough room to allocate and put more metadata
          // If we allocate the requested amount of memory, it will compromise the structure of the memory
          // So we have to give back a pointer to a block containing more memory than what was requested.
          else{
            myblock[count] = 'Y';
            count += 5;
            ptr = &myblock[count];        
            return ptr;
          }
        }
      }


      else{
        fprintf(stderr, "ERROR at File: %s, Line: %d\nNo more memory can be allocated at this time. Please free() any nonessential pointers you have created, and try again.\n", file, line);
        return ptr;
      }   
    } 
      
  }

    
  fprintf(stderr, "ERROR at File: %s, Line: %d\nNo more memory can be allocated at this time. Please free() any nonessential pointers you have created, and try again.\n", file, line);
  return ptr;
}

void myfree(void *ptr, char* file, int line){


  // First, consider the possibility that free() is called before malloc is ever called.
  // We should check for a magic number before we do anything

  
  unsigned short magic = 20497;
  unsigned short isitmagic = myblock[3] | myblock[4] << 8;
 
 
 
  if(isitmagic != magic){
    fprintf(stderr, "ERROR at File: %s, Line: %d\nfree() was called before malloc() ever was called. Please malloc() something first, then free() it.\nUSAGE: char *ptr = malloc(sizeof(char));\nfree(ptr); \n", file, line);
    return;
  }

  /* Now we can presume at least one thing has been malloc'd */

  /* what if a null pointer is passed? */ 
  if(!(ptr)){
    return;
  }

  /* What if we try to step back 2 and we fall off the array?
 *   What if the user is really mean and conniving and they pass us a pointer that 
 *   is not even in our array?  */
  

  // Since our metadata comes first, and is always 5 bytes, any pointer pointing to 
  // myblock[0],myblock[1], myblock[2], myblock[3], or myblock[4] is invalid
  
 
  void *start = &myblock[5];
  void *end = &myblock[4095];


  if(ptr < start || ptr > end ){
    fprintf(stderr, "ERROR at File: %s, Line: %d\nAn invalid pointer was requested to be freed. Please pass a valid pointer.\n", file, line);
    return;
  } 
  

  /* business as usual */
  else{
    
     // skip over magic
    
     ptr += -2;
 
     unsigned short b1 = *((unsigned short*)(ptr));
     unsigned short b2 = *((unsigned short*)(ptr+1));
  
     isitmagic = b1 | b2 << 8;
    

   

     // IF IT IS METADATA
     if(isitmagic == magic){
       
       // Check in-use flag
       ptr += -3;
       
       // You cannot dereference a void*.So we will cast ptr to a char*.
       if(*((char*)ptr) == 'Y'){
         
         *((char*)ptr) = 'N';
  //       printf("---freed ptr = %p\n", ptr + 5);
         defragment();
         return; 
       }
       else{
         fprintf(stderr, "ERROR at File: %s, Line: %d\nPointer has already been freed.\n", file, line);
         defragment();
         return;
       }
     }

     else{
       fprintf(stderr, "ERROR at File: %s, Line: %d\nAn invalid pointer was requested to be freed. Please passs a valid pointer.\n", file, line);
       defragment();
       return;
     }
    
  }

  defragment();
  return;
}

/* This function is designed to subdue the effects of external fragmentation. 
 * If there are two adjacent free blocks, then this function will merge them 
 * into one free block and erase the second block's metadata giving us one 
 * block that could meet a larger request, and also an extra 5 bytes to use for 
 * potential storage */



void defragment(){
  
  int count = 0;
 
  int mark1 = 0;
  int mark2 = 0;

  unsigned short skipsize = 0;
  unsigned short skipsize2 = 0;  

  while(count < 4096){
    /* check each block's in use flag */
    if(myblock[count] == 'N'){
       mark1 = 1;
    }
 
  
    ++count;
    skipsize = myblock[count] | myblock[count + 1] << 8;
    count += (4+skipsize);
   
    if(count >= 4096){
      return;
    }  
 
    if(myblock[count] == 'N'){
      mark2 = 1;
    } 

    ++count;
    skipsize2 = myblock[count] | myblock[count + 1] << 8;
    
    // merge the two free blocks
    if(mark1 == 1 && mark2 == 1){;
      
      // corrupt magic number of the second block
      myblock[count+3] = 'a';

      //update the first block size
      unsigned short *df = (unsigned short*)(&myblock[count-5-skipsize]);
      *df = skipsize + skipsize2 + 5;
    
      count += (-6-skipsize);
      mark1 = 0;
      mark2 = 0;
    } 

    //set the count to the next to be
    else{
      --count;
      mark1 = 0;
      mark2 = 0;
    }
  }
  
  return;
}
