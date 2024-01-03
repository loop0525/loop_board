#include <Arduino.h>
#include <Preferences.h>  // nvs分区数据保存

#include <stdlib.h>

#include "./vTaskLoopCard.h"

#include "./utility/loopserver.h"
#include "./utility/bluetooth.h"



void setup()
{
    Serial.begin(115200);
    
    vTaskStart();
    // BLE_Init();
    

   
}

void loop()
{
    // DNS_handleRequest();
    // Monitor_Connect();
}
