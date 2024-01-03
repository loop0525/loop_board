#include "vTaskLoopCard.h"



//硬件定时器指针
hw_timer_t *timer = NULL;
//接受NTP时间
struct tm timeinfo;


//标志
int selectIndex = 1; // 记录主页选中点
int selectPoint[] = {51,53,125,53,199,53,51,117,125,117,199,117}; // 主页所有选中点刷新坐标
int selectIndex_Set = 1; // 记录设置页选中点
int selectPoint_Set[] = {53,82,125,82,197,82}; // 设置页所有选中点刷新坐标


//信号量
SemaphoreHandle_t sM_S_Sem = NULL; //创建按键按下信号量


SemaphoreHandle_t sTime_Min_Sem = NULL; //创建即使刷新信号量


// 队列句柄
QueueHandle_t xM_S_Queue; // 按键长短按消息队列
QueueHandle_t xM_Encoder_Queue; // 滚轮方向消息队列
QueueHandle_t xRefreshTime_Queue; // 刷新时间消息队列

TickType_t sTimeOut = 1000; //获取信号量的延时时间



// 任务句柄
TaskHandle_t mEncoderHandle; // 监听滚轮任务句柄

TaskHandle_t clockTime; // 后台计算时间

TaskHandle_t scanwifiHandle; // 扫描wifi


TaskHandle_t showWeatherHandle;
TaskHandle_t showTomatoHandle;
TaskHandle_t showSleepHandle;
TaskHandle_t showTimeHandle;
TaskHandle_t showReadHandle;
TaskHandle_t showSetHandle;


TaskHandle_t switchHomeHandle; //主页任务切换

TaskHandle_t switchWeatherHandle; //天气页任务切换
TaskHandle_t switchTomatoHandle; //番茄页任务切换
TaskHandle_t switchSleepHandle; //睡眠页任务切换
TaskHandle_t switchTimeHandle; //时钟页任务切换
TaskHandle_t switchReadHandle; //阅读页任务切换
TaskHandle_t switchSetHandle; //设置页任务切换

TaskHandle_t bluetoothServerHandle; // 蓝牙服务任务


TaskHandle_t showHomeHandle;




void IRAM_ATTR interrupt_clockTime(){ // 定时器中断处理函数
    timeinfo.tm_sec++;
    if (timeinfo.tm_sec == 60) 
    {
      xSemaphoreGiveFromISR(sTime_Min_Sem,NULL); //在中断释放分钟计时信号量
    } 
}

void IRAM_ATTR interrupt_M_S(){ //滚轮按键中断
  // IRAM_ATTR 声明编译后的代码将放置在内部 RAM (IRAM) 中。否则代码将放在 Flash 中，闪存比内部 RAM 慢得多。

    // M_S_ON_time = xTaskGetTickCountFromISR(); // 在中断中获取按键按下时间
    xSemaphoreGiveFromISR(sM_S_Sem,NULL); //在中断释放按键按下信号量
} 

void timer_init(){ //初始化硬件定时器 计算时间
    timer = timerBegin(0,80,true);// 初始化定时器  1/(80MHZ/80) = 1us
    timerAttachInterrupt(timer,interrupt_clockTime,true);// 配置定时器 执行的代码时间必须尽量短，否则会发生看门狗超时
    timerAlarmWrite(timer,1000000,true); // 定时模式，单位us，自动装载闹钟
    //启动定时器
    // timerAlarmEnable(timer); 
}






void vTaskClockTime(void * pvParameters){  // 处理挂起时间后后台计算时间
  bool fHour; // 是否刷新小时
  while(1){
    if(xSemaphoreTake(sTime_Min_Sem,sTimeOut) == pdTRUE){ //获取分钟计时信号量
      timeinfo.tm_min++;
      timeinfo.tm_sec = 0;
      if (timeinfo.tm_min == 60) 
      {
        fHour = true;
        timeinfo.tm_hour++;
        timeinfo.tm_min = 0;
        if (timeinfo.tm_hour == 24) 
        {
          timeinfo.tm_hour = 0;
          timeinfo.tm_min = 0;
          timeinfo.tm_sec = 0;
        }
      }
      xQueueSend(xRefreshTime_Queue,&fHour,sTimeOut);
      fHour = false;
    }
  } 
}

