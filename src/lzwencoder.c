#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "linkedlist.h"

//#define DEBUG 1

typedef struct LZWTreeEntry_t{
  int count;
  uint8_t data;
  uint16_t code_number;
  struct LinkedList *children;
  int level;
} LZWTreeEntry;

uint16_t reset_rules(LinkedList *dictionary, uint8_t *color_list);

int input_length = 100;
uint8_t input[100] = {1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,0,0,0,0,2,2,2,1,1,1,0,0,0,0,2,2,2,2,2,2,0,0,0,0,1,1,1,2,2,2,0,0,0,0,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1};

int color_list_size = 4;
uint8_t color_list[4] = {0,1,2,3};//{'W','R','B','L'};

int clear_code_number;
int end_code_number;

int bits_used = 0;
uint32_t tmpcodebyte = 0;
uint8_t *output;
int output_active_length=0;
int output_size=0;
void outputCode(uint16_t code, uint8_t bit_length){
  tmpcodebyte|=(code<<bits_used);
  bits_used+=bit_length;

  #ifdef DEBUG
  printf("CodeStream: \e[1;32m%d (%d bits)\e[0m\n", code, bit_length);
  printf("Bits Used: %d\n", bits_used);
  #endif

  while(bits_used>=8||(bits_used>0&&code==end_code_number)){
    uint8_t outbyte = tmpcodebyte&0xff;
    tmpcodebyte = tmpcodebyte>>8;
    bits_used-=8;
    #ifdef DEBUG
    printf("Output Byte: %02x\n", outbyte);
    #endif
    output[output_active_length] = outbyte;
    output_active_length++;
  }
}

void freeRuleDictionary(LinkedList *dictionary){
  LinkedListItem *item = dictionary->first;
  LZWTreeEntry *lzwentry;;
  while(item){
    lzwentry = (LZWTreeEntry *)item->data;
    if(lzwentry->children){
      freeRuleDictionary((LinkedList *)lzwentry->children);
      free(lzwentry->children);
      free(lzwentry);
    }
    item=item->next;
  }
  disposeLinkedList(dictionary);
}

int main(int argc, char* argv[]){
  output = malloc(sizeof(uint8_t)*input_length);
  output_size=input_length;

  int deepest_level = 0;
  #ifdef DEBUG
  printf("TEST: %d\n", 7>>3);
  #endif
  LinkedList dictionary;
  memset(&dictionary, 0, sizeof(LinkedList));
  
  uint16_t current_rule_id = reset_rules(&dictionary, (uint8_t*)&color_list);
  clear_code_number = current_rule_id;
  end_code_number = current_rule_id+1;
  current_rule_id+=2;
  
  int LZWmin = (int)ceil(log2(current_rule_id));

  int start_index = 0;
  int resolved_indexes = 0;
  LinkedList *dictionary_branch = &dictionary;
  LZWTreeEntry *last_matched_rule = NULL;
  uint16_t i;
  outputCode(clear_code_number, LZWmin);
  for(i=1; i<=input_length; i++){
    #ifdef DEBUG
    printf("\nStarting Search from %d to %d: ", start_index, i);
    int k;
    for(k=0; k<input_length; k++){
      if(k==start_index)printf("\e[1;34m");
      printf("%d ",input[k]);
      if(k+1==i)printf("\e[0m");
    }
    printf("\e[0m\n");
    #endif

    int j;
    #ifdef DEBUG
    for(j=0; j<i-start_index; j++) printf("%d", input[start_index+j]);
    printf("\n");
    #endif
    LinkedListItem *item = dictionary_branch->first;
    LinkedListItem *nextitem;

    for(j=resolved_indexes; j<i-start_index; j++){
      #ifdef DEBUG
      printf("Checking symbol #%d (%d)\n", start_index+j, input[start_index+j]);
      #endif
      uint8_t found = 0;
      do{
	if(item==NULL){
	  #ifdef DEBUG
	  printf("NO CHILDREN FOUND\n");
	  #endif
	  break;
	}
	LZWTreeEntry *treeentry = (LZWTreeEntry*)item->data;
        #ifdef DEBUG
	printf("checking against level %d, rule id: %d value: %d\n", treeentry->level, treeentry->code_number, treeentry->data);
	#endif
	if(treeentry->data==input[start_index+j]){
	  #ifdef DEBUG
	  printf("FOUND level %d rule %d; id %d\n", treeentry->level, treeentry->data, treeentry->code_number);
	  #endif
	  found=1;
	  resolved_indexes++;
	  dictionary_branch=(LinkedList *)treeentry->children;
	  last_matched_rule = treeentry;
	  break;
	}
	nextitem = item->next;
	item=nextitem;
      }while(item!=0);
      if(found!=1){
	#ifdef DEBUG
	printf("\e[1;33mADDING RULE id: %d; ", current_rule_id);
	for(j=0; j<i-start_index; j++) printf("%d", input[start_index+j]);
	printf("\e[0m\n");
	#endif
	outputCode(last_matched_rule->code_number, LZWmin);
	LinkedListItem *new_rule_entry = addNewLinkedListItem(dictionary_branch);
	LZWTreeEntry *treeentry = malloc(sizeof(LZWTreeEntry));
	new_rule_entry->data = (void*)treeentry;
	treeentry->children = malloc(sizeof(LinkedList));
	memset(treeentry->children, 0, sizeof(LinkedList));
	treeentry->data = input[start_index+j-1];
	treeentry->code_number = current_rule_id;
	treeentry->level = last_matched_rule->level+1;
	if(treeentry->level>deepest_level)deepest_level=treeentry->level;
	dictionary_branch = &dictionary;
	start_index=i-1;
	i--;
	resolved_indexes = 0;
	last_matched_rule=NULL;
	
	current_rule_id++;
	int nextLZWmin = (int)ceil(log2(current_rule_id));
	if(nextLZWmin>12){
	  outputCode(clear_code_number, LZWmin);
	  freeRuleDictionary(&dictionary);
	  current_rule_id = reset_rules(&dictionary, (uint8_t*)&color_list);
	  clear_code_number = current_rule_id;
	  end_code_number = current_rule_id+1;
	  current_rule_id+=2;
	  LZWmin = (int)ceil(log2(current_rule_id));
	}else
	  LZWmin = nextLZWmin;
	  
	break;
      }
    }
  }
  outputCode(last_matched_rule->code_number, LZWmin);
  outputCode(end_code_number, LZWmin);
  
  freeRuleDictionary(&dictionary);

  int m;
  for(m=0; m<output_active_length; m++) printf("%02x ", output[m]);
  printf("\n");

  free(output);
  return 0;
}

uint16_t reset_rules(LinkedList *dictionary, uint8_t *color_list){
  uint16_t current_rule_id;
  for(current_rule_id=0; current_rule_id < color_list_size; current_rule_id++){
    LinkedListItem *dentry = addNewLinkedListItem(dictionary);
    LZWTreeEntry *treeentry = malloc(sizeof(LZWTreeEntry));
    treeentry->children = malloc(sizeof(LinkedList));
    memset(treeentry->children, 0, sizeof(LinkedList));
    treeentry->data = color_list[current_rule_id];
    treeentry->code_number = current_rule_id;
    treeentry->level=0;
    dentry->data = (int*)treeentry;
    #ifdef DEBUG
    printf("Added %d: %d\n", current_rule_id, color_list[current_rule_id]);
    #endif
  }
  return current_rule_id;
}
