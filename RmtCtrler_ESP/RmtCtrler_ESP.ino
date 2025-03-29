#include <Preferences.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "RemoSystemYX.h"
#include "up70logo.h"
#include "up70name.h"
#include "HardwareSerial.h"        //调用串口库
#include "UartTransmitYX.h"

Preferences pref;

#define RBut_value my_joystick.R_But
#define LBut_value my_joystick.L_But
#define RX_value my_joystick.R_X
#define RY_value my_joystick.R_Y
#define LX_value my_joystick.L_X
#define LY_value my_joystick.L_Y

// #define R_Button 22
// #define R_Vy 32
// #define R_Vx 33

#define LCD_PWM 33
#define LCD_PWM_CHANNEL 0

#define BAT_CHECK_PIN 5
#define VBAT_CHECK_PIN 34
#define VBUS_CHECK_PIN 35
// #define L_Vx 34
// #define L_Vy 35
// #define L_Button 19
// #define JOYSTICK_DEBUG
#define LED_1 19                //设置RX管脚
#define LED_2 18                //设置TX管脚

#define STM_RxPin 16                //设置RX管脚
#define STM_TxPin 17                //设置TX管脚
HardwareSerial Serial_STM(2);       //定向串口2
uint32_t Running_Time = 0;

uint8_t Calib_sendbuf[4];
uint16_t Calib_Infos[16];

QueueHandle_t xQueue1;      // 消息队列

int Byte_InQueue = 0;
int BatVoltCode = 0;
float BatVolt = 0;
float VBUSVolt = 0;

float bat_volt_print = 0;   // 电池电压
int bat_integral_onesec = 0;
int bat_integral_twosec = 0;

int width = 0;
int height = 0;

int Uart_Count = 0;

bool Clear_Flag = 0;
int fps_counter = 0;
int fps_print_flag = 0;
int fps_for_print = 0;
int fps_for_print_preserve = 0;


TaskHandle_t* menu_mission;
TaskHandle_t* anim_mission;
TaskHandle_t* response_mission;
TaskHandle_t* rmt_mission;
TaskHandle_t* rmtRs_mission;

ElementYX my_But = ElementYX();
/**
 * @details 新增 纯遥控模式
 */
