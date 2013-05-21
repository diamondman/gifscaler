#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include "gif.h"

int loadFile(uint8_t **filebuffer, uint32_t* data_copied, char *path){
  struct stat buf;
  FILE *fp;

  if (stat(path, &buf) < 0){
    printf("FILE DOESN'T EXIST!\n");
    return -1;
  }
  if (!S_ISREG(buf.st_mode)){
   printf("Please Specify a file, not a directory/block device/etc.\n");
   return -2;
  }
  if ((fp = fopen(path, "r")) == NULL){
    printf("Error Opening Image!\n");
    return -3;
  }
  if (buf.st_size > 0x80000000){
    printf("Provided file too big! Max Size 2GB.\n");
    return -4;
  }

  *filebuffer = (uint8_t*)malloc(buf.st_size);
  *data_copied = fread(*filebuffer, sizeof(char), buf.st_size, fp);

  fclose(fp);
  return 0;
}

int main(int argc, char* argv[]){
  if (argc<2){ printf("Please Specify a gif Image to process.\n"); return -1; }

  Gif g;
  memset(&g, 0, sizeof(Gif));
  uint8_t *filebuffer = NULL;
  int32_t data_copied = 0;

  if (loadFile(&filebuffer, &data_copied, argv[1])<0) return -2;
  if (gif_load(&g, filebuffer, data_copied)>=0) gif_printImageData(&g);
  free(filebuffer);
  gif_free(&g);
}
