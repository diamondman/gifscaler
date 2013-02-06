#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "linkedlist.h"

void addLinkedListItem(LinkedList *list, LinkedListItem *item){
  if(list->length==0){
    //printf("Adding First Item\n");
    list->first=item; 
  }else list->last->next=item;
  list->last=item;
  list->last->next=NULL;
  list->length++;
  //printf("Length: %d\n", list->length);
}

LinkedListItem* addNewLinkedListItem(LinkedList *list){
  LinkedListItem *item = malloc(sizeof(LinkedListItem));
  memset(item, 0, sizeof(LinkedListItem));
  addLinkedListItem(list, item);
  return item;
}

void disposeLinkedList(LinkedList *list){
  if(list->length>0){
    LinkedListItem *item = list->first;
    LinkedListItem *nextitem;
    do{
      nextitem = item->next;
      free(item);
      item=nextitem;
    }while(item!=0);
    memset(list, 0, sizeof(LinkedList));
    list->length=0;
    list->last=NULL;
    list->first=NULL;
  }
}
