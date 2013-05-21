#ifndef LINKEDLIST
#define LINKEDLIST

typedef struct LinkedListItem_t LinkedListItem;
struct LinkedListItem_t{
  void* data;
  LinkedListItem* next;
};

typedef struct LinkedList_t{
  LinkedListItem *first;
  LinkedListItem *last;
  long length;
}LinkedList;

void addLinkedListItem(LinkedList *list, LinkedListItem *item);
LinkedListItem* addNewLinkedListItem(LinkedList *list);
void disposeLinkedList(LinkedList *list);
#endif
