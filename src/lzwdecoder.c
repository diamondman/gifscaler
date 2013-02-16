#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define DEBUG
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

void clear_codes(LZWEncoderData *ed){
  memset(&ed->codes[ed->end_code_number+1], 0, (4096-ed->number_of_codes)*sizeof(LZWDecoderEntry));
}

void initialize_lzwdecoder(LZWEncoderData *ed, int color_list_size, int decode_buffer_start_size){
  memset(ed, 0, sizeof(LZWEncoderData));
  ed->output_indexes = malloc(sizeof(uint8_t)*decode_buffer_start_size);
  ed->color_list_size = color_list_size;
  ed->last_code=-1;

  for(int i=0; i<ed->color_list_size; i++){
    ed->codes[i].length=1;
    ed->codes[i].data = malloc(sizeof(uint8_t)*ed->codes[i].length);
    ed->codes[i].data[0] = ed->number_of_codes;
    #ifdef DEBUG
    printf("Code num: %d\n", ed->number_of_codes);
    #endif
    ed->number_of_codes++;
  }

  ed->clear_code_number = ed->number_of_codes++;
  ed->end_code_number = ed->number_of_codes++;

  ed->LZWmin = (int)ceil(log2(ed->number_of_codes));
}

void lzwDecode(LZWEncoderData *ed, uint8_t *encoded_source, int encoded_source_length){
  if(encoded_source_length<1) return -2;
  uint8_t rawbyte;
  for(int i=0; i<encoded_source_length; i++){
    rawbyte = encoded_source[i];
    ed->current_bit=0;
    #ifdef DEBUG
    printf("\nraw byte: 0x%02x\n", rawbyte);
    #endif
    while(ed->current_bit<8 && i<encoded_source_length){
      int mask;
      if(ed->straddling_bits>0){
	ed->code=0;
	mask = ((int)pow(2, ed->LZWmin))-1;
      }else{
	printf("Making Mask on straddling bits\n");
	mask = ((int)pow(2, ed->LZWmin-ed->straddling_bits))-1;
      }
      #ifdef DEBUG
      printf("\nmodified raw byte: 0x%02x; Current bit: %d; LZW: %d\n", rawbyte, ed->current_bit, ed->LZWmin);
      #endif
      ed->LZWmin = (int)ceil(log2(ed->number_of_codes+1));
      if(ed->current_bit+ed->LZWmin>8){
	i++;
	ed->current_bit=(ed->current_bit+ed->LZWmin)-8;
	int tmpmask = ((int)pow(2, ed->current_bit))-1;
	if(i<encoded_source_length){
	  ed->code=((encoded_source[i]&tmpmask)<<(ed->LZWmin-ed->current_bit))|(rawbyte&mask);
	}else{
	  ed->code=(rawbyte&mask);
	  printf("\e[1;32mCode Straddles Data Chunks! Missing %d bits\e[0m\n\n", ed->current_bit);
	  ed->straddling_bits = ed->current_bit;
	  ed->current_bit=0;
	  break;
	}
	#ifdef DEBUG
	printf("Current tmpbitmask: %02x\n", tmpmask);
	printf("Current bit: %02x\n", ed->current_bit);
	printf("Boundary Byte: %02x | %02x\n", ((encoded_source[i]&tmpmask)<<(ed->LZWmin-ed->current_bit)), (rawbyte&mask));
	printf("Pre shift raw byte: 0x%02x\n", encoded_source[i]);
	#endif
	rawbyte=encoded_source[i]>>ed->current_bit;
	#ifdef DEBUG
	printf("preemptive rawbyte 0x%02x\n", rawbyte);
	#endif
      }else{
	ed->current_bit+=ed->LZWmin;
	ed->code = rawbyte&mask;
	rawbyte=rawbyte>>ed->LZWmin;
      }
      #ifdef DEBUG
      printf("Code: %d\n", ed->code);
      printf("Current Bit %d\n",ed->current_bit);
      #endif
      if(ed->code==ed->end_code_number)break;
      if(ed->last_code==-1 && ed->code!=ed->clear_code_number){// && code!=end_code_number){
	ed->last_code=ed->code;
      }else if(ed->code!=ed->clear_code_number){// && code!=end_code_number){
	uint8_t suffix;
	if(ed->code<ed->number_of_codes){
	  suffix=ed->codes[ed->code].data[0];
	}else{
	  suffix=ed->codes[ed->last_code].data[0];
	}
	ed->codes[ed->number_of_codes].length = ed->codes[ed->last_code].length+1;
	ed->codes[ed->number_of_codes].data = malloc(ed->codes[ed->number_of_codes].length*sizeof(uint8_t));
	memcpy(ed->codes[ed->number_of_codes].data, ed->codes[ed->last_code].data, ed->codes[ed->last_code].length);
	ed->codes[ed->number_of_codes].data[ed->codes[ed->number_of_codes].length-1] = suffix;
	#ifdef DEBUG
	printf("Adding Code %d based on code %d; ", ed->number_of_codes, ed->last_code);
	int j;
	for(j=0;j<ed->codes[ed->number_of_codes].length; j++)printf("%d ", ed->codes[ed->number_of_codes].data[j]);
	printf("\nNew LZW: %d; new number_of_codes: %d\n", ed->LZWmin, ed->number_of_codes+1);
	#endif
	ed->last_code=ed->code;
	ed->number_of_codes++;
      }else ed->last_code=-1;
      if(ed->code!=ed->end_code_number&&ed->code!=ed->clear_code_number){
	if(ed->code>=ed->number_of_codes){
	  printf("\e[1;32mEncountered Invalid Code. Terminating Decode.\e[0m\n");
	  return -2;
        }
	memcpy(ed->output_indexes+ed->output_index_position,ed->codes[ed->code].data,ed->codes[ed->code].length);
	ed->output_index_position+=ed->codes[ed->code].length;
	#ifdef DEBUG
	printf("Index Stream: \e[1;32m");
	int j;
	for(j=0;j<ed->codes[ed->code].length; j++)printf("%d ", ed->codes[ed->code].data[j]);
	printf("\e[0m\n");
        #endif
      }
    }
  }
}

