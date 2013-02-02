#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "main.h"
#include "linkedlist.h"

typedef struct GifExtHeader_t{
  uint8_t type;
  uint8_t *data;
} GifExtHeader;

typedef struct GifImageDescriptor_t{
  uint16_t left;
  uint16_t top;
  uint16_t width;
  uint16_t height;
  uint8_t has_lct;
  uint8_t interlaced;
  uint8_t sort;
  uint16_t lct_size;
  uint32_t *color_table;
  LinkedList *extensions;
  uint8_t LZW;
  int image_data_size;
  uint8_t *image_data;
} GifImageDescriptor;

typedef struct Gif_t{
  char version[7];
  //Logical Screen Descriptor
  uint16_t width;
  uint16_t height;
  uint8_t bgcolor;
  uint8_t aspect_ratio;
  //GCT Data
  uint8_t has_gct;
  uint8_t color_resolution;
  uint8_t colors_sorted;
  uint16_t gct_size;
  //Global Color Table
  uint32_t *color_table;
  //Graphics Control Ext
  uint32_t ext_count;
  //Image Descriptor
  uint32_t image_count;
  LinkedList image_descriptor_linked_list;
  LinkedList *trailing_extensions;
} Gif;

const uint8_t EXT_MARKER = 0x21;
const uint8_t IMG_MARKER = 0x2C;
const uint8_t GIF_TRAILER = 0x3B;

const uint8_t EXT_GCE_MARKER = 0xF9;
const uint8_t EXT_TEXT_MARKER = 0x01;
const uint8_t EXT_APP_MARKER = 0xFF;
const uint8_t EXT_COMMENT_MARKER = 0xFE;

void printGifData(Gif g);
uint8_t* loadFile(char *path, uint32_t* data_copied);
uint32_t* extract_color_table(uint8_t *data, uint16_t table_size, int *delta_data);

