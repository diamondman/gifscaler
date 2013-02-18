#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "lzwdecoder.h"

void free_lzwdecoder(LZWEncoderData *ed, uint8_t free_result){
  for(int i=0; i<ed->number_of_codes; i++)free(ed->codes[i].data);
  if(free_result)
    free(ed->output_indexes);
}

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

int lzwDecode(LZWEncoderData *ed, uint8_t *encoded_source, int encoded_source_length){
  printf("\n****NEXT CALL %d BYTES****\n\n", encoded_source_length);
  if(encoded_source_length<1) return -2;
  uint8_t rawbyte;
  for(int i=0; i<encoded_source_length; i++){
    rawbyte = encoded_source[i];
    ed->current_bit=0;
#ifdef DEBUG
    printf("\nraw byte: 0x%02x\n", rawbyte);
#endif

    while(ed->current_bit<8 && i<encoded_source_length){
#ifdef DEBUG
      printf("\nmodified raw byte: 0x%02x; Current bit: %d; LZW: %d\n", rawbyte, ed->current_bit, ed->LZWmin);
#endif

      int mask;
      if(ed->straddling_bits>0){
#ifdef DEBUG
	printf("Making Mask on straddling bits (%d bits)\n", ed->straddling_bits);
#endif
	mask = ((int)pow(2, ed->straddling_bits))-1;
      }else{
	ed->code=0;
	mask = ((int)pow(2, ed->LZWmin))-1;
	ed->code = rawbyte&mask;
#ifdef DEBUG
	printf("Clearing code\n");
	printf("Code stage 1 set to %d\n",ed->code);
#endif
      }
      
      ed->LZWmin = (int)ceil(log2(ed->number_of_codes+1));
      if(ed->current_bit+ed->LZWmin>8||ed->straddling_bits>0){
	int tmpmask;
	int bits_to_use;
	if(ed->straddling_bits>0){
#ifdef DEBUG
	  printf("stuff for straddling bits\n");
#endif
	  ed->current_bit=(ed->straddling_bits);
	  bits_to_use = ed->straddling_bits;
	  ed->straddling_bits = 0;
	}else{
#ifdef DEBUG
	  printf("normal stuff\n");
#endif
	  i++;
	  ed->current_bit=(ed->current_bit+ed->LZWmin)-8;
	  bits_to_use = ed->current_bit;//LZWmin;
	}
	tmpmask = ((int)pow(2, bits_to_use))-1;
#ifdef DEBUG
	printf("Current tmpbitmask: %02x\n", tmpmask);
	
	printf("Mask Bits: %d\n", bits_to_use);
#endif
	if(i<encoded_source_length){
#ifdef DEBUG
	  printf("Right Shift Count: %d\n", ed->LZWmin-ed->current_bit);
	  printf("masked byte %02x\n", encoded_source[i]&tmpmask);
	  printf("Combining %02x (%d) | %02x (%d)\n", ((encoded_source[i]&tmpmask)<<(ed->LZWmin-ed->current_bit)), bits_to_use, ed->code, ed->LZWmin-bits_to_use);
#endif
	  ed->code=((encoded_source[i]&tmpmask)<<(ed->LZWmin-ed->current_bit))|ed->code;
	}else{
	  //ed->code=(rawbyte&mask);
#ifdef DEBUG
	  printf("\e[1;32mCode Straddles Data Chunks! Missing %d bits\e[0m\n\n", ed->current_bit);
#endif
	  ed->straddling_bits = ed->current_bit;
	  ed->current_bit=0;
	  break;
	}

	rawbyte=encoded_source[i]>>ed->current_bit;
#ifdef DEBUG
	printf("preemptive rawbyte 0x%02x\n", rawbyte);
	printf("Current bit: %02x\n", ed->current_bit);
	//printf("Boundary Byte: %02x | %02x\n", ((encoded_source[i]&tmpmask)<<(ed->LZWmin-ed->current_bit)), (rawbyte&mask));
	printf("Pre shift raw byte: 0x%02x\n", encoded_source[i]);
#endif
      }else{
	ed->current_bit+=ed->LZWmin;
	rawbyte=rawbyte>>ed->LZWmin;
      }


#ifdef DEBUG
      printf("Code: %d\n", ed->code);
      printf("Current Bit %d\n",ed->current_bit);
#endif

      if(ed->code==ed->end_code_number){ ed->finished = 1; break;}
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
	for(int j=0;j<ed->codes[ed->code].length; j++)printf("%d ", ed->codes[ed->code].data[j]);
	printf("\e[0m\n");
#endif
      }
    }
  }
}

