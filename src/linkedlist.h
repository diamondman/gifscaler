#ifndef LINKEDLIST_STRUCTS
#define LINKEDLIST_STRUCTS

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
#endif

#ifndef LINKEDLIST_FUNCTIONS
#define LINKEDLIST_FUNCTIONS
void addLinkedListItem(LinkedList *list, LinkedListItem *item);
LinkedListItem* addNewLinkedListItem(LinkedList *list);
void disposeLinkedList(LinkedList *list);
#endif
