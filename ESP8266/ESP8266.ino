/* //ESP32 PWM函数 Arduino-ESP32 LEDC API
bool ledcAttach(uint8_t Pin, uint32_t freq, uint8_t resolution)                        // GPIO,频率,分辨率(1位-14位,ESP1 20位-32位)
bool ledcAttachChannel(uint8_t Pin, uint32_t freq, uint8_t resolution, int8_t channel) // GPIO,频率,分辨率,通道分组
bool ledcWrite(uint8_t Pin, uint32_t duty)                                             // 写引脚占空比(GPIO,占空比(分辨率范围))
bool ledcWriteChannel(uint8_t channel, uint32_t duty)                                  // 写通道占空比(通道,占空比)       
uint32_t ledcRead(uint8_t Pin)                                                         // 取引脚频率(GPIO)
uuint32_t ledcReadFreq(uint8_t Pin);                                                   // 取通道频率(GPIO) 
bool ledcDetach(uint8_t Pin);                                                          // 分离引脚(GPIO) 
uint32_t ledcChangeFrequency(uint8_t Pin, uint32_t freq, uint8_t resolution);          // 修改频率(GPIO,频率,分辨率)
bool ledcOutputInvert(uint8_t Pin, bool out_invert);                                   // 反向输出(GPIO，真=反向)
uint32_t ledcWriteTone(uint8_t Pin, uint32_t freq);                                    // 50%
uint32_t ledcWriteNote(uint8_t Pin, note_t note, uint8_t octave);                      // 特定八音符
bool ledcFade(uint8_t Pin, uint32_t start_duty, uint32_t target_duty, int max_fade_time_ms); //淡入淡出
bool ledcFadeWithInterrupt(uint8_t Pin, uint32_t start_duty, uint32_t target_duty, int max_fade_time_ms, void (*userFunc)(void));
bool ledcFadeWithInterruptArg(uint8_t Pin, uint32_t start_duty, uint32_t target_duty, int max_fade_time_ms, void (*userFunc)(void*), void * arg);

pinMode(Pin[i], OUTPUT);
void analogWriteFrequency(uint8_t Pin, uint32_t freq);                                  //写模拟引脚频率(GPIO,频率)
void analogWriteResolution(uint8_t Pin, uint8_t resolution);                            //写模拟引脚分辨率(GPIO,分辨率)
void analogWrite(uint8_t Pin, int value);                                               //模拟写入(GPIO,0-255) 8位分辨率
int map(值,最小,最大,新小,新大);                                                          //数值映射

//ESP32系列 16  个通道 0~15，0-7高速通道由80MHz时钟驱动,8-15低速通道1MHz时钟驱动
//ESP32-S2  8
//ESP32-S3  8
//ESP32-C3  6
//ESP32-C6  6
//ESP32-H2  6
//使用時請跳過 GPIO 6~11, GPIO 34, GPIO 35, GPIO 36, GPIO 39 這幾隻接腳

//分辨率范围为 1-14 位（ESP1 为 20-32 位）
*/
//创客与编程      https://shop104415339.taobao.com/
//四轴机械臂      https://item.taobao.com/item.htm?ft=t&id=634846120104
//板子与舵机      https://item.taobao.com/item.htm?ft=t&id=665636638155

//模拟舵机 橙色线接~PWM针脚,红线5V,棕线GND; SG90转180度约500毫秒,MG996R约1000毫秒
//模拟舵机不防堵,转不过去的角度不能一直转,自已观察范围并限制,比如夹子不能一直夹着,舵机会发烫久了烧掉
#include <ESP8266WiFi.h>                     //
String AP_SSID  ="ESP8266";                 //ESP32创建自已的热点名称,也当做 设备名称
String AP_PSK   ="12345678";              //热点密码,若启用了 BlinKey 点灯,会自动关掉板子热点

String STA_SSID ="MYHOME";                //路由器WIFI或手机电脑的移动热点名称
String STA_PSK  ="12345678";             //热点密码,删掉密码则不连路由器,刚无法连远程TCP服务,并禁用 定时 命令
//#include <ESPmDNS.h>                  //本地域名解析服务  http://esp32.local
#include <DNSServer.h>                //域名解析服务
DNSServer DNS;                        //创建dnsServer实例,默认端口53,强制门户登录

#include <ESP8266WebServer.h>                //HTTP WEB网页服务
ESP8266WebServer     Web(80);                //建立Web服务对象,HTTP端口80
#include <ESP8266HTTPUpdateServer.h>       //寄生网页服务,接受 Firmware:固件.bin,FileSystem:文件系统.bin  更新 http://X.X.X.X/upbin  
ESP8266HTTPUpdateServer Updater;           //网络[更新固件]服务,因为程序代码复杂功能多,板子4MB空间不够用,默认不启用这个功能

#include <WiFiServer.h>                   //NetworkServer.h
#include <WiFiClient.h>                   //NetworkClient.h
#include <WiFiUdp.h>                      //NetworkUdp.h
String SIP = "";                          //连远程TCP服务,remote server 192.168.31.132:SPort
int SPort = 8000;                         //Port for the TCP server 
WiFiServer server(SPort);                 //TCP本地服务,可以接受远程TCP客户进入
WiFiClient C;                             //TCP客户端 Networkclient
int UPort = 8888;                         //UDP port 8888
WiFiUDP    U;                             //NetworkUDP 可以接受UDP信息命令


//arduino 菜单->项目->加载库->管理库 搜 ArduinoJson 安装.
#include <ArduinoJson.h>                   //使用Json文件格式做配置文件
#include "FS.h"                            //ESP32开发板自带4MB闪存空间,可以用来读写存删命令文件
//#include "SPIFFS.h"                        //ESP32内置的4MB空间使用SPIFFS文件格式
//#include "FFat.h"

//----------------点灯科技 物连网 外网控制-填你自已的 独立设备密钥-----------------------
//arduino 菜单->项目->加载库->管理库 搜 Blinker 安装.
#define BLINKER_WIFI
//#define BLINKER_WITHOUT_SSL             //堆栈不足要采用非SSL加密方式连网通信,BLINKER_WITH_SSL
//#define BLINKER_ESP_SMARTCONFIG
//#define BLINKER_ESP_TASK                //ESP32支持多任务；Blinker.delay(1000); 
#include <Blinker.h>                      //引用 点灯科技 物连网功能库,可以外网控制机械臂
//#include "ESP32_CAM_SERVER.h"                  
String BlinKey = "";//"cf10615bfb94";     //点灯.blinker APP里创建 独立设备的密钥 填这里
// 新建组件对象
BlinkerText       Text1("IP");         //标签   组件对象 对象名,无事件
BlinkerText       Text2("RSSI");       //标签   组件对象 对象名,无事件
BlinkerSlider     Slider1("X");        //滑动条
BlinkerSlider     Slider2("Y");        //滑动条
BlinkerSlider     Slider3("Z");        //滑动条
BlinkerSlider     Slider4("B");        //滑动条
BlinkerSlider     Slider5("T");        //滑动条
BlinkerSlider     Slider6("E");        //滑动条
//-------------------------------------------------------------------------------------------------
#include <Servo.h>                         //引用 舵机 功能库头文件
Servo    S[6];                             //创建舵机对象

int         ss = 6;                    //6=6轴.此变量值决定使用下面数组几个成员值
String XYZE[6] = {"X","Y","Z","B","T","E"};     //定义4个电机从底座到夹子为 //O 电子阀 插D6;P 真空泵 插D7;
int     Pin[6] = {  5,  4,  0, 16, 14, 12};     //6个支持PWM数字引脚,舵机橙线插PWM数字引脚,红线插5V,棕线插GND
int     Raw[6] = { 90, 90, 90, 90, 90, 90};     //原点,起点,通电后或H指令会让所有舵机归位到此脉宽.500=0度，1500=居中90度，2500=180度
float   Old[6] = { 90, 90, 90, 90, 90, 90};     //每次转动舵机后保存脉宽值到此,作为下次转动的起点.
float   New[6] = { 90, 90, 90, 90, 90, 90};     //每次转动舵机后保存脉宽值到此,作为下次转动的起点.
int     Min[6] = {  0, 55,  0,  0,  0, 45};     //定义6个电机在机械臂中可转动的最小信号脉冲微秒值
int     Max[6] = {180,180,180,180,180,100};     //定义6个电机在机械臂中可转动的最大信号脉冲微秒值

int     Maxdms=1500;                         //MG996R舵机转180度需要的毫秒时间,与供电压电流有关
float    Step = 0.0;                         //0.0原速,SG90用1.0减速,控制舵机循环每次转MG99R用0.4度角，直到目标角度

int B1P=15,B1G=1,B1N=0;                      //接按钮1 [pin,0=GND 1=5V],触发时执行 B1R
int B2P=13,B2G=1,B2N=0;                      //接按钮2 [pin,0=GND 1=5V],触发时执行 B2R
String B1R="",B2R="";     
//QueueHandle_t  QueueHandle;                //ESP 32多任务句柄

struct TimeData {                            //定时器和倒计时用的结构
  unsigned long Dms=0;
  struct tm T={0,0,0,-1,-1,-1};
  String Cmd="";
};
TimeData Tm[10];                             //定时器和倒计时共用10个变量
unsigned long  Dms=0;                        //delay ? 供延时命令用

bool sync=true;                              //true=同步，false=异步
String     Cmd,Cmdret="";                    //把一些指令放在这个变量,下次loop循环时执行
String     Autorun="";


String Command(String a,bool b=false);       //提前声明一下这个函数, true false
//--------------------------------下面3个函数是控制舵机转动的核心代码------------------------------------
//模拟舵机信号是单向的,发PWM信号给舵机要转到指定角度.有没有转到或当前状态是不会返回的
//转动需要时间,转速与电压电流有关,要保存上次角度与现在要转的角度..计算需要的时间进行等待处理
//-----------------------把 0.0-180.0 可带1位小数的角度值转为14位PWM信号分辨率 --------------------------
int toPWM(int I,float Value){                 //根据舵机数组设定把角度值转为14位PWM信号分辨率
   int  type=180;                       //默认为180度舵机

   if(I>=0 && I<ss){                          //模拟舵机一个信号为 20毫秒=20000微秒,高电平500微秒到2500微秒对应角度0度到180度,其余时间为低电平
     Value=constrain(Value,Min[I],Max[I]);    //限定角度值范围
     if(Max[I]==270) type=270;        //X底座为270度舵机
   } else { 
     Value=constrain(Value,0,180);            //限定为180度舵机
   }
   Value=(2000/type)*Value+500;           //转为14位分辨率PWM值
   return (int)Value;                         //返回PWM值
}

