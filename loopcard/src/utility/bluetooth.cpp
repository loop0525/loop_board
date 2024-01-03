#include "bluetooth.h"


std::string txValue = "hello";                         //后面需要发送的值
BLEServer *pServer = NULL;                   //BLEServer指针 pServer //创建BLE服务器
BLEService *pService = NULL;                    //创建BLE服务
BLECharacteristic *pTxCharacteristic = NULL; //BLECharacteristic指针 pTxCharacteristic
bool deviceConnected = false;                //本次连接状态
bool oldDeviceConnected = false;             //上次连接状态d
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
 
class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
        Serial.println("已连接此设备");
    };
 
    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
        Serial.println("断开连接此设备");
    }
};

String reStr;
String reStr_Flag;
String STA_SSID;
String STA_PASSWORD;
String AP_SSID;
String AP_PASSWORD;

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue();

        if (rxValue.length() > 0) {
            for (int i = 0; i < rxValue.length(); i++){
                if(reStr == "STA_N"){
                    STA_SSID  += rxValue[i];
                    continue;
                }
                else if(reStr == "STA_P"){
                    STA_PASSWORD  += rxValue[i];
                    continue;
                }
                else if(reStr == "AP_N"){
                    AP_SSID  += rxValue[i];
                    continue;
                }
                else if(reStr == "AP_P"){
                    AP_PASSWORD  += rxValue[i];
                    continue;
                }
                reStr += rxValue[i];
            }  
            if(reStr == "保存配置"){
                if(STA_SSID != "" && STA_PASSWORD != ""){
                    savePref_Str("wifi", "STA_SSID", STA_SSID);
                    savePref_Str("wifi", "STA_PASSWORD", STA_PASSWORD);

                    if(testWifiConnect()){
                        pTxCharacteristic->setValue(getLocalIP().c_str()); // 设置要发送的值为1
                        pTxCharacteristic->notify();              // 广播
                        pTxCharacteristic->setValue("连接成功"); 
                        pTxCharacteristic->notify();             
                        
                        STA_SSID  = "";
                        STA_PASSWORD = "";

                    }else{
                        pTxCharacteristic->setValue("连接失败"); // 设置要发送的值为1
                        pTxCharacteristic->notify();              // 广播

                        STA_SSID  = "";
                        STA_PASSWORD = "";
                    }
                }else if(AP_SSID != "" && AP_PASSWORD != ""){
                    savePref_Str("wifi", "AP_SSID", AP_SSID);
                    savePref_Str("wifi", "AP_PASSWORD", AP_PASSWORD);
                    pTxCharacteristic->setValue("设置成功"); // 设置要发送的值为1
                    pTxCharacteristic->notify();              // 广播


                }
                
            }

            reStr = "";
        }
    }
};


void BLE_Stop(){
    btStop();

    Serial.println("BLE已关闭");
}


void BLE_Init()
{
    // 创建一个 BLE 设备
    BLEDevice::init("LOOP_CARD");  //创建BLE并设置名称
    pServer = BLEDevice::createServer();  //创建BLE服务器
    pServer->setCallbacks(new MyServerCallbacks());  //设置连接和断开调用类
    pService = pServer->createService(SERVICE_UUID); //创建BLE服务

    // 创建一个 BLE 特征
    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());
    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
    pRxCharacteristic->setCallbacks(new MyCallbacks()); //设置回调
 
    pService->start();                  // 开始服务
    pServer->getAdvertising()->start(); // 开始广播
    Serial.println("卡片BLE已开启,等待连接...");

}

 
