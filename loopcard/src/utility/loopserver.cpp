#include "loopserver.h" 



String ssid_nvs = "";                     //存储wifi账号密码
String password_nvs = "";                     //存储wifi账号密码

// Preferences prefs; // nvs存储操作对象
IPAddress local_IP;

AsyncWebServer server(80);

// 域名解析
DNSServer dnsserver; //声明DNSServer对象

//用于储存扫描到的WiFi ID
   
StaticJsonDocument<200> scanNetworksID;   //声明一个JsonDocument对象，长度200  
String scanNetworksID_Str;



bool testWifiConnect(){  // 测试WIFI是否连接成功
  WiFi.mode(WIFI_STA); //设置为STA模式
  WiFi.begin(getPref_Str("wifi","STA_SSID"),getPref_Str("wifi","STA_PASSWORD"));         //连接WIFI
  Serial.print("正在测试连接："); 
  Serial.println(getPref_Str("wifi","STA_SSID"));
  //循环，直到连接成功
  int count = 0;
  while(WiFi.status() != WL_CONNECTED){
    vTaskDelay(500 / portTICK_PERIOD_MS);
    count++;
    if(count>6){ // 超过3秒
      return false;
    }
  }
  Serial.println();
  local_IP = WiFi.localIP();
  Serial.print("WIFI is connected,The local IP address is "); //连接成功提示
  Serial.println(local_IP); 

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("WiFi disconnected!");

  // scanWiFi();
  return true;
}

String getLocalIP(){
  return local_IP.toString();
}

bool scanWiFi(){ //扫描WiFi
    Serial.println("开始扫描！");
    scanNetworksID.clear();


    // prefs.begin("wifi"); //命名空间初始化函数
    // if(prefs.isKey("ssid_nvs")) { // 如果当前命名空间中有键名为"name"的元素
    //   ssid_nvs =  prefs.getString("ssid_nvs");
    // }
    // else{
    //   prefs.putString("ssid_nvs", "loop");
    //   ssid_nvs = "loop";
    // }

    // if(prefs.isKey("password_nvs")) { // 如果当前命名空间中有键名为"name"的元素
    //   password_nvs = prefs.getString("password_nvs"); 
    // }
    // else{
    //   prefs.putString("password_nvs", "12345678");
    //   password_nvs = "12345678";
    // }

    scanNetworksID["ssid_nvs"] = ssid_nvs;
    scanNetworksID["password_nvs"] = password_nvs;


    if(WiFi.status() == WL_CONNECTED){ // 连接成功
      scanNetworksID["wifi_status"] = true;
    }else{
      scanNetworksID["wifi_status"] = false;
    }

    int n = WiFi.scanNetworks();
    
    if (n == 0) {
        Serial.println("没有网络发现！");
        scanNetworksID = "没有网络发现！";
        return false;
    }else{
        Serial.print(n);
        for (int i = 0; i < n; ++i) {
          scanNetworksID[WiFi.SSID(i)] = WiFi.RSSI(i);
        }
        scanNetworksID_Str = "";
        serializeJson(scanNetworksID, scanNetworksID_Str);  // 序列化JSON数据并导出字符串
        // Serial.println(scanNetworksID_Str);
        return true;
    }
  }


void webserver_STA_init(){
  //STA模式 SSID & Password
  const char* wifi_ssid = "loop";
  const char* wifi_password = "loop1477824757";
  WiFi.mode(WIFI_STA); //设置为STA模式
  WiFi.begin(wifi_ssid, wifi_password);         //连接WIFI
  Serial.print("Connected");
  //循环，直到连接成功
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  IPAddress local_IP = WiFi.localIP();
  Serial.print("WIFI is connected,The local IP address is "); //连接成功提示
  Serial.println(local_IP); 
  scanWiFi();
}

