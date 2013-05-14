#include <stdint.h>
#include "linkedlist.h"

#ifndef LZW_STRUCTS_AND_FUNCTIONS
#define LZW_STRUCTS_AND_FUNCTIONS
typedef struct LZWDecoderEntry_t{
  int length;
  uint8_t *data;
} LZWDecoderEntry;

typedef struct LZWEncoderData_t{
  int clear_code_number;
  int end_code_number;
  LZWDecoderEntry codes[4096];
  int number_of_codes;// = 0;

  int color_list_size;// = 4;
  int output_index_position;//=0;
  int last_code;//=-1;
  int current_bit;// = 0;
  int LZWmin;
  uint8_t *output_indexes;
  
  int code;
  uint8_t finished;
  int current_code_length;

  int code_subsection_count;
  int reset_codes;
} LZWEncoderData;


typedef struct LZWTreeEntry_t{
  int count;
  uint8_t data;
  uint16_t code_number;
  struct LinkedList *children;
  int level;
} LZWTreeEntry;

typedef struct LZWDecoderData_t{
  int clear_code_number;
  int end_code_number;
  
  int bits_used;
  uint32_t tmpcodebyte;
  uint8_t *output;
  int output_active_length;
  int output_size;

  LinkedList dictionary;
  uint16_t next_rule_id;

  int LZWmin;
  int start_index;
  int resolved_indexes;
  LinkedList *dictionary_branch;
  LZWTreeEntry *last_matched_rule;
  int initial_dictionary_size;
}LZWDecoderData;


void lzw_decode_initialize(LZWEncoderData *ed, int color_list_size, int decode_buffer_start_size);
int  lzw_decode(LZWEncoderData *ed, uint8_t *encoded_source, int encoded_source_length);
void lzw_decode_free(LZWEncoderData *ed, uint8_t free_result);

void lzw_encode_initialize(LZWDecoderData *ld, int initial_lzw_dictionary_size);
void lzw_encode(LZWDecoderData *ld, uint8_t *input, int input_length);
void lzw_encode_free(LZWDecoderData *ld);
#endif
