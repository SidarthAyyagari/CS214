Author Name: Sidarth Ayyagari
RUID: 164007024
Class: Systems Programming (CS214)
Asst0

-------
SUMMARY
-------

This program is named "pointersorter.c". The program takes exactly one input string, and parses it using non-alphabetic characters as separators. Each continuous sequence of alphabetic characters is called a token. Each token is put into a node, and then each node is inserted into a linked list. I chose to use a linked list to hold the tokens, because the total amount of tokens in the input string is unknown until the string has already been parsed. I thought it would be easier to create a linked list, and hold on to the end of it and keep appending the linked list as each new token was obtained. After the end of the input string has been reached, there exists an unsorted linked list of nodes containing every token in the input string. Then the MergeSort function is called. The MergeSort function takes the head of the linked list as a parameter and applies the mergesort algorithm to sort the linked list of tokens. The mergesort algorithm recursively divides the linked list into halves, until the individual members are isolated. Then they are compared using the stringCompare() function. I originally tried using strcmp() but that did not yield the exact type of sorting that this assignment required. So I looked at the strcmp code, and tweaked it to give me the proper output. After mergeSort splits the linked list recursively, the comparison part of the algorithm is my modified string compare function. This puts the nodes back together in descending alphabetical order, while treating capital letters as greater than lower case. The reason mergesort was chosen to sort the linked list is because it is the most efficient way to sort through the linked list. The time complexity of mergesort is O(n*log(n)). Plus, because a linked list was chosen, other sorting algorithms were harder to implement. The MergeSort function returns nothing, but after it has been called, the linked list has been completely sorted. Then I just loop through the sorted linked list and print each token on a new line, which is the output.


---------------------------
Before the main function...
---------------------------

STRUCTS: node... pretty standard stuff here
- A node structure is declared with 2 parameters: char* name, node* next. node_t->name will store a token as I parse the tokens from the input string.

HELPER FUNCTIONS: stringCompare(), SortedMerge(), fbsplit(), MergeSort()
- All of the helper functions are used in the sorting part of this program. stringCompare() is called in SortedMerge(); SortedMerge() and fbsplit() are called in MergeSort(); MergeSort() is called in the main function.

1. stringCompare(char* p1, char* p2):
   - I originally did not think I would need to write my own string comparing function, but I found that strcmp() did not work for our esired order. It would not be able to work when the tokens had capitalized letters. So I looked at the code for strcmp, and added in 2 new cases involving uppercase letters. If the current letter of string 1, c1, is upper case and the current letter of string 2, c2, is not uppercase, then the function should return the opposite of what it was returing in strcmp. It should return c2 - c1 in this case. Same in the opposite case. This gives capitalized letters greater value in the comparing process, which is exactly what we want.   

2. fbsplit(node_t* source, node_t** front, node_t** back)
   - This function is passed in a node_t* and two node_t**. The first is the starting point of the linked list, and the next two are just pointers which will be reassigned to other pointers at the end of the function. Two node_t* are declared (fast and slow). They are called fast and slow, because in the while loop, fast is incremented twice, so long as it doesn't step on a null, and slow is incremented only once. By the end of the loop, slow will be at the halfway point of the list and fast will be null. The reassignments of pointers at the end make sure that the list is split in half.  


3. SortedMerge(node_t *a, node_t b)
  - Sorted Merge is passed in two node pointers, each referring to the head of a sublist. Before anything, a node_t* result is intialized to null. If either a or b are null, then the other is returned. If both are not null, then the names are compared. I call stringCompare here, and if the int returned by stringCompare is positive, then SortedMerge recursively calls itself and passes in a->next and b as the two nodes to be compared. If, stringCompare(a->name, b->name) <0), then SortedMerge calls itself passing in a->next and b as parameters.   

4. MergeSort(node_t** headRef)
   - This function should be passed in a node_t**. In the main function, MergeSort() is called once and passed in &head. head is of type node_t*, so &head will be of type node_t**. First, we check if the linked list is of size 0 or 1, because then no operation needs to be done, so we just return. (Since this function is recursive, it will reach this point inevitably once it gets to the bottom recursion level.) Then it splits the list in half using fbsplit() and recursively calls itselg on the two sublists. Lastly, it sets the head equal to  the output of SortedMerge().


-------------
MAIN FUNCTION
-------------

- First, I check how may arguments were passed. If argc = 1, then no input string was given, so a usage string is printed. If argc > 2, then two many input strings have been given, so a usage string is printed. If argc = 2, then I catch the input string, and move on like business as usual.

- Before the loop, I create a node pointer for head, ptr, and prev. These will be the handles that I use to create the unsorted linked list. I then malloc the char* token with enough memory to hold any substring of argv[1]. (Remember, argv[1] is a substring of itself. It is possible that an input string can be passed where the whole input string is one token.) Then I do memset on the token, just to make sure we don't get any garbage from memory being carried over into the token. Lastly, I create char* temp and set it equal to token. temp is what will actually be used to keep building the token, while we keep char* token as a handle to the beginning of the token, so we can put it into the node later.

- Next, I create a char* lp and catch argv[1] in it. The goal is to loop through lp character by character. The loop bounds are set from 0 to the length of argv[1]. ALL SYSTEMS GO FOR THE BIG FOR LOOP!


For Loop:

- For every character in argv[1], a check is run to see if the current character is alphabetic or not. 

   - If it is, then that means it is either the beginning of our token, or somewhere in the middle. either way I want to catch it in temp, and increment temp.

   - If the current character is not alphabetic, then there are two possibilities. Either I have reached the end of a token, in which case I want to catch it and put it in a node to be inserted in the linked list.. Or, the token is empty, and I am just encountering the second, third, etc. non alphabetic character, in which case I don't want to do anything except move on using lp++. If the token variable is not empty, then we got something. So either it is the first to be inserted in the linked list, or it is not. If it is the first, then I malloc head, save the name, set next to null, and set prev to head. If head != NULL, then that means I already did this, so I just need to append this new node to the linked list. So I use ptr, malloc it, and set the name. But here I want to make sure that I connect the linked list in the right way. I set prev->next = ptr (ptr is the current node), and I set ptr->next = null, and then I set prev = ptr, so that the next time I append the linked list, I have a handle on the last node in the linked list.

  - I just increment lp, so we can move along the input string. 


Post-For loop:

- After I had tested up until this point, I found that if the input string ended with an alphabetic character, the last token did not get printed. I was a little heartbroken :(
- But I realized that the last token was in the variable token, it just didn't get a chance to get put in the linked list. So I just copied the code from what I did for "all the rest of the nodes" (see comment, lines 228-235 ) and I copied it outside of the for loop under the if condition checking if token was empty or not. It's basically like knocking on the door, and if something is in there, then you go in and grab it. If nobody's home, then you can just leave.


At this point I have an unsorted linked list containing all of the tokens. This is where MergeSort comes in.


MergeSort:

Within, the main, we just call MergeSort() and pass in the address of the head of the linked list. MergeSort() calls fbsplit(), and then calls itself on the two sublists. fbsplit() just splits the list in half. By the end of all of this, our poor little linked list has been cut up into tiny pieces. Then SortedMerge() is called, which puts everything back together. SortedMerge() merges the lists and uses stringCompare to order the sublists. Once SortedMerge() is finished, I get a sorted Linked List.

Then I start at the head, and visit every node in the linked list, print out its name with a newline, and return 0.

-----------
END OF MAIN
----------- 

