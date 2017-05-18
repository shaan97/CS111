#include "SortedList.h"
#include <stdio.h>
#include <assert.h>

int main(){
  SortedList_t head;
  head.prev = NULL;
  head.key = NULL;
  head.next = NULL;
  
  SortedListElement_t nodes[10];
  int i;
  for(i = 0; i < 10; i++){
    char* arr = (char*) malloc(sizeof(char)*2);
    arr[1] = 0;
    arr[0] = 'a' + i;
    nodes[i].key = arr;
    printf("%c\n", (nodes[i].key)[0]);
    SortedList_insert(&head, &(nodes[i]));
    assert(SortedList_lookup(&head, arr) != NULL);
    assert(SortedList_length(&head) == i + 1);
  }

  SortedListElement_t * itr = &head;
  while(itr = itr->next)
    printf("%s\n", itr->key);
  for(i = 0; i < 10; i++){
    assert(!SortedList_delete(&(nodes[i])));
    printf("%c\n", (nodes[i].key)[0]);
    assert(SortedList_lookup(&head, nodes[i].key) == NULL);
    assert(SortedList_length(&head) == 9 - i);
  }
}