void vTaskHandleEncoder(void * pvParameters){ //处理滚轮任务
  bool clockwise;   // 是否顺时针方向
  MD_REncoder R = MD_REncoder(M_A, M_B);
  TickType_t M_Encoder_LastTime = 0; // 记录滚轮上次滚动时间
  R.begin();
  while(1){
    uint8_t x = R.read();
    if (x)
    {
      if((xTaskGetTickCount() - M_Encoder_LastTime)>500){ // 滚动编码触发间隔限制
        if (x==0x20) //顺时针
        {
          clockwise = true;
          xQueueSend(xM_Encoder_Queue,&clockwise,sTimeOut);
        }
        else{
          clockwise = false;
          xQueueSend(xM_Encoder_Queue,&clockwise,sTimeOut);
        }  
        M_Encoder_LastTime = xTaskGetTickCount();

      }

    }
  }
}

void vTaskHandleM_S(void * pvParameters){ //处理按键任务
  int on_count = 0; // 按下计数
  bool longPress; // 按键长按标志消息
  while(1){
    if(xSemaphoreTake(sM_S_Sem,sTimeOut) == pdTRUE){ //获取按键信号量并判断是否按下
      vTaskDelay(10 / portTICK_PERIOD_MS); //消抖延时
      if(!digitalRead(M_S)){
        while(!digitalRead(M_S)){
          on_count++;
          vTaskDelay(100 / portTICK_PERIOD_MS); 
        }
        if(on_count>5){
          longPress = true;
          Serial.println("长按");
          xQueueSend(xM_S_Queue,&longPress,sTimeOut); // 发送长按消息
        }else{
          longPress = false;
          Serial.println("短按");
          xQueueSend(xM_S_Queue,&longPress,sTimeOut); 
        }
        on_count = 0;
      }
      vTaskDelay(500 / portTICK_PERIOD_MS); 
    }
  }
}

void vTaskRefreshHomePoint(void * pvParameters){ //刷新主页选中点
  int selectIndex_last = 1;
  bool clockwise;

  showHome(selectPoint,selectIndex);
  while(1) 
  {
    if(xQueueReceive(xM_Encoder_Queue,&clockwise,NULL) == pdPASS){ // 获取到滚轮方向队列消息
        if(clockwise){ // 顺时针
          selectIndex++;
        }
        else{
          selectIndex--;
        }
        if(selectIndex>6){
          selectIndex = 1;
        }
        if(selectIndex==0){
          selectIndex = 6;
        }
        // Paint_ClearWindows(46, 48, 55, 57, WHITE);
        // 计算清除选中点的最小范围
        Paint_ClearWindows(selectPoint[selectIndex_last*2-2]-5, selectPoint[selectIndex_last*2-1]-5, selectPoint[selectIndex_last*2-2]+4, selectPoint[selectIndex_last*2-1]+4, WHITE); // 擦除所有选择点的位置
        Paint_DrawCircle(selectPoint[selectIndex*2-2],selectPoint[selectIndex*2-1],3, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY); // 选中点
        Display_Partial(); // 刷新当前背景

        selectIndex_last = selectIndex;
        // vTaskDelay(1000 / portTICK_PERIOD_MS); // portTICK_PERIOD_MS心跳周期时间

      }
  }
}