void setup() {
  // put your setup code here, to run once:
  tft.init();
  Serial.begin(115200);

  ledcSetup(LCD_PWM_CHANNEL, 1000, 10);
  ledcWriteTone(LCD_PWM_CHANNEL, 1000);
  ledcAttachPin(LCD_PWM, LCD_PWM_CHANNEL);
  ledcWrite(LCD_PWM_CHANNEL, 0);

  tft.setRotation(1);

  /******   打开命名空间，以只读模式  ******/
  pref.begin("JsInfos", true);

  // 读取数组：摇杆校准原点
  for (int i = 0; i < 16; i++) {
    char key[8];
    sprintf(key, "jsori%d", i);
    Calib_Infos[i] = pref.getUShort(key, 0);
    // Serial.printf("Name:%s, Value:%d\n", key, Calib_Infos[i]);
    delay(10);
  }

  Left_JoyStick_4_Divisions[Joys_4Div_Right] = Calib_Infos[0];
  Left_JoyStick_4_Divisions[Joys_4Div_Left] = Calib_Infos[1];
  Left_JoyStick_4_Divisions[Joys_4Div_Down] = Calib_Infos[2];
  Left_JoyStick_4_Divisions[Joys_4Div_Up] = Calib_Infos[3];

  Right_JoyStick_4_Divisions[RJoys_4Div_Right] = Calib_Infos[4];
  Right_JoyStick_4_Divisions[RJoys_4Div_Down] = Calib_Infos[6];
  Right_JoyStick_4_Divisions[RJoys_4Div_Up] = Calib_Infos[7];

  Left_JoyStick_X_Origin = Calib_Infos[8];
  Left_JoyStick_Y_Origin = Calib_Infos[9];
  Right_JoyStick_X_Origin = Calib_Infos[10];
  Right_JoyStick_Y_Origin = Calib_Infos[11];

  pref.end();
  /******       关闭命名空间      ******/
  
  delay(1000);
  /**********   发送摇杆数据    ************************/
  Calib_sendbuf[0] = 1;
  for (int i = 0; i < 12; i++)
  {
    // 一般是左大右小
    Calib_sendbuf[1] = i; // 顺序: LX_L_H, LY_L_H, RX_L_H, RY_L_H => LL_R, LD_U, RL_R, RD_U, LX_M, LY_M, RX_M, RY_M
    Calib_sendbuf[2] = Calib_Infos[i] >> 8;
    Calib_sendbuf[3] = Calib_Infos[i] & 0xff;
    Serial_STM.write(Calib_sendbuf, 4);
    delay(10);
  }


  
  tft.fillScreen(TFT_BLACK);
  delay(100);
  for (int i = 0; i < 256; i++)
  {
    ledcWrite(LCD_PWM_CHANNEL, i);
    delay(3);
  }
  
  BatVolt = ((analogRead(VBAT_CHECK_PIN) / 4096.0) * 3.3) * 2.222 + 0.2967;
  VBUSVolt = ((analogRead(VBUS_CHECK_PIN) / 4096.0) * 3.3);

  /******   显示战队的LOGO    *******/
  tft.pushImage(130, 150, 220, 20, gImage_up70name, 0, nullptr);
  delay(2000);
  tft.fillScreen(TFT_BLACK);
  tft.pushImage(150, 123, 180, 74, gImage_up70logo, 0, nullptr);


  /******   初始化    *******/
  Serial_STM.begin(115200, SERIAL_8N1, STM_RxPin, STM_TxPin);  //初始化串口2，初始化参数可以去HardwareSerial.h文件中查看
  Serial_STM.onReceive(UartRX_Callback);    //定义串口中断函数
  
  delay(2500);    // 等待 STM32 发送消息

  // Serial.printf("%s\n", stm_pure);
  if(strcmp(stm_pure, "#debug#") == 0)
  {
    Pure_Control_Mode = false;
    Serial.println("DebugMode");
  }
  else
  {
    Serial.println("ControlMode");
  }

  if (Pure_Control_Mode)    // 如果什么都没收到，就是纯净遥控模式
  {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(7);
    tft.drawChar('3', 200, 120);
    delay(1000);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(7);
    tft.drawChar('2', 200, 120);
    delay(1000);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(7);
    tft.drawChar('1', 200, 120);
    delay(1000);

    // 发送状态号:m-212代表进入普通无动作模式
    Serial_STM.printf("m212");
    delay(100);
    tft.fillScreen(TFT_BLACK);

    xTaskCreate(RemoteControlOnly, "RmtCOnly", 4096, NULL, 0, rmt_mission);
    xTaskCreate(RemoteControlOnly_Responser, "RmtCOnlyR", 2048, NULL, 0, rmtRs_mission);
  }
  else    // 否则就是，调试模式
  {
    // 发送状态号:m-212代表进入普通无动作模式
    Serial_STM.printf("m212");
    delay(100);

    tft.fillScreen(TFT_BLACK);
    Init_SysYX();
      
    pinMode(BAT_CHECK_PIN, OUTPUT);
    digitalWrite(BAT_CHECK_PIN, HIGH);
    pinMode(VBUS_CHECK_PIN, INPUT);
    pinMode(LED_1, OUTPUT);
    digitalWrite(LED_1, HIGH);
    delay(250);

    xTaskCreate(Screen_Handler, "ScreenHandler", 8192, NULL, 0, NULL);
    xTaskCreate(Fps_Calc, "FpsCalc", 1024, NULL, 0, NULL);
    xTaskCreate(Joystick_Watcher, "Joys_Wt", 2048, NULL, 0, NULL);
    delay(250);
  }
}


bool CanResp_Flag = true;

bool user_actions[16];
void loop()
{
  if (Pure_Control_Mode)    // 纯净遥控模式
  {
  }
  else
  {
    estimate_userInput(user_actions, &my_joystick);

    // 如果处在菜单页面
    if(CanResp_Flag && PageYX_Now->page_type == MenuPage)
    {
      CanResp_Flag = false;
      xTaskCreate(Menu_Responser, "Menu_Responser", 8192, NULL, 0, anim_mission);
    }
  
  
    // 如果处在自定义页面，根据页面号来响应
    if(CanResp_Flag && PageYX_Now->page_type == CustomPage)
    {
      CanResp_Flag = false;
      switch (PageYX_Now->Page_id)
      {
        case 3: {xTaskCreate(Calib_Responser, "Calib_Resp", 2048, NULL, 0, anim_mission);break;}
        case 4: {xTaskCreate(RmtCtrl_Responser, "Rmt_Resp", 2048, NULL, 0, anim_mission);break;}
        case 5: {xTaskCreate(Connect_Responser, "Cnct_Resp", 2048, NULL, 0, anim_mission);break;}
      }
    }
  }
  vTaskDelay(5);
}



void UartRX_Callback()
{
  uint8_t rxBuffs[16];
  int i = 0;

  while(Serial_STM.available()){                 //用While判断缓冲区是否有内容
    rxBuffs[i] = Serial_STM.read();              //取出缓冲区内容
    i++;
  }
  Decode_Uart_Msg(rxBuffs);
  Uart_Count++;
}