int main(int argc, char* argv[]){
  int image_width = 10;
  int image_height = 10;
  //int encoded_source_length = 22;
  //uint8_t encoded_source[22] = {0x8c, 0x2d, 0x99, 0x87, 0x2a, 0x1c, 0xdc, 0x33, 0xa0, 0x02, 0x75, 0xec, 0x95, 0xfa, 0xa8, 0xde, 0x60, 0x8c, 0x04, 0x91, 0x4c, 0x01};

  int encoded_source_length_a = 11;
  uint8_t encoded_source_a[11] = {0x8c, 0x2d, 0x99, 0x87, 0x2a, 0x1c, 0xdc, 0x33, 0xa0, 0x02, 0x75};
  int encoded_source_length_b = 11;
  uint8_t encoded_source_b[11] = {0xec, 0x95, 0xfa, 0xa8, 0xde, 0x60, 0x8c, 0x04, 0x91, 0x4c, 0x01};


  LZWEncoderData ed;
  initialize_lzwdecoder(&ed, 4, image_width*image_height);
  lzwDecode(&ed, encoded_source_a, encoded_source_length_a);
  printf("\n****NEXT CALL****\n\n");
  lzwDecode(&ed, encoded_source_b, encoded_source_length_b);

  for(int j=0;j<image_width*image_height; j++)printf("%d ", ed.output_indexes[j]); printf("\e[0m\n");

  for(int i=0; i<ed.number_of_codes; i++)free(ed.codes[i].data);
  free(ed.output_indexes);
  printf("1 1 1 1 1 2 2 2 2 2 1 1 1 1 1 2 2 2 2 2 1 1 1 1 1 2 2 2 2 2 1 1 1 0 0 0 0 2 2 2 1 1 1 0 0 0 0 2 2 2 2 2 2 0 0 0 0 1 1 1 2 2 2 0 0 0 0 1 1 1 2 2 2 2 2 1 1 1 1 1 2 2 2 2 2 1 1 1 1 1 2 2 2 2 2 1 1 1 1 1\n");

  return 0; 
}
