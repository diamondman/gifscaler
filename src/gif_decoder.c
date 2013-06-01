#include "gif.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "linkedlist.h"
#include "lzw.h"

#define GIF87A_MAGIC         "GIF87a"
#define GIF89A_MAGIC         "GIF89a"
#define COLORRESOLUTION_MASK 0x70 //0b01110000
#define CT_MASK              128
#define CTSIZE_MASK          7
#define GIFFILEHEADERSIZE    13

#define FLAGS(field, flags) ((field & flags)==flags)
#define U16LTOU16(p)        ((uint16_t)((*(p+1))<<8 | *p))
#define U24LTOU24(p)        ((*p<<16) | (*(p+1)<<8) | *(p+2))
#define HAS_GCT(p)          FLAGS(p[10], CT_MASK)
#define COLOR_RESOLUTION(p) ((p[10] & COLORRESOLUTION_MASK)>>4)+1
#define GCT_SIZE(p)         (HAS_GCT(p) ? 2<<(p[10] & CTSIZE_MASK) : 0)
#define MAX(a, b)           ((a)>(b)?(a):(b))

uint32_t* extract_color_table (uint8_t hastable, uint8_t *data, uint16_t table_size, int *delta_data){
  *delta_data=0;
  if (!hastable){ return 0;}
  uint32_t *color_table = (uint32_t*)malloc(table_size*3*sizeof(uint32_t));
  for(int i=0; i<table_size; i++){
    color_table[i] = U24LTOU24(data);
    *delta_data+=3;
    data+=3;
  }
  return color_table;
}

void gif_load_initialize (Gif *gif){
  memset(gif, 0, sizeof(Gif));
  gif->status = DECODERSTATE_START;
  gif->tmp_extensions = (LinkedList *)calloc(1,sizeof(LinkedList));
  gif->stream_error = GIFERROR_NOERROR;
  gif->tmp_buffer = NULL;
  gif->stage_status = DATADECODERSTATE_START;
  gif->decoder_scratch = NULL;
}

int gif_load_header (Gif *gif, uint8_t *p, uint32_t p_length, uint32_t *used_bytes){
  printf("CALLING LOAD HEADER\n");
  *used_bytes = 0;
  if (p_length < GIFFILEHEADERSIZE) return GIFRET_NEEDMOREDATA;
  memcpy(gif->version, p, 6);
  if (strcmp(gif->version, GIF87A_MAGIC)!=0 && strcmp(gif->version, GIF89A_MAGIC)!=0){
    printf("The file header is not a valid gif header: mismatched magic number.\n");
    gif->stream_error = GIFERROR_UNSUPPORTEDFORMAT;
    return GIFRET_STREAMDATAERROR;
  }

  //Extract Main Gif Header
  gif->width = U16LTOU16(&p[6]);
  gif->height = U16LTOU16(&p[8]);
  gif->has_gct = HAS_GCT(p);
  gif->color_resolution = COLOR_RESOLUTION(p);
  gif->colors_sorted = FLAGS(p[10], 8);
  gif->gct_size = GCT_SIZE(p);
  gif->bgcolor = p[11];
  gif->aspect_ratio = p[12];
  gif->ext_count = 0;
  gif->image_count = 0;

  *used_bytes = GIFFILEHEADERSIZE;
  printf("HEAD USED BYTES  %d\n", *used_bytes);
  return GIFRET_DONE;
}

int gif_load_gct (Gif *gif, uint8_t *p, uint32_t p_length, uint32_t *used_bytes){
  printf("CALLING LOAD GCT; p_length: %d\n",p_length);
  *used_bytes = 0;
  if (!gif->has_gct) return GIFRET_DONE;
  if (p_length < gif->gct_size*3){ return GIFRET_NEEDMOREDATA; }
  gif->color_table = extract_color_table(1, p, gif->gct_size, used_bytes);
  printf("GCT USED BYTES   %d\n", *used_bytes);
  *used_bytes = gif->gct_size*3;
  printf("+GCT USED BYTES  %d\n", *used_bytes);
  return GIFRET_DONE;
}

int gif_load_get_ext (Gif *gif, uint8_t *p, uint32_t p_length, uint32_t *usedbytes){
  *usedbytes = 0;
  if (p[0] == EXT_MARKER){
    printf("Found an EXT\n");
    if (p_length < 3){ return GIFRET_NEEDMOREDATA; }
    GifExtHeader *exth = (GifExtHeader*)calloc(1, sizeof(GifExtHeader));
    exth->type = p[1];
    
    uint8_t *pbufcheck = &p[2];
    while(*pbufcheck != 0) pbufcheck += ((*pbufcheck) + 1);
    uint32_t extdata_size = sizeof(uint8_t) * (pbufcheck - &p[2]);
    exth->data = calloc(1, extdata_size + sizeof(uint8_t));
    memcpy(exth->data, &p[2], extdata_size);
    
    LinkedListItem *item = addNewLinkedListItem(gif->tmp_extensions);
    gif->ext_count++;
    item->data = exth;
    
    *usedbytes = extdata_size + 3;
    printf("Finished an EXT\n");
    return GIFRET_DONE;
  }
  return GIFRET_NOCHANGE;
}

