#include "gif.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "linkedlist.h"
#include "lzw.h"

#define GIF87A_MAGIC "GIF87a"
#define GIF89A_MAGIC "GIF89a"
#define COLORRESOLUTION_MASK 0x70 //0b01110000
#define CT_MASK 128
#define CTSIZE_MASK 7

#define FLAGS(field, flags) ((field & flags)==flags)
#define U16LTOU16(p) ((uint16_t)((*(p+1))<<8 | *p))
#define U24LTOU24(p) ((*p<<16) | (*(p+1)<<8) | *(p+2))
#define HAS_GCT(p) FLAGS(p[10], CT_MASK)
#define COLOR_RESOLUTION(p) ((p[10] & COLORRESOLUTION_MASK)>>4)+1
#define GCT_SIZE(p) (HAS_GCT(p) ? 2<<(p[10] & CTSIZE_MASK) : 0)

uint32_t* extract_color_table(uint8_t *data, uint16_t table_size, int *delta_data){
  uint32_t *color_table = (uint32_t*)malloc(table_size*3*sizeof(uint32_t));
  for(int i=0; i<table_size; i++){
    color_table[i] = U24LTOU24(data);
    *delta_data+=3;
    data+=3;
  }
  return color_table;
}

int gif_load(Gif *gif, uint8_t *p, uint32_t p_length){
  if(p_length<=14){ printf("File Corrupt. Not enough data for Gif Header.\n"); return -1;}
  LinkedList *images = &(gif->image_descriptor_linked_list);
  images->length=0;

  LinkedList *extensions = (LinkedList *)malloc(sizeof(LinkedList));
  memset(extensions, 0, sizeof(LinkedList));

  memcpy(gif->version, p, 6);
  if(strcmp(gif->version, GIF87A_MAGIC)!=0 && strcmp(gif->version, GIF89A_MAGIC)!=0){
    printf("The file header does not appear to be a gif imagheader: mismatched magic number.\n");
    return -2;
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

  //Get Global Color Table if it exists
  printf("Size: %d; ctablesize: %d\n\n", p_length, gif->gct_size*3);
  if(p_length<=13+(gif->gct_size*3)){ 
    printf("File Corrupt. Not enough data for requested Global Color Table.\n");
    return -1;
  }
  int deltap = 0;
  gif->color_table = gif->has_gct?extract_color_table(&p[13], gif->gct_size, &deltap):0;
  p+=13+deltap;

  //Image and Extension check
  uint8_t *pbackup = p;
  LZWDecoderData ed;
  while(*p==EXT_MARKER||*p==IMG_MARKER){
    if(*p==EXT_MARKER){
      gif->ext_count++;
      p+=1;
      int type=*p;
      LinkedListItem *item = addNewLinkedListItem(extensions);
      item->data = malloc(sizeof(GifExtHeader));
      memset(item->data, 0, sizeof(GifExtHeader));
      GifExtHeader *exth = (GifExtHeader*)item->data;
      exth->type = type;

      p+=1;
      uint8_t *pbeforebody=p;
      while(*p!=0) p+=(*p)+1;
      exth->data = malloc(sizeof(uint8_t)*((p-pbeforebody)+1));
      memset(exth->data, 0, sizeof(uint8_t)*((p-pbeforebody)+1));
      memcpy(exth->data, pbeforebody, sizeof(uint8_t)*(p-pbeforebody));
      
      p+=1;
      //printf("END OF EXT %02x->%02x->%02x\n",*(p-1),*p,*(p+1));
    }

    //Detect Images
    if(*p==IMG_MARKER){
      LinkedListItem *item = addNewLinkedListItem(images);
      item->data = malloc(sizeof(GifImageDescriptor));
      memset(item->data, 0, sizeof(GifImageDescriptor));
      GifImageDescriptor *d = (GifImageDescriptor*)item->data;
      d->extensions = (LinkedList *)extensions;
      extensions = (LinkedList *)malloc(sizeof(LinkedList));
      memset(extensions, 0, sizeof(LinkedList));
      
      p+=1;
      d->left   = U16LTOU16(&p[0]);
      d->top    = U16LTOU16(&p[2]);
      d->width  = U16LTOU16(&p[4]);
      d->height = U16LTOU16(&p[6]);

      int tmp = (uint8_t)p[8];
      d->has_lct = FLAGS(tmp,CT_MASK);
      d->interlaced = FLAGS(tmp, 64);
      d->sort = FLAGS(tmp, 32);
      d->lct_size = (d->has_lct) ? (2<<(tmp & CTSIZE_MASK)) : 0;

      int deltap = 0;
      d->color_table = d->has_lct ? extract_color_table(&p[9], d->lct_size, &deltap) : 0;
      p+=8+deltap+1;
            
      d->LZW = p[0];
      p+=1;
      
      uint32_t LZW_dict_init_size = 2<<(d->LZW-1);
      memset(&ed, 0, sizeof(LZWDecoderData));
      printf("Init Dict Size: %d\n", LZW_dict_init_size);
      lzw_decode_initialize(&ed, LZW_dict_init_size, d->width*d->height);
      uint8_t *beginning_of_image = p; 
      while(*p!=0){
	printf("\n\e[1;34m****NEXT CALL %d BYTES****\e[0m\n\n", *p);
	int result = lzw_decode(&ed, p+1, *p);
	if(result<0){printf("Error Decoding image data, Terminating scaling operation\n"); return -1;}
	p+=(*p)+1;
	//printf("%02x->%02x->%02x\n",*(p-1),*p,*(p+1));
      }
      //printf("Image Data: ");
      //for(int j=0;j<d->width*d->height; j++)printf("%d ", ed.output_indexes[j]); printf("\n");
      d->image_color_index_data = ed.output_indexes;
      lzw_decode_free(&ed, 0);

      int image_data_size = (p-beginning_of_image)+1;
      d->image_data_size = image_data_size;
      d->image_data = (uint8_t *)malloc(image_data_size);
      memset(d->image_data, 0, d->image_data_size);
      memcpy(d->image_data, beginning_of_image, image_data_size);

      p+=1;
      gif->image_count++;
      //printf("END OF IMG %02x->%02x->%02x\n",*(p-1),*p,*(p+1));      
    }
  }
  gif->trailing_extensions = (LinkedList *)extensions;
  extensions=NULL;

  if(*p!=GIF_TRAILER){
    printf("At Missing Trailer: %02x->(%02x)->%02x\n",*(p-1),*p,*(p+1));
    printf("Extra Data found at end of Gif. Expected 0x3B\n");
    return -2;
  }else printf("All Bytes Processed and Accounted for.\n");
  return 0;
}

void gif_free(Gif *gif){
  LinkedList *images = &gif->image_descriptor_linked_list;

  if(images->length>0){
    LinkedListItem *item = images->first;
    LinkedListItem *nextitem;

    do{
      GifImageDescriptor *d = (GifImageDescriptor *)item->data;
      if(d->image_data!=NULL) free(d->image_data);
      if(d->has_lct) free(d->color_table);
      if(d->image_color_index_data) free(d->image_color_index_data);

      LinkedList *image_descriptor_extensions = d->extensions;
      LinkedListItem *extitem = image_descriptor_extensions->first;
      LinkedListItem *nextextitem;
      
      if(image_descriptor_extensions->length){
	do{
	  GifExtHeader *extheader = (GifExtHeader *)extitem->data;
	  if(extheader->data!=NULL) free(extheader->data);
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

  if(gif->has_gct) free(gif->color_table);
}