//-----------------------设定PWM信号占空比控制舵机转动----------------------------
bool ServoGo(int I,float Value) {                 //支持 0.0~180.0 带1位小数的角度值
  int MAX=0;

  //------------------------------------------------------------------------
  if(I>=0 && I<ss){               
    New[I]=constrain(Value,Min[I],Max[I]); //把角度转为PWM数值存入         
    //Serial.printf("ServoGo I=%d  %.1f\n",I,New[I]);
  } //else Serial.printf("ServoGo I=%d  %.1f\n",I,Value);

  //----------------------------------------------
  if(Step>0.0)Step=1.0;                                    //ESP8266设1.0 ，ESP32板子CPU强点,可以设0.4
  for(I=0;I<ss;I++){MAX=max(MAX,(int)abs(New[I]-Old[I]));} //找出一次需要转动的最大角度差
  float p[6]={0.0,0.0,0.0,0.0};
  for(I=0;I<ss;I++){p[I]=abs(New[I]-Old[I])/MAX*Step;}    //Step=0.4
  unsigned long ms=millis();                              //记录板子启动到现在的时间
  if(Step>0.0){
    for(int J=0;J<(MAX/Step);J++){             //根据最大要转角度进行循环，逐角度转动
      for(I=0;I<ss;I++){
       if(Old[I]==New[I]) continue;            //到循环尾
         if(Old[I]<New[I]) {             
           Old[I]+=p[I];                        //正转
        } else {
           Old[I]-=p[I];                        //反转
        }
        S[I].writeMicroseconds(toPWM(I,Old[I]));           //发送信号
      }
      //yield()=delay(0)延时可避免长时间循环触发看门狗重启开发板.Soft WDT reset  
      yield(); //循环超过500MS易触发看门狗重启开发板..重置看门狗计数 ESP.wdtFeed();禁止看门狗 ESP.wdtDisable() 超过5秒触发硬件看门狗
    }
    Serial.printf("耗时:%d ms\n",millis()-ms);  //X 0;X 180;X 0;X 180;H; 耗时:1800 ms    //X 0;X 180;X 90  ESP8266耗时:3576 ms
    delay(16);
  }

  for(I=0;I<ss;I++){                         //发信号修正角度
    S[I].writeMicroseconds(toPWM(I,New[I])); 
    Old[I]=New[I];                           //保存新的角度
  }
  
  ms+=MAX*4;
  if(ms>millis()){       //检查目前已消耗的时间与转到目标角度需要的时间
     delay(constrain(ms-millis(),0,Maxdms));
  }
  return MAX!=0;         //有发送过舵机信号返回 true
}

//-------------------------------点灯科技 物连网 控制-----------------------------------------
//
//
//-----------------------点灯APP创建的独立设备,心跳回调, online 表示在线-----------------------
void heartbeat(){                            //Blinker 心跳回调,每30s-60会发送一次心跳包，表示在线
    static bool first=false;                 //第一次心跳,更新 IP 与 RSSI WIFI信号强度
    static  int timer=0;

    //BLINKER_LOG("heartbeat 心跳");
    if(!first){                                        //首次心跳通信把IP与信号强度发给服务器
      first=true;
      Text1.print(WiFi.localIP().toString().c_str());  //  "IP":{"tex":"192.168.31.122"}
      Text2.print("RSSI:"+String(WiFi.RSSI())+"dBm");  //"RSSI":{"tex":"RSSI:-51dBm"} WIFI信号强度
    }
    Slider1.print(New[0]);                             //输出各轴舵机当前角度 "X":{"val":90}           
    Slider2.print(New[1]);                               
    Slider3.print(New[2]);
    Slider4.print(New[3]);
    Slider5.print(New[4]);  
    Slider6.print(New[5]);                    //100毫秒内的数据会合并发送
    Blinker.print("timer",timer);timer++;              //加一条递增的数据编号
    Blinker.print("version","1.0"); 
    Blinker.print("state","online");                   //在线状态,Blinker.delay(1000); 
}
//----------------------物连网 点灯科技APP 滑动各舵机进度条会产生并执行下列事件--------------------------------
void sliderX(int32_t value){dataRead("X "+String(value));}   //X 109
void sliderY(int32_t value){dataRead("Y "+String(value));}   //Y 90
void sliderZ(int32_t value){dataRead("Z "+String(value));}   
void sliderB(int32_t value){dataRead("B "+String(value));}
void sliderT(int32_t value){dataRead("T "+String(value));}
void sliderE(int32_t value){dataRead("E "+String(value));}


//------------点灯APP创建的独立设备,组件被触发又未绑定专用事件处理函数,这里执行----------------------------------
void dataRead(const String & data) {            //上面各舵机滑块条或调试组件传进来的data数据.如 X 109   X 50;Y 90
    BLINKER_LOG("dataRead 未附加事件:", data);  //未绑定的按钮data数据.如 {"H":"tap"} {"R":"press"} {"R":"pressup"}
                                                //滑块和调试组件发来的命令 按键名:点击  按键名:按住   按键名:松开      
    String S=String(data);
    int i=S.indexOf("\"",0);                    //查找 " 符号区分按钮事件,还是调试组件发来的控制命令
    if (i>0){                                   //按钮事件 {"H":"tap"}   
      S.remove(0,i+1);                          //删除左侧的 {"
      i=S.indexOf("\"",0);                      //查找右侧 " 符号
      S=S.substring(0,i);                       //提取组件名 H ,当机械臂控制命令用
    }
    //BLINKER_LOG(S);                           //串口输出指令内容  dataRead 未附加事件: X 117  

    if(S.equalsIgnoreCase("IP")){               //如果指令为 IP 输出 WIFI信号强度
      Text1.print(WiFi.localIP().toString().c_str());
      Text2.print("RSSI:"+String(WiFi.RSSI())+"dBm");   
      return;
    } else if(S.equalsIgnoreCase("DIR")){       //枚举文件并输出
      S="";
      Dir dir=SPIFFS.openDir("/");     //打开根目录,枚举文件名
      while(dir.next()) {              //循环所有对象                            //循环所有对象
        S+=String(dir.fileName())+";";                //Tab 制表符
      }
      Blinker.notify(S);
      return;
    } else {
       //Cmd=S;                                   
       S=CheckCmd(S);
       if(S!=""){                                //把未识别指令存到变量,供下次LOOP循环时转给Command()执行
          S.replace("{","");
          S.replace("}","");
          S.replace("\"","");                       //删除 " 符号
          S.replace(":"," ");
          S.replace(",",";");
          Blinker.notify(S);                        //把指令结果返回给点灯APP
       }
    }
    Slider1.print(New[0]);                       //输出各轴舵机当前角度 "X":{"val":90}
    Slider2.print(New[1]);                               
    Slider3.print(New[2]);
    Slider4.print(New[3]);
    Slider5.print(New[4]);
    Slider6.print(New[5]);
}
//----------------------------------------------------------------------
//delayMicroseconds(v);
//-----------------------------------------------------------------------
ICACHE_RAM_ATTR void ISR_Btn1(){            //按钮1中断事件处理 
  static unsigned long  ms=0;
  if(millis()-ms>1000) B1N=1;               //防止1秒内产生多次按钮事件
  ms=millis();
}

ICACHE_RAM_ATTR void ISR_Btn2(){            //按钮2中断事件处理
  static unsigned long  ms=0;
  if(millis()-ms>1000) B2N=1;               //防止1秒内产生多次按钮事件
  ms=millis();
}

