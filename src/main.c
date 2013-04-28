#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "main.h"
#include "linkedlist.h"
#include "lzw.h"
#include "gif.h"

//void printGifData(Gif g);
//uint8_t* loadFile(char *path, uint32_t* data_copied);

uint8_t* loadFile(char *path, uint32_t* data_copied){
  printf("File: %s\n",path);
  FILE *fp = fopen(path, "r");
  //FILE *out = fopen("/home/diamondman/Documents/gifscaler/testcpy.gif","w+");
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

  //printf("BUFFER 6 %c\n", buffer[6]);
  //buffer[6]=00;
  //fwrite(buffer, 1, returnedlength, out);

  fclose(fp);
  //fclose(out);
  printf("Closing the file\n");
  return buffer;
}

int main(int argc, char* argv[]){
  if(argc<2){ printf("Please Specify a gif Image to process.\n"); return 1; }

  Gif g;
  memset(&g, 0, sizeof(Gif));
  uint32_t data_copied = 0;
  uint8_t *source_data = loadFile(argv[1], &data_copied);;
  uint8_t *p = source_data;
  if(data_copied<0) return data_copied;

  memcpy(g.version, source_data, 6);

  gif_load(&g, p);

  LinkedList *images = &g.image_descriptor_linked_list;
  //images->length=0;

  printGifData(g);

  printf("%d Images\n",(int)images->length);

  if(*p==GIF_TRAILER){
    printf("All Bytes Processed and Accounted for.\n");
  }
  
  printf("Cleaning up\n");
  free(source_data);  

  gif_free(&g);
}

