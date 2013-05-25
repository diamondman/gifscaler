#include <stdio.h>
#include "gif.h"
#include "linkedlist.h"

void gif_printColorTable(uint16_t size, uint32_t *color_table){
  printf("COLOR TABLE VALUES");
  for(int i=0; i<size; i++){
    if(i%29==0){
      printf("\n  %d: ", i);
      if(i<100) printf(" ");
      if(i<10) printf(" ");
    }
    printf("%06x ", color_table[i]);
  }
  printf("\n\n");
}

void gif_printImageData(Gif *g){
  printf("Gif Decode Status: %s\n\n", (g->status==DECODERSTATE_FINISHED ? "Complete!" : "Incomplete Or Error!"));
  printf("Logical Screen Descriptor\n");
  printf("WIDTH:    %d (0x%04x)\n", g->width, g->width);
  printf("HEIGHT:   %d (0x%04x)\n", g->height, g->height);
  printf("has_gct:  %d (0x%02x)\n", g->has_gct, g->has_gct);
  printf("c_res:    %d (0x%02x)\n", g->color_resolution, g->color_resolution);
  printf("c_sorted: %d (0x%02x)\n", g->colors_sorted, g->colors_sorted);
  printf("gct_size: %d (0x%02x)\n", g->gct_size, g->gct_size);
  printf("BGCOLOR:  %d (0x%02x)\n", g->bgcolor, g->bgcolor);
  printf("ARATIO:   %d (0x%02x)\n", g->aspect_ratio, g->aspect_ratio);

  if(g->has_gct) gif_printColorTable(g->gct_size, g->color_table);

  if(g->ext_count)
    printf("FOUND %d EXTENSION(S)\n\n", g->ext_count);
  else
    printf("NO EXTENSIONS FOUND\n\n");

  printf("Number of Images: %d\n", g->image_count);
  if(g->image_descriptor_linked_list.length>0){
    LinkedListItem *item = g->image_descriptor_linked_list.first;
    LinkedListItem *nextitem;

    int i = 0;
    do{
      nextitem = item->next;
      GifImageDescriptor *d = (GifImageDescriptor*)item->data;

      printf("\nImage %d/%d\n", i+1, g->image_count);
      printf("LEFT:       %d (0x%04x)\n", d->left, d->left);
      printf("TOP:        %d (0x%04x)\n", d->top, d->top);
      printf("WIDTH:      %d (0x%02x)\n", d->width, d->width);
      printf("HEIGHT:     %d (0x%02x)\n", d->height, d->height);
      printf("has_lct:    %d (0x%02x)\n", d->has_lct, d->has_lct);
      printf("interlaced: %d (0x%02x)\n", d->interlaced, d->interlaced);
      printf("SORT:       %d (0x%02x)\n", d->sort, d->sort);
      printf("LCT_SIZE:   %d (0x%02x)\n", d->lct_size, d->lct_size);

      if(d->has_lct) gif_printColorTable(d->lct_size, d->color_table);
      LinkedList *extensions = (LinkedList *)d->extensions;
      printf("ImageData: %d bytes\n", d->image_data_size);      
      uint8_t *p = d->image_data;
      while(*p!=0){
	//printf("%x\n",*p);
	int size=*p;
	int j;
	for(j=0; j<size; j++){
	  //printf("%02x ",*p);
	  p+=1;
	}
	p+=1;
      }
      printf("\n");

      printf("Decoded Image Data: ");
      printf("%d bytes\n", d->width*d->height);
      if(d->has_lct){
	for(int j=0;j<d->width*d->height; j++)printf("%06x ", d->color_table[d->image_color_index_data[j]]); printf("\n");
      }else{
	for(int j=0;j<d->width*d->height; j++)printf("%06x ", g->color_table[d->image_color_index_data[j]]); printf("\n");
      }

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
	  case EXT_GCE_MARKER:
	    printf("GCE Header.\n");
	    printf("    Disposal Method:   %d\n", (*(exth->data+1)&0b11100)>>2);
            printf("    User Input Flag:   %d\n", (*(exth->data+1)&0b10)>0);
	    printf("    Transparency:      %d\n", *(exth->data+1)&0b1);
	    printf("    Delay Time:        %d\n", (uint16_t)(*(exth->data+3)<<8 | *(exth->data+1)));
	    printf("    Transparent Index: %x\n", *(exth->data+4));
	    break;
	  case EXT_TEXT_MARKER:
	    printf("TEXT Header.\n");
	    printf("    (\"%s\")", exth->data+1);
	    break;
	  case EXT_APP_MARKER:
	    printf("APP Header.");
	    break;
	  case EXT_COMMENT_MARKER:
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
