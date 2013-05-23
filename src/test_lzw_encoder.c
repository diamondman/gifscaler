#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "lzw.h"

int main(int argc, char* argv[]){
  int input_length = 100;
  uint8_t input[100] = {1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,0,0,0,0,2,2,2,1,1,1,0,0,0,0,2,2,2,2,2,2,0,0,0,0,1,1,1,2,2,2,0,0,0,0,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1};
  int color_list_size = 4;

  LZWEncoderData ld;
  lzw_encode_initialize(&ld, color_list_size);
  lzw_encode(&ld, input, input_length);
  lzw_encode_free(&ld);

  for(int m=0; m<ld.output_active_length; m++) printf("%02x ", ld.output[m]); printf("\n");

  free(ld.output);
  return 0;
}
