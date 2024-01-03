#ifndef  _VTASKLOOPCARD_H_
#define _VTASKLOOPCARD_H_

#include <Arduino.h>
#include "./epaper/DEV_Config.h"
#include "./epaper/EPD.h"
#include "./epaper/GUI_Paint.h"
#include "./epaper/ImageData.h"

#include "./utility/display.h"
#include "./utility/encoder.h"
#include "./utility/loopserver.h"
#include "./utility/bluetooth.h"



void StartBLE(); // 开始蓝牙通信
void StopBLETask(); // 停止监听BLE任务


void vTaskWeatherSwitch(void * pvParameters); //处理天气页任务切换
void vTaskTomatoSwitch(void * pvParameters); //处理番茄页任务切换
void vTaskSleepSwitch(void * pvParameters); //处理睡眠页任务切换
void vTaskTimeSwitch(void * pvParameters); //处理时钟页任务切换
void vTaskReadSwitch(void * pvParameters); //处理阅读页任务切换
void vTaskSetSwitch(void * pvParameters); //处理设置页任务切换



void vTaskStart();

void timer_init(); //初始化硬件定时器 计算时间



void SuspendTask_ScanWifi();//挂起扫描wifi
void ResumeTask_ScanWifi();//恢复扫描wifi


#endif // ! _VTASKLOOPCARD_H_