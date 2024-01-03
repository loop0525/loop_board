#ifndef _LOOPSERVER_H_
#define _LOOPSERVER_H_


#include <string>  // int to string
#include <ArduinoJson.h> 
#include <WiFi.h>

#include <esp_wifi.h>     //用于esp_wifi_restore() 删除保存的wifi信息
// #include <Preferences.h> // NVS存储

#include <DNSServer.h> //域名解析

#include <SPIFFS.h>
#include "ESPAsyncWebServer.h"

#include "../vTaskLoopCard.h"
#include "preference_utl.h"

#define NTP1 "pool.ntp.org"
#define NTP2 "ntp.aliyun.com"
#define NTP3 "ntp3.aliyun.com"


bool testWifiConnect(); // 测试WIFI是否连接成功

String getLocalIP(); // 获取连接局域网内的IP


bool scanWiFi(); //扫描WiFi

void webserver_STA_init(); //配置STA模式webserver
void webserver_AP_init(); //配置AP模式webserver
void web_server(); //初始化启动webserver

void DNS_handleRequest();


void SetNtpTime(struct tm * timeinfo); // 获取NTP时间

#endif // !_WIFISERVER_H_