void ISR_Btn(String s){
  String S=s; S.toUpperCase();               //转为大写
  
  if(S.startsWith("STOP")){                  //
    S.remove(0,4);S.trim();                  //删首尾空格与换行
    if(S.toInt()==0){                       //Stop 0 停止并清除指令池
      Cmd="";                               
      Dms=millis();
  } else if(S.toInt()==-1){                  //stop -1 长时间暂停，需要用 Start 恢复
      Dms=-1;
  } else if(S.toInt()>0){                    //stop 10000 暂停10秒，可用  Start 立即恢复
      Dms=millis()+S.toInt();
    } 
  } else if (S.equalsIgnoreCase("START")){   //恢复 stop -1 或 stop 10000 的暂停
      Dms=millis();
  } else Cmd+="\n"+s;
}
//-----------------------------------------------------------------------
/*
void Task1(void *pvParameters){  //线程1  https://www.freertos.org/
   bool message;
   xQueueReceive(QueueHandle,&message,portMAX_DELAY);   //阻塞等待
   while(1){
    Serial.println("Task1");
    vTaskDelay(600);             //FreeRTOS 多线程里用的延时(毫秒)
   }
}
void Task2(void *pvParameters){  //线程2  息队列,任务通知,信号量,互斥锁
   while(1){                     //https://wokwi.com/projects/406191778863034369     
    Serial.println("Task2");
    vTaskDelay(600);               //FreeRTOS 多线程里用的延时(毫秒)  消
   }
}*/
//-------------------------------------------------------------------------------------------
//
//
//
//------------------载入配置文件 /config.json 修改变量里的默认数据设定------------------------
void loadConfig() {                            //载入配置文件/config.json替换掉一些变量中的值
    File F = SPIFFS.open("/config.json", "r"); //打开读取配置文件
    if(!F) return;
    F.seek(0);                                 //到首位置
    String S=F.readString();                   //读入文本
    F.close();                                 //关闭文件

    if (S.length()>32) {
     //StaticJsonDocument<1024> doc;          //栈内存JSON文档对象
     DynamicJsonDocument doc(2048);           //堆内存JSON文档对象
     deserializeJson(doc,S);                  //把内容装载到JSON对象
     //serializeJson(doc, Serial);              //输出JSON格式内容到串口
     //Serial.println();                        //输出换行符
     if(ss != doc["ss"]){                     //ss 4=4轴机械臂,5=5轴机械臂,其他值或6=六轴机械臂
        doc.clear(); 
        SPIFFS.remove("/config.json");        //删除与固件不批配的配置文件
        return;
     }    
     AP_SSID  =String(doc["AP_SSID"]);        //读取配置参数到变量
     AP_PSK   =String(doc["AP_PSK"]);
     STA_SSID =String(doc["STA_SSID"]);
     STA_PSK  =String(doc["STA_PSK"]);
    
     BlinKey  =String(doc["BlinKey"]);        //物联网 点灯.blinke 独立设备 密钥..

     SIP   = String(doc["SIP"]);      //远程TCP服务端IP地址
     SPort = doc["SPort"];            //TCP端口
     UPort = doc["UPort"];            //UTP端口
 
     for(int I=0;I<ss;I++){                //读取舵机参数到数组变量
       //XYZE[I] = String(doc["XYZE"][I]); //舵机名称  Servo
       Pin[I]  = doc["Pin"][I];           //舵机GPIO
       Raw[I]  = doc["Raw"][I];           //默认原始角度
       Min[I]  = doc["Min"][I];           //最小角度限制
       Max[I]  = doc["Max"][I];           //最大角度限制
     }
     B1P = doc["B1"][0];        //
     B1G = doc["B1"][1];        //
     B1R = String(doc["B1R"]);
     B2P = doc["B2"][0];        //
     B2G = doc["B2"][1];        //
     B2R = String(doc["B2R"]);

      Autorun = String(doc["Auto"]);         //板子通电自动运行的命令,或者 Auto.txt 次数
      Autorun.trim();                        //删首尾空
      if(Autorun.length()>0) {               //有内容
       if(Autorun.toInt()==0)                     
         Cmd=Autorun;                        //非数字，视为正常命令，放入待执行变量中
       else
          Cmd="Auto.txt "+Autorun;          //如果为数字,则设定开机就执行 Auto.txt 次数
      }
     doc.clear();                           //清空 JSON 文档。
     //doc.iterator()         用于遍历 JSON 对象或数组的元素。
     //doc.next()             用于获取下一个元素。
     //doc.size()             获取 JSON 对象或数组的元素数量。
     //doc.isEmpty()          检查 JSON 对象或数组是否为空。
     //doc.remove()           从 JSON 对象或数组中删除元素。
    }
}
//-------------------------返回Json格式的所有舵机当前角度信息--------------------------
String output(){                
  StaticJsonDocument<256> doc;              //栈内存JSON文档对象

  for(int I=0;I<ss;I++){
     String s=XYZE[I];                     //舵机名称
     float  v=New[I];
     doc[s]=String(v,1);                   //输出带1位小数精度的角度值
  }
  doc["Cmd"]=Cmd.length();
  String ret;
  serializeJson(doc,ret);                   //单行格式 到变量
  //serializeJsonPretty(doc,ret);           //多行格式 到变量
  doc.clear();
  return  ret;   //{"X":90,"Y":90,"Z":90,"B":90,"T":90,"E":90,"Cmd":0}
}
//-----------------------------setup 是开发板通电后最先被运行1次的函数,这里进行各种初始化---------------------------------------
void setup() {
    Serial.begin(115200);                                  //开启串口通信 波特率115200,串口监视器也要相同波特率,不然会乱码
    pinMode(LED_BUILTIN, OUTPUT);                          //初始化8266开发板LED信号灯的GPIO口为输出.
    digitalWrite(LED_BUILTIN,LOW);                        //Mini ESP8266板LED_BUILTIN=GPIO 2,LOW=亮灯,HIGH=灭灯
    for(int i=0;i<1000;i++){delay(2);}                     //延时1秒,让arduino编程软件的串口监视器与板子COM串口能完成连接
    Serial.printf("\nsetup  LED_BUILTIN:%d\n",LED_BUILTIN);//输出 LED信号灯用的 芯片GPIO编号

    //------------ arduino 菜单->工具->spiffs -----------------
    //ESP32开发板代码.格式化并建立内置闪存文件系统,用来保存 Auto.txt 等机械臂自动化动作指令
    //SPIFFS.format();     //第一次使用 格式化SPIFFS,可清除所有内容
    SPIFFS.begin();        //bool begin(bool formatOnFail = false, const char *basePath = "/spiffs", uint8_t maxOpenFiles = 10, const char *partitionLabel = NULL);
    FSInfo info;               //信息
    SPIFFS.info(info);         //取闪存文件系统信息
    Serial.printf("闪存.已用:%d 字节,可用:%d 字节;\n",info.usedBytes,info.totalBytes);

    //FFat.format();
    //FFat.begin()
    //Serial.printf("闪存.已用:%d 字节,可用:%d 字节;\n",FFat.usedBytes(),FFat.totalBytes());
    loadConfig();             //装载配置文件信息
    //-------------------------配置舵机插口信息,驱动舵机到初始角度---------------
  for(int I=0;I<ss;I++){                        //循环舵机数组变量
      S[I].attach(Pin[I],500,2500);            //绑定针脚,设置信号脉冲宽度范围//S[I].detach();
      //pinMode(Pin[I], OUTPUT);  
      Old[I]=Raw[I]*1.0;                       //Raw变量拷贝给Old变量
      New[I]=Raw[I]*1.0;  
      S[I].write(Raw[I]);             //写入新角度值,控制舵机转动
  }

  for(int i=0;i<10;i++){
    Tm[i].Dms=0;
    Tm[i].Cmd="";
    Tm[i].T.tm_sec=-1;
  }
  //xTaskCreate(Task1,"Task1",1024,NULL,1,NULL);  //FreeRTOS 创建任务(回调函数,"别名",内存分配, ,优先级0-24, ) 0=空闲,值越大越优先
  //xTaskCreate(Task2,"Task2",1024,NULL,1,NULL);
  //QueueHandle = xQueueCreate(2,sizeof(bool));              //创建队列(最大项目数,最大项目字节数)
  //bool message=1;
  //xQueueSend(QueueHandle,(void*)&message,portMAX_DELAY);  //发送队列,超时时间，0=立即返回
//-------------------------------------------按钮触发----------------------------------------------------------
    if(B1P>0){ //有设定按钮触发的 GPIO  设置特定GPIO引脚接物理按钮开关,触发时执行指定动作文件
    //---使用D1引脚为脚本启动开关,当D1与 5V正极接触,中断调用 ISR 设置 Autorun=1 运行 Auto.txt 动作脚本文件-----
      pinMode(B1P,OUTPUT);                     //为输出模式
      if(B1G){
        digitalWrite(B1P,LOW);                   //设置D1为低电平 LOW状态
        attachInterrupt(digitalPinToInterrupt(B1P),ISR_Btn1,RISING);//启用硬件中断实时捕获 D1 引脚低电平LOW变为高电平HIGH
      } else {  //---使用D1引脚为脚本启动开关,当D1与GND负极接触,中断调用 ISR 设置 Autorun=1 运行 Auto.txt 动作脚本文件-----
        digitalWrite(B1P,HIGH);                  //设置D1为高电平HIGH状态
        attachInterrupt(digitalPinToInterrupt(B1P),ISR_Btn1,FALLING);//启用硬件中断实时捕获 D1 引脚高电平HIGH变为低电平LOW
      }//CHANGE（改变沿，电平从低到高或者从高到低）、RISING（上升沿，电平从低到高）、FALLING（下降沿，电平从高到低）
    }
    if(B2P>0){ //有设定按钮触发的 GPIO  设置特定GPIO引脚接物理按钮开关,触发时执行指定动作文件
    //---使用D1引脚为脚本启动开关,当D1与 5V正极接触,中断调用 ISR 设置 Autorun=1 运行 Auto.txt 动作脚本文件-----
      pinMode(B2P,OUTPUT);                     //为输出模式
      if(B2G){
        digitalWrite(B2P,LOW);                   //设置D1为低电平 LOW状态
        attachInterrupt(digitalPinToInterrupt(B2P),ISR_Btn2,RISING);//启用硬件中断实时捕获 D1 引脚低电平LOW变为高电平HIGH
      } else {  //---使用D1引脚为脚本启动开关,当D1与GND负极接触,中断调用 ISR 设置 Autorun=1 运行 Auto.txt 动作脚本文件-----
        digitalWrite(B2P,HIGH);                  //设置D1为高电平HIGH状态
        attachInterrupt(digitalPinToInterrupt(B2P),ISR_Btn2,FALLING);//启用硬件中断实时捕获 D1 引脚高电平HIGH变为低电平LOW
      }//CHANGE（改变沿，电平从低到高或者从高到低）、RISING（上升沿，电平从低到高）、FALLING（下降沿，电平从高到低）
    }

    //-----------------------下面是连接WIFI网络.可以连路由器或电脑与手机的创建的热点网络-----------------------
    WiFi.setHostname((char*)STA_SSID.c_str());              //设定主机名称,也当设备名称 ESP32
    WiFi.mode(WIFI_AP_STA);                 //WIFI_STA=客户端模式，WIFI_AP=热点模式 
    if(STA_SSID != "" && STA_PSK != "" ) {  //如果有设定要连的路由器WIFI密码,则进行连网
      Serial.printf("\n连接WIFI.SSID:%s 密码:%s\n",STA_SSID,STA_PSK);                    //输出要连到的热点                  
      WiFi.begin(STA_SSID, STA_PSK);          //开发板连 路由器,手机或电脑创建WIFI无线热点,也可开网页控制机械臂
      for(int i=0;i<15;i++){                  //循环十五次共15秒.若连网成功就跳出循环
        if(WiFi.status() == WL_CONNECTED){    //成功连网
            IPAddress IP=WiFi.localIP();      //获取 DHCP 分配的随机IP地址 192.168.X.X
            String S=IP.toString();           //转为字符串IP地址
            //unsigned int V4=IP.v4();        //提取32位的IP地址
            /*
            S.remove(S.lastIndexOf(".")+1);   //提取IP前面字符串 "192.168.X."
            for(int i=6;i<10;i++){            //"192.168.X." + "6" "7" "8" "9"修改末位为做静态地址
               IP.fromString(S+String(i));    //新IP存入变量
               if(WiFi.config(IP,WiFi.gatewayIP(),WiFi.subnetMask())){    //尝试设置静态IP
                  break;                      //设置静态IP成功,跳出循环
               }
            }
            IP=WiFi.localIP(); //重新提取静态IP地址,可能是 192.168.1.6  192.168.31.6  192.168.43.6
            S=IP.toString();
            */
            Serial.print("IP:");Serial.println(S);//输出连网得到的IP地址 
            //很多手机做移动热点时不显示IP地址
            File F=SPIFFS.open("/ip.txt","w"); //"w"=重写文件所有内容
            F.print(S);F.close();              //保存IP到文件可供查阅;关闭文件

            //S.replace(".","-");                //把 192.168.X.X 转成 192-168-X-X 设为网络主机名称
            //WiFi.setHostname((char*)S.c_str());               //修改的名称不一定成功显示.多刷新几次手机里已连接设备 查看
            break;                             //连网成功,跳出循环;在同网络里 手机或电脑打开 http://ip/ 就能控制机械臂
        }
        Serial.print(".");
        digitalWrite(LED_BUILTIN,LOW);       //灭灯
        delay(500);                           //延时500毫秒
        digitalWrite(LED_BUILTIN,HIGH);        //亮灯
        delay(500);                           //延时500毫秒
      }
    }   
    digitalWrite(LED_BUILTIN,LOW);          //亮灯

    if(WiFi.status() == WL_CONNECTED){           //已连网的情况下获取当前网络时间
       //TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3"   //本地时区定义 (柏林)
       //configTzTime(TIMEZONE, "pool.ntp.org"); //获取网络时间  ntp.aliyun.com ~ ntp7.aliyun.com
       configTime(8*3600,0,"pool.ntp.org");
       struct tm timeinfo;
       getLocalTime(&timeinfo,5000);             //超时
       //Serial.println(&timeinfo,"%F %T");        //格式化输出时间 年-月-日 时:分:秒 星期
    }

    //----------------创建WIFI热点.手机电脑连这个热点后打开 http://192.168.1.1/ 就能控制机械臂------------
    if(WiFi.status()!=WL_CONNECTED) WiFi.mode(WIFI_AP);             //如果连网失败改为纯AP模式
    WiFi.softAPConfig({192,168,1,1},{192,168,1,1},{255,255,255,0}); //配置开发板IP,网关与子网掩码.
    if(WiFi.softAP(AP_SSID,AP_PSK)){                                //开启WIFI_AP无线网络,热点 与 密码
       Serial.printf("\n创建WIFI.SSID:%s",WiFi.softAPSSID());
       Serial.printf(" 密码:%s",AP_PSK);
       Serial.printf(" IP:%s\n",WiFi.softAPIP().toString().c_str());
     //Serial.print("MAC );Serial.println(WiFi.softAPmacAddress()); WiFi.getHostname()
    }


    //MDNS.begin(STA_SSID);                   //本地域名解析服务 http://esp32.local  MDNS.setInstanceName(STA_SSID);
    DNS.start(53, "*",{192,168,1,1});       //开启DNS域名解析服务,把所有的域名解析自已的地址,强制门户登录
    //DNS.isCaptive();                      //true=强制门户状态(全部解析到板子AP),false=活动状态
    //DNS.isUp()                            //true=DNS服务正常运行并接受UDP信息
    //DNS.stop();                           //停止DNS域名解析服务,关闭UDP 53端口

    //--------------开启WEB网页服务器.支持网页控制------------------------------------------///
    Web.on("/upload.html",                  //上传文件upload页面.   http://IP/upload.html 可以上传 index.html 等网页文件
            HTTP_POST,                      //POST方式向服务器上传文件
            FileUpload_OK,                  //回复状态码OK=200给客户端  //[](){Web.send(200);}
            handleFileUpload);              //并且运行处理文件上传函数
    Web.onNotFound(handleUserRequest);      //对于未定义请求统统让他处理  /机械臂指令处理回调函数  http://IP/?cmd=x 50;Y _10;Z -10
    //Web.on("/config",HTTP_GET,Config);      //机械臂设置处理回调函数  http://IP/config
    Updater.setup(&Web, "/upbin","admin","12345678");//开启网络更新固件服务,帐密登陆 http://IP/upbin 第一项Firmware固件更新
    Web.begin(80);                          //启动WEB服务器,端口为80
    Serial.println("\nWeb 网页控制服务器开启");


    //----------------开启 TCP/IP 服务端-------------------------  
    if(SPort>4){
      server.begin(); //开启TCP服务接收数据服务,端口:8080
      Serial.printf("TCP服务端口: %d\n",SPort);  //默认 8080
    }
    if(UPort>4){
      U.begin(UPort); //开启UDP服务接收数据服务,端口:8888
      Serial.printf("UDP服务端口: %d\n",UPort);   //默认 8888
    }

    //---------------------------初始化 点灯.科技 物连网 外网控制 功能-----------https://www.diandeng.tech/doc/arduino-support
    if(WiFi.status() == WL_CONNECTED && 6<BlinKey.length()) {
       BLINKER_DEBUG.stream(Serial);          //绑定调试信息输出串口对象
       //BLINKER_LOG("Blinker");
       
       char KEY[16],SID[32],PSK[32];
       BlinKey.toCharArray(KEY,16);           //转为字符数组保存
       STA_SSID.toCharArray(SID,32);          //转为字符数组保存
       STA_PSK.toCharArray(PSK,32);           //转为字符数组保存
       Blinker.begin(KEY, SID, PSK);  
       //BLINKER_TAST_INIT();                   //启动Blinker多任务模式 Blinker.delay(1000); 
       Blinker.setTimezone(8.0);              //设置时区, 如: 北京时间为+8:00 
       //Blinker.connect();                     //设备间连接
       //Blinker.disconnect();                  //断开设备间连接
       //Blinker.connected();                   //设备间连接状态
       //Serial.print(Blinker.init());                          //设备初始化,返回状态
       Blinker.attachData(dataRead);          //附加 默认回调函数
       Blinker.attachHeartbeat(heartbeat);    //附加 心跳回调函数,每30s-60会发送一次心跳包
       
       Slider1.attach(sliderX);               //附加 滑动条  事件
       Slider2.attach(sliderY);               //附加 滑动条  事件
       Slider3.attach(sliderZ);               //附加 滑动条  事件
       Slider4.attach(sliderB);               //附加 滑动条  事件
       Slider5.attach(sliderT);               //附加 滑动条  事件
       Slider6.attach(sliderE);               //附加 滑动条  事件
    }
}
//----------------开发板会循环不断的调用此函数.正式获取外部信息,进行解板并执行相应功能---------------------------
void loop() {
    static  unsigned long timer=0;            //保存板子启动经历过毫秒数

    DNS.processNextRequest();                 //DNS域名解析,强制门户登录,
    if(6<BlinKey.length()) Blinker.run();     //多任务不用执行这句，处理 点灯科技 物连网 通信数据
    Web.handleClient();                       //处理客户HTTP访问,上传文件,更新固件

   if(SPort<4){

   } else if(!C || !C.connected()){              //客户端对象 状态 未连接
      if(SIP==""){                               //未配置远程服务端地址
        C=server.available();                    //TCP服务端 检查获取新客户端
        if(C) Serial.println("接入新客户");  
      } else if(WiFi.status()== WL_CONNECTED && millis()>timer && Cmd=="") {
          timer=millis()+120000;                 //2分钟
          C.connect(SIP.c_str(), SPort);         //有配置服务端地址,尝试连服务端  
      } 
   } else if(C.available()) {                    //检查数据
        String S = C.readStringUntil(';');       //读取数据直到;命令分隔符
        if(S.startsWith("GET") || S.startsWith("POST")){
           C.stop();Serial.println("断开TCP客户");//客户端断开连接
        } else {
           Serial.println("TCP数据: " + S);  
           String ret=CheckCmd(S);
           C.print(ret);                         //给客户端发送数据
        }
   }

  if (UPort>4 && U.parsePacket()) {  // 获取UDP端口数据 端口: 8888  int available();
    char data[255];  
    int len = U.read(data, 255); 
    if (len > 0) {   
      data[len] = '\0';  
      Serial.print("UDP data: ");  
      Serial.println(data);  
      String ret=CheckCmd(String(data));              //串口回显指令
      U.beginPacket(U.remoteIP(), U.remotePort());    //UDP返回执行结果
      U.print(ret);
      //U.write((uint8_t*)ret.c_str());     //void flush();  // Print::flush tx   .c_str()
      U.endPacket(); 
    } //  void clear();  // clear rx
  }

   //---------------监听串口输入指令-------------------
   if(Serial.available()>0){                   //有串口数据>0字节
     String S=Serial.readString();             //?SDCHRA XYZBTE
     Serial.println(S);                        //串口回显指令
     Serial.println(CheckCmd(S));              //串口回显指令
   }

   if(B1N==1){B1N=0;ISR_Btn(B1R);}  //检查按钮1事件并执行
   if(B2N==1){B2N=0;ISR_Btn(B2R);}  //检查按钮2事件并执行

    if(millis()>Dms && 0<Cmd.length()){       //如果cmd全局变量里有未执行命令,现在执行
      //Serial.printf("loop.Cmd  %s \n",Cmd); 
      Cmdret=FirstCmd();                      //分析指令并调用Command执行
    }

    bool   online=false;
    static  struct tm T;                      //获取并保存最新的网络时间
    if(WiFi.status()== WL_CONNECTED){         //需要连路由器有外网状态才能获取网络时间
      //configTime(8*3600,0,"pool.ntp.org");
      if(getLocalTime(&T,3000)){           //最多3秒时间来获取网络时间;成功=true //Serial.println(&T,"%F %T");  格式化输出时间 年-月-日 时:分:秒 星期
        T.tm_year+=1900;                        //获取的时间与现实可读时间差 1900年1个月
        T.tm_mon +=1;                           //Serial.printf("%d-%d-%d %d:%d:%d \n",T.tm_year,T.tm_mon,T.tm_mday,T.tm_hour,T.tm_min,T.tm_sec);
        online=true;
        //currentTime.year = timeinfo.tm_year + 1900;  
        //currentTime.month = timeinfo.tm_mon + 1;   
        //Serial.printf("%d-%d-%d %d:%d:%d \n",T.tm_year,T.tm_mon,T.tm_mday,T.tm_hour,T.tm_min,T.tm_sec);
      }
    }
    for(int i=0;i<10;i++){                       //循环 倒计时 和 定时 的10个共用变量
      if(Tm[i].Dms>0 && (millis()>Tm[i].Dms)){   //判断倒计时
        Cmd+="\n"+Tm[i].Cmd;
        Tm[i].Dms=0;
        Tm[i].Cmd="";
        break; 
      }
      if(Tm[i].T.tm_sec!=-1 && online){ //tm_year tm_mon tm_mday tm_hour tm_min tm_sec
        //Serial.printf("%d-%d-%d %d:%d:%d Cmd=%s \n",Tm[i].T.tm_year,Tm[i].T.tm_mon,Tm[i].T.tm_mday,Tm[i].T.tm_hour,Tm[i].T.tm_min,Tm[i].T.tm_sec,Tm[i].Cmd);
        //JavaScript:

       //编码: encodeURIComponent(string)
       //解码: decodeURIComponent(string)
       //编码: urlencode(string)
       //解码: urldecode(string)
        bool del=false;  //true=删除这个定时命令,false=不删除       
        if(T.tm_sec>3) {
           continue;       //秒已过 只在前3秒执行命令 
        } else if(Tm[i].T.tm_year<T.tm_year && Tm[i].T.tm_year>0) {
           del=true;       //年已过
        } else if(Tm[i].T.tm_year>T.tm_year){
          continue;        //年未到
        } else if(Tm[i].T.tm_mon<T.tm_mon && Tm[i].T.tm_mon>0){
           del=true;       //月已过
        } else if(Tm[i].T.tm_mon>T.tm_mon){
          continue;        //月未到
        } else if(Tm[i].T.tm_mday<T.tm_mday && Tm[i].T.tm_mday>0){
           del=true;       //日已过
        } else if(Tm[i].T.tm_mday>T.tm_mday){
          continue;        //日未到
        } else if(Tm[i].T.tm_hour<T.tm_hour && Tm[i].T.tm_hour!=-1 ){
          del=true;        //时已过
        } else if(Tm[i].T.tm_hour>T.tm_hour){
          continue;        //时未到
        } else if(Tm[i].T.tm_min<T.tm_min && Tm[i].T.tm_min!=-1){
          del=true;        //分已过
        } else if(Tm[i].T.tm_min>T.tm_min){
          continue;        //分未到
        } 
        String S=Tm[i].Cmd; //time 2024-10-1 12:10:0,H.txt
        if(del){                       //已过时,删除
          int i=S.indexOf("\nTime",0); //查找  Time 指令 
          if(i>0){
            S.remove(0,i+1);
            Cmd+="\n"+S;
          } 
        } else  Cmd+="\n"+S;           //执行定时命令
        Tm[i].Dms=0;
        Tm[i].Cmd="";
        Tm[i].T.tm_sec=-1;
        //Serial.println(Cmd);
        break;   // Y 60;time 2024-9-14 21:10:0,H.txt,1;   X 90;Y 60;time 2024-10-14 21:?:0,X -10,8;
      }
    }

/*if(WiFi.status()!= WL_CONNECTED && millis()>timer  && Cmd==""){  //掉线 2分钟
    timer=millis()+120000;          
    WiFi.begin(STA_SSID, STA_PSK);                                           //重连路由器WIFI
  }*/
}