int main(int argc, char* argv[]){
  if(argc<2){ printf("Please Specify a gif Image to process.\n"); return 1; }

  Gif g;
  memset(&g, 0, sizeof(Gif));
  uint32_t data_copied = 0;
  uint8_t *source_data = loadFile(argv[1], &data_copied);;
  uint8_t *p = source_data;
  if(data_copied<0) return data_copied;
  memcpy(g.version, source_data, 6);

  LinkedList *images = &g.image_descriptor_linked_list;
  images->length=0;

  LinkedList *extensions = (LinkedList *)malloc(sizeof(LinkedList));
  memset(extensions, 0, sizeof(LinkedList));
  
  //Extract Main Gif Header
  p+=6;
  g.width = (uint16_t)(*(p+1)<<8 | *p);
  p+=2;
  g.height = (uint16_t)(*(p+1)<<8 | *p);
  p+=2;
  int gct = (uint8_t)*p;
  g.has_gct = (gct&128)==128;
  g.color_resolution = ((gct&0b01110000)>>4)+1;
  g.colors_sorted = gct&8;
  g.gct_size = g.has_gct?pow(2.0, (float)((gct&0b111)+1)):0;
  p+=1;
  g.bgcolor = (uint8_t)(*p);
  p+=1;
  g.aspect_ratio = (uint8_t)(*p);
  p+=1;
  g.ext_count = 0;
  g.image_count = 0;

  //Get Global Color Table if it exists
  int deltap = 0;
  g.color_table = g.has_gct?extract_color_table(p, g.gct_size, &deltap):0;
  p+=deltap;

  //CHECK PASS
  uint8_t *pbackup = p;
  while(*p==EXT_MARKER||*p==IMG_MARKER){
    if(*p==EXT_MARKER){
      g.ext_count++;
      p+=1;
      int type=*p;
      LinkedListItem *item = addNewLinkedListItem(extensions);
      item->data = malloc(sizeof(GifExtHeader));
      memset(item->data, 0, sizeof(GifExtHeader));
      GifExtHeader *exth = (GifExtHeader *)item->data;
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
      d->left = (uint16_t)(*(p+1)<<8 | *p);
      p+=2;
      d->top = (uint16_t)(*(p+1)<<8 | *p);
      p+=2;
      d->width = (uint16_t)(*(p+1)<<8 | *p);
      p+=2;
      d->height = (uint16_t)(*(p+1)<<8 | *p);
      p+=2;

      int tmp = (uint8_t)*p;
      d->has_lct = (tmp&128)>0;
      d->interlaced = (tmp&64)>0;
      d->sort = (tmp&32)>0;
      d->lct_size = (d->has_lct)?pow(2.0, (float)((tmp&0b111)+1)):0;

      int deltap = 0;
      d->color_table = d->has_lct?extract_color_table(p, d->lct_size, &deltap):0;
      p+=deltap;
            
      p+=1;
      d->LZW = *p;
      p+=1;
      
      uint8_t *beginning_of_image = p; 
      while(*p!=0){
	p+=(*p)+1;
	//printf("%02x->%02x->%02x\n",*(p-1),*p,*(p+1));
      }
      int image_data_size = p-beginning_of_image;
      d->image_data_size = image_data_size;
      d->image_data = (uint8_t *)malloc(image_data_size);
      //p = beginning_of_image;
      memcpy(d->image_data, beginning_of_image, image_data_size);

      p+=1;
      g.image_count++;
      //printf("END OF IMG %02x->%02x->%02x\n",*(p-1),*p,*(p+1));      
    }
  }
  g.trailing_extensions = (LinkedList *)extensions;
  extensions=NULL;

  if(*p!=GIF_TRAILER){
    printf("At Missing Trailer: %02x->%02x->%02x\n",*(p-1),*p,*(p+1));
    printf("Extra Data found at end of Gif. Expected 0x3B\n");
  }
  
  printGifData(g);

  printf("%d Images\n",(int)images->length);

  if(*p==GIF_TRAILER){
    printf("All Bytes Processed and Accounted for.\n");
  }
  
  printf("Cleaning up\n");
  free(source_data);  

  if(images->length>0){
    LinkedListItem *item = images->first;
    LinkedListItem *nextitem;

    do{
      GifImageDescriptor *d = (GifImageDescriptor *)item->data;
      if(d->image_data!=NULL) free(d->image_data);
      if(d->has_lct) free(d->color_table);

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
  free(g.trailing_extensions);
  disposeLinkedList(images);

  //free(g.image_descriptors);
  if(g.has_gct) free(g.color_table);
}

uint8_t* loadFile(char *path, uint32_t* data_copied){
  printf("File: %s\n",path);
  FILE *fp = fopen(path, "r");
  uint8_t *buffer;

  if(fp == NULL){
    printf("Error Opening Image!\n");
    *data_copied = -2;
    return buffer;
  }
  if(fseek(fp, 0L, SEEK_END)!=0){
    printf("Error Seeking In File!\n");
    *data_copied = -3;
    return buffer;
  }

  long filesize = ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  printf("File Size: %lu bytes\n",filesize);
  buffer = (uint8_t*)malloc(filesize);
  size_t returnedlength = fread(buffer, sizeof(char), filesize, fp);
  fclose(fp);
  printf("Closing the file\n");
  return buffer;
}

uint32_t* extract_color_table(uint8_t *data, uint16_t table_size, int *delta_data){
  uint32_t *color_table = (uint32_t*)malloc(table_size*3*sizeof(uint32_t));
  int i;
  for(i=0; i<table_size; i++){
    color_table[i] = (*data<<16) | (*(data+1)<<8) | *(data+2);
    *delta_data+=3;
    data+=3;
  }
  return color_table;
}

void printColorTable(uint16_t size, uint32_t *color_table){
  printf("COLOR TABLE VALUES");
  int i = 0;
  for(i=0; i<size; i++){
    if(i%29==0){
      printf("\n  %d: ", i);
      if(i<100) printf(" ");
      if(i<10) printf(" ");
    }
    printf("%06x ", color_table[i]);
  }
  printf("\n\n");
}

void printGifData(Gif g){
  //printf("\nThe file format is: %s\n", g.version);
  printf("Logical Screen Descriptor\n");
  printf("WIDTH:    %d (0x%04x)\n", g.width, g.width);
  printf("HEIGHT:   %d (0x%04x)\n", g.height, g.height);
  printf("has_gct:  %d (0x%02x)\n", g.has_gct, g.has_gct);
  printf("c_res:    %d (0x%02x)\n", g.color_resolution, g.color_resolution);
  printf("c_sorted: %d (0x%02x)\n", g.colors_sorted, g.colors_sorted);
  printf("gct_size: %d (0x%02x)\n", g.gct_size, g.gct_size);
  printf("BGCOLOR:  %d (0x%02x)\n", g.bgcolor, g.bgcolor);
  printf("ARATIO:   %d (0x%02x)\n", g.aspect_ratio, g.aspect_ratio);

  if(g.has_gct)//{
    //printf("\nGLOBAL COLOR TABLE FOUND\n");
    printColorTable(g.gct_size, g.color_table);
    //}

  if(g.ext_count){
    //printf("FOUND %d EXTENSION(S)\n\n", g.ext_count);
  }else{
    printf("NO EXTENSIONS FOUND\n\n");
  }

  printf("Number of Images: %d\n", g.image_count);
  if(g.image_descriptor_linked_list.length>0){
    LinkedListItem *item = g.image_descriptor_linked_list.first;
    LinkedListItem *nextitem;

    int i = 0;
    do{
      nextitem = item->next;
      GifImageDescriptor *d = (GifImageDescriptor*)item->data;
      printf("\nImage %d/%d\n", i+1, g.image_count);
      //GifImageDescriptor *d = g.image_descriptor_linked_list[i];
      printf("LEFT:       %d (0x%04x)\n", d->left, d->left);
      printf("TOP:        %d (0x%04x)\n", d->top, d->top);
      printf("WIDTH:      %d (0x%02x)\n", d->width, d->width);
      printf("HEIGHT:     %d (0x%02x)\n", d->height, d->height);
      printf("has_lct:    %d (0x%02x)\n", d->has_lct, d->has_lct);
      printf("interlaced: %d (0x%02x)\n", d->interlaced, d->interlaced);
      printf("SORT:       %d (0x%02x)\n", d->sort, d->sort);
      printf("LCT_SIZE:   %d (0x%02x)\n", d->lct_size, d->lct_size);
      if(d->has_lct) 
	printColorTable(d->lct_size, d->color_table);
      LinkedList *extensions = (LinkedList *)d->extensions;
      printf("ImageData: %d bytes\n", d->image_data_size);
      /*uint8_t *p = d->image_data;
      while(*p!=0){
	//printf("%x\n",*p);
	int size=*p;
	int j;
	for(j=0; j<size; j++){
	  printf("%02x",*p);
	  p+=1;
	}
	p+=1;
      }
      printf("\n");*/


      printf("Number of Extensions: %d\n", (int)extensions->length);
      if(extensions->length>0){
	LinkedListItem *item = extensions->first;
	LinkedListItem *nextitem;

	int i = 0;
	do{
	  nextitem = item->next;
	  GifExtHeader *exth = (GifExtHeader *)item->data;
	  printf("  Header %d/%d: ", i+1, (int)extensions->length);
	  switch(exth->type){
	  case 0xF9: //EXT_GCE_MARKER:
	    printf("GCE Header.\n");
	    printf("    Disposal Method:   %d\n", (*(exth->data+1)&0b11100)>>2);
            printf("    User Input Flag:   %d\n", (*(exth->data+1)&0b10)>0);
	    printf("    Transparency:      %d\n", *(exth->data+1)&0b1);
	    printf("    Delay Time:        %d\n", (uint16_t)(*(exth->data+3)<<8 | *(exth->data+1)));
	    printf("    Transparent Index: %x\n", *(exth->data+4));
	    break;
	  case 0x01: //EXT_TEXT_MARKER:
	    printf("TEXT Header.\n");
	    printf("    (\"%s\")", exth->data+1);
	    break;
	  case 0xff: //EXT_APP_MARKER:
	    printf("APP Header.");
	    break;
	  case 0xFE: //EXT_COMMENT_MARKER:
	    printf("COMMENT Header.\n");
    	    printf("    (\"%s\")", exth->data+1);
	    break;
	  default:
	    printf("Unknown Ext '%02x'",exth->type);
	  }
	  //if(exth->type!=EXT_GCE_MARKER)//&&exth->type!=EXT_APP_MARKER)
	  i++;
	  item=nextitem;
	  printf("\n");
	}while(item!=0);
	//	printf("\n");
      }
      i++;
      item=nextitem;
    }while(item!=0);
  }
}