void webserver_AP_init(){ //初始化开启AP模式webserver
  //AP模式 SSID & Password
  const char* ssid = "loop card";  // Enter your SSID here
  const char* password = "12345678";  //Enter your Password here

  const byte DNS_PORT = 53; //DNS服务端口号，一般为53

  // IP Address details
  IPAddress local_ip(192, 168, 1, 1); //IP地址
  IPAddress gateway(192, 168, 1, 1);  ////网关地址
  IPAddress subnet(255, 255, 255, 0);  //子网掩码
   // Create SoftAP
  WiFi.mode(WIFI_AP_STA); //打开AP和STA共存模式
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);

  
  dnsserver.start(DNS_PORT, "loopcard.com", local_ip);  //设置DNS的端口、网址、和IP

  Serial.print("WIFI AP is started,The IP address is "); 
  Serial.println(local_ip);
}
 

void scanWifi_onload_callback(AsyncWebServerRequest *request){
  ResumeTask_ScanWifi();//恢复扫描wifi
  request->send(200,"text/plain","开始刷新WIFI");
}
void scanWifi_onbeforeunload_callback(AsyncWebServerRequest *request){
  SuspendTask_ScanWifi();//挂起扫描wifi
  request->send(200,"text/plain","结束刷新");
}
void scanWifi_refresh_callback(AsyncWebServerRequest *request){
  request->send(200,"application/json",scanNetworksID_Str);
}


void configWifiSet_callback(AsyncWebServerRequest *request){
  if(request->hasParam("wifiname",true)){
    AsyncWebParameter* wifiname = request->getParam("wifiname",true);					    //获取POST数据
    AsyncWebParameter* wifipassword = request->getParam("wifipassword",true);			//获取POST数据
    String wn  = wifiname->name().c_str();
    String wnv = wifiname->value().c_str();
    String wp  = wifipassword->name().c_str();
    String wpv = wifipassword->value().c_str();


    Serial.print(wn+":");
    Serial.println(wnv);
    Serial.print(wp+":");
    Serial.println(wpv);


    // //把SSID和password写成一个JSON格式
    // StaticJsonDocument<200> wifi_json;                                            //创建一个JSON对象,wifi_json
    // wifi_json[wn] = wnv;                                                          //写入一个键和值
    // wifi_json[wp] = wpv;                                                          //写入一个键和值
    // String wifi_json_str;                                                         //定义一个字符串变量
    // serializeJson(wifi_json, wifi_json_str);                                      //生成JOSN的字符串
  }


}

void web_server(){
  if(!SPIFFS.begin(true)){
    Serial.println("挂载SPIFFS时发生错误");
    return;
  }
  
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");


  server.serveStatic("/configWifi", SPIFFS, "/").setDefaultFile("wificonfig.html");

  server.on("/configWifi/onload",HTTP_GET,scanWifi_onload_callback);    //注册回调函数
  server.on("/configWifi/onbeforeunload",HTTP_GET,scanWifi_onbeforeunload_callback);
  server.on("/configWifi/refresh",HTTP_GET,scanWifi_refresh_callback);

  server.on("/configWifi_set",HTTP_POST , configWifiSet_callback); 
  server.begin();                       //初始化
}

void DNS_handleRequest(){
  dnsserver.processNextRequest(); //处理来自客户端的请求DNS解析, 需要循环处理
}




void SetNtpTime(struct tm * timeinfo){ // 获取NTP时间
  WiFi.begin("loop", "loop1477824757");

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  configTime(8 * 3600, 0, NTP1);
  if (!getLocalTime(timeinfo))//一定要加这个条件判断，否则内存溢出
  { 
    Serial.println("!!获取时间失败,初始化为 00:00:00");
    (*timeinfo).tm_hour = 0; 
    (*timeinfo).tm_min = 0;
    (*timeinfo).tm_sec = 0;
  }else{
    Serial.println("成功获取时间");
  }
  WiFi.disconnect(true);//断开wifi网络
  WiFi.mode(WIFI_OFF);//关闭网络
}
