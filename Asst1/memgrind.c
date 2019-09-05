//#include <ctype.h>
//#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"



int main(int argc, char* argv[]){

long int arr[6][100];

struct timeval start, end;

int jk = 0;

  while(jk < 100){

    
/* PART A: malloc() 1 byte and immediately free it (do this 150 times) */

    gettimeofday(&start, NULL); 
   
    int i = 1; 

    while(i <= 150){
      char *ptr = malloc(sizeof(char));
      free(ptr);
      i++;
    }
  
    gettimeofday(&end,NULL);

    arr[0][jk] = (end.tv_sec * 1000000 + end.tv_usec)  - (start.tv_sec * 1000000 + start.tv_usec);

/* PART B: malloc() 1 byte, store the pointer in an array (do this 150 
 *         times) Once you have malloc()ed 50 byte chunks, then free() 
 *         the 50 byte pointers one by one                             */

   gettimeofday(&start, NULL);
  

   char *arrB[150];

   i = 0;
   int j = 0;

   while(i < 150){
     char *ptr = malloc(sizeof(char));
     arrB[i] = ptr;
     if(i % 50 == 49){
       j = i-49;
       while(j<=i){
         free(arrB[j]);
         ++j;
       }
     }
     ++i;
   }

   gettimeofday(&end,NULL);

   arr[1][jk] = (end.tv_sec * 1000000 + end.tv_usec)  - (start.tv_sec * 1000000 + start.tv_usec);

//WORKLOAD B: COMPLETE :)


/* PART C: Randomly choose between a 1 byte malloc() or free()ing a 
 *         1 byte pointer. (do this until you have allocated 50 times) 
 *         
 *         - Keep track of each operation that you eventually malloc() 50 bytes, in total
 *            > if you have already allocated 50 times, 
 *            disregard the random and just free() on each iteration
 *           
 *         - keep track of each operation so that you eventually free() all pointers 
 *            > (don't allow a free() if you have no pointers to free().   */

    gettimeofday(&start, NULL);

    time_t t;
  
    int mcount = 0;
    int fcount = 0;

    srand((unsigned)time(&t));
  
    int n;
  
    char *arrC[50];

    i = 0;

    while(mcount < 50){
      n = rand() % 2;
      i++;
      if(n == 1){
        char* ptr = malloc(sizeof(char));
        arrC[mcount] = ptr;
        ++mcount;
      }

      else{ 
        free(arrC[fcount]);
        ++fcount;
      }
    }
  
    i = 0;
    while(i<50){
      free(arrC[i]);
      i++;
    }

    gettimeofday(&end,NULL);

    arr[2][jk] = (end.tv_sec * 1000000 + end.tv_usec)  - (start.tv_sec * 1000000 + start.tv_usec);

/* PART D: Randomly choose between a randomly-sized malloc() or 
 *         free()ing a pointer (do this many times)
 * 	    - Keep track of each malloc so that all mallocs do not 
 * 	      exceed your total memory capacity
 * 	    - Keep track of each operation so that you eventually 
 * 	      malloc() 50 times
 * 	    - Keep track of each operation so that you eventually 
 * 	      free() all pointers
 * 	    - Choose a random allocation size between 1 and 64 bytes   */


  /* This program uses 5 bytes of metadata per malloc. In this workoad, the maximum allocation
 *  allocation request is 64 bytes. Let's just say randomly, I have to malloc 64 bytes 50 times,
 *  with no calls to free(). Then I would have to allocate (64+5) * 50 bytes of memory.
 *  (64+5) * 50 = 3450. So in the worst case scenario, I would still have 646 bytes left in memory.
 *  For that reason, we do not need to keep track of total memory capacity. */

 
    gettimeofday(&start, NULL);

    mcount = 0;
    fcount = 0;
 
    int capacity = 4096;

  
    int q;

    char *arrD[50];

    while(mcount < 50){
      n = rand() % 2;
      q = rand() % 65;
 
      if(n == 1){
        char* ptr = malloc(q*sizeof(char));
        arrD[mcount] = ptr;
        ++mcount;
        capacity += -(q+5); // 5 for metadata per malloc
    }

      else{ 
        free(arrD[fcount]);
      }
    
      if(capacity < 0){
      //printf("ooh im shaking in my tuuubessoocckks\n");
        break;
      }
      
    }
  
    i = 0;
    while(i<50){
      free(arrD[i]);
      i++;
    }

    gettimeofday(&end,NULL);

    arr[3][jk] = (end.tv_sec * 1000000 + end.tv_usec)  - (start.tv_sec * 1000000 + start.tv_usec);

/* PART E: defragment tester: malloc 100 bytes 40 times
 * 			      free them 
 * 			      malloc 2000 bytes
 * 			      free it */


    gettimeofday(&start, NULL);
    
    char *arrE[40];
    i = 0;
 

    while(i < 40){
      char* ptr = malloc(100*sizeof(char));
      arrE[i] = ptr;
      ++i;
    }

    while(i < 40){
      free(arrE[i]);
      ++i;
    }

    char* blingblaowwow = malloc(2000*sizeof(char));
    free(blingblaowwow);

    
    gettimeofday(&end,NULL);

    arr[4][jk] = (end.tv_sec * 1000000 + end.tv_usec)  - (start.tv_sec * 1000000 + start.tv_usec);

/* PART F: test the capacity: request to malloc 36 bytes
 *         100 times. There is 5 bytes of metadata per 
 *         allocation. So, that means that it has to 
 *         allocate 41 bytes 100 times. There is not enough space... 
 *         Let's see what happens */

    gettimeofday(&start, NULL);
    
    i = 0;
  
    char *arrF[100];

    while(i < 100){  
      char* ptr = malloc(36*sizeof(char));
      arrF[i] = ptr;
      ++i;
    }
  
    
    gettimeofday(&end,NULL);

    arr[5][jk] = (end.tv_sec * 1000000 + end.tv_usec)  - (start.tv_sec * 1000000 + start.tv_usec);
  


    ++jk;
  }


/* get the averages */
int ij = 0;
jk = 0;

long int final[6];
long int tempsum = 0; 


for(ij = 0; ij < 6; ij++){
  for(jk = 0; jk < 100; jk++){
    tempsum += arr[ij][jk];
  }
  final[ij] = (tempsum/100);
  tempsum = 0;
} 

ij = 0;
while (ij < 6){
  printf("{ %ld microseconds }, ", final[ij]);
  ++ij;
}

printf("\n");
 
return 0;

}

