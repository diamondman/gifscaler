#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "linkedlist.h"

typedef struct LZWTreeEntry_t{
  int count;
  uint8_t data;
  uint16_t code_number;
  struct LinkedList *children;
  int level;
} LZWTreeEntry;

void outputCode(uint16_t code, uint8_t bit_length){
  printf("Code %d\n", code);
}

int input_length = 100;
uint8_t input[100] = {1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,0,0,0,0,2,2,2,1,1,1,0,0,0,0,2,2,2,2,2,2,0,0,0,0,1,1,1,2,2,2,0,0,0,0,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1};

int color_list_size = 4;
uint8_t color_list[4] = {0,1,2,3};//{'W','R','B','L'};

int main(int argc, char* argv[]){
  LinkedList dictionary;
  memset(&dictionary, 0, sizeof(LinkedList));
  
  uint16_t current_rule_id;
  for(current_rule_id=0; current_rule_id < color_list_size; current_rule_id++){
    LinkedListItem *dentry = addNewLinkedListItem(&dictionary);
    LZWTreeEntry *treeentry = malloc(sizeof(LZWTreeEntry));
    treeentry->children = malloc(sizeof(LinkedList));
    treeentry->data = color_list[current_rule_id];
    treeentry->code_number = current_rule_id;
    treeentry->level=0;
    dentry->data = (int*)treeentry;
    printf("Added %d: %d\n", current_rule_id, color_list[current_rule_id]);
  }
  int clear_code_number = current_rule_id;
  int end_code_number = current_rule_id+1;
  current_rule_id+=2;

  uint16_t i;
  int start_index = 0;
  int resolved_indexes = 0;
  LinkedList *dictionary_branch = &dictionary;
  LZWTreeEntry *last_matched_rule = NULL;
  uint8_t clear = 0;
  printf("CodeStream: %d\n", clear_code_number);
  for(i=1; i<=input_length; i++){
    /*if(clear){
      printf("CLEARING DICTIONARY LEVEL\n")
      dictionary_branch = &dictionary;
      clear=0;
      }*/
    printf("\nStarting Search from %d to %d: ", start_index, i);
    int k;
    printf("Values: ");
    for(k=0; k<input_length; k++){
      if(k==start_index)printf("\e[1;34m");
      printf("%d ",input[k]);
      if(k+1==i)printf("\e[0m");
    }
    printf("\e[0m\n");

    int j;
    for(j=0; j<i-start_index; j++) printf("%d", input[start_index+j]);
    printf("\n");
    LinkedListItem *item = dictionary_branch->first;
    LinkedListItem *nextitem;

    for(j=resolved_indexes; j<i-start_index; j++){
      printf("Checking symbol #%d (%d)\n", start_index+j, input[start_index+j]);
      uint8_t found = 0;
      do{
	if(item==NULL){
	  printf("NO CHILDREN FOUND\n");
	  break;
	}
	//printf("DEBUG1 item: %d;\n", item);
	//printf("DEBUG1 item->data: %d\n", item->data);
	LZWTreeEntry *treeentry = (LZWTreeEntry*)item->data;
	//printf("DEBUG treeentry: %d; start_index: %d\n", treeentry, start_index);
	printf("checking against level %d, rule id: %d value: %d\n", treeentry->level, treeentry->code_number, treeentry->data);
	if(treeentry->data==input[start_index+j]){
	  printf("FOUND level %d rule %d; id %d\n", treeentry->level, treeentry->data, treeentry->code_number);
	  found=1;
	  resolved_indexes++;
	  dictionary_branch=treeentry->children;
	  last_matched_rule = treeentry;
	  break;
	}
	//printf("MISS\n");
	nextitem = item->next;
	item=nextitem;
      }while(item!=0);
      if(found!=1){
	//outputCode();
	printf("ADDING RULE id: %d; ", current_rule_id);
	for(j=0; j<i-start_index; j++) printf("%d", input[start_index+j]);
	printf("\n");
	printf("CodeStream: %d\n", last_matched_rule->code_number);
	LinkedListItem *new_rule_entry = addNewLinkedListItem(dictionary_branch);
	LZWTreeEntry *treeentry = malloc(sizeof(LZWTreeEntry));
	new_rule_entry->data = treeentry;
	treeentry->children = malloc(sizeof(LinkedList));
	treeentry->data = input[start_index+j-1];
	treeentry->code_number = current_rule_id;
	treeentry->level = last_matched_rule->level+1;
	current_rule_id++;
	dictionary_branch = &dictionary;
	start_index=i-1;//(i-start_index)-(last_matched_rule->level+1);
	i--;
	resolved_indexes = 0;
	last_matched_rule=NULL;
	break;
      }
    }
  }
  printf("CodeStream: %d\n", last_matched_rule->code_number);
  printf("CodeStream: %d\n", end_code_number);
  
  return 0;
}