//---------------------------------------------------------------------------------------------
String CheckCmd(String S) {                 //命令分两类，一类是需要立即执行的,二类是排队执行的..
     int i[2]={0};                          //[0]=; [1]=\n
     String ret="";

     //Serial.println("Cmdcheck");  
     //S.toUpperCase();                      //指令转为大写
     S.trim();                               //删首尾空格与换行
     S.replace("；",";");                    //把中文 ；符号替换成英文 ; 符号
     S.replace("，",",");                    //把中文 ，符号替换成英文 , 符号
     S.replace("？","?");                    //把中文 ，符号替换成英文 , 符号
     do{                                     //剔除命令后面的注释内容或注释整行
       i[0]=S.indexOf("//" ,0);              //查找  // 注释行，全部删除，以免干扰后面处理
       if(i[0]!=-1){                         //有注释行
          i[1]=S.indexOf("\n",i[0]);
          if(i[1]>0){                        //还有剩余行
            S.remove(i[0],i[1]-i[0]);        //除中间 长度
          } else {
            S.remove(i[0]);break;            //除尾
            } 
       }
     }while(i[0]>0 && i[1]>0);               //未完 继续循环
    
    if(0<S.indexOf(";",0) || 0<S.indexOf("\n",0)){ //有多条命令,直接加入命令池
        Cmd.concat(S);
        Cmd.concat("\n");
        ret=output();
    } else ret=Command(S,true);                  //单个一类命令立即执行并返回结果
   return ret;
}
//---------------------------------------------------
// 舵机控制命令支持 单个执行也支持多个同时执行
// 单命令   X 50
// 多命令   X 50;Y +10;Z -10
// 多行命令 X 150;Y 100
//         Z 100;E 80
//---------------------------------------------------
String FirstCmd() {                    //从指令池提取一个命令,拆分以;为分隔的多个命令 或折分\n多行命令 
   int i[2]={0};                       //[0]=; [1]=\n
   String S="",ret="";

   if(Cmd.length()<1) return "";          //已无指令.返回
    //Serial.println(Cmd);                //串口输出拆分出来的命令和参数
   Cmd.trim();                            //删首尾空格与换行
   do{
     i[0]=Cmd.indexOf(";" ,0);            //查找  ; 分隔符 
     if(i[0]==0) Cmd.remove(0,1);         //若首字符就是 ; 删除继续查找
   } while (i[0]==0);                       
   
   i[1]=Cmd.indexOf("\n",0);              //查找 \n 换行符 
   //Serial.printf("FirstCmd.Cmd%s 长 %d i%d %d\n",Cmd,Cmd.length(), i[0],i[1]); 
   if(i[0]==-1 && i[1]==-1){              //没有 ; 和 \n 符号 
      S=Cmd;                              //单行单指令
      Cmd.remove(0);//Cmd="";             //删除指令
   } else if(i[0]!=0 && i[1]==-1){        //单行有 ; 分隔的多个指令
      S=Cmd.substring(0,i[0]);            //提取一个指令执行
      Cmd.remove(0,i[0]+1);               //删除已提取的指令
   } else if(i[0]==-1 && i[1]!=0){        //当前无;分隔的单指令 且多行 
      S=Cmd.substring(0,i[1]);            //提取单行指令
      Cmd.remove(0,i[1]+1);               //删除已提取的此行
  } else if(i[0]<i[1]) {                  // ; 在 \n 之前
      S=Cmd.substring(0,i[0]);            //提取一个指令执行
      Cmd.remove(0,i[0]+1);               //删除已提取指令
   } else if(i[0]>i[1]){                  // \n 在 ; 之前
     S=Cmd.substring(0,i[1]);             //提取一个指令执行
     Cmd.remove(0,i[1]+1);                //删除已提取指令      
  } 
  ret=Command(S);                        //执行提取的这个指令
  if(ret=="") ret=output();
  return ret;
}
//---------------------------------------------------------------------------
//收到 test 指令会执行这个测试程序,控制 X 和 E 舵机转动
//可供参考用,自已编程使用 变量存取 与 逻辑条件 做复杂功能
//==============================================================
String Test(String S1="",String S2="",String S3="",String S4="",String S5="",String S6=""){ //一个简单的测试底座来回转到的功能
    Command("H");                        //所有舵机复位
    for(int i=1;i<6;i++){
      Command("X " + String(90 + i*10));  //X底座转向一侧
      Command("E -40");                   //E开夹20度
      Command("X " + String(90 - i*10));  //X底座转向另一侧
      Command("E +40");                   //E闭夹20度
    }
    Command("X ?;Y ?;Z ?");               //XYZ随机角度
    Command("H");                         //所有舵机复位
    return "Test()";
}
//------------------------------------------------------------------------
//
//
//
//-----------------解析并执行命令 已定义常用命令 ?SDCHRA XYZBTE---------------
String Command(String t,bool CheckCmd)  {//=false
  static String  file="/Auto.txt";        //静态变量,存放FSDRC命令将操作的文件

  String S[7]={"","","","","","",""};    //[0]命令,[1]参一,[2]参二,[3]参三,[4]参四,[5]参五,[6]参六
  int    i=0,j=0,v=0;                    //定位指令文本中位置
  String Auto,ret="";                    //Auto.txt文件内容,返回结果

    //-------------------拆分以空格分隔的命令与参数--最多6个参数-------------
    for(i=0;i<7;i++){                     //拆分 1个命令 六个参数
      t.trim();                           //删首尾空格与换行
      if(t=="") break;                    //无内容 跳出
      if(i==1) Auto=t;                    //备份所有参数内容
      if(i==0) 
         j=t.indexOf(" ",0);              //查找空格 拆分命令与参数
      else
         j=t.indexOf(",",0);              //查找逗号 拆分命令与参数
      S[i]=t.substring(0,j);              //提取命令或参数内容
      S[i].trim();                        //删首尾空格与换行
      if(j==-1) {t="";break;}             //无空格 跳出
      t=t.substring(j+1);                 //留下空格后边内容,继续循环提取        
    } t=Auto;//t.trim();                           //若有多余内容会留存在 t 参数里
    //Serial.printf("Command.命令:%s 参数:%s 参数:%s 参数:%s 参数:%s\n",S[0],S[1],S[2],S[3],S[4]);//串口输出拆分出来的命令和参数

    //----------------解析执行一些命令 下面为立即执行并返回结果的指令 -----------------------------
    if(S[0].startsWith("//")){                 //跳过动作文件中的备注行
      return "";
    }
    if(S[0]=="?" || S[0]=="？"){                                         //？HELP.txt 输出指定文件的内容
      if(S[1]=="") return output();                                      //?          输出 {"X":90,"Y":90,"Z":90,"E":90,"B":90,"T":90,"Cmd":0}
      if(S[1].equalsIgnoreCase("name")) return AP_SSID;                  //? name     输出设备名称 AP_SSID
      if(S[1].equalsIgnoreCase("F")) return "{\"F\":\"" + file + "\"}";  //? F        输出默认文件名 {"F":"Auto.txt"}
      if(S[1].equalsIgnoreCase("IP")) return WiFi.localIP().toString();  //? IP       获取 DHCP 分配的随机IP地址 192.168.X.X 未联网返回 (IP unset)
      if(!S[1].startsWith("/")) S[1]="/"+S[1]; //比较字符串前缀
      if(1>S[1].indexOf(".",1)) S[1]+=".txt";  //比较字符串后缀
      File F=SPIFFS.open(S[1],"r");            //"r"=只读内容,"w"=重写内容
      if(F){
        F.seek(0);                             //到首位置
        ret=F.readString();                    //读入文本
        F.close();                             //关闭文件
      }
      return ret;                              //返回文件内容
    }
    if(S[0].startsWith("sync")){               //sync;X 180;X 0;X 90; 前面插一条啥也不干的指令,可以立即返回,其他指令放入指令池
      if(S[1].toInt()==0){                     //异步
              sync=false;
      } else {sync=true;}                     
      return "";
    }
    if(S[0].equalsIgnoreCase("DIR")){         //枚举文件并输出
        FSInfo info;                     //信息
        SPIFFS.info(info);               //取闪存文件系统信息
        ret="闪存.已用:"+String(info.usedBytes)+"字节;可用:"+String(info.totalBytes)+"字节;\n";

        Dir dir=SPIFFS.openDir("/");     //打开根目录,枚举文件名
        while(dir.next()) {              //循环所有对象
           ret+=dir.fileName();
           File F=dir.openFile("r");
           ret+="\t";                    //Tab 制表符
           ret+=String(F.size());        //取文件长度
           ret+="\n";
           F.close();
        }
        return ret;                      //返回输出所有枚举到的文件信息
    }
    if(S[0].equalsIgnoreCase("format")){      //格式化 开发板闪存文件系统 清除所有文件,需要重新上传
        SPIFFS.format();                      //格式化 
        FSInfo info;                     //信息
        SPIFFS.info(info);               //取闪存文件系统信息
        ret="闪存.已用:"+String(info.usedBytes)+"字节;可用:"+String(info.totalBytes)+"字节;\n";
        ret+="已清除所有文件,重新 [上传文件]";
        return ret;
    }
    if(S[0].equalsIgnoreCase("RE")){          //re 重启开发板 Reboot
      ESP.restart();                          //软重启
      return "";
    }
    if(S[0].equalsIgnoreCase("Stop")){        //
      if(S[1].toInt()==0){                    //Stop 0 停止并清除指令池
         Cmd="";                              
         Dms=millis();
      } else if(S[1]=="-1"){                  //stop -1 长时间暂停，需要用 Start 恢复
         Dms=-1;
      } else if(S[1].toInt()>0){              //stop 10000 暂停10秒，可用  Start 立即恢复
         Dms=millis()+S[1].toInt();
      }
      return "Stop "+S[1];
    }if(S[0].equalsIgnoreCase("Start")){     //恢复 stop -1 或 stop 10000 的暂停
      Dms=millis();
      return "Start";
    }if(S[0].equalsIgnoreCase("Time")){                    //定时 和 倒计时和  time 10000,H.txt,1  倒计时10秒执行 H.txt 一次
      if(S[1].indexOf("-",0)>0 || S[1].indexOf(":",0)>0){  //time 2024-10-1 12:10:0,H.txt,1    定时执行命令
       struct tm T={0,0,0,0,0,-1};  //T(2024, 10, 1, 15, 30, 45)  
       //year  month  day  hour  minute  second
       //tm_year tm_mon tm_mday tm_hour tm_min tm_sec
       //%Y年-%m月-%d日 %H时:%M分:%S秒  周期%w  time_info.get_timestamp()
       String s;
       S[0]=S[1];                                          //备份参数一 日期
       //-----------------------------------------------------
       i=S[1].indexOf("-",0); 
       if(i!=-1){
        s=S[1].substring(0,i);             //年
        T.tm_year=s.toInt();
        //Serial.println(T.tm_year);
        S[1]=S[1].substring(i+1);          //留下空格后边内容

        i=S[1].indexOf("-",0); 
        s=S[1].substring(0,i);             //月
        T.tm_mon=s.toInt();
        //Serial.println(s);
        S[1]=S[1].substring(i+1);          //留下空格后边内容 

        i=S[1].indexOf(" ",0); 
        s=S[1].substring(0,i);             //日
        T.tm_mday=s.toInt();
        //Serial.println(s);

        S[1]=S[1].substring(i+1);          //留下空格后边内容 
        S[i].trim();                       //删首尾空格与换行
       }
       
       i=S[1].indexOf(":",0); 
       if(i!=-1){
        s=S[1].substring(0,i);             //时
        T.tm_hour=s.toInt();
        if(String(T.tm_hour)!=s) T.tm_hour=-1;
        //Serial.println(s);
        S[1]=S[1].substring(i+1);          //留下空格后边内容

        i=S[1].indexOf(":",0); 
        s=S[1].substring(0,i);             //分
        T.tm_min=s.toInt();
        if(String(T.tm_min)!=s) T.tm_min=-1;
        //Serial.println(s);
        s=S[1].substring(i+1);             //留下空格后边内容 

        T.tm_sec=0; //s.toInt();           //秒固定为 0
        //Serial.println(s);
        }
        for(i=0;i<10;i++){
          if(Tm[i].Dms==0 && Tm[i].T.tm_sec==-1){
            Tm[i].T=T;
            Tm[i].Dms=0;
            v=S[3].toInt();
            if(v>1) {
              v--;
              Tm[i].Cmd=S[2]+"\ndelay 3000;\nTime "+S[0]+","+S[2]+","+String(v)+";"; 
            } else {
              Tm[i].Cmd=S[2]; 
            }
            break;                              //跳出循环
          }
        } 
       //Serial.println(&Tm[i].T,"%F %T"); //输出时间 年-月-日 时:分:秒  %w星期
       //Serial.printf("Time %d-%d-%d %d:%d:%d\n",Tm[i].T.tm_year,Tm[i].T.tm_mon,Tm[i].T.tm_mday,Tm[i].T.tm_hour,Tm[i].T.tm_min,Tm[i].T.tm_sec);
       //Serial.println(Tm[i].Cmd);
      } else if(S[1].toInt()>=10) {               //time 10000,H.txt,1    倒计时10秒执行 H.txt 一次
        for(i=0;i<10;i++){
           if(Tm[i].Dms==0 && Tm[i].T.tm_sec==-1){
              Tm[i].Dms=millis()+S[1].toInt();
              v=S[3].toInt();
              if(v>1) {
                v--;
                Tm[i].Cmd=S[2]+"\nTime "+S[1]+","+S[2]+","+String(v); 
              } else {
                Tm[i].Cmd=S[2]; 
              }  
              break;                              //跳出循环
           }
        }
      } else if(S[1]==""){                       //time 无参数  查询命令
        for(i=0;i<10;i++){
          if(Tm[i].Dms>0){
            ret += String(i) + " " ;
            j=Tm[i].Cmd.indexOf("\n",0);
            if(j>0) ret+=Tm[i].Cmd.substring(0,j);  else  ret+=Tm[i].Cmd;   ret+="\n";
          } else if(Tm[i].T.tm_sec!=-1){
            ret += String(i)+" ";
            if(Tm[i].T.tm_year!=0)  ret+=String(Tm[i].T.tm_year); else ret+="?";  ret+="-";
            if(Tm[i].T.tm_mon!=0)   ret+=String(Tm[i].T.tm_mon);  else ret+="?";  ret+="-";
            if(Tm[i].T.tm_mday!=0)  ret+=String(Tm[i].T.tm_mday); else ret+="?";  ret+=" ";
            if(Tm[i].T.tm_hour!=-1) ret+=String(Tm[i].T.tm_hour); else ret+="?";  ret+=":";
            if(Tm[i].T.tm_min!=-1)  ret+=String(Tm[i].T.tm_min);  else ret+="?";  ret+=":0\n";
          }
        }
        return ret;
      } else if(S[1].toInt()<0){                   //time -1  清空所有定时与倒计时指令
         for(i=0;i<10;i++){                       
           Tm[i].Dms=0;
           Tm[i].Cmd=""; 
           Tm[i].T.tm_sec=-1;
          }
        return "time -1";
      } else if(S[1].toInt()>=0){                  //time 1 删除 time[1] 内容
         i=S[1].toInt();
         Tm[i].Dms=0;
         Tm[i].Cmd=""; 
         Tm[i].T.tm_sec=-1;
      }
      return "Time "+String(i);
    }
 

   if(CheckCmd && (0<Cmd.length() || !sync)){      //如果当前指令来自 CheckCmd() 的调用,并且指令池非空,追加进指令池排队
      Cmd+="\n" + S[0] + " " + t + ";";
      return output();
    }
//----------------------来自 CheckCmd() 调用并且 指令池为空，会继续往下执行---------------------------------------
    if(S[0].equalsIgnoreCase("SH")){               //保存XYZBTE舵机当前位置到 H.txt 文件
      File F=SPIFFS.open("/H.txt","w");            //"w"=重写文件所有内容
      F.seek(0);                                   //到首位置
      for(int I=0;I<ss;I++){                       //循环所有舵机变量进行输出保存
        F.printf("%s %.1f\n",XYZE[I],New[I]);      //保存新位置到文件
      }
      F.close();                                   //关闭文件
      return Command("? /H.txt",CheckCmd);         //返回 保存的内容
    }
    if(S[0].equalsIgnoreCase("F")){            //用 "F test.txt" 指定动作文件名
      if(S[1]=="") S[1]="/Auto.txt";           //用 "F Auto.txt" 改变动作文件名
      if(!S[1].startsWith("/")) S[1]="/"+S[1]; //比较字符串前缀
      if(1>S[1].indexOf(".",1)) S[1]+=".txt";  //比较字符串后缀
      file=S[1];                               //保存文件名,之后 R S D C 将对该文件操作
      return ("{\"F\":\""+file+"\"}");         //输出新动作文件名称 {"F":"/abc.txt"}
    }
    if(S[0].equalsIgnoreCase("C")){              //C Auto;C test;清空指定文件内容并删除文件
      if(S[1]=="") S[1]=file;                    //参数若未指令文件，删除file
      if(!S[1].startsWith("/"))  S[1]="/"+S[1];  //比较字符串前缀
      if(1>S[1].indexOf(".",1))  S[1]+=".txt";   //比较字符串后缀
      SPIFFS.remove(S[1]);                       //删除文件
      return (S[0] + " " + S[1]);                //输出删除的文件
    }
    if(S[0].equalsIgnoreCase("S")){                       //保存XYZETB电机新角度指令到自动执行文件
        ret="";
        if(S[1].toInt()>0){                               //S xx 转为 delay xx 保存到文件
            ret="delay "+String(S[1].toInt());            //保存delay xx到文件
        } else if(S[1]!=""){                              //S delay xx 保存 delay xx  到文件  
            ret=t;                                        //S Y 1500;S test.txt;直接存到文件
        } else {                                          //判断位置发生变化的所有舵机并保存
            ret="XYZ "+String(New[0],1);                  //X
            ret+=","+String(New[1],1);                    //Y
            ret+=","+String(New[2],1);                    //Z
            ret+=","+String(New[3],1);                    //B
            ret+=","+String(New[4],1);                    //T
            ret+=","+String(New[5],1);                    //E
        }
        if(ret!=""){                            //有 待追加命令
            File F=SPIFFS.open(file,"r");       //"r"=读取内容
            if(F){
              F.seek(0);                          //到首位置
              Auto=F.readString();                //读入文本
              Auto.trim();                        //删首尾空格与换行
              F.close();                          //关闭文件
            } else Auto="";

            F=SPIFFS.open(file,"w");            //"w"=重写文件所有内容
            if(Auto!="") F.println(Auto);       //先保存原指令
            F.println(ret);                     //再追加新指令
            F.flush();F.close();                //结束关闭文件
        }
        return ret;                             //返回信息串口输出新指令
    }
    if(S[0].equalsIgnoreCase("D")){            //D 1 删除最后一行;  D 3 删除多行动作指令
        File F=SPIFFS.open(file,"r");          //"r"=只读内容,"w"=重写内容
        F.seek(0);                             //到首位置
        Auto=F.readString();                   //读入文本
        Auto.trim();                           //删首尾空格与换行
        F.close();                             //关闭文件

        v=S[1].toInt();                        //D 3 删除末尾3行
        if(v==0) v=1;
        if(v>0){
          for(;v>0;v--){                      //循环次数是要删除几行
            i=Auto.lastIndexOf("\n");         //倒找换行符
            if(i<0) i=0;                      //-1=无换行符(改0,删除所有内容)
            Auto.remove(i);                   //删除i后面的文本
          }
        } else Auto="";                       //D -1 清空文件内容

        F=SPIFFS.open(file,"w");            //"w"=重写文件所有内容
        F.seek(0);                          //到首位置
        F.print(Auto);                      //保存剩余指令到文件
        F.close();                          //关闭文件
        return Auto;                        //输出删行后的剩余 动作指令
    }
    if(S[0].equalsIgnoreCase("delay")){        //动作脚本中 delay xx 由这段代码执行
        Dms=millis()+S[1].toInt();             //delay的参数,即是要延时的毫秒时间+板子开机已经过的毫秒时间
        return "delay "+S[1];
    }
    if(S[0].equalsIgnoreCase("H")){          //HOME 让XYZBTE六个电机到指定位置
        if(SPIFFS.exists("/H.txt")){         //如果有 H.txt 则执行
           Command("H.txt");
        } else for(int I=0;I<ss;I++){        //无 H.txt 则执行Raw变量值
           ServoGo(I,Raw[I]);
        } 
        return output();
    }
    if(S[0].equalsIgnoreCase("Test")){         //执行一段底座左右转动的测试动作
      return Test(S[1],S[2],S[3],S[4],S[5],S[6]);
    }
    if(S[0].equalsIgnoreCase("step")){         //改变舵机速度,step 0=原速,step 1=减速;                              
      if(S[1].toFloat()>0.0) 
        Step=1.0;                              //MG996R 设0.4，，SG90 设1.0
      else
        Step=0.0;
      return "Step "+String(Step,0);           //
    }
    if(S[0].equalsIgnoreCase("XYZ")){            //XYZ(+10,-10,170,50)  同时转动多个舵机 X加转10度,Y减转10度,Z转到170度,E开到50度
      float BS=Step;
      Step=1.0;
      for(int i=1;i<=ss;i++){                     //循环6个参数 
        int I=i-1;                               //舵机数组下标
        if(S[i]=="++"){                          //X ++;  加转1度
          New[I]=New[I]+1.0;
        } else if(S[i]=="--"){                   //X --;  减转1度
          New[I]=New[I]-1.0;
        } else if(S[i]=="?" || S[i]=="？"){      //X ?    随机角度
          New[I]=rand() % (Max[I]-Min[I])+Min[I];
        } else if(S[i].startsWith("+")){         //X +10  加转10度
          New[I]=New[I]+S[i].substring(1).toFloat();
        } else if(S[i].startsWith("-")){         //X -10  减转10度
          New[I]=New[I]-S[i].substring(1).toFloat(); //tofloat
        } else if(S[i]!=""){
          New[I]=S[i].toFloat();                 //X 90   直接角度值
        }continue;
        New[I]=constrain(New[I],Min[I],Max[I]);  //限制角度范围
      }
      ServoGo(-1,0);                             //执行电机指令
      Step=BS;
      return output();
    }

    //----------解析XYZE舵机指令  底座X 前后Y 上下Z 平衡B 旋转T 夹子E
    if(t==""){ //正确舵机命令如 X 180 应该有空格隔开，为了方便输入,兼容无空格的如 X180 没有空格分开的命令 舵机名与角度值
      ret=S[0].substring(0,1);
      for(int I=0;I<ss;I++){
        if(ret.equalsIgnoreCase(XYZE[I])){     //判断舵机
          char c=S[0].charAt(1);               //第二字符
          if(c>=48 && c<=57){                  //0-9
          } else if(c==45){                    //'-'
          } else if(c==43){                    //'+'
          } else break;
          S[1]=S[0].substring(1);
          S[0]=ret;
          break;
        }
      }
    }

    for(int I=0;I<ss;I++){                    //循环所有舵机
      if(S[0].equalsIgnoreCase(XYZE[I])){     //判断哪个舵机
        float f=0.0;
        if(S[1]=="++"){                       //X ++;  加转1度
          f=New[I]+1.0;
        } else if(S[1]=="--"){                //X --;  减转1度
          f=New[I]-1.0;
        } else if(S[1]=="?" || S[1]=="？"){   //X ?    随机角度
           f=rand() % (Max[I]-Min[I])+Min[I];
        } else if(S[1].startsWith("+")){      //X +10  加转10度
           f=New[I]+S[1].substring(1).toFloat();
        } else if(S[1].startsWith("-")){       //X -10  减转10度
           f=New[I]-S[1].substring(1).toFloat();
        } else f=S[1].toFloat();               //X 90   直接角度值

        ServoGo(I,f);                          //执行电机指令
        return output();
      }
    }

   if(CheckCmd){      //如果当前指令来自 CheckCmd() 的调用,因为指令文件可能很耗时,现在阻止往下执行指令文件
      Cmd+="\n" + S[0] + " " + t + ";";
      return output();
    }
   //---------------------执行 指令文件 如 Auto.txt 1 ----------------------------
   ret="未知命令 "+S[0];
    if(S[0].equalsIgnoreCase("R")){        //R   运行file变量指定文件的动作指令
       S[0]=file;
       if(String(S[1].toInt())!=S[1]){     //R Auto  执行Auto.txt 1次
         if(S[1]!="") 
            S[0]=S[1];
         S[1]="";
       } 
    } 
    //对于不可识别的指令尝试以动作文件执行,如 "Q" 变会成 "/Q.txt" 若存在则执行                  
    if(!S[0].startsWith("/"))  S[0]="/"+S[0];  //比较字符串前缀
    if(1>S[0].indexOf(".",1))  S[0]+=".txt";   //比较字符串后缀
    if(SPIFFS.exists(S[0])){                   //文件是否存在.若有此文件则执行
     ret=S[0];

     i=S[1].toInt()-1;                         //Auto.txt 5 提取执行次数
     if(i>0) Cmd=S[0] + " "+String(i)+"\n";

     File F=SPIFFS.open(S[0],"r");             //打开文件 "r"=只读取文件内容  
      //F.seek(0);                             //到文件首位置
     Auto=F.readString();                      //读内容到变量
     F.close();                                //关闭文件

     //考虑命令文件有可能用户直接上传进来的,对内容进行简单处理一下
     Auto.trim();                      //删首尾空格与换行
     Auto.replace("；",";");           //把中文 ；符号替换成英文 ; 符号
     Auto.replace("，",",");           //把中文 ，符号替换成英文 , 符号
     do{                               //剔除命令后面的注释内容或注释整行
       i=Auto.indexOf("//" ,0);        //查找  // 注释行，全部删除，以免干扰后面处理
       if(i!=-1){                      //有注释行
          j=Auto.indexOf("\n",i);
          if(j>0){                     //还有剩余行
            Auto.remove(i,j-i);        //除中间 长度
          } else {
            Auto.remove(i);break;      //除尾
            } 
       }
     }while(i>0 && j>0);               //未完 继续循环
    
     Cmd=Auto+"\n"+Cmd;                //把内容加入命令池
     return ret;
    }
  return ret;
}
//---------------接受机械臂网页控制指令函数 -------------------------------------
//网址里不能用 + 符号,会变成空格，所以要用 _ 替换掉 + 符号
//
//---------------处理用户浏览器的HTTP访问----------------------------------------
void handleUserRequest(){
  String host=Web.hostHeader(),url = Web.uri();  //获取用户请求网址信息

  //获取并输出HTTP请求数据信息
  String  message = (Web.method() == HTTP_GET) ? "GET " : "POST ";   //请求 类型
  message +=   host;
  message +=   url;
  if(Web.args()>0) message += "?";                                   //参数 数目   
  for (int i=0;i<Web.args();i++) {                                   //枚举所有参数
    if(i>0) message += "&";
    message += Web.argName(i);
    String t = Web.arg(i);
    if(t != ""){
      message += "=" + t;  //参数名称与值    
    }
  }//String urlDecode(const String& text);
  Serial.println();Serial.println(message);                          //串口输出信息

  //-------------------------http://192.168.1.1/   访问首页 index.html ---------------
  if(!host.startsWith("192.168")){
    Web.sendHeader("Location", "http://192.168.1.1/index.html");
    Web.send(303);                               //303跳转
    return;
  }if(url=="/favicon.ico"){
    Web.send(404, "image/x-icon", "");
    return;
  }
  if (url.endsWith("/") || url.startsWith("/index")) {      // 访问地址以"/" 结尾 或 /index 开头
    url = "/index.html";                                    //修改为 /index.html 文件
    if (!SPIFFS.exists(url)) url = "/upload.html"; 
  } else if(url.equalsIgnoreCase("/config.html")){
    if (!SPIFFS.exists(url)) url = "/upload.html"; 
  } else if(url.startsWith("/config")){                     //config.json  config?
    if(Config()){
     //Web.sendHeader("Location", "http://192.168.1.1/index.html");
     Web.send(200, "text/html ; charset=utf-8", "<meta http-equiv='refresh' content=18;URL='/index.html'>已保存配置,正在重启控制板,稍后重连WIFI,重开网页<a href='/index.html'>[回首页]</a>"); 
     for(int i=0;i<50;i++) delay(10);
     ESP.restart();                     //软重启开发板
     return;
    }
  } else if(url.equalsIgnoreCase("/cmd")) {     //HTTP命令接口  http://192.168.1.1/cmd?cmd=X 160;X 60;X 90&time=1111 
    if(Web.args()==0){                          //无命令参数     http://192.168.1.1/cmd  返回所有舵机状态
      message=output(); 
      if(WiFi.status() == WL_CONNECTED){        //成功连网
         IPAddress IP=WiFi.localIP(); 
         message=IP.toString()+"\n"+message;
        }
       Web.send(200, "text/plain; charset=utf-8", message); 
       return;                                 //返回
    } 
    //--------------------//有参数的请求交给命令接口处理 +换成_  GET http://192.168.1.1/cmd?x_10;y90;E-10;time=时间戳或随机值-------
    String s="";
    static String time;                                      // 静态变量时间戳
    for(int i=0;i<Web.args();i++){ 
      String t=Web.argName(i);                                 // 提取一个指令
      if(t.equalsIgnoreCase("time")){                          // 判断参数名
        if(time==Web.arg(i)){                                  // 若 time 的值 时间戳或随机值 与上次相同, 返回
          Web.send(200, "text/plain; charset=utf-8", "");      // 若参数 time 与上次重复,直接返回,避免重复执行
          return;
        } else {time=Web.arg(i);continue;}                     //记录 time 的值,防重复命令用 并 跳过 time 参数
      }
      t.trim();                       //删首尾空格与换行
      // 网页脚本XMLHttpRequest的GET请求把"+"号变成" "空格,而" "空格还是" "空格
      t.replace("_" ,"+");            // 把"_"恢复为"+"; 网页脚本arg=arg.replace(/\+/g ,"_"); //把'+'替换为'_'
      Serial.print("HTTP命令:");Serial.println(t); 
      if(s != "") s += ";" ;
      s += t;
    }
    message=CheckCmd(s);              //解析 并 执行 命令
    Web.send(200, "text/plain; charset=utf-8", message); 
  } else if(url.equalsIgnoreCase("/upload.html")) {    //[上传文件] http://192.168.1.1/upload.html 
    String html="<!DOCTYPE html>";
    html+="<html lang='zh-CN'>";
    html+="<head>";
    html+="  <meta  charset='UTF-8'>";
    html+="  <title>创客与编程</title>";
    html+="</head>";
    html+="<body>";
    html+="  <center>";
    html+="    <h1>机械臂 [上传文件]</h1>";
    html+="    <form method='POST' enctype='multipart/form-data'>";
    html+="      <input type='file' name='data' multiple>";
    html+="      <input class='button' type='submit' value='上传'>&nbsp&nbsp<a href='/index.html'>[回首页]</a>";
    html+="    </form>";
    html+="    <p><span style='color: red;'>index.html</span> &nbsp&nbsp&nbsp&nbsp&nbsp&nbsp 机械臂控制主页网页 [首页]</br>";
    html+="    <span style='color: red;'>HELP.html</span>  &nbsp&nbsp&nbsp&nbsp&nbsp &nbsp 机械臂指令网页 [指令说明]</br>";
    html+="    <span style='color: red;'>config.html</span>  &nbsp&nbsp&nbsp&nbsp&nbsp  机械臂设置网页 [系统设置]</br>";
    html+="    config.json  &nbsp&nbsp&nbsp&nbsp&nbsp  设置文件,由[系统设置]生成</br>";
    html+="    ip.txt  &nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp 连上路由器或移动热点生成</br>";
    html+="    H.txt  &nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp 归位脚本文件,可用 SH 生成</br>";
    html+="    Auto.txt  &nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp 默认脚本文件,可用 S &nbsp&nbsp&nbsp生成</br>";
    html+="    <p><a href='/cmd?dir' target='_blank'>Dir 指令</a>可列出板内所有文件,<span style='color: red;'>红字文件</span>是必要的</br>若缺失用电脑重新上传或UTF-8编码脚本txt文件</br></p>";
    html+="  </center>";
    html+="</body>";
    html+="</html>";
    Web.send(200, "text/html; charset=utf-8", html);
    return;
  }

 //---------------------------读取发送要访问的网页文件 ----------------------
  if(Web.args()>0 || url.indexOf("/",1)>0){
    //GET 192.168.1.1/config?AP_SSID=ESP8266&AP_PSK=12345678&STA_SSID=MYHOME&STA_PSK=12345678&BlinKey=&SIP=&SPort=8000&UPort=8888&XPin=5&YPin=4&ZPin=0&BPin=16&TPin=14&EPin=12&XRaw=90&YRaw=90&ZRaw=90&BRaw=90&TRaw=90&ERaw=90&XMin=0&YMin=0&ZMin=0&BMin=0&TMin=0&EMin=0&XMax=180&YMax=180&ZMax=180&BMax=180&TMax=180&EMax=180&B1P=5&B1G=0&B2P=14&B2G=0&Auto=&

    Web.send(404, "text/html ; charset=utf-8", "404 Not Found 文件不存在<a href='/index.html'>[回首页]</a>");                                 //404 文件不存在
  } else if (SPIFFS.exists(url)) {                // 测试文件是否存在
    File file = SPIFFS.open(url, "r");            // "r"=只读打开文件
    Web.streamFile(file,getType(url));            // 发送文件给浏览器
    file.close();                                 // 关闭文件
    return;
  } else {
    Web.sendHeader("Location", "http://192.168.1.1/index.html");    //访问的文件不存在 可跳转 [上传文件] http://192.168.1.1/upload.html
    Web.send(303);                                 //303跳转
    //Web.send(404, "text/plain ; charset=utf-8", "404 Not Found 访问的文件不存在<a href='/index.html'>[回首页]</a>"); 
  }
  return;
}
//------------------保存一些变量数据参数到配置文件------------------------
//
//
//----------------------------------------------------------------------
bool Config(){                  //保存一些变量中的值到配置文件/config.json
  bool re=false;                //true=重启板子

  for(int i=0;i<Web.args();i++){
    String s=Web.argName(i);
    String v=Web.arg(i);
    if(s.equalsIgnoreCase("AP_SSID"))        { AP_SSID=v;re=true;
    } else if(s.equalsIgnoreCase("AP_PSK"))  {  AP_PSK=v;re=true;
    } else if(s.equalsIgnoreCase("STA_SSID")){STA_SSID=v;re=true;
    } else if(s.equalsIgnoreCase("STA_PSK")) { STA_PSK=v;re=true;
    } else if(s.equalsIgnoreCase("BlinKey")) { BlinKey=v;re=true;
    } else if(s.equalsIgnoreCase("SIP"))     {     SIP=v;re=true;
    } else if(s.equalsIgnoreCase("SPort")) {  SPort=v.toInt();re=true;
    } else if(s.equalsIgnoreCase("UPort")) {  UPort=v.toInt();re=true;

    } else if(s.equalsIgnoreCase("XPin")) {  Raw[0]=v.toInt();re=true;
    } else if(s.equalsIgnoreCase("YPin")) {  Raw[1]=v.toInt();re=true;
    } else if(s.equalsIgnoreCase("ZPin")) {  Raw[2]=v.toInt();re=true;
    } else if(s.equalsIgnoreCase("BPin")) {  Raw[3]=v.toInt();re=true;
    } else if(s.equalsIgnoreCase("TPin")) {  Raw[4]=v.toInt();re=true;
    } else if(s.equalsIgnoreCase("EPin")) {  Raw[5]=v.toInt();re=true;



    } else if(s.equalsIgnoreCase("XRaw")) {  Raw[0]=v.toInt();
    } else if(s.equalsIgnoreCase("YRaw")) {  Raw[1]=v.toInt();
    } else if(s.equalsIgnoreCase("ZRaw")) {  Raw[2]=v.toInt();
    } else if(s.equalsIgnoreCase("BRaw")) {  Raw[3]=v.toInt();
    } else if(s.equalsIgnoreCase("TRaw")) {  Raw[4]=v.toInt();
    } else if(s.equalsIgnoreCase("ERaw")) {  Raw[5]=v.toInt();
    } else if(s.equalsIgnoreCase("XMin")) {  Min[0]=v.toInt();
    } else if(s.equalsIgnoreCase("YMin")) {  Min[1]=v.toInt();
    } else if(s.equalsIgnoreCase("ZMin")) {  Min[2]=v.toInt();
    } else if(s.equalsIgnoreCase("BMin")) {  Min[3]=v.toInt();
    } else if(s.equalsIgnoreCase("TMin")) {  Min[4]=v.toInt();
    } else if(s.equalsIgnoreCase("EMin")) {  Min[5]=v.toInt();
    } else if(s.equalsIgnoreCase("XMax")) {  Max[0]=v.toInt();
    } else if(s.equalsIgnoreCase("YMax")) {  Max[1]=v.toInt();
    } else if(s.equalsIgnoreCase("ZMax")) {  Max[2]=v.toInt();
    } else if(s.equalsIgnoreCase("BMax")) {  Max[3]=v.toInt();
    } else if(s.equalsIgnoreCase("TMax")) {  Max[4]=v.toInt();
    } else if(s.equalsIgnoreCase("EMax")) {  Max[5]=v.toInt();

    } else if(s.equalsIgnoreCase("B1P"))   { B1P=v.toInt();re=true;
    } else if(s.equalsIgnoreCase("B1G"))   { B1G=v.toInt();re=true;
    } else if(s.equalsIgnoreCase("B1R"))   { B1R=v;
    } else if(s.equalsIgnoreCase("B2P"))   { B2P=v.toInt();re=true;
    } else if(s.equalsIgnoreCase("B2G"))   { B2G=v.toInt();re=true;
    } else if(s.equalsIgnoreCase("B2R"))   { B2R=v;
    } else if(s.equalsIgnoreCase("Auto"))  { Autorun=v;    re=true;
    }
  }

  DynamicJsonDocument  doc(2048);           //堆内存JSON文档对象
  doc["AP_SSID"]  = AP_SSID;     //控制板要创建的WIFI热点
  doc["AP_PSK"]   = AP_PSK;      //控制板要创建的WIFI密码
  doc["STA_SSID"] = STA_SSID;    //控制板要连接的路由器热点
  doc["STA_PSK"]  = STA_PSK;     //控制板要连接的路由器密码
  doc["BlinKey"]  = BlinKey;     //物连网 点灯APP 创建的独立设备密钥

  doc["SIP"]   =     SIP;
  doc["SPort"] =   SPort;
  doc["UPort"] =   UPort;

  doc["ss"]    =      6;//ss;
     
  doc["Servo"][0]   = XYZE[0];           //舵机编号
  doc["Servo"][1]   = XYZE[1];           //舵机编号
  doc["Servo"][2]   = XYZE[2];           //舵机编号
  doc["Servo"][3]   = XYZE[3];           //舵机编号
  doc["Servo"][4]   = XYZE[4];           //舵机编号
  doc["Servo"][5]   = XYZE[5];           //舵机编号
  doc["Pin"][0]     = Pin[0];                    //舵机GPIO
  doc["Pin"][1]     = Pin[1];                    //舵机GPIO
  doc["Pin"][2]     = Pin[2];                    //舵机GPIO
  doc["Pin"][3]     = Pin[3];                    //舵机GPIO
  doc["Pin"][4]     = Pin[4];                    //舵机GPIO
  doc["Pin"][5]     = Pin[5];                    //舵机GPIO

  doc["Raw"][0]=Raw[0];  
  doc["Raw"][1]=Raw[1];
  doc["Raw"][2]=Raw[2]; 
  doc["Raw"][3]=Raw[3];
  doc["Raw"][4]=Raw[4]; 
  doc["Raw"][5]=Raw[5];

  doc["Min"][0]=Min[0]; 
  doc["Min"][1]=Min[1];
  doc["Min"][2]=Min[2]; 
  doc["Min"][3]=Min[3];  
  doc["Min"][4]=Min[4];   
  doc["Min"][5]=Min[5];  

  doc["Max"][0]=Max[0]; 
  doc["Max"][1]=Max[1];
  doc["Max"][2]=Max[2];
  doc["Max"][3]=Max[3];
  doc["Max"][4]=Max[4];
  doc["Max"][5]=Max[5];

  doc["B1"][0] =  B1P;
  doc["B1"][1] =  B1G;
  doc["B2"][0] =  B2P;
  doc["B2"][1]  = B2G;
  doc["B1R"]   =  B1R;
  doc["B2R"]   =  B2R;
  doc["Auto"]  = Autorun;                       //板子通电自动运行Auto.txt次数  
  doc["null"]  ="null";

  File F = SPIFFS.open("/config.json", "w");    //创建重写文件
  serializeJson(doc, F);                        //输出JSON格式内容到文件
  serializeJson(doc, Serial);                   //输出JSON格式内容到串口
  F.flush();
  F.close();                                    //关闭文件
  doc.clear();
  return re;
}
//----------------------------处理文件上传函数,支持同时上传多个文件---------------------
void handleFileUpload(){                             
  static File F;                               // 静态变量 文件对象用于文件上传

  HTTPUpload& UP = Web.upload();               // 上传对象
  if(UP.status == UPLOAD_FILE_START){          // 如果 状态=UPLOAD_FILE_START 开始上传
    String filename = UP.filename;             // 上传的文件名
    if(!filename.startsWith("/"))
       filename = "/" + filename;              // 为文件名前加上"/"
    F = SPIFFS.open(filename, "w");            // 用SPIFFS建立重写文件写入用户上传的文件数据
  } 
  if(UP.status == UPLOAD_FILE_WRITE){          // 如果上传状态为 UPLOAD_FILE_WRITE    
    if(F) 
       F.write(UP.buf,UP.currentSize);         // SPIFFS文件写入浏览器发来的文件数据 2048
  } 
  if(UP.status == UPLOAD_FILE_END){            // 如果上传状态为UPLOAD_FILE_END 
      F.close();                               // 关闭文件,完成上传
  }
  if(UP.status == UPLOAD_FILE_ABORTED){        // 中断或取消上传
      F.close();
  }
}
//---------------------上传文件完成,返回页处理------------------------------------
void FileUpload_OK(){
      HTTPUpload& UP = Web.upload();                                  //上传对象
      String html="<!DOCTYPE html>";
      html+="<html lang='zh-CN'>";
      html+="<head>";
      html+="  <meta  charset='UTF-8'>";
      html+="  <title>创客与编程</title>";
      html+="</head>";
      html+="<body>";
      html+="  <center>";                                   
      html+="    <h1>上传文件成功.总共: " + String(UP.currentSize) + " 字节</h1>";
      html+="    <a href='/index.html'>[首页]</a>";
      html+="    <a href='/upload.html'>[继续上传]</a>";
      html+="  </center>";
      html+="</body>";
      html+="</html>";
      Web.send(200, "text/html", html);
}
//-------------------------HTTP 文件类型 标头信息 utf-8 -----------------------------
String getType(String filename){                  
  if(filename.endsWith(".htm"))       return "text/html;charset=utf-8";
  else if(filename.endsWith(".html")) return "text/html;charset=utf-8";
  else if(filename.endsWith(".css"))  return "text/css";
  else if(filename.endsWith(".js"))   return "application/javascript;charset=utf-8";
  else if(filename.endsWith(".png"))  return "image/png";
  else if(filename.endsWith(".gif"))  return "image/gif";
  else if(filename.endsWith(".jpg"))  return "image/jpeg";
  else if(filename.endsWith(".ico"))  return "image/x-icon";
  else if(filename.endsWith(".xml"))  return "text/xml;charset=utf-8";
  else if(filename.endsWith(".pdf"))  return "application/x-pdf";
  else if(filename.endsWith(".zip"))  return "application/x-zip";
  else if(filename.endsWith(".gz"))   return "application/x-gzip";
  else if(filename.endsWith(".txt"))  return "text/plain;charset=UTF-8";
  else if(filename.endsWith(".json")) return "text/plain;charset=UTF-8";
  return "text/plain;charset=utf-8";
} 
/*
Arduino ESP32 中与 TimeInfo 结构体相关的函数主要取决于你如何获取和处理时间信息。以下是一些常见的函数和方法：
#include <time.h>: 包含 time.h 库，提供时间和日期相关的函数和结构。
1. 获取当前时间: sizeof
RTC_DATA_ATTR:               用于声明一个全局变量，并在程序重启后保留其值。你可以使用它来存储 TimeInfo 结构体，并在 setup() 函数中初始化它。
RTC_NOINIT:                  用于声明一个全局变量，并在程序重启后不保留其值。你可以使用它来存储 TimeInfo 结构体，并在程序运行时获取当前时间并更新它。
esp_timer_create():          用于创建定时器，可以用来定期更新 TimeInfo 结构体。
esp_timer_start_periodic():  用于启动定时器，并设置定时器的时间间隔。
esp_timer_stop():            用于停止定时器。
esp_timer_delete():          用于删除定时器。

2. 处理时间信息:
getLocalTime():              用于获取 ESP32 的当前时间，并将其存储在 struct tm 结构体中。函数将时间戳转换为本地时间，需要设置 ESP32 的时区和夏令时。
mktime():                    用于将 struct tm 结构体转换为时间戳。将 tm 结构体转换为自 1970 年 1 月 1 日 00:00:00 UTC 起的秒数。
strftime():                  用于将时间戳格式化为字符串。
time():                      用于获取当前时间戳。需要 time.h 库，该库在 Arduino 库中包含。函数需要 ESP32 的 RTC 模块正常工作。
difftime():                  用于计算两个时间戳之间的差值。
localtime()                  函数将时间戳转换为本地时间结构。

3. 设置时间:
setenv():                    用于设置环境变量，可以用来存储时间信息。
rtc_time_t:                  用于存储时间信息，可以用来设置 ESP32 的 RTC 模块。
rtc_get_time():              用于获取 ESP32 的 RTC 模块的时间信息。
rtc_set_time():              用于设置 ESP32 的 RTC 模块的时间信息。

示例代码:
#include <Arduino.h>  

RTC_DATA_ATTR TimeInfo currentTime;  
void setup() {  
  Serial.begin(115200);  
  // 初始化 RTC 模块  
  // ...  
  // 获取当前时间  
  struct tm timeinfo;  
  getLocalTime(&timeinfo);  

  // 将当前时间存储到 currentTime 结构体中  
  currentTime.year = timeinfo.tm_year + 1900;  
  currentTime.month = timeinfo.tm_mon + 1;  
  currentTime.day = timeinfo.tm_mday;  
  currentTime.hour = timeinfo.tm_hour;  
  currentTime.minute = timeinfo.tm_min;  
  currentTime.second = timeinfo.tm_sec;  

  // 打印当前时间  
  Serial.print("Current time: ");  
  Serial.print(currentTime.year);  
  Serial.print("-");  
  Serial.print(currentTime.month);  
  Serial.print("-");  
  Serial.print(currentTime.day);  
  Serial.print(" ");  
  Serial.print(currentTime.hour);  
  Serial.print(":");  
  Serial.print(currentTime.minute);  
  Serial.print(":");  
  Serial.println(currentTime.second);  
}  

void loop() {  
// 获取当前时间戳（以秒为单位）  
  unsigned long current_time = time(nullptr);  

  // 将时间戳转换为人类可读的格式  
  char time_string[20];  
  strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", localtime(&current_time));  

  // 打印时间  
  Serial.println(time_string);  

  delay(1000);  

       //time_t a=time(nullptr);
       //Serial.println(a);     

       //getLocalTime(&T,1000);         //超时
       //T.tm_sec++;
       //time_t b=mktime(&T);
       //Serial.println(b);    //time_t timestamp =
  
}  
*/