void Joystick_Watcher(void* Param)
{
  while(1)
  {
    // Serial.printf("Jy_Data:%d %d %d %d %d %d %d\n", my_joystick.L_X, my_joystick.L_Y, my_joystick.R_X, my_joystick.R_Y, my_joystick.L_But, my_joystick.R_But, Uart_Count);
    // Serial.printf("R_x:%d, R_y:%d\n", my_joystick.R_X, my_joystick.R_Y);
    // Serial.printf("BatV:%.2f\n", ((analogRead(VBAT_CHECK_PIN) / 4096.0) * 3.3) * 2.222 + 0.2967);
    // Serial.printf("BatV:%.2f\n", ((analogRead(VBAT_CHECK_PIN) / 4096.0) * 3.3));
    // Serial.printf("BatV:%d\n", bat_integral_onesec);
    // for (int i = 0; i < 4; i++)
    // {
    //   if (FindNodes[i].addr != 0)
    //   {
    //     // Serial.printf("Ind:%d,Addr:%#x\n", i, FindNodes[i].addr);
    //   }
    // }

    Running_Time++;

    Serial.printf("Runned:%d sec\n", Running_Time);
    // Serial.printf("batv:%f\n", analogRead(VBUS_CHECK_PIN) / 4096.0) * 3.3 * 4;
    // Serial.printf("state:%s\n", stm_real_state);
    
    // #ifdef JOYSTICK_DEBUG
    // Serial.printf("Jy,x:%d,y:%d\n", my_joystick.L_X, my_joystick.L_Y);
    // #endif
    vTaskDelay(2000);
  }
}



void Screen_Handler(void* Param)
{
  
  while(1)
  {
    Page_Handler();
    fps_counter++;

    vTaskDelay(max((55 - DynamicFpsSetting), 5.0f));
  }
}


/// @brief 处在 纯净遥控 页面时，运行的主线程
/// @param Param 
void RemoteControlOnly(void* Param)
{
  while(1)
  {
    PureRmtCtrl_Refresh();
    vTaskDelay(20);
  }
}



/// @brief 处在 纯净遥控 页面时，运行的副线程
/// @param Param 
void RemoteControlOnly_Responser(void* Param)
{
  static int time_watcher = 0;

  while (1)
  {
    /****   时间记录   ****/
    if(time_watcher == 0)  time_watcher = millis();
    if(millis() - time_watcher > 500)
    {
      // 发送状态号:0-101代表进入遥控模式
      Serial_STM.printf("m101");
      time_watcher = millis();
    }

    vTaskDelay(50);
  }
}




void Fps_Calc(void* Param)
{
  int fps_calc_count_flag = 0;
  // int bat_inte_count_flag = 0;
  float BatVoltTemp = 0;
  while(1)
  {
    VBUSVolt = ((analogRead(VBUS_CHECK_PIN) / 4096.0) * 3.3);   // 测量充电口电压

    // BatVoltTemp = ((analogRead(VBAT_CHECK_PIN) / 4096.0) * 3.3) * 2.429 - 0.295714;
    BatVoltTemp = ((analogRead(VBAT_CHECK_PIN) / 4096.0) * 3.3) * 2.222 + 0.2967;
    // bat_inte_count_flag++;

    
    if(VBUSVolt > 1.65)   // 充电口电压大于3v3 / 2，认为在充电
    {
      ledcWrite(LCD_PWM_CHANNEL, 768);
      IsCharging = 1;
    }
    else   // 认为停止充电
    {
      ledcWrite(LCD_PWM_CHANNEL, 256);
      IsCharging = 0;
    }
    

    BatVolt = BatVoltTemp * 0.5 + BatVolt * 0.5;
    fps_calc_count_flag++;

    if(fps_calc_count_flag > 10)
    {
      fps_print_flag = 1;
      fps_for_print = fps_counter;
      fps_counter = 0;
      fps_calc_count_flag = 0;
      // Uart_Count = 0;
    }
    vTaskDelay(50);
  }
}









