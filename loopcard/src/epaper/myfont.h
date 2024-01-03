#ifndef _MYFONT_H_

#define _MYFONT_H_

#include <Arduino.h>

void test_ds_font();

int ds_get_bitmap(uint32_t letter,uint8_t *bitmap_buf,uint8_t *box_w,uint8_t *box_h,uint8_t *offset_x,uint8_t *offset_y);

#endif