void vTaskScanWifi(void * pvParameters){ // 扫描附近wifi
  while (1)
  {
    scanWiFi();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
  
}

void vTaskBluetoothServer(void * pvParameters){ // 蓝牙服务
  vTaskSuspend(mEncoderHandle);//挂起滚轮任务
  vTaskSuspend(showSetHandle);//挂起设置页刷新任务
  vTaskSuspend(switchSetHandle);//挂起设置页任务切换

  // BLE_Init(); // 初始化蓝牙
  Paint_Clear(WHITE);
  Display_Partial();

  showTest();

  bool longPress;
  while(1){
    if(xQueueReceive(xM_S_Queue,&longPress,sTimeOut) == pdPASS){
      if(longPress){ // 长按
        if(eTaskGetState(showSetHandle) == eSuspended){
            Paint_Clear(WHITE);
            Display_Partial();
            showSet(selectPoint_Set,selectIndex_Set);
            vTaskResume(mEncoderHandle); // 恢复滚轮任务
            vTaskResume(showSetHandle); // 恢复设置主页任务
            vTaskResume(switchSetHandle); // 恢复设置页任务切换
            vTaskDelete(NULL); // 删除当前任务
          }
        }
        
      }else{ // 短按
      }
    }
}



void vTaskRefreshWeather(void * pvParameters){//刷新天气
} 

void vTaskRefreshTomato(void * pvParameters){//刷新番茄
}

void vTaskRefreshSleep(void * pvParameters){//刷新睡眠
}

void vTaskRefreshTime(void * pvParameters){ //刷新时间
  bool fHour;
  showTime(timeinfo);
  int offset = 3; // 时间部分刷新区域偏置
  while(1) 
  {
    if(xQueueReceive(xRefreshTime_Queue,&fHour,sTimeOut) == pdPASS){ //获取刷新时间信号量
      // 擦除分钟显示区
      Paint_ClearWindows(5+Font68.Width * 3 - offset, 15, 5+Font68.Width * 5, Font68.Height+15, WHITE);
      if (fHour) 
      {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        // 擦除小时显示区
        Paint_ClearWindows(5 - offset, 15, 5 + Font68.Width * 2, 15 + Font68.Height, WHITE);
        fHour = false;
      }
      vTaskDelay(100 / portTICK_PERIOD_MS);
      showTime(timeinfo);
      
    }
  }
}

void vTaskRefreshRead(void * pvParameters){ //刷新阅读
}

void vTaskRefreshSet(void * pvParameters){//刷新设置
  int selectIndex_Set_last = 1;
  bool clockwise;
  showSet(selectPoint_Set,selectIndex_Set);
  while(1){
    if(xQueueReceive(xM_Encoder_Queue,&clockwise,NULL) == pdPASS){ // 获取到滚轮方向队列消息
        if(clockwise){ // 顺时针
          selectIndex_Set++;
        }
        else{
          selectIndex_Set--;
        }
        if(selectIndex_Set>3){
          selectIndex_Set = 1;
        }
        if(selectIndex_Set==0){
          selectIndex_Set = 3;
        }
        // Paint_ClearWindows(46, 48, 55, 57, WHITE);
        // 计算清除选中点的最小范围
        Paint_ClearWindows(selectPoint_Set[selectIndex_Set_last*2-2]-5, selectPoint_Set[selectIndex_Set_last*2-1]-5, selectPoint_Set[selectIndex_Set_last*2-2]+4, selectPoint_Set[selectIndex_Set_last*2-1]+4, WHITE); // 擦除所有选择点的位置
        Paint_DrawCircle(selectPoint_Set[selectIndex_Set*2-2],selectPoint_Set[selectIndex_Set*2-1],3, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY); // 选中点
        Display_Partial(); // 刷新当前背景

        selectIndex_Set_last = selectIndex_Set;
        // vTaskDelay(1000 / portTICK_PERIOD_MS); // portTICK_PERIOD_MS心跳周期时间

      }
    // vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}



void vTaskHomeSwitch(void * pvParameters){ //处理主页任务切换
  bool longPress;
  while(1){
    if(xQueueReceive(xM_S_Queue,&longPress,sTimeOut) == pdPASS){
      if(longPress){ // 长按   
      }else{ // 短按 (在主页使用)
        switch (selectIndex)
        {
        case 1: // 天气
          vTaskSuspend(showHomeHandle); // 挂起显示主页任务
          vTaskSuspend(mEncoderHandle); // 挂起滚轮监听任务
          Paint_Clear(WHITE);
          Display_Partial();
          xTaskCreate(vTaskRefreshWeather,NULL,2048,NULL,1,&showWeatherHandle);  // 创建刷新天气任务
          xTaskCreate(vTaskWeatherSwitch,NULL,2048,NULL,2, &switchWeatherHandle); // 创建天气页任务切换
          break;
        case 2: // 番茄
          vTaskSuspend(showHomeHandle); // 挂起显示主页任务
          vTaskSuspend(mEncoderHandle); // 挂起滚轮监听任务
          Paint_Clear(WHITE);
          Display_Partial();
          xTaskCreate(vTaskRefreshTomato,NULL,2048,NULL,1,&showTomatoHandle);  // 创建刷新番茄任务
          xTaskCreate(vTaskTomatoSwitch,NULL,2048,NULL,2, &switchTomatoHandle); // 创建番茄页任务切换
          break;
        case 3: // 睡眠
          vTaskSuspend(showHomeHandle); // 挂起显示主页任务
          vTaskSuspend(mEncoderHandle); // 挂起滚轮监听任务
          Paint_Clear(WHITE);
          Display_Partial();
          xTaskCreate(vTaskRefreshSleep,NULL,2048,NULL,1,&showSleepHandle);  // 创建刷新睡眠任务
          xTaskCreate(vTaskSleepSwitch,NULL,2048,NULL,2, &switchSleepHandle); // 创建睡眠页任务切换
          break;
        case 4: // 时钟
          vTaskSuspend(showHomeHandle); // 挂起显示主页任务
          vTaskSuspend(mEncoderHandle); // 挂起滚轮监听任务
          Paint_Clear(WHITE);
          Display_Partial();
          xTaskCreate(vTaskRefreshTime,NULL,2048,NULL,1,&showTimeHandle); // 创建刷新时间任务
          xTaskCreate(vTaskTimeSwitch,NULL,2048,NULL,2, &switchSetHandle); // 创建时钟页任务切换
          break;
        case 5: // 阅读
          vTaskSuspend(showHomeHandle); // 挂起显示主页任务
          vTaskSuspend(mEncoderHandle); // 挂起滚轮监听任务
          Paint_Clear(WHITE);
          Display_Partial();
          xTaskCreate(vTaskRefreshRead,NULL,2048,NULL,1,&showReadHandle);  // 创建刷新阅读任务
          xTaskCreate(vTaskReadSwitch,NULL,2048,NULL,2, &switchReadHandle); // 创建阅读页任务切换
          break;
        case 6: // 设置
          vTaskSuspend(showHomeHandle); // 挂起显示主页任务
          // vTaskSuspend(mEncoderHandle); // 挂起滚轮监听任务
          Paint_Clear(WHITE);
          Display_Partial();
          xTaskCreate(vTaskRefreshSet,NULL,2048,NULL,1,&showSetHandle); // 创建刷新设置页任务
          xTaskCreate(vTaskSetSwitch,NULL,2048,NULL,2, &switchSetHandle); // 创建设置页任务切换
          break;
        
        default:
          break;
        }
      }
    }
  }
}


void vTaskWeatherSwitch(void * pvParameters){ //处理天气页任务切换
  vTaskSuspend(switchHomeHandle);//挂起主页任务切换
  bool longPress;
  while(1){
    if(xQueueReceive(xM_S_Queue,&longPress,sTimeOut) == pdPASS){
      if(longPress){ // 长按
        if(eTaskGetState(showHomeHandle) == eSuspended){
            Paint_Clear(WHITE);
            Display_Partial();
            showHome(selectPoint,selectIndex);
            vTaskResume(showHomeHandle); // 恢复显示主页任务
            vTaskResume(mEncoderHandle); // 恢复滚轮监听
            vTaskResume(switchHomeHandle); // 恢复主页任务切换

            vTaskDelete(showWeatherHandle); // 删除刷新天气页任务

            vTaskDelete(NULL); // 删除当前任务
          }
        }  
      }else{ // 短按
      }
    }
  }

void vTaskTomatoSwitch(void * pvParameters){ //处理番茄页任务切换
  vTaskSuspend(switchHomeHandle);//挂起主页任务切换
  bool longPress;
  while(1){
    if(xQueueReceive(xM_S_Queue,&longPress,sTimeOut) == pdPASS){
      if(longPress){ // 长按
        if(eTaskGetState(showHomeHandle) == eSuspended){
            Paint_Clear(WHITE);
            Display_Partial();
            showHome(selectPoint,selectIndex);
            vTaskResume(showHomeHandle); // 恢复显示主页任务
            vTaskResume(mEncoderHandle); // 恢复滚轮监听
            vTaskResume(switchHomeHandle); // 恢复主页任务切换

            vTaskDelete(showTomatoHandle); // 删除刷新番茄页任务

            vTaskDelete(NULL); // 删除当前任务
          }
        }
        
      }else{ // 短按
        
      }
    }
  }

void vTaskSleepSwitch(void * pvParameters){ //处理睡眠页任务切换
  vTaskSuspend(switchHomeHandle);//挂起主页任务切换
  bool longPress;
  while(1){
    if(xQueueReceive(xM_S_Queue,&longPress,sTimeOut) == pdPASS){
      if(longPress){ // 长按
        if(eTaskGetState(showHomeHandle) == eSuspended){
            Paint_Clear(WHITE);
            Display_Partial();
            showHome(selectPoint,selectIndex);
            vTaskResume(showHomeHandle); // 恢复显示主页任务
            vTaskResume(mEncoderHandle); // 恢复滚轮监听
            vTaskResume(switchHomeHandle); // 恢复主页任务切换

            vTaskDelete(showSleepHandle); // 删除刷新睡眠页任务

            vTaskDelete(NULL); // 删除当前任务
          }
        }
        
      }else{ // 短按
        
      }
    }
  }

void vTaskTimeSwitch(void * pvParameters){ //处理时钟页任务切换
  vTaskSuspend(switchHomeHandle);//挂起主页任务切换
  bool longPress;
  while(1){
    if(xQueueReceive(xM_S_Queue,&longPress,sTimeOut) == pdPASS){
      if(longPress){ // 长按
        if(eTaskGetState(showHomeHandle) == eSuspended){
            Paint_Clear(WHITE);
            Display_Partial();
            showHome(selectPoint,selectIndex);
            vTaskResume(showHomeHandle); // 恢复显示主页任务
            vTaskResume(mEncoderHandle); // 恢复滚轮监听
            vTaskResume(switchHomeHandle); // 恢复主页任务切换

            vTaskDelete(showTimeHandle); // 删除刷新时钟页任务

            vTaskDelete(NULL); // 删除当前任务
          }
        }
        
      }else{ // 短按
        
      }
    }
  }

void vTaskReadSwitch(void * pvParameters){ //处理阅读页任务切换
  vTaskSuspend(switchHomeHandle);//挂起主页任务切换
  bool longPress;
  while(1){
    if(xQueueReceive(xM_S_Queue,&longPress,sTimeOut) == pdPASS){
      if(longPress){ // 长按
        if(eTaskGetState(showHomeHandle) == eSuspended){
            Paint_Clear(WHITE);
            Display_Partial();
            showHome(selectPoint,selectIndex);
            vTaskResume(showHomeHandle); // 恢复显示主页任务
            vTaskResume(mEncoderHandle); // 恢复滚轮监听
            vTaskResume(switchHomeHandle); // 恢复主页任务切换

            vTaskDelete(showReadHandle); // 删除刷新阅读页任务

            vTaskDelete(NULL); // 删除当前任务
          }
        }
        
      }else{ // 短按
        
      }
    }
  }

void vTaskSetSwitch(void * pvParameters){ //处理设置页任务切换
  vTaskSuspend(switchHomeHandle);//挂起主页任务切换
  bool longPress;
  while(1){
    if(xQueueReceive(xM_S_Queue,&longPress,sTimeOut) == pdPASS){
      if(longPress){ // 长按
        if(eTaskGetState(showHomeHandle) == eSuspended){
            Paint_Clear(WHITE);
            Display_Partial();
            showHome(selectPoint,selectIndex);
            vTaskResume(showHomeHandle); // 恢复显示主页任务
            vTaskResume(switchHomeHandle); // 恢复主页任务切换

            vTaskDelete(showSetHandle); // 删除刷新设置页任务

            vTaskDelete(NULL); // 删除当前任务
          }
        }
        else{ // 短按
          switch (selectIndex_Set)
          {
          case 1:  // wifi server

            
            break;
          case 2: // bluetooth server
            xTaskCreate(vTaskBluetoothServer,NULL,3072,NULL,2, &bluetoothServerHandle); // 创建蓝牙配网任务

            
            break;
          case 3: // miniprogram config
            
            break;
          
          default:
            break;
          }
        }
      }
    }
  }



void vTaskStart(){
 
// webserver_STA_init();
//   web_server();

  display_init();
  pinMode(M_S,INPUT_PULLDOWN);
  attachInterrupt(M_S, interrupt_M_S, FALLING); // 按键中断

  sM_S_Sem = xSemaphoreCreateBinary(); //创建按键按下信号量

  sTime_Min_Sem = xSemaphoreCreateBinary(); //创建分钟计时信号量

  xM_S_Queue = xQueueCreate(3,1); // 创建按键长短按队列
  xM_Encoder_Queue = xQueueCreate(1,1);  // 创建滚轮方向队列
  xRefreshTime_Queue = xQueueCreate(3,1); // 创建刷新时间(时、分)队列

  timer_init();

  // SetNtpTime(&timeinfo);

  timerAlarmEnable(timer);;//开启硬件定时计算时间


  xTaskCreate(vTaskClockTime,NULL,2048,NULL,3,&clockTime); // 开启后台计算时间

  xTaskCreate(vTaskHandleM_S,NULL,2048,NULL,2,NULL);
  xTaskCreate(vTaskHandleEncoder,NULL,2048,NULL,1, &mEncoderHandle);
  xTaskCreate(vTaskHomeSwitch,NULL,2048,NULL,2, &switchHomeHandle);

  xTaskCreate(vTaskRefreshHomePoint,NULL,2048,NULL,1,&showHomeHandle); 

  // xTaskCreate(vTaskScanWifi,NULL,2048,NULL,1,&scanwifiHandle); 
  // vTaskSuspend(scanwifiHandle);//挂起扫描wifi

  // StartBLE();
}


/* 任务的挂起与恢复(用于其他函数的调用) */ 

void SuspendTask_ScanWifi(){
  Serial.println("挂起扫描！");
  vTaskSuspend(scanwifiHandle);//挂起扫描wifi
}
void ResumeTask_ScanWifi(){
  if(eTaskGetState(scanwifiHandle) == eSuspended){
    vTaskResume(scanwifiHandle);//恢复扫描wifi
  }
}