void Menu_Responser(void* Param)
{
  static int menu_time_watcher = 0;

  if (PageYX_Now->page_type != MenuPage)     // 如果不是菜单类页面，直接不干了
  {
    vTaskDelete(NULL);    // 直接结束本进程
  }

  /***********      如果正在往上拨动  右摇杆      ***********/
  if (user_actions[RJs_Up])
  {
    if (!button_focoused->IsExPanel())  // 如果不是一个打开的面板
    {
      button_focoused = Menu_SlideUpButton(button_focoused);    // 这个函数是会移动button_focused的哦！
    }
  }
  
  /***********      如果正在往下拨动  右摇杆      ***********/
  if (user_actions[RJs_Down])
  {
    if (!button_focoused->IsExPanel())  // 如果不是一个打开的面板
    {
      button_focoused = Menu_SlideDownButton(button_focoused);    // 这个函数是会移动button_focused的哦！
    }
  }

  /***********      如果正在往左拨动  右摇杆      ***********/
  if (user_actions[RJs_Left])
  {
  }

  /***********      如果正在往右拨动  右摇杆      ***********/
  if (user_actions[RJs_Right])
  {
  }

  /***********      如果正在按住拨动  右摇杆      ***********/
  if (user_actions[RJs_Pressed])
  {
      Menu_PressingButton(button_focoused);
  }
  else    /***********      如果正在松开了  右摇杆      ***********/
  {
    if(!button_focoused->IsExPanel()) button_focoused = Menu_ReleaseButtonOpen(button_focoused);
    else Menu_ReleaseButtonClose(button_focoused);
  }

  /*************************************************************************/

  /***********      如果正在往上拨动  左摇杆      ***********/
  if (user_actions[LJs_Up])
  {
    if (button_focoused->panel_expanded)  // 如果是一个打开的面板
    {
      Menu_DragUpButton(button_focoused);    // 这个函数是会移动button_focused的哦！
    }
  }
  
  /***********      如果正在往下拨动  左摇杆      ***********/
  if (user_actions[LJs_Down])
  {
    if (button_focoused->panel_expanded)  // 如果是一个打开的面板
    {
      Menu_DragDownButton(button_focoused);    // 这个函数是会移动button_focused的哦！
    }
  }

  /***********      如果正在往左拨动  左摇杆      ***********/
  if (user_actions[LJs_Left])
  {
    if (button_focoused->panel_expanded)  // 如果是一个打开的面板
    {
      Menu_DragLeftButton(button_focoused);    // 这个函数是会移动button_focused的哦！
    }
  }

  /***********      如果正在往右拨动  左摇杆      ***********/
  if (user_actions[LJs_Right])
  {
  }

  /***********      如果正在按住拨动  左摇杆      ***********/
  if (user_actions[LJs_Pressed])
  {
  }
  else    /***********      如果正在松开了  左摇杆      ***********/
  {
  }


  /*        自定义响应        */
  if (Menu_IsDragButtonOK(button_focoused))
  {
    button_focoused = Menu_DragButtonClose(button_focoused);
  }

  if (!Menu_DragedButtonThisTime(button_focoused))
  {
    Menu_ClearDragButton(button_focoused);
  }


  /*  与STM32同步 */
  if (menu_time_watcher == 0)
  {
    menu_time_watcher = millis();
  }
  if(millis() - menu_time_watcher > 500)
  {
    // 发送状态号:m-212代表进入普通无动作模式
    Serial_STM.printf("m212");
    menu_time_watcher = millis();
  }

  vTaskDelay(15);
  CanResp_Flag = true;
  vTaskDelete(NULL);    // 直接结束本进程
}




/// @brief 处在CALIBRATION页面时，对输入的响应
/// @param Param 


