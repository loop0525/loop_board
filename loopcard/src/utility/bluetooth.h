#ifndef _BLUETOOTH_H_
#define _BLUETOOTH_H_

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "../vTaskLoopCard.h"

#include "preference_utl.h"
#include "loopserver.h"


void BLE_Init(); //初始化蓝牙
void BLE_Stop(); // 关闭BLE



#endif // ! _BLUETOOTH_H_