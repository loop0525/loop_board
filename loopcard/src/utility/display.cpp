#include "display.h"
#include <Arduino.h>

#include "../epaper/DEV_Config.h"
#include "../epaper/EPD.h"
#include "../epaper/GUI_Paint.h"
#include "../epaper/ImageData.h"

#include "../epaper/myfont.h"


// 墨水屏背景屏
UBYTE *BlackImage;

// 显示时间所用变量
int32_t lastMin = 61;
String Hour;
String Min;


void display_init(){
     DEV_Module_Init();
    EPD_2in13_V3_Init();
    EPD_2in13_V3_Clear();

    UWORD Imagesize = ((EPD_2in13_V3_WIDTH % 8 == 0)? (EPD_2in13_V3_WIDTH / 8 ): (EPD_2in13_V3_WIDTH / 8 + 1)) * EPD_2in13_V3_HEIGHT;
	if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) 
	{
		printf("Failed to apply for black memory...\r\n");
		while (1);
	}

    Paint_NewImage(BlackImage, EPD_2in13_V3_WIDTH, EPD_2in13_V3_HEIGHT, 270, WHITE);  	
	Paint_SelectImage(BlackImage);
	Paint_Clear(WHITE);
}

void showTime(struct tm timeinfo){
    Hour = (String)timeinfo.tm_hour;
    Min =  (String)timeinfo.tm_min;

    // Paint_ClearWindows(5, 15, Font68.Width * 5 + 5, Font68.Height + 15, WHITE);

    // Paint_ClearWindows(5, 15, 5 + Font68.Width * 2, 15 + Font68.Height, WHITE);
    // Paint_ClearWindows(5+Font68.Width * 3, 15, 5+Font68.Width * 5, Font68.Height+15, WHITE);
    if(timeinfo.tm_hour<10){
      Paint_DrawString_EN(5, 15, ("0"+Hour).c_str(), &Font68, WHITE, BLACK);
    }else{
      Paint_DrawString_EN(5, 15, Hour.c_str(), &Font68, WHITE, BLACK);
    }
    
    Paint_DrawString_EN(Font68.Width*2, 8,":", &Font68, WHITE, BLACK);

    if(timeinfo.tm_min<10){
      Paint_DrawString_EN(Font68.Width*3, 15, ("0" + Min).c_str(), &Font68, WHITE, BLACK);
    }else{
      Paint_DrawString_EN(Font68.Width*3, 15, Min.c_str(), &Font68,WHITE, BLACK);
    }
    
    EPD_2in13_V3_Display_Partial(BlackImage);
}


void showHome(int * selectPoint,int selectIndex){
  Paint_Clear(WHITE);
  Paint_DrawImage(gImage_weather,5,175,48,48);

  Paint_DrawCircle(selectPoint[selectIndex*2-2],selectPoint[selectIndex*2-1],3, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY); // 选中点

// Paint_DrawCircle(51,53,3, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY); // 默认选中点
// Paint_DrawCircle(125,53,3, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
// Paint_DrawCircle(199,53,3, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
// Paint_DrawCircle(51,117,3, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
// Paint_DrawCircle(125,117,3, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
// Paint_DrawCircle(199,117,3, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);

  Paint_DrawImage(gImage_clock,69,175,48,48);
  Paint_DrawImage(gImage_tomato,5,101,48,48);
  Paint_DrawImage(gImage_read,69,101,48,48);
  Paint_DrawImage(gImage_sleep,5,27,48,48);
  Paint_DrawImage(gImage_set,69,27,48,48);
  Display_Partial(); // 刷新当前背景
}

void showSet(int * selectPoint,int selectIndex){
  Paint_Clear(WHITE);

  Paint_DrawImage(gImage_set_wifi,25,174,48,48);
  Paint_DrawImage(gImage_set_bluetooth,25,101,48,48);
  Paint_DrawImage(gImage_set_miniprogram,25,28,48,48);

  Paint_DrawCircle(53,82,3, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY); // 选中点
  // Paint_DrawCircle(125,82,3, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
  // Paint_DrawCircle(197,82,3, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
  Display_Partial(); // 刷新当前背景
}


void showTest(){
  // Paint_DrawCircle(53,82,10, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY); // 选中点
  Paint_DrawString_CN2(0,25,"哈哈,张鲁滇大傻逼！",  WHITE, BLACK);

  // test_ds_font();

  Display_Partial(); // 刷新当前背景

}


void Display_Partial(){ //刷新当前背景
  EPD_2in13_V3_Display_Partial(BlackImage);
}