void Calib_Responser(void* Param)
{
  static int read_origin_times = 0;
  static int time_ticks = 0;
  static int temp_X;
  static int temp_Y;
  static int temp_MaxX;
  static int temp_MinX = 9999;
  static int temp_MaxY;
  static int temp_MinY = 9999;

  static bool READOK_flag;    // 读取是否完成的标志

  static bool start_calibration_flag;     // 矫正是否开始的标志

  Calib_sendbuf[0] = 1;
  for (int i = 0; i < 12; i++)
  {
    // 一般是左大右小
    Calib_sendbuf[1] = i; // 顺序: LX_L_H, LY_L_H, RX_L_H, RY_L_H => LL_R, LD_U, RL_R, RD_U, LX_M, LY_M, RX_M, RY_M
    Calib_sendbuf[2] = Calib_Infos[i] >> 8;
    Calib_sendbuf[3] = Calib_Infos[i] & 0xff;
    Serial_STM.write(Calib_sendbuf, 4);
    delay(10);
  }


  /***********      如果正在按住  右摇杆      ***********/
  if (user_actions[RJs_Pressed])
  {
    PageYX_Now->pageReady = false;
    PageYX_Now->page_state_ID = 0;
    read_origin_times = 0;
    time_ticks = 0;
    temp_X = 0;
    temp_Y = 0;
    temp_MaxX = 0;
    temp_MinX = 9999;
    temp_MaxY = 0;
    temp_MinY = 9999;
    READOK_flag = 0;
    button_focoused = To_MainMenu();
    vTaskDelay(500);    // 所有的切换页面至少Delay0.5s

    CanResp_Flag = true;
    vTaskDelete(NULL);
  }

  switch (PageYX_Now->page_state_ID)
  {
    case 0:   // 处在0页面时
    {
      if (time_ticks == 0)
      {
        time_ticks = millis();
      }
      
      if (millis() - time_ticks < 2500)
      {
        read_origin_times++;
        temp_X += my_joystick.L_X;
        temp_Y += my_joystick.L_Y;
        temp_MaxX = max((int)my_joystick.L_X, temp_MaxX);
        temp_MinX = min((int)my_joystick.L_X, temp_MinX);
        temp_MaxY = max((int)my_joystick.L_Y, temp_MaxY);
        temp_MinY = min((int)my_joystick.L_Y, temp_MinY);
      }
      else if (READOK_flag == 0)
      {
        Left_JoyStick_X_Origin = temp_X * 1.0 / read_origin_times * 1.0;
        Left_JoyStick_Y_Origin = temp_Y * 1.0 / read_origin_times * 1.0;
        Serial.printf("%d, %d, %d, %d\n", temp_MaxX, temp_MinX, temp_MaxY, temp_MinY);
        
        Serial.printf("%d, %d\n", Left_JoyStick_X_Origin, Left_JoyStick_Y_Origin);
        Serial.printf("wave: %d, %d\n", temp_MaxX - temp_MinX, temp_MaxY - temp_MinY);
        time_ticks = 0;

        // 清零各个量，右摇杆可能还要用
        read_origin_times = 0;
        temp_X = 0;
        temp_Y = 0;
        temp_MaxX = 0;
        temp_MinX = 9999;
        temp_MaxY = 0;
        temp_MinY = 9999;

        READOK_flag = 1;
      }
      break;
    }

    case 2:   // 处在3 ： 左摇杆校准    页面时
    {
      /*      以300个Counting作为采集好了的标志    */
      if (my_joystick.L_X < Left_JoyStick_X_Origin * 0.7 && JoyStickPushingCounting[Joys_4Div_Right] < 300)
      {
        JoyStickPushingCounting[Joys_4Div_Right]++;   // 右
        if (JoyStickPushingCounting[Joys_4Div_Right] > 20)    // 丢弃掉前20个数据
        {
          Left_JoyStick_4_Divisions[Joys_4Div_Right] += my_joystick.L_X;
        }
      }
      if (my_joystick.L_X > Left_JoyStick_X_Origin * 1.3 && JoyStickPushingCounting[Joys_4Div_Left] < 300)
      {
        JoyStickPushingCounting[Joys_4Div_Left]++;   // 左
        if (JoyStickPushingCounting[Joys_4Div_Left] > 20)
        {
          Left_JoyStick_4_Divisions[Joys_4Div_Left] += my_joystick.L_X;
        }
      }
      if (my_joystick.L_Y < Left_JoyStick_Y_Origin * 0.7 && JoyStickPushingCounting[Joys_4Div_Down] < 300)
      {
        JoyStickPushingCounting[Joys_4Div_Down]++;   // 下
        if (JoyStickPushingCounting[Joys_4Div_Down] > 20)
        {
          Left_JoyStick_4_Divisions[Joys_4Div_Down] += my_joystick.L_Y;
        }
      }
      if (my_joystick.L_Y > Left_JoyStick_Y_Origin * 1.3 && JoyStickPushingCounting[Joys_4Div_Up] < 300)
      {
        JoyStickPushingCounting[Joys_4Div_Up]++;   // 上
        if (JoyStickPushingCounting[Joys_4Div_Up] > 20)
        {
          Left_JoyStick_4_Divisions[Joys_4Div_Up] += my_joystick.L_Y;
        }
      }



      // 如果中途松开了，就不算，重来
      if (my_joystick.L_X > Left_JoyStick_X_Origin * 0.7 && JoyStickPushingCounting[Joys_4Div_Right] < 300) 
      {JoyStickPushingCounting[Joys_4Div_Right] = 0;Left_JoyStick_4_Divisions[Joys_4Div_Right] = 0;}

      if (my_joystick.L_X < Left_JoyStick_X_Origin * 1.3 && JoyStickPushingCounting[Joys_4Div_Left] < 300)
      {JoyStickPushingCounting[Joys_4Div_Left] = 0;Left_JoyStick_4_Divisions[Joys_4Div_Left] = 0;}

      if (my_joystick.L_Y < Left_JoyStick_Y_Origin * 1.3 && JoyStickPushingCounting[Joys_4Div_Up] < 300)
      {JoyStickPushingCounting[Joys_4Div_Up] = 0;Left_JoyStick_4_Divisions[Joys_4Div_Up] = 0;}

      if (my_joystick.L_Y > Left_JoyStick_Y_Origin * 0.7 && JoyStickPushingCounting[Joys_4Div_Down] < 300)
      {JoyStickPushingCounting[Joys_4Div_Down] = 0;Left_JoyStick_4_Divisions[Joys_4Div_Down] = 0;}



      
      if (JoyStickPushingCounting[Joys_4Div_Up] == 300)     // 完事了给他++一下，避免再次触发
      {
        JoyStickPushingCounting[Joys_4Div_Up]++;
        Left_JoyStick_4_Divisions[Joys_4Div_Up] = Left_JoyStick_4_Divisions[Joys_4Div_Up] / 280.0;
        Serial.printf("LUP均值:%d\n", Left_JoyStick_4_Divisions[Joys_4Div_Up]);
      }
      if (JoyStickPushingCounting[Joys_4Div_Down] == 300)     // 完事了给他++一下，避免再次触发
      {
        JoyStickPushingCounting[Joys_4Div_Down]++;
        Left_JoyStick_4_Divisions[Joys_4Div_Down] = Left_JoyStick_4_Divisions[Joys_4Div_Down] / 280.0;
        Serial.printf("LDOWN均值:%d\n", Left_JoyStick_4_Divisions[Joys_4Div_Down]);
      }
      if (JoyStickPushingCounting[Joys_4Div_Right] == 300)     // 完事了给他++一下，避免再次触发
      {
        JoyStickPushingCounting[Joys_4Div_Right]++;
        Left_JoyStick_4_Divisions[Joys_4Div_Right] = Left_JoyStick_4_Divisions[Joys_4Div_Right] / 280.0;
        Serial.printf("LRIGHT均值:%d\n", Left_JoyStick_4_Divisions[Joys_4Div_Right]);
      }
      if (JoyStickPushingCounting[Joys_4Div_Left] == 300)     // 完事了给他++一下，避免再次触发
      {
        JoyStickPushingCounting[Joys_4Div_Left]++;
        Left_JoyStick_4_Divisions[Joys_4Div_Left] = Left_JoyStick_4_Divisions[Joys_4Div_Left] / 280.0;
        Serial.printf("LLEFT均值:%d\n", Left_JoyStick_4_Divisions[Joys_4Div_Left]);
      }
      break;
    }

    case 4:   // 处在4页面时，清空标志位
    {
      for (int i = 0; i < 4; i++)
      {
        JoyStickPushingCounting[i] = 0;
      }
      time_ticks = 0;
      READOK_flag = 0;

        // 清零各个量，右摇杆可能还要用
        read_origin_times = 0;
        temp_X = 0;
        temp_Y = 0;
        temp_MaxX = 0;
        temp_MinX = 9999;
        temp_MaxY = 0;
        temp_MinY = 9999;
      
      break;
    }

    case 5:   // 处在5页面时，捕捉右摇杆静态位置
    {
      if (time_ticks == 0)
      {
        time_ticks = millis();
      }
      
      if (millis() - time_ticks < 2500)
      {
        read_origin_times++;
        temp_X += my_joystick.R_X;
        temp_Y += my_joystick.R_Y;
        if (my_joystick.R_X > temp_MaxX)
        {
          // Serial.printf("%d, %d, %d, %d\n", temp_MaxX, temp_MinX, temp_MaxY, temp_MinY);
        }
        
        temp_MaxX = max((int)my_joystick.R_X, temp_MaxX);
        temp_MinX = min((int)my_joystick.R_X, temp_MinX);
        temp_MaxY = max((int)my_joystick.R_Y, temp_MaxY);
        temp_MinY = min((int)my_joystick.R_Y, temp_MinY);
      }
      else if (READOK_flag == 0)
      {
        Right_JoyStick_X_Origin = temp_X * 1.0 / read_origin_times * 1.0;
        Right_JoyStick_Y_Origin = temp_Y * 1.0 / read_origin_times * 1.0;
        // Serial.printf("%d, %d, %d, %d\n", temp_MaxX, temp_MinX, temp_MaxY, temp_MinY);

        Serial.printf("R:%d, %d\n", Right_JoyStick_X_Origin, Right_JoyStick_Y_Origin);
        Serial.printf("wave: %d, %d\n", temp_MaxX - temp_MinX, temp_MaxY - temp_MinY);

        time_ticks = 0;
        READOK_flag = 1;
      }
      break;
    }

    case 7:   // 处在7 ： 右摇杆校准    页面时
    {
      /*      以300个Counting作为采集好了的标志    */
      if (my_joystick.R_X < Left_JoyStick_X_Origin * 0.7 && JoyStickPushingCounting[RJoys_4Div_Right] < 300)
      {
        JoyStickPushingCounting[RJoys_4Div_Right]++;   // 右
        if (JoyStickPushingCounting[RJoys_4Div_Right] > 20)    // 丢弃掉前20个数据
        {
          Right_JoyStick_4_Divisions[RJoys_4Div_Right] += my_joystick.R_X;
        }
      }
      if (my_joystick.R_X > Left_JoyStick_X_Origin * 1.3 && JoyStickPushingCounting[RJoys_4Div_Left] < 300)
      {
        JoyStickPushingCounting[RJoys_4Div_Left]++;   // 左
        if (JoyStickPushingCounting[RJoys_4Div_Left] > 20)
        {
          Right_JoyStick_4_Divisions[RJoys_4Div_Left] += my_joystick.R_X;
        }
      }
      if (my_joystick.R_Y < Left_JoyStick_Y_Origin * 0.7 && JoyStickPushingCounting[RJoys_4Div_Down] < 300)
      {
        JoyStickPushingCounting[RJoys_4Div_Down]++;   // 下
        if (JoyStickPushingCounting[RJoys_4Div_Down] > 20)
        {
          Right_JoyStick_4_Divisions[RJoys_4Div_Down] += my_joystick.R_Y;
        }
      }
      if (my_joystick.R_Y > Left_JoyStick_Y_Origin * 1.3 && JoyStickPushingCounting[RJoys_4Div_Up] < 300)
      {
        JoyStickPushingCounting[RJoys_4Div_Up]++;   // 上
        if (JoyStickPushingCounting[RJoys_4Div_Up] > 20)
        {
          Right_JoyStick_4_Divisions[RJoys_4Div_Up] += my_joystick.R_Y;
        }
      }



      // 如果中途松开了，就不算，重来
      if (my_joystick.R_X > Left_JoyStick_X_Origin * 0.7 && JoyStickPushingCounting[RJoys_4Div_Right] < 300) 
      {JoyStickPushingCounting[RJoys_4Div_Right] = 0;Right_JoyStick_4_Divisions[RJoys_4Div_Right] = 0;}

      if (my_joystick.R_X < Left_JoyStick_X_Origin * 1.3 && JoyStickPushingCounting[RJoys_4Div_Left] < 300)
      {JoyStickPushingCounting[RJoys_4Div_Left] = 0;Right_JoyStick_4_Divisions[RJoys_4Div_Left] = 0;}

      if (my_joystick.R_Y < Left_JoyStick_Y_Origin * 1.3 && JoyStickPushingCounting[RJoys_4Div_Up] < 300)
      {JoyStickPushingCounting[RJoys_4Div_Up] = 0;Right_JoyStick_4_Divisions[RJoys_4Div_Up] = 0;}

      if (my_joystick.R_Y > Left_JoyStick_Y_Origin * 0.7 && JoyStickPushingCounting[RJoys_4Div_Down] < 300)
      {JoyStickPushingCounting[RJoys_4Div_Down] = 0;Right_JoyStick_4_Divisions[RJoys_4Div_Down] = 0;}



      
      if (JoyStickPushingCounting[RJoys_4Div_Up] == 300)     // 完事了给他++一下，避免再次触发
      {
        JoyStickPushingCounting[RJoys_4Div_Up]++;
        Right_JoyStick_4_Divisions[RJoys_4Div_Up] = Right_JoyStick_4_Divisions[RJoys_4Div_Up] / 280.0;
        Serial.printf("RUP均值:%d\n", Right_JoyStick_4_Divisions[RJoys_4Div_Up]);
      }
      if (JoyStickPushingCounting[RJoys_4Div_Down] == 300)     // 完事了给他++一下，避免再次触发
      {
        JoyStickPushingCounting[RJoys_4Div_Down]++;
        Right_JoyStick_4_Divisions[RJoys_4Div_Down] = Right_JoyStick_4_Divisions[RJoys_4Div_Down] / 280.0;
        Serial.printf("RDOWN均值:%d\n", Right_JoyStick_4_Divisions[RJoys_4Div_Down]);
      }
      if (JoyStickPushingCounting[RJoys_4Div_Right] == 300)     // 完事了给他++一下，避免再次触发
      {
        JoyStickPushingCounting[RJoys_4Div_Right]++;
        Right_JoyStick_4_Divisions[RJoys_4Div_Right] = Right_JoyStick_4_Divisions[RJoys_4Div_Right] / 280.0;
        Serial.printf("RRIGHT均值:%d\n", Right_JoyStick_4_Divisions[RJoys_4Div_Right]);
      }
      if (JoyStickPushingCounting[RJoys_4Div_Left] == 300)     // 完事了给他++一下，避免再次触发
      {
        JoyStickPushingCounting[RJoys_4Div_Left]++;
        Right_JoyStick_4_Divisions[RJoys_4Div_Left] = Right_JoyStick_4_Divisions[RJoys_4Div_Left] / 280.0;
        Serial.printf("RLEFT均值:%d\n", Right_JoyStick_4_Divisions[RJoys_4Div_Left]);
      }
      break;
    }

    case 10:     // 校准完了，先给STM32发消息，再退出
    {
      Calib_sendbuf[0] = 1;
      Calib_Infos[0] = Left_JoyStick_4_Divisions[Joys_4Div_Right];
      Calib_Infos[1] = Left_JoyStick_4_Divisions[Joys_4Div_Left];
      Calib_Infos[2] = Left_JoyStick_4_Divisions[Joys_4Div_Down];
      Calib_Infos[3] = Left_JoyStick_4_Divisions[Joys_4Div_Up];

      Calib_Infos[4] = Right_JoyStick_4_Divisions[RJoys_4Div_Right];
      Calib_Infos[5] = Right_JoyStick_4_Divisions[RJoys_4Div_Left];
      Calib_Infos[6] = Right_JoyStick_4_Divisions[RJoys_4Div_Down];
      Calib_Infos[7] = Right_JoyStick_4_Divisions[RJoys_4Div_Up];

      Calib_Infos[8] = Left_JoyStick_X_Origin;
      Calib_Infos[9] = Left_JoyStick_Y_Origin;
      Calib_Infos[10] = Right_JoyStick_X_Origin;
      Calib_Infos[11] = Right_JoyStick_Y_Origin;

      for (int i = 0; i < 12; i++)
      {
        // 一般是左大右小
        Calib_sendbuf[1] = i; // 顺序: LX_L_H, LY_L_H, RX_L_H, RY_L_H => LL_R, LD_U, RL_R, RD_U, LX_M, LY_M, RX_M, RY_M
        Calib_sendbuf[2] = Calib_Infos[i] >> 8;
        Calib_sendbuf[3] = Calib_Infos[i] & 0xff;
        Serial_STM.write(Calib_sendbuf, 4);
        delay(15);
      }

      // 将数据写入Flash

      // 打开命名空间，以读写模式
      pref.begin("JsInfos", false);
    
      // 存储数组
      for (int i = 0; i < 16; i++) {
        char key[8];
        sprintf(key, "jsori%d", i);
        pref.putUShort(key, Calib_Infos[i]);
        delay(10);
      }
    
      // 关闭命名空间
      pref.end();
      
      PageYX_Now->pageReady = false;
      PageYX_Now->page_state_ID = 0;
      read_origin_times = 0;
      time_ticks = 0;
      temp_X = 0;
      temp_Y = 0;
      temp_MaxX = 0;
      temp_MinX = 9999;
      temp_MaxY = 0;
      temp_MinY = 9999;
      READOK_flag = 0;

      for (int i = 0; i < 4; i++)
      {
        JoyStickPushingCounting[i] = 0;
      }
      time_ticks = 0;
      READOK_flag = 0;

      button_focoused = To_MainMenu();
      vTaskDelay(500);    // 所有的切换页面至少Delay0.5s

      CanResp_Flag = true;
      vTaskDelete(NULL);
      break;
    }

  }
  CanResp_Flag = true;
  vTaskDelete(NULL);
}