int gif_load_get_image (Gif *gif, uint8_t *p, uint32_t p_length, uint32_t *usedbytes, LZWDecoderData *ed){
  *usedbytes = 0;
  if (p[0] == IMG_MARKER)
    printf("Found an IMG\n");{
    uint8_t *pbackup = p;

    LinkedListItem *item = addNewLinkedListItem(&(gif->image_descriptor_linked_list));
    item->data = malloc(sizeof(GifImageDescriptor));
    memset(item->data, 0, sizeof(GifImageDescriptor));
    GifImageDescriptor *d = (GifImageDescriptor*)item->data;
    d->extensions = gif->tmp_extensions;
    gif->tmp_extensions = (LinkedList *) calloc(1, sizeof(LinkedList));

    d->left   = U16LTOU16(&p[1]);
    d->top    = U16LTOU16(&p[3]);
    d->width  = U16LTOU16(&p[5]);
    d->height = U16LTOU16(&p[7]);

    int tmp = (uint8_t)p[9];
    d->has_lct = FLAGS(tmp,CT_MASK);
    d->interlaced = FLAGS(tmp, 64);
    d->sort = FLAGS(tmp, 32);
    d->lct_size = (d->has_lct) ? (2<<(tmp & CTSIZE_MASK)) : 0;

    int deltap;
    d->color_table = extract_color_table(d->has_lct, &p[10], d->lct_size, &deltap);
    p+=10+deltap;
          
    d->LZW = p[0];
    p+=1;
    
    uint32_t LZW_dict_init_size = 2<<(d->LZW-1);
    memset(ed, 0, sizeof(LZWDecoderData));
    printf("CONTINUING EXT\n");
    lzw_decode_initialize(ed, LZW_dict_init_size, d->width*d->height);
    uint8_t *beginning_of_image = p; 
    while(*p!=0){
#ifdef DEBUG
      printf("\n\e[1;34m****NEXT CALL %d BYTES****\e[0m\n\n", *p);
#endif
      int result = lzw_decode(ed, p+1, *p);
      if (result<0){printf("Error Decoding image data, Terminating scaling operation\n"); return -1;}
      p+=(*p)+1;
    }
    //printf("Image Data: ");
    //for(int j=0;j<d->width*d->height; j++)printf("%d ", ed->output_indexes[j]); printf("\n");
    d->image_color_index_data = ed->output_indexes;
    lzw_decode_free(ed, 0);
    
    int image_data_size = (p-beginning_of_image)+1;
    d->image_data_size = image_data_size;
    d->image_data = (uint8_t *)malloc(image_data_size);
    memset(d->image_data, 0, d->image_data_size);
    memcpy(d->image_data, beginning_of_image, image_data_size);
    
    p+=1;
    gif->image_count++;
    //printf("END OF IMG %02x->%02x->%02x\n",*(p-1),*p,*(p+1));
    *usedbytes = p-pbackup;
    printf("Frame used bytes = %d\n", *usedbytes);
    return GIFRET_DONE;
  }
  return GIFRET_NOCHANGE;
}

int gif_load_images_and_extensions(Gif *gif, uint8_t *p, uint32_t p_length, uint32_t *usedbytes){
  printf("CALLING LOAD IMAGES\n");
  uint8_t *startpoint = p;
  LinkedList *images = &(gif->image_descriptor_linked_list);
  *usedbytes = 0;

  //Image and Extension check
  LZWDecoderData ed;
  uint32_t sub_usedbytes;
  while(*p == EXT_MARKER || *p == IMG_MARKER){
    gif_load_get_ext(gif, p, p_length, &sub_usedbytes);
    p += sub_usedbytes;

    //Detect Images
    gif_load_get_image(gif, p, p_length, &sub_usedbytes, &ed);
    p += sub_usedbytes;
  }
  gif->trailing_extensions = gif->tmp_extensions;
  *usedbytes = p-startpoint;
  printf("FINISHING EXT AND IMAGES\n");
  return GIFRET_DONE;
}

