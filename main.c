#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

typedef struct GifExtHeader{
  uint8_t type;
  
}ExtHeader;

typedef struct GifImageDescriptor{
  uint16_t left;
  uint16_t top;
  uint16_t width;
  uint16_t height;
  uint8_t has_lct;
  uint8_t interlaced;
  uint8_t sort;
  uint16_t lct_size;
  uint32_t *color_table;
  uint8_t LZW;
  uint8_t image_data;
} ImageDescriptor;

struct Gif{
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
  struct GifImageDescriptor **image_descriptors;
};//struct GifImageDescriptor

const uint8_t EXT_MARKER = 0x21;
const uint8_t IMG_MARKER = 0x2C;
const uint8_t GIF_TRAILER = 0x3B;

const int EXT_GCE_MARKER = 0xF9;
const uint8_t EXT_TEXT_MARKER = 0x01;
const uint8_t EXT_APP_MARKER = 0xFF;
const uint8_t EXT_COMMENT_MARKER = 0xFE;

void printGifData(struct Gif g);
uint8_t* loadFile(char *path, uint32_t* data_copied);

int main(int argc, char* argv[]){
  if(argc<2){ printf("Please Specify a gif Image to process.\n"); return 1; }

  struct Gif g;
  uint32_t data_copied = 0;
  uint8_t *source_data = loadFile(argv[1], &data_copied);;
  uint8_t *p = source_data;
  if(data_copied<0) return data_copied;

  memcpy(g.version, source_data, 6);
  
  p+=6;
  g.width = (uint16_t)(*(p+1)<<8 | *p);
  p+=2;
  g.height = (uint16_t)(*(p+1)<<8 | *p);
  p+=2;
  int gct = (uint8_t)*p;
  g.has_gct = gct&127>0;
  g.color_resolution = ((gct&0b01110000)>>4)+1;
  g.colors_sorted = gct&8;
  g.gct_size = g.has_gct?pow(2.0, (float)((gct&0b111)+1)):0;

  p+=1;
  g.bgcolor = (uint8_t)(*p);
  p+=1;
  g.aspect_ratio = (uint8_t)(*p);
  p+=1;

  if(g.has_gct){
    g.color_table = (uint32_t*)malloc(g.gct_size*3*sizeof(uint32_t));
    int i = 0;
    for(i=0; i<g.gct_size; i++){
      g.color_table[i] = (*p<<16) | (*(p+1)<<8) | *(p+2);
      p = p+3;
    }
  }else g.color_table=0;

  //CHECK PASS
  g.ext_count = 0;
  g.image_count = 0;
  uint8_t *pbackup = p;
  while(*p==EXT_MARKER||*p==IMG_MARKER){
    //printf("beginning of Preparse Loop\n");
    //printf("%02x->%02x->%02x\n",*(p-1),*p,*(p+1));
    if(*p==EXT_MARKER){
      //printf("Found EXT %02x\n", *(p+1));
      g.ext_count++;
      p+=2;
      while(*p!=0)
	p+=(*p)+1;
      p+=1;
      //printf("%02x->%02x->%02x\n",*(p-1),*p,*(p+1));
    }

    //Detect Images
    if(*p==IMG_MARKER){
      //printf("Found Image\n");
      //printf("%02x->%02x->%02x\n",*(p-1),*p,*(p+1));
      g.image_count++;
      p+=9;
      int has_lct = *p&128>0;
      int lct_size = (has_lct)?pow(2.0, (float)((*p&0b111)+1)):0;
      
      if(has_lct){
	int i = 0;
	for(i=0; i<lct_size; i++)
	  p = p+3;
      }

      p+=2;
      while(*p!=0){
	p+=(*p)+1;
      }
      p+=1;
    }
  }
  p=pbackup;
  
  printf("Allocating %d for descriptors\n", (int)sizeof(struct GifImageDescriptor*)*g.image_count);
  g.image_descriptors = malloc(sizeof(struct GifImageDescriptor*)*g.image_count);  

  //*************************************************************************
  int imageid = 0;
  while(*p==EXT_MARKER||*p==IMG_MARKER){
    //Detect Extentions
    if(*p==EXT_MARKER){
      p+=1;
      //int type = *p;
      //printf("%02x->%02x->%02x\n",*(p-1),*p,*(p+1));
      int type=*p;
      switch(type){
      case 0xF9: //EXT_GCE_MARKER:
	printf("Found GCE Header.");
	break;
      case 0x01: //EXT_TEXT_MARKER:
	printf("Found TEXT Header.");
	break;
      case 0xff: //EXT_APP_MARKER:
	printf("Found APP Header.");
	break;
      case 0xFE: //EXT_COMMENT_MARKER:
	printf("Found COMMENT Header.");
	break;
      default:
	printf("Unknown Ext '%02x'",*p);
      }
      p+=1;
      while(*p!=0){
	if(type!=EXT_GCE_MARKER&&type!=EXT_APP_MARKER)
	  printf(" (\"%s\")", p+1);
	p+=(*p)+1;
      }
      printf("\n");
      p+=1;
      /*if(source_data-p>0)
	printf("%02x->%02x->%02x\n",*(p-1),*p,*(p+1));
      else
        printf("%02x->%02x->EOF\n",*(p-1),*p);*/
    }
    
    if(*p==IMG_MARKER){
      //printf("Image Found Pass 2, StartByte: %02x\n", *p);
      p+=1;
      ImageDescriptor *d = malloc(sizeof(ImageDescriptor));
      d->left = (uint16_t)(*(p+1)<<8 | *p);
      p+=2;
      d->top = (uint16_t)(*(p+1)<<8 | *p);
      p+=2;
      d->width = (uint16_t)(*(p+1)<<8 | *p);
      p+=2;
      d->height = (uint16_t)(*(p+1)<<8 | *p);
      p+=2;
      int tmp = (uint8_t)*p;
      d->has_lct = tmp&128>0;
      d->interlaced = tmp&64>0;
      d->sort = tmp&32>0;
      d->lct_size = (d->has_lct)?pow(2.0, (float)((tmp&0b111)+1)):0;
      
      if(d->has_lct){
	d->color_table = (uint32_t*)malloc(d->lct_size*3*sizeof(uint32_t));
	int i = 0;
	for(i=0; i<d->lct_size; i++){
	  d->color_table[i] = (*p<<16) | (*(p+1)<<8) | *(p+2);
	  p = p+3;
	}
      }else d->color_table=0;
      
      p+=1;
      d->LZW = *p;
      p+=1;

      while(*p!=0){
	p+=(*p)+1;
	//printf("%02x->%02x->%02x\n",*(p-1),*p,*(p+1));
      }
      p+=1;
      g.image_descriptors[imageid] = d;
      imageid++;
      /*if(source_data-p>0)
	printf("%02x->%02x->%02x\n",*(p-1),*p,*(p+1));
      else
        printf("%02x->%02x->EOF\n",*(p-1),*p);*/
    }
  }

  if(*p!=GIF_TRAILER){
    printf("At Missing Trailer: %02x->%02x->%02x\n",*(p-1),*p,*(p+1));
    printf("Extra Data found at end of Gif. Expected 0x3B\n");
  }

  printGifData(g);

  if(*p==GIF_TRAILER){
    printf("All Bytes Processed and Accounted for.\n");
  }

  int i = 0;
  for(;i<g.image_count; i++){
    if(g.image_descriptors[i]->has_lct){
      free(g.image_descriptors[i]->color_table);
    }
    free(g.image_descriptors[i]);
  }
  free(g.image_descriptors);
  
  printf("Cleaning up\n");

  free(source_data);
  
  
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

void printGifData(struct Gif g){
  //printf("\nThe file format is: %s\n", g.version);
  printf("\nLogical Screen Descriptor\n");
  printf("WIDTH:    %d (0x%04x)\n", g.width, g.width);
  printf("HEIGHT:   %d (0x%04x)\n", g.height, g.height);
  printf("has_gct:  %d (0x%02x)\n", g.has_gct, g.has_gct);
  printf("c_res:    %d (0x%02x)\n", g.color_resolution, g.color_resolution);
  printf("c_sorted: %d (0x%02x)\n", g.colors_sorted, g.colors_sorted);
  printf("gct_size: %d (0x%02x)\n", g.gct_size, g.gct_size);
  printf("BGCOLOR:  %d (0x%02x)\n", g.bgcolor, g.bgcolor);
  printf("ARATIO:   %d (0x%02x)\n", g.aspect_ratio, g.aspect_ratio);

  if(g.has_gct){
    printf("\nGLOBAL COLOR TABLE FOUND\n");
    /*printf("\nGLOBAL COLOR TABLE VALUES");
    int i = 0;
    for(i=0; i<g.gct_size; i++){
      if(i%16==0){
	printf("\n%d: ", i);
	if(i<100) printf(" ");
	if(i<10) printf(" ");
      }
      printf("%06x ", g.color_table[i]);
      }
      printf("\n");*/
  }
  printf("\n");
  
  if(g.ext_count){
    printf("FOUND %d EXTENSION(S)\n\n", g.ext_count);
  }else{
    printf("NO EXTENSIONS FOUND\n\n");
  }

  printf("Number of Images: %d\n", g.image_count);
  int i = 0;
  for(;i<g.image_count; i++){
    printf("\nImage %d/%d\n", i+1, g.image_count);
    struct GifImageDescriptor *d = g.image_descriptors[i];
    printf("LEFT:       %d (0x%04x)\n", d->left, d->left);
    printf("TOP:        %d (0x%04x)\n", d->top, d->top);
    printf("WIDTH:      %d (0x%02x)\n", d->width, d->width);
    printf("HEIGHT:     %d (0x%02x)\n", d->height, d->height);
    printf("has_lct:    %d (0x%02x)\n", d->has_lct, d->has_lct);
    printf("interlaced: %d (0x%02x)\n", d->interlaced, d->interlaced);
    printf("SORT:       %d (0x%02x)\n", d->sort, d->sort);
    printf("LCT_SIZE:   %d (0x%02x)\n", d->lct_size, d->lct_size);
    //printf("%d left\n",(g.image_count-(i+1)));
    }
  printf("\n");
}



	//if(type!=EXT_GCE_MARKER){
	  //uint8_t *comment = malloc(sizeof(uint8_t)*(*(p+1)+1));
	  //memcpy(comment, p+1, *p);
	  //printf("%s\n", comment);
	  //printf("FREEING");
	  //free(comment);
	//}


/*

      int type=*p;
      switch(type){
      case 0xF9: //EXT_GCE_MARKER:
	printf("Found GCE Header.\n");
	break;
      case 0x01: //EXT_TEXT_MARKER:
	printf("Found TEXT Header.\n");
	break;
      case 0xff: //EXT_APP_MARKER:
	printf("Found APP Header.\n");
	break;
      case 0xFE: //EXT_COMMENT_MARKER:
	printf("Found COMMENT Header.\n");
	break;
      default:
	printf("Unknown Ext '%02x'\n",*p);
      }

*/
