#ifndef  _DISPLAY_H_
#define  _DISPLAY_H_

#include <SPIFFS.h>


void display_init(); //初始化显示
void showTime(struct tm timeinfo); //显示时间

void showHome(int * selectPoint,int selectIndex); // 显示主页

void showSet(int * selectPoint,int selectIndex); // 显示设置

void showTest();

void Display_Partial(); // 刷新当前背景

#endif // ! _DISPLAY_H_
#define  _DISPLAY_H_

