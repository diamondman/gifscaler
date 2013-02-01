
#ifndef LINKEDLIST_STRUCTS
#define LINKEDLIST_STRUCTS
struct LinkedListItem{
  int* data;
  struct LinkedListItem* next;
};

struct LinkedList{
  struct LinkedListItem *first;
  struct LinkedListItem *last;
  long length;
};
#endif

#ifndef LINKEDLIST_FUNCTIONS
#define LINKEDLIST_FUNCTIONS
void addLinkedListItem(struct LinkedList *list, struct LinkedListItem *item);
struct LinkedListItem* addNewLinkedListItem(struct LinkedList *list);
void disposeLinkedList(struct LinkedList *list);
#endif
