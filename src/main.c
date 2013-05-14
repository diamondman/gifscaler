#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include "gif.h"

uint8_t* loadFile(char *path, uint32_t* data_copied){
  struct stat buf;
  FILE *fp;
  uint8_t *buffer;

  if(stat(path, &buf) < 0){
    printf("FILE DOESN'T EXIST!\n");
    *data_copied = -1;
    return NULL;
  }
  if(!S_ISREG(buf.st_mode)){
   printf("Please Specify a file, not a directory/block device/etc.\n");
   *data_copied = -2;
   return NULL;
  }
  if((fp = fopen(path, "r")) == NULL){
    printf("Error Opening Image!\n");
    *data_copied = -3;
    return NULL;
  }
  if(buf.st_size > 0x80000000){
    printf("Provided file too big! Max Size 2GB.\n");
    *data_copied = -4;
    return NULL;
  }

  buffer = (uint8_t*)malloc(buf.st_size);
  *data_copied = fread(buffer, sizeof(char), buf.st_size, fp);

  fclose(fp);
  return buffer;
}

int main(int argc, char* argv[]){
  if(argc<2){ printf("Please Specify a gif Image to process.\n"); return 1; }

  Gif g;
  memset(&g, 0, sizeof(Gif));
  int32_t data_copied = 0;
  int8_t *source = loadFile(argv[1], &data_copied);
  if(source==NULL) return -1;

  int loadres = gif_load(&g, source, data_copied);
  free(source);
  if (loadres>0) gif_printImageData(&g);
  gif_free(&g);
}
