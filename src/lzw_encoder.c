#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "linkedlist.h"
#include "lzw.h"

//#define DEBUG 1

uint16_t reset_rules(LZWDecoderData *ld){
  uint16_t next_rule_id;
  for(next_rule_id=0; next_rule_id < ld->initial_dictionary_size; next_rule_id++){
    LinkedListItem *dentry = addNewLinkedListItem(&ld->dictionary);
    LZWTreeEntry *treeentry = malloc(sizeof(LZWTreeEntry));
    treeentry->children = malloc(sizeof(LinkedList));
    memset(treeentry->children, 0, sizeof(LinkedList));
    treeentry->data = next_rule_id;
    treeentry->code_number = next_rule_id;
    treeentry->level=0;
    dentry->data = (int*)treeentry;
    #ifdef DEBUG
    printf("Added %d: %d\n", next_rule_id, next_rule_id);
    #endif
  }
  return next_rule_id;
}

void outputCode(LZWDecoderData *ld, uint16_t code){
  ld->tmpcodebyte|=(code<<ld->bits_used);
  ld->bits_used+=ld->LZWmin;

  #ifdef DEBUG
  printf("CodeStream: \e[1;32m%d (%d bits)\e[0m\n", code, ld->LZWmin);
  printf("Bits Used: %d\n", ld->bits_used);
  #endif
  //printf("\e[1;32m%d ", code);

  while(ld->bits_used>=8||(ld->bits_used>0&&code==ld->end_code_number)){
    uint8_t outbyte = ld->tmpcodebyte&0xff;
    ld->tmpcodebyte = ld->tmpcodebyte>>8;
    ld->bits_used-=8;
    #ifdef DEBUG
    printf("Output Byte: %02x\n", outbyte);
    #endif
    ld->output[ld->output_active_length] = outbyte;
    ld->output_active_length++;
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

void lzw_encode_initialize(LZWDecoderData *ld, int initial_lzw_dictionary_size){
  int input_length = 100;
  memset(ld,0,sizeof(LZWDecoderData));
  ld->initial_dictionary_size = initial_lzw_dictionary_size;
  ld->output = malloc(sizeof(uint8_t)*input_length);
  ld->output_size=input_length;
  ld->next_rule_id = reset_rules(ld);
  ld->clear_code_number = ld->next_rule_id;
  ld->end_code_number = ld->next_rule_id+1;
  ld->next_rule_id+=2;
  
  ld->LZWmin = (int)ceil(log2(ld->next_rule_id));
  ld->start_index = 0;
  ld->resolved_indexes = 0;
  ld->dictionary_branch = &ld->dictionary;
  ld->last_matched_rule = NULL;
}

void lzw_encode(LZWDecoderData *ld, uint8_t *input, int input_length){
  outputCode(ld, ld->clear_code_number);
  for(uint8_t end_index=1; end_index<=input_length; end_index++){
    #ifdef DEBUG
    printf("\nStarting Search from %d to %d: ", ld->start_index, end_index);
    for(int input_byte_index=0; input_byte_index<input_length; input_byte_index++){
      if(input_byte_index==ld->start_index)printf("\e[1;34m");
      printf("%d ",input[input_byte_index]);
      if(input_byte_index+1==end_index)printf("\e[0m");
    }
    printf("\e[0m\n");
    for(int j=0; j<end_index-ld->start_index; j++) printf("%d", input[ld->start_index+j]);
    printf("\n");
    #endif

    LinkedListItem *item = ld->dictionary_branch->first;
    LinkedListItem *nextitem;
    for(int subselect_index=ld->resolved_indexes; subselect_index<end_index-ld->start_index; subselect_index++){
      #ifdef DEBUG
      printf("Checking symbol #%d (%d)\n", ld->start_index+subselect_index, input[ld->start_index+subselect_index]);
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
	if(treeentry->data==input[ld->start_index+subselect_index]){
	  #ifdef DEBUG
	  printf("FOUND level %d rule %d; id %d\n", treeentry->level, treeentry->data, treeentry->code_number);
	  #endif
	  found=1;
	  ld->resolved_indexes++;
	  ld->dictionary_branch=(LinkedList *)treeentry->children;
	  ld->last_matched_rule = treeentry;
	  break;
	}
	nextitem = item->next;
	item=nextitem;
      }while(item!=0);

      if(found!=1){
	#ifdef DEBUG
	printf("\e[1;33mADDING RULE id: %d; ", ld->next_rule_id);
	for(int rule_data_index=0; rule_data_index<end_index-ld->start_index; rule_data_index++) printf("%d", input[ld->start_index+rule_data_index]);
	printf("\e[0m\n");
	#endif
	outputCode(ld, ld->last_matched_rule->code_number);
	LinkedListItem *new_rule_entry = addNewLinkedListItem(ld->dictionary_branch);
	LZWTreeEntry *treeentry = malloc(sizeof(LZWTreeEntry));
	new_rule_entry->data = (void*)treeentry;
	treeentry->children = malloc(sizeof(LinkedList));
	memset(treeentry->children, 0, sizeof(LinkedList));
	treeentry->data = input[ld->start_index+(end_index-ld->start_index)-1];
	treeentry->code_number = ld->next_rule_id;
	treeentry->level = ld->last_matched_rule->level+1;
       	ld->dictionary_branch = &ld->dictionary;
	ld->start_index=end_index-1;
	end_index--;
	ld->resolved_indexes = 0;
	ld->last_matched_rule=NULL;
	
	ld->next_rule_id++;
	int nextLZWmin = (int)ceil(log2(ld->next_rule_id));
	if(nextLZWmin>12){
	  outputCode(ld, ld->clear_code_number);
	  freeRuleDictionary(&ld->dictionary);
	  ld->next_rule_id = reset_rules(ld);
	  ld->clear_code_number = ld->next_rule_id;
	  ld->end_code_number = ld->next_rule_id+1;
	  ld->next_rule_id+=2;
	  ld->LZWmin = (int)ceil(log2(ld->next_rule_id));
	}else
	  ld->LZWmin = nextLZWmin;
	  
	break;
      }
    }
  }
  outputCode(ld, ld->last_matched_rule->code_number);
  outputCode(ld, ld->end_code_number);
}

void lzw_encode_free(LZWDecoderData *ld){
  freeRuleDictionary(&(ld->dictionary));
}
