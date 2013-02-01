#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "linkedlist.h"

void addLinkedListItem(struct LinkedList *list, struct LinkedListItem *item){
  if(list->length==0){
    //printf("Adding First Item\n");
    list->first=item; 
  }else list->last->next=item;
  list->last=item;
  list->last->next=NULL;
  list->length++;
  //printf("Length: %d\n", list->length);
}

struct LinkedListItem* addNewLinkedListItem(struct LinkedList *list){
  struct LinkedListItem *item = malloc(sizeof(struct LinkedListItem));
  addLinkedListItem(list, item);
  return item;
}

void disposeLinkedList(struct LinkedList *list){
  if(list->length>0){
    struct LinkedListItem *item = list->first;
    struct LinkedListItem *nextitem;
    do{
      nextitem = item->next;
      free(item);
      item=nextitem;
    }while(item!=0);
    list->length=0;
    list->last=NULL;
    list->first=NULL;
  }
}
