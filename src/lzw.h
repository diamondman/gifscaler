#include <stdint.h>
#include "linkedlist.h"

#ifndef LZW_STRUCTS_AND_FUNCTIONS
#define LZW_STRUCTS_AND_FUNCTIONS
typedef struct LZWDecoderEntry_t{
  int length;
  uint8_t *data;
} LZWDecoderEntry;

typedef struct LZWDecoderData_t{
  int clear_code_number;
  int end_code_number;
  LZWDecoderEntry codes[4096];
  int number_of_codes;

  int initial_dictionary_size;
  int LZWmin;
  int output_index_position;
  int last_code;
  int current_bit;
  uint8_t *output_indexes;
  
  int code;
  uint8_t finished;
  uint8_t has_been_cleared;
  int current_code_length;

  int code_subsection_count;
  int reset_codes;
} LZWDecoderData;

typedef struct LZWTreeEntry_t{
  int count;
  uint8_t data;
  uint16_t code_number;
  struct LinkedList *children;
  int level;
} LZWTreeEntry;

typedef struct LZWEncoderData_t{
  int clear_code_number;
  int end_code_number;
  
  int initial_dictionary_size;
  int LZWmin;
  int bits_used;
  uint32_t tmpcodebyte;
  uint8_t *output;
  int output_active_length;
  int output_size;

  LinkedList dictionary;
  uint16_t next_rule_id;

  int start_index;
  int resolved_indexes;
  LinkedList *dictionary_branch;
  LZWTreeEntry *last_matched_rule;
}LZWEncoderData;


void lzw_decode_initialize(LZWDecoderData *ed, int color_list_size, int decode_buffer_start_size);
int  lzw_decode(LZWDecoderData *ed, uint8_t *encoded_source, int encoded_source_length);
void lzw_decode_free(LZWDecoderData *ed, uint8_t free_result);

void lzw_encode_initialize(LZWEncoderData *ld, int initial_lzw_dictionary_size);
void lzw_encode(LZWEncoderData *ld, uint8_t *input, int input_length);
void lzw_encode_free(LZWEncoderData *ld);
#endif
