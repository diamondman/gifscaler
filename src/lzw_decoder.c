#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "lzw.h"

#define DECODE_BUFFER_START_SIZE malloc(sizeof(uint8_t)*1024*1024)

int process_code(LZWDecoderData *ed);

void lzw_decode_free(LZWDecoderData *ed, uint8_t free_result){
  for(int i=0; i<ed->number_of_codes; i++){
    printf("Clearing %d/%d\n", i, ed->number_of_codes-1);
    free(ed->codes[i].data);
  }
  if(free_result) free(ed->output_indexes);
}

void reset_code_list(LZWDecoderData *ed){
#ifdef DEBUG
  printf("Resetting Code List!\n");
#endif
  printf("HAS BEEN CLEARED YET: %d\n", ed->has_been_cleared);
  ed->last_code=-1;
  for(int i=0; i<ed->number_of_codes; i++){
    free(ed->codes[i].data);
    printf("Clearing %d/%d\n", i, ed->number_of_codes-1);
  }
  ed->number_of_codes = 0;
  for(int i=0; i<ed->initial_dictionary_size; i++){
    ed->codes[i].length=1;
    ed->codes[i].data = malloc(sizeof(uint8_t)*ed->codes[i].length);
    ed->codes[i].data[0] = ed->number_of_codes;
    ed->number_of_codes++;
    printf("Initializing %d/%d\n", i, ed->initial_dictionary_size+1);
  }

  ed->clear_code_number = ed->number_of_codes++;
  printf("Initializing %d (CLEAR)\n", ed->clear_code_number);
  ed->end_code_number = ed->number_of_codes++;
  printf("Initializing %d (END)\n", ed->end_code_number);

  ed->LZWmin = (int)ceil(log2(ed->number_of_codes));
  //ed->has_been_cleared = 1;
#ifdef DEBUG
  printf("New LZWmin after clear: %d\n", ed->LZWmin);
#endif
}

void clear_codes(LZWDecoderData *ed){
  memset(&ed->codes, 0, (4096)*sizeof(LZWDecoderEntry));
}

void lzw_decode_initialize(LZWDecoderData *ed, int initial_dictionary_size, int decode_buffer_start_size){
  memset(ed, 0, sizeof(LZWDecoderData));
  ed->output_indexes = DECODE_BUFFER_START_SIZE;
  ed->initial_dictionary_size = initial_dictionary_size;
  reset_code_list(ed);
}

int lzw_decode(LZWDecoderData *ed, uint8_t *source, int source_length){
  if(ed->finished) return -1;
  if(source_length<1) return -2;
  
  uint8_t rawbyte;
  for(int byteindex=0; byteindex<source_length; byteindex++){
    rawbyte = source[byteindex];
    ed->current_bit=0;
#ifdef DEBUG
    printf("\e[1;34mraw byte: 0x%02x\e[0m\n", rawbyte);
#endif

    while(ed->current_bit<8){
      int code_length = ed->LZWmin;
      int masklength = (code_length-ed->current_code_length)>(8-ed->current_bit)?(8-ed->current_bit):(code_length-ed->current_code_length);
      int mask = (2 << (masklength-1)) -1;
      int code_piece = (rawbyte & mask) << ed->current_code_length;
#ifdef DEBUG
      printf("Piece: %02x (%d bits)\n", code_piece, masklength);
#endif
      ed->current_code_length += masklength;
      rawbyte = rawbyte >> masklength;
      ed->current_bit += masklength;
      ed->code = ed->code | code_piece;
      ed->code_subsection_count++;

      if(ed->current_code_length == code_length){
#ifdef DEBUG
	printf("\e[1;32mCODE: %d (%d bits) [%d section(s)]\e[0m\n", ed->code, ed->LZWmin, ed->code_subsection_count);
#endif
	if(ed->code==ed->end_code_number){ ed->finished = 1; break;}
	if(process_code(ed)==-1) return -3;
	
	code_length = ed->LZWmin;
	ed->code = 0;
	ed->current_code_length = 0;
	ed->code_subsection_count = 0;
      }	
    }
  }
}

int process_code(LZWDecoderData *ed){
  if(ed->last_code==-1 && ed->code!=ed->clear_code_number){
    ed->last_code=ed->code;
  }else if(ed->code==ed->clear_code_number){
    printf("Reset\n");
    if(ed->has_been_cleared){
      clear_codes(ed);
      reset_code_list(ed);
    }else{
      printf("Dictionary preinitialized: No need to clear it.");
      ed->has_been_cleared = 1;
    }
  }else if(ed->code!=ed->clear_code_number){
    uint8_t suffix;
    if(ed->code<ed->number_of_codes) suffix=ed->codes[ed->code].data[0];
    else suffix=ed->codes[ed->last_code].data[0];
    
    ed->codes[ed->number_of_codes].length = ed->codes[ed->last_code].length+1;
    ed->codes[ed->number_of_codes].data = malloc(ed->codes[ed->number_of_codes].length*sizeof(uint8_t));
    memcpy(ed->codes[ed->number_of_codes].data, ed->codes[ed->last_code].data, ed->codes[ed->last_code].length);
    ed->codes[ed->number_of_codes].data[ed->codes[ed->number_of_codes].length-1] = suffix;

#ifdef DEBUG
    printf("Adding Code %d based on code %d; ", ed->number_of_codes, ed->last_code);
    for(int j=0;j<ed->codes[ed->number_of_codes].length; j++)printf("%d ", ed->codes[ed->number_of_codes].data[j]);
    printf("\nNew LZW: %d; new number_of_codes: %d\n", ed->LZWmin, ed->number_of_codes+1);
#endif

    ed->last_code=ed->code;
    ed->number_of_codes++;
    
    ed->LZWmin = (int)ceil(log2(ed->number_of_codes+1));
    if(ed->LZWmin==13) ed->LZWmin=12;
  }else ed->last_code=-1;
  
  if(ed->code!=ed->end_code_number&&ed->code!=ed->clear_code_number){
    if(ed->code>=ed->number_of_codes){
      printf("\e[1;31mEncountered Invalid Code. Terminating Decode.\e[0m\n");
      ed->finished=1;
      return -1;
    }
    memcpy(ed->output_indexes+ed->output_index_position,ed->codes[ed->code].data,ed->codes[ed->code].length);
    ed->output_index_position+=ed->codes[ed->code].length;
#ifdef DEBUG
    printf("Index Stream: \e[1;32m");
    for(int j=0;j<ed->codes[ed->code].length; j++)printf("%d ", ed->codes[ed->code].data[j]);
    printf("\e[0m\n");
    printf("Index Count: %d\n\n", ed->output_index_position);
#endif
  }
}
