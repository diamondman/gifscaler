#include <stdint.h>

#ifndef LZWDECODER_STRUCTS
#define LZWDECODER_STRUCTS
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
  int straddling_bits;
} LZWEncoderData;
#endif

#ifndef LZWDECODER_FUNCTIONS
#define LZWDECODER_FUNCTIONS
void initialize_lzwdecoder(LZWEncoderData *ed, int color_list_size, int decode_buffer_start_size);
int lzwDecode(LZWEncoderData *ed, uint8_t *encoded_source, int encoded_source_length);
void free_lzwdecoder(LZWEncoderData *ed, uint8_t free_result);
#endif