/// @brief 处在RMTCTRL页面时，对输入的响应
/// @param Param 
void RmtCtrl_Responser(void* Param)
{
  static int time_watcher = 0;


  /****   时间记录   ****/
  if(time_watcher == 0)  time_watcher = millis();
  if(millis() - time_watcher > 500)
  {
    // 发送状态号:0-101代表进入遥控模式
    Serial_STM.printf("m101");
    time_watcher = millis();
  }



  /***********      如果正在按住  右摇杆（所有页面基本通用的退出逻辑）      ***********/
  if (user_actions[RJs_Pressed])
  {
    PageYX_Now->pageReady = false;
    PageYX_Now->page_state_ID = 0;
    time_watcher = 0;
    // 发送状态号:0-212 代表进入 主菜单
    Serial_STM.printf("m212");
    button_focoused = To_MainMenu();
    vTaskDelay(500);    // 所有的切换页面至少 Delay 0.5s

    CanResp_Flag = true;
    vTaskDelete(NULL);
  }

  CanResp_Flag = true;
  vTaskDelete(NULL);
}


/// @brief 处在CnctCTRL页面时，对输入的响应
/// @param Param 
void Connect_Responser(void* Param)
{
  static int time_watcher = 0;


  /****   时间记录   ****/
  if(time_watcher == 0)  time_watcher = millis();
  if(millis() - time_watcher > 500)
  {
    // 发送状态号:0-501代表进入 连接模式
    Serial_STM.printf("m501");
    time_watcher = millis();
  }



  /***********      如果正在按住  右摇杆（所有页面基本通用的退出逻辑）      ***********/
  if (user_actions[RJs_Pressed])
  {
    PageYX_Now->pageReady = false;
    PageYX_Now->page_state_ID = 0;
    time_watcher = 0;
    // 发送状态号:0-212 代表进入 主菜单
    Serial_STM.printf("m212");
    button_focoused = To_MainMenu();
    vTaskDelay(500);    // 所有的切换页面至少 Delay 0.5s

    CanResp_Flag = true;
    vTaskDelete(NULL);
  }

  CanResp_Flag = true;
  vTaskDelete(NULL);
}