int gif_load_tail(Gif *gif, uint8_t *p, uint32_t p_length, uint32_t *usedbytes){
  printf("CALLING LOAD TAIL\n");
  *usedbytes = 0;
  if (p_length < 1) return GIFRET_NEEDMOREDATA;
  if (*p != GIF_TRAILER){
    gif->stream_error = GIFERROR_INVALIDBYTE;
    return GIFRET_STREAMDATAERROR;
  }
  *usedbytes = 1;
  return GIFRET_DONE;
}

int gif_load_streamingwrapper(Gif *gif, uint8_t *p, uint32_t p_length, uint32_t *used_bytes, 
			      int (*funct)(Gif*, uint8_t*, uint32_t, uint32_t*)){
  printf("\nCalling Stream Wrapper\n");
  *used_bytes = 0;
  //if (p_length==0) return GIFRET_NEEDMOREDATA;
  int funct_res;
  if (gif->tmp_buffer_data_length==0){
    printf("No tmp buffer data\n");
    funct_res = funct(gif, p, p_length, used_bytes);
    if (funct_res == GIFRET_DONE){ return GIFRET_DONE;
    }else{
      if (*used_bytes < p_length){
	if ((p_length-*used_bytes) >= gif->tmp_buffer_length){
	  if (gif->tmp_buffer_length != 0) free(gif->tmp_buffer);
	  gif->tmp_buffer_length = MAX(128, 2*(p_length-*used_bytes));
	  gif->tmp_buffer = (uint8_t*)malloc(gif->tmp_buffer_length);
	  printf("Allocating Buffer (%d bytes)\n", gif->tmp_buffer_length);
	}
	gif->tmp_buffer_data_length = p_length-*used_bytes;
	memcpy(gif->tmp_buffer, p+*used_bytes, gif->tmp_buffer_data_length);
	printf("Copied %d bytes to BLANK tmp_buffer starting at %d.\n", gif->tmp_buffer_data_length, *used_bytes);
	gif->tmp_buffer_offset = 0;
      }
      *used_bytes = p_length;
      return GIFRET_NEEDMOREDATA;
    }
  }else{
    printf("TMP BUFFER FOUND; BUFF SIZE: %d; DATA SIZE: %d\n", gif->tmp_buffer_length, gif->tmp_buffer_data_length);
    if (p_length+gif->tmp_buffer_data_length < gif->tmp_buffer_length){
      printf("New buf data fits in tmp_buffer.\n");
    }else if (p_length+(gif->tmp_buffer_data_length-gif->tmp_buffer_offset) < gif->tmp_buffer_length){
      memcpy(gif->tmp_buffer, gif->tmp_buffer+gif->tmp_buffer_offset, 
	     gif->tmp_buffer_data_length-gif->tmp_buffer_offset);
      printf("Moving back the tmp_buffer to remove offset overhead\n");
    }else{
      gif->tmp_buffer_length = (2*p_length)+
	(gif->tmp_buffer_data_length-gif->tmp_buffer_offset);
      printf("Resizing buffer: %d\n", gif->tmp_buffer_length);
      uint8_t *replacement_buffer = malloc(gif->tmp_buffer_length);
      printf("Moving back the tmp_buffer to remove offset overhead\n");
      memcpy(replacement_buffer, gif->tmp_buffer+gif->tmp_buffer_offset, 
	     gif->tmp_buffer_data_length-gif->tmp_buffer_offset);
      free(gif->tmp_buffer);
      gif->tmp_buffer = replacement_buffer;
      gif->tmp_buffer_data_length -= gif->tmp_buffer_offset;
      gif->tmp_buffer_offset = 0;
    }
    memcpy(gif->tmp_buffer+gif->tmp_buffer_data_length, p, p_length);
    printf("Copied %d bytes to EXISTING tmp_buffer starting at %d.\n", p_length, gif->tmp_buffer_data_length);
    uint32_t old_tmp_buffer_data_length = gif->tmp_buffer_data_length;
    gif->tmp_buffer_data_length = gif->tmp_buffer_data_length + p_length;
    printf("After copy, tmp buffer length: %d\n", gif->tmp_buffer_data_length);
    funct_res = funct(gif, gif->tmp_buffer, gif->tmp_buffer_data_length, used_bytes);
    gif->tmp_buffer_offset += *used_bytes;
    if (gif->tmp_buffer_offset == gif->tmp_buffer_data_length){
      gif->tmp_buffer_offset = 0;
      gif->tmp_buffer_data_length = 0;
    }
    *used_bytes = p_length;
    return funct_res;
  }
}

