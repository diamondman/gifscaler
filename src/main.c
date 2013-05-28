#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include "gif.h"

int loadFile(uint8_t **filebuffer, uint32_t* data_copied, char *path){
  struct stat filestatus;
  FILE *file;

  if (stat(path, &filestatus) < 0){
    printf("FILE DOESN'T EXIST!\n");
    return -1;
  }else if (!S_ISREG(filestatus.st_mode)){
   printf("Please Specify a file, not a directory/block device/etc.\n");
   return -2;
  }else if ((file = fopen(path, "r")) == NULL){
    printf("Error Opening Image!\n");
    return -3;
  }else if (filestatus.st_size > 0x80000000){
    printf("Provided file too big! Max Size 2GB.\n");
    return -4;
  }

  *filebuffer = (uint8_t*)malloc(filestatus.st_size);
  *data_copied = fread(*filebuffer, sizeof(char), filestatus.st_size, file);

  fclose(file);
  return 0;
}

int main(int argc, char* argv[]){
  if (argc<2){ printf("Please Specify a gif Image to process.\n"); return -1; }

  uint8_t *filebuffer = NULL;
  int32_t data_copied = 0;
  if (loadFile(&filebuffer, &data_copied, argv[1])<0) return -2;

  Gif *gif = (Gif*)malloc(sizeof(Gif));
  gif_load_initialize(gif);

  //if (gif_load(g, filebuffer, data_copied)>=0) gif_printImageData(g);
  gif_load(gif, filebuffer, 5);
  gif_load(gif, filebuffer+5, 15);
  //gif_load(gif, filebuffer+20, 180);
  gif_load(gif, filebuffer+20, data_copied-20);
  //gif_load(gif, filebuffer+34, data_copied-14);
  //gif_load(gif, filebuffer+(data_copied-1), 1);
  gif_printImageData(gif);

  free(filebuffer);
  gif_free(gif);
  free(gif);
}
