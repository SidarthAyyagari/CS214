#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>




//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
///////                                                                   ////////
///////			     FINAL VERSION:                               ////////                
///////                                                                   ////////                  
///////      What it can do: 	          				  ////////
///////       				                                  ////////		     
///////            This program can take 1 argument and cut it up         ////////
///////            into tokens. This program also creates a Linked        ////////
///////            List structure which can hold all of the values.       ////////
///////            This program then takes that linked list and sorts     ////////
///////            it in descending alphabetical order, however           //////// 
///////            capitalized words take precedence and come first       ////////
///////            in the list. It does this using mergesort, which has   ////////
///////            big O of n*log(n). This is an efficient way to sort    ////////
///////            this linked list.                                      ////////		     
///////           		                                          ////////		     
///////                		                                          ////////		     
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////
/*///////////  FUNCTIONS  ///////////// */             
//////////////////////////////////////////

/* declare node struct */
typedef struct node{
  char* name;
  struct node *next;
}node_t;

/* declare functions */
/*struct node_t* SortedMerge(struct node_t* a, struct node_t* b);
void fbsplit(struct node* src, struct node** front, struct node** back);
*/

/* STRING COMPARE FUNCTION */
int stringCompare(char* p1, char* p2){

  const unsigned char *s1 = (const unsigned char *) p1;
  const unsigned char *s2 = (const unsigned char *) p2;
  unsigned char c1, c2;

  do
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0')
        return c1 - c2;
      if(isupper(c1) && !isupper(c2)){
        return c2 - c1;
      }
      else if(!isupper(c1) && isupper(c2)){
        return c2 - c1;
      }
    } while (c1 == c2);

  return c1 - c2;
}


/* merge sort helper function */
node_t* SortedMerge(node_t *a, node_t *b){

  node_t* result = NULL;

  /* base cases */

  if(a == NULL){
    return b;
  }

  else if(b == NULL){
    return a;
  }

/* strcmp... BAAAADDDDDD. stringCompare... GOOOOOOOODDD!!! 
 * strcmp will print in descending or ascending, but it does
 * not take capitalization as a factor, which is something 
 * this code should do. stringCompare just adds the uppercase 
 * testcase and does a pretty good job.                 */

  if (stringCompare(a->name, b->name) >= 0){
    result = a;
    result->next = SortedMerge(a->next, b);
  }
  else{
    result = b;
    result->next = SortedMerge(a, b->next);
  }

  return result;
}


/* split linked list into two */
void fbsplit(node_t* source, node_t** front, node_t** back){

  node_t* fast;
  node_t* slow;

  slow = source;
  fast = source->next;


  while(fast != NULL){

    fast = fast->next;

    if(fast != NULL){
      slow = slow->next;
      fast = fast->next;
    }
  }

  *front = source;
  *back = slow->next;
  slow->next = NULL;

  return;
}


/* merge sort function */
void MergeSort(node_t** headRef){
  node_t* head = *headRef;
  node_t* a;
  node_t* b;


  /* if linked list length = 0 or 1 */
  if((head == NULL) || (head->next == NULL)){
    return;
  }

  /* Split head into 'a' and 'b' sublists */
  fbsplit(head, &a, &b);

  /* Recursively sort the sublists */
  MergeSort(&a);
  MergeSort(&b);

  /* Put it back together*/
  *headRef = SortedMerge(a,b);
  
  return;
}


//////////////////////////////////////////
//////////////////////////////////////////
/*//////////////  MAIN  /////////////// */             
//////////////////////////////////////////
//////////////////////////////////////////

int main(int argc, char* argv[]){
  

  // Wrong inputs
  
  if(argc == 1 || argc > 2) {
    printf("Fatal Error: Found %d arguments, was expecting 1\n", argc - 1);
    printf("Usage: ./pointersorter 'input string'\n");

    return 0;
  }

  if(strlen(argv[1]) == 0){
    return 0;
  }


  ////////////////////////////
  /*   BUSINESS AS USUAL:   */
  ////////////////////////////



  // Create head and ptr nodes
  node_t *head = NULL;
  node_t *ptr = NULL;
  node_t *prev = NULL; 

  // make sure you are able to catch every 'token' 
  char *token = (char*)malloc(sizeof(char)*strlen(argv[1]));
  memset(token, '\0', strlen(argv[1]));  
  char *temp = token;

  /* SET UP LOOP  */

  // Catch argv[1] in lp
  char *lp = argv[1];

  // loop bounds
  int i = 0;
  int end = strlen(argv[1]);


  /* HERE WE GOOOOO!!!! loop thru string and gather tokens  */
  for(i =0; i<end; i++){ 
    if(isalpha(*lp)){
      *temp = *lp;
      temp++;
    }
    else{
      /* CATCH THE TOKEN */
      if(token[0]!= '\0'){
        
        // For the first node
        if(head == NULL){
          head = malloc(sizeof(node_t));
          head->name = token;
          head->next = NULL;
          prev = head;
        }
        
        // all the rest of the nodes
        else{
          ptr = malloc(sizeof(node_t));
          ptr->name = token;
          prev->next = ptr;
	  ptr->next = NULL;
          prev = ptr;
	}

        // reset token variable
        token = (char*)malloc(sizeof(char)*strlen(argv[1]));
        memset(token, '\0', strlen(argv[1]));  
        temp = token;
      }
     
       //MOVE ON THERE IS NO TOKEN
      /* else{
         printf("DO NOTHING\n");
       } */    
    }
    lp++;
  }
  

  
  //last token, IF NECCESSARY:
  if(token[0] != '\0'){
    ptr = malloc(sizeof(node_t));
    ptr->name = token;
    
    if(prev != NULL){
      prev->next = ptr;
      ptr->next = NULL;
      prev = ptr;
    }
   
    else{ 
      head = malloc(sizeof(node_t));
      head->name = token;
      head->next = NULL;
    }

  }
  
/*else{
    printf("DO NOTHING\n");
  } */
 
  
  /* See if u got the linked list u want */
  ptr = head;

/*
  while(ptr != NULL){
    printf("%s\n", ptr->name);
    ptr = ptr->next;
  }
  */
  ////////////////////////////
  /*  SORTING (MERGESORT):  */
  ////////////////////////////
   
  MergeSort(&head);
  ptr = head;  
 
  while(ptr != NULL){
    printf("%s\n", ptr->name);
    ptr = ptr->next;
  }
  
/* yeaaaaaa u alredy kno who it isssss */

  return 0;
}