int gif_load(Gif *gif, uint8_t *p, uint32_t p_length){
  printf("\n**********************************\n");
  printf("LOAD CALL p_length: %d\n", p_length);
  printf("**********************************\n");
  if (p_length < 0){
    printf("WTF - value!\n");
    return GIFRET_NOCHANGE;
  }
  if (gif->stream_error != GIFERROR_NOERROR){ return GIFRET_STREAMDATAERROR; }
  if (p_length == 0){ return GIFRET_NOCHANGE; }
  uint32_t usedbytes;
  uint32_t availablebytes = p_length;
  int stage_return;

  switch(gif->status){
  case DECODERSTATE_START:
    stage_return = gif_load_streamingwrapper(gif, p, availablebytes, &usedbytes, gif_load_header);
    printf("REMAINING BYTES IN TMP BUF HEAD %d\n", gif->tmp_buffer_data_length-gif->tmp_buffer_offset);
    if (stage_return == GIFRET_NEEDMOREDATA) return GIFRET_NEEDMOREDATA;
    if (gif->stream_error != GIFERROR_NOERROR) return GIFRET_STREAMDATAERROR;
    p+=usedbytes;
    availablebytes -= usedbytes;
    gif->status = DECODERSTATE_GCT;

  case DECODERSTATE_GCT:
    stage_return = gif_load_streamingwrapper(gif, p, availablebytes, &usedbytes, gif_load_gct);
    printf("This better fucking show up\n");
    printf("REMAINING BYTES IN TMP BUF GCT  %d\n", gif->tmp_buffer_data_length-gif->tmp_buffer_offset);
    if (stage_return == GIFRET_NEEDMOREDATA) return GIFRET_NEEDMOREDATA;
    if (gif->stream_error != GIFERROR_NOERROR) return GIFRET_STREAMDATAERROR;
    p += usedbytes;
    availablebytes -= usedbytes;
    gif->status = DECODERSTATE_IMAGES;

  case DECODERSTATE_IMAGES:
    stage_return = gif_load_streamingwrapper(gif, p, availablebytes, &usedbytes, gif_load_images_and_extensions);
    printf("REMAINING BYTES IN TMP BUF IMG  %d\n", gif->tmp_buffer_data_length-gif->tmp_buffer_offset);
    if (stage_return == GIFRET_NEEDMOREDATA) return GIFRET_NEEDMOREDATA;
    if (gif->stream_error != GIFERROR_NOERROR) return GIFRET_STREAMDATAERROR;
    p += usedbytes;
    availablebytes -= usedbytes;
    gif->status = DECODERSTATE_ENDING;

  case DECODERSTATE_ENDING:
    stage_return = gif_load_streamingwrapper(gif, p, availablebytes, &usedbytes, gif_load_tail);
    printf("REMAINING BYTES IN TMP BUF END  %d\n", gif->tmp_buffer_data_length-gif->tmp_buffer_offset);
    if (stage_return == GIFRET_NEEDMOREDATA) return GIFRET_NEEDMOREDATA;
    if (gif->stream_error != GIFERROR_NOERROR) return GIFRET_STREAMDATAERROR;
    p+=usedbytes;
    availablebytes -= usedbytes;
    gif->status = DECODERSTATE_FINISHED;

    if (availablebytes > 0){
      printf("Extra bytes found at end of valid gif file (%d bytes).\n", availablebytes);
      return GIFRET_DONEEXTRADATA;
    }
    return GIFRET_DONE;

  case DECODERSTATE_FINISHED:
    printf("ALREADY DONE!\n");
    return GIFRET_ALREADYFINISHED;

  default:
    printf("Gif structure doesn't appear to be correctly initialized or in an invalid state. Be sure to call gif_load_initialize() with this structure before trying to call gif_load().\n");
    return GIFRET_INVALIDSTATE;
  }
}

void gif_free(Gif *gif){
  LinkedList *images = &gif->image_descriptor_linked_list;
  if (images->length>0){
    LinkedListItem *item = images->first;
    LinkedListItem *nextitem;

    do{
      GifImageDescriptor *d = (GifImageDescriptor *)item->data;
      if (d->image_data!=NULL) free(d->image_data);
      if (d->has_lct) free(d->color_table);
      if (d->image_color_index_data) free(d->image_color_index_data);

      LinkedList *image_descriptor_extensions = d->extensions;
      LinkedListItem *extitem = image_descriptor_extensions->first;
      LinkedListItem *nextextitem;
      
      if (image_descriptor_extensions->length){
	do{
	  GifExtHeader *extheader = (GifExtHeader *)extitem->data;
	  if (extheader->data!=NULL) free(extheader->data);
	  free(extheader);
	  nextextitem = extitem->next;
	  extitem=nextextitem;
	}while(extitem!=0);
      }

      disposeLinkedList(d->extensions);
      free(d->extensions);
      free(d);
      nextitem = item->next;
      item=nextitem;
    }while(item!=0);
  }
  free(gif->trailing_extensions);
  disposeLinkedList(images);

  if (gif->has_gct) free(gif->color_table);
}
