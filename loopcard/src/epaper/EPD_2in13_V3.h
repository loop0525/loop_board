
#ifndef __EPD_2in13_V3_H_
#define __EPD_2in13_V3_H_

#include "DEV_Config.h"

// Display resolution
#define EPD_2in13_V3_WIDTH       122
#define EPD_2in13_V3_HEIGHT      250

void EPD_2in13_V3_Init(void);
void EPD_2in13_V3_Clear(void);
void EPD_2in13_V3_Display(UBYTE *Image);
void EPD_2in13_V3_Display_Base(UBYTE *Image);
void EPD_2in13_V3_Display_Partial(UBYTE *Image);
void EPD_2in13_V3_Sleep(void);

#endif
