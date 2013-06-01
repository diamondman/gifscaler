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

#define DECODERSTATE_START      1
#define DECODERSTATE_GCT        2
#define DECODERSTATE_IMAGES     3
#define DECODERSTATE_ENDING     4
#define DECODERSTATE_FINISHED   5

#define DATADECODERSTATE_START  0
#define DATADECODERSTATE_  1

#define GIFRET_ALREADYFINISHED  5
#define GIFRET_NEEDMOREDATA     4
#define GIFRET_DONE             3
#define GIFRET_DONEEXTRADATA    2
#define GIFRET_NOCHANGE         0
#define GIFRET_INVALIDSTATE    -1
#define GIFRET_STREAMDATAERROR -2

#define GIFERROR_NOERROR        0
#define GIFERROR_INVALIDBYTE    1
#define GIFERROR_UNSUPPORTEDFORMAT 2

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

typedef struct Gif_t Gif;
struct Gif_t {
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
  //Streaming data
  uint8_t status;
  uint8_t stage_status;
  uint8_t stream_error;
  LinkedList *tmp_extensions;
  uint8_t *tmp_buffer;
  uint32_t tmp_buffer_offset;
  uint32_t tmp_buffer_length;
  uint32_t tmp_buffer_data_length;
  void *decoder_scratch;
};

void gif_load_initialize(Gif *g);
int gif_load(Gif *g, uint8_t *p, uint32_t p_length);
void gif_free(Gif *g);

//UTILS
void gif_printColorTable(uint16_t size, uint32_t *color_table);
void gif_printImageData(Gif *g);
#endif
