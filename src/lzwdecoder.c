#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "linkedlist.h"

typedef struct LZWDecoderEntry_t{
  int length;
  uint8_t *data;
} LZWDecoderEntry;

int encoded_source_length = 22;
int image_width = 10;
int image_height = 10;
uint8_t encoded_source[22] = {0x8c, 0x2d, 0x99, 0x87, 0x2a, 0x1c, 0xdc, 0x33, 0xa0, 0x02, 0x75, 0xec, 0x95, 0xfa, 0xa8, 0xde, 0x60, 0x8c, 0x04, 0x91, 0x4c, 0x01};
uint8_t *output_indexes;

int color_list_size = 4;
uint8_t color_list[4] = {0,1,2,3};//{'W','R','B','L'};

int clear_code_number;
int end_code_number;
LZWDecoderEntry codes[4096];
int number_of_codes = 0;

void clear_codes(){
  memset(&codes[end_code_number+1], 0, (4096-number_of_codes)*sizeof(LZWDecoderEntry));
}

int main(int argc, char* argv[]){
  output_indexes = malloc(sizeof(uint8_t)*image_height*image_width);
  int output_index_position=0;
  int last_code=-1;
  int current_bit = 0;

  int i;
  for(i=0; i<color_list_size; i++){
    codes[i].length=1;
    codes[i].data = malloc(sizeof(uint8_t)*codes[i].length);
    codes[i].data[0] = number_of_codes;
    printf("Code num: %d\n", number_of_codes);
    number_of_codes++;
  }

  clear_code_number = number_of_codes++;
  end_code_number = number_of_codes++;
 
  int LZWmin = (int)ceil(log2(number_of_codes));
 
  uint8_t rawbyte;
  for(i=0; i<encoded_source_length; i++){
    rawbyte = encoded_source[i];
    printf("\nraw byte: 0x%02x\n", rawbyte);
    current_bit=0;
    while(current_bit<8){
      int code=0;
      int mask = ((int)pow(2, LZWmin))-1;
      printf("\nmodified raw byte: 0x%02x; Current bit: %d; LZW: %d\n", rawbyte, current_bit, LZWmin);
      LZWmin = (int)ceil(log2(number_of_codes+1));
      if(current_bit+LZWmin>8){
	i++;
	current_bit=(current_bit+LZWmin)-8;
	int tmpmask = ((int)pow(2, current_bit))-1;
	printf("Current tmpbitmask: %02x\n", tmpmask);
	printf("Current bit: %02x\n", current_bit);
	printf("Boundary Byte: %02x | %02x\n", ((encoded_source[i]&tmpmask)<<(LZWmin-current_bit)), (rawbyte&mask));
	code=((encoded_source[i]&tmpmask)<<(LZWmin-current_bit))|(rawbyte&mask);
	printf("Pre shift raw byte: 0x%02x\n", encoded_source[i]);
	rawbyte=encoded_source[i]>>current_bit;
	printf("preemptive rawbyte 0x%02x\n", rawbyte);
      }else{
	current_bit+=LZWmin;
	code = rawbyte&mask;
	rawbyte=rawbyte>>LZWmin;
      }
      printf("Code: %d\n", code);
      if(code==end_code_number)break;
      printf("Current Bit %d\n",current_bit);
      if(last_code==-1 && code!=clear_code_number){// && code!=end_code_number){
	last_code=code;
      }else if(code!=clear_code_number){// && code!=end_code_number){
	uint8_t suffix;
	if(code<number_of_codes){
	  suffix=codes[code].data[0];
	}else{
	  suffix=codes[last_code].data[0];
	}
	codes[number_of_codes].length = codes[last_code].length+1;
	codes[number_of_codes].data = malloc(codes[number_of_codes].length*sizeof(uint8_t));
	memcpy(codes[number_of_codes].data, codes[last_code].data, codes[last_code].length);
	codes[number_of_codes].data[codes[number_of_codes].length-1] = suffix;
	printf("Adding Code %d based on code %d; ", number_of_codes, last_code);
	int j;
	for(j=0;j<codes[number_of_codes].length; j++)printf("%d ", codes[number_of_codes].data[j]);
	printf("\n");
	last_code=code;
	number_of_codes++;
	printf("New LZW: %d; new number_of_codes: %d\n", LZWmin, number_of_codes);
      }else last_code=-1;
      if(code!=end_code_number&&code!=clear_code_number){
	printf("Index Stream: \e[1;32m");
	memcpy(output_indexes+output_index_position, codes[code].data, codes[code].length);
	output_index_position+=codes[code].length;
	int j;
	for(j=0;j<codes[code].length; j++)printf("%d ", codes[code].data[j]);
	printf("\e[0m\n");
      }
    }
  }

  int j;
  for(j=0;j<image_width*image_height; j++)printf("%d ", output_indexes[j]);
  printf("\e[0m\n");

  for(i=0; i<number_of_codes; i++)free(codes[i].data);
  free(output_indexes);
  return 0; 
}
