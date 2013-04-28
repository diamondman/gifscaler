#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "lzw.h"

//#define DEBUG

int main(int argc, char* argv[]){
  int image_width = 10;
  int image_height = 10;
  //int encoded_source_length = 22;
  //uint8_t encoded_source[22] = {0x8c, 0x2d, 0x99, 0x87, 0x2a, 0x1c, 0xdc, 0x33, 0xa0, 0x02, 0x75, 0xec, 0x95, 0xfa, 0xa8, 0xde, 0x60, 0x8c, 0x04, 0x91, 0x4c, 0x01};

  /*int encoded_source_length_a = 11;
  uint8_t encoded_source_a[11] = {0x8c, 0x2d, 0x99, 0x87, 0x2a, 0x1c, 0xdc, 0x33, 0xa0, 0x02, 0x75};
  int encoded_source_length_b = 11;
  uint8_t encoded_source_b[11] = {0xec, 0x95, 0xfa, 0xa8, 0xde, 0x60, 0x8c, 0x04, 0x91, 0x4c, 0x01};*/

  LZWEncoderData ed;
  lzw_decode_initialize(&ed, 4, image_width*image_height);

  /*lzw_decode(&ed, encoded_source_a, encoded_source_length_a);
  printf("\n****NEXT CALL****\n\n");
  lzw_decode(&ed, encoded_source_b, encoded_source_length_b);*/
  
  uint8_t encoded_source_a[2] = {0x8c, 0x2d};
  lzw_decode(&ed, encoded_source_a, 2);
  uint8_t encoded_source_b[2] = {0x99, 0x87};
  lzw_decode(&ed, encoded_source_b, 2);
  uint8_t encoded_source_c[2] = {0x2a, 0x1c};
  lzw_decode(&ed, encoded_source_c, 2);
  uint8_t encoded_source_d[2] = {0xdc, 0x33};
  lzw_decode(&ed, encoded_source_d, 2);
  uint8_t encoded_source_e[3] = {0xa0, 0x02, 0x75};
  lzw_decode(&ed, encoded_source_e, 3);
  uint8_t encoded_source_f[2] = {0xec, 0x95};
  lzw_decode(&ed, encoded_source_f, 2);
  uint8_t encoded_source_g[2] = {0xfa, 0xa8};
  lzw_decode(&ed, encoded_source_g, 2);
  uint8_t encoded_source_h[2] = {0xde, 0x60};
  lzw_decode(&ed, encoded_source_h, 2);
  uint8_t encoded_source_i[2] = {0x8c, 0x04};
  lzw_decode(&ed, encoded_source_i, 2);
  uint8_t encoded_source_j[2] = {0x91, 0x4c};
  lzw_decode(&ed, encoded_source_j, 2);
  uint8_t encoded_source_k[1] = {0x01};
  lzw_decode(&ed, encoded_source_k, 1);

  for(int j=0;j<image_width*image_height; j++)printf("%d ", ed.output_indexes[j]); printf("\e[0m\n");
  printf("1 1 1 1 1 2 2 2 2 2 1 1 1 1 1 2 2 2 2 2 1 1 1 1 1 2 2 2 2 2 1 1 1 0 0 0 0 2 2 2 1 1 1 0 0 0 0 2 2 2 2 2 2 0 0 0 0 1 1 1 2 2 2 0 0 0 0 1 1 1 2 2 2 2 2 1 1 1 1 1 2 2 2 2 2 1 1 1 1 1 2 2 2 2 2 1 1 1 1 1\n");

  lzw_decode_free(&ed, 1);
  return 0; 
}
