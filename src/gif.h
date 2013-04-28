#include "linkedlist.h"
#include <stdint.h>

#ifndef GIF
#define GIF

#define EXT_MARKER (uint8_t)0x21
#define IMG_MARKER (uint8_t)0x2C
#define GIF_TRAILER (uint8_t)0x3B

#define EXT_GCE_MARKER (uint8_t)0xF9
#define EXT_TEXT_MARKER (uint8_t)0x01
#define EXT_APP_MARKER (uint8_t)0xFF
#define EXT_COMMENT_MARKER (uint8_t)0xFE


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
  uint8_t *image_color_index_data;
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

int gif_load(Gif *g, uint8_t *p);
void gif_free(Gif *g);
#endif
