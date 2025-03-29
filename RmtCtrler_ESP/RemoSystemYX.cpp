#include "RemoSystemYX.h"
#include "math.h"
#include "sstream"



TFT_eSPI tft = TFT_eSPI(360, 480);
TFT_eSprite white_spr = TFT_eSprite(&tft);
TFT_eSprite black_spr = TFT_eSprite(&tft);
char temp5[16];

int FpsDelayTimeBias = 0;
float DynamicFpsSetting = 45;

bool BlackRefresh = 0;
int PanelOutIntg = 0;

bool PanelOutAnimFlag;
int PanelOutAnim_CountTime = 0;

// 连接节点暂存列表
NodeInfo MyFavNode;
NodeInfo FindNodes[4];   // 最多显示4个节点

// 正在充电指示位
bool IsCharging = 0;
float bat_volt_precent = 0;

char stm_real_state[5] = {'?', '?','?', '?','?'};
char stm_pure[16] = {0};

/******         遥控器系统变量        ******/
uint32_t Left_JoyStick_4_Divisions[4];
uint32_t Right_JoyStick_4_Divisions[4];

uint16_t JoyStickPushingCounting[4];    // 推动摇杆的计时
bool JoyStick_8_SampleOK[8] = {0};

uint16_t Left_JoyStick_X_Origin = 2048;
uint16_t Left_JoyStick_Y_Origin = 2048;

uint16_t Right_JoyStick_X_Origin = 2048;
uint16_t Right_JoyStick_Y_Origin = 2048;
uint16_t Right_JoyStick_X_Wave;
uint16_t Right_JoyStick_Y_Wave;


/// @brief 当前所在页面
PageYX* PageYX_Now;

/* 所有注册的页面，如果想新增页面，这里必须改 */

PageYX MainMenu_Page;
PageYX Config_Page;
PageYX Calib_Page;
PageYX Anim_Page;
PageYX RmtCtrl_Page;
PageYX Connect_Page;


/// @brief 最多支持24个页面
PageYX* PagePointers[24];


// 注册页面中的控件，如果想新增页面，这里必须改
ElementYX new_button0 = ElementYX(0, 240, 60, "Fps", Panel);
ElementYX new_button1 = ElementYX(1, 240, 160, "Anim", &Anim_Page);
ElementYX new_button2 = ElementYX(2, 240, 260, "Config", &Config_Page);
ElementYX new_button3 = ElementYX(3, 240, 260, "Calibrate", &Calib_Page);
ElementYX new_button4 = ElementYX(4, 240, 260, "RmtCtrl", &RmtCtrl_Page);
ElementYX new_button5 = ElementYX(5, 240, 260, "Connect", &Connect_Page);
ElementYX new_bar8 = ElementYX(100, 240, 160, "", Bar);


ElementYX anim_button0 = ElementYX(0, 100, 100, "Test", Panel);
ElementYX anim_button1 = ElementYX(1, 100, 100, "Test1", Panel);
ElementYX anim_button2 = ElementYX(2, 100, 100, "Test2", Panel);
ElementYX anim_button3back = ElementYX(3, 100, 100, "Back", &MainMenu_Page);

ElementYX config_button0 = ElementYX(0, 100, 100, "CTest0", Panel);
ElementYX config_button1 = ElementYX(1, 100, 100, "CTest1", Panel);
ElementYX config_button2 = ElementYX(2, 100, 100, "CTest2", Panel);
ElementYX config_button3back = ElementYX(3, 100, 100, "Back", &MainMenu_Page);

// ElementYX* widget_focoused;
ElementYX* button_focoused;




/////////////////////////////////////////////////////////////////////////
int RGB888_2_565(uint8_t R, uint8_t G, uint8_t B)
{
  uint8_t new_R = R >> 3;
  uint8_t new_G = G >> 2;
  uint8_t new_B = B >> 3;
  Serial.printf("R:%d,G:%d,B:%d\n;", new_R,new_G,new_B);

  int temp_color565 = new_R << 11 | new_G << 5 | new_B;
  return temp_color565;
}

float AnimYX_SpeedGain(float err)
{
  err = abs(err);
  
  if(err < 20)
  {
    // 0.5~1
    return (err / 20.0) / 2.0 + 0.5;
  }
  if(err >= 20)
  {
    //  1~1.5
    return ((err - 20) / 460.0) * 0 + 1;
  }
  return 1;
}

ElementYX::ElementYX(int id)
{
  Elem_id = id;
}

ElementYX::ElementYX(int id, int x, int y)
{
  Elem_id = id;
  x_targ = x;
  x_now = x;
  y_targ = y;
  y_now = y;
  w_targ = 0;
  h_targ = 0;
}

ElementYX::ElementYX(int id, int x, int y, char targ_title[])
{
  Elem_id = id;
  x_targ = x;
  x_now = x;
  y_targ = y;
  y_now = y;
  w_targ = 0;
  h_targ = 0;
  strcpy(title, targ_title);
}

/**
 * @brief 如果输入的是PageType，那么这就是一个通向页面的普通按钮
 */
ElementYX::ElementYX(int id, int x, int y, char targ_title[], PageYX* page)
{
  Elem_id = id;
  x_targ = x;
  x_now = x;
  y_targ = y;
  y_now = y;
  w_targ = 0;
  h_targ = 0;
  strcpy(title, targ_title);
  elem_type = Button;
  targ_page = page;
}

/**
 * @brief 如果输入的是某一特定的 elemType类型，那么这就是一个该类型的elem
 */
ElementYX::ElementYX(int id, int x, int y, char targ_title[], ElementType elemtype)
{
  Elem_id = id;
  x_targ = x;
  x_now = x;
  y_targ = y;
  y_now = y;
  w_targ = 0;
  h_targ = 0;
  strcpy(title, targ_title);
  elem_type = elemtype;
}

void ElementYX::Set_Pos(int x, int y)
{
  x_targ = x;
  y_targ = y;
}

void ElementYX::Tele_To_Pos(int x, int y)
{
  x_targ = x;
  y_targ = y;
  x_now = x;
  y_now = y;
}

void ElementYX::Set_Size(int w, int h)
{
  w_targ = w;
  h_targ = h;
}

void ElementYX::Tele_To_Size(int w, int h)
{
  w_targ = w;
  h_targ = h;
  w_now = w;
  h_now = h;
}

void ElementYX::To_Targ_State(float Fps_Gain = 1.0)
{
  float x_err = x_targ - x_now;
  float y_err = y_targ - y_now;
  float w_err = w_targ - w_now;
  float h_err = h_targ - h_now;

  #ifdef fps30
  Fps_Gain = 2.0;
  #endif
  #ifdef fps15
  Fps_Gain = 4.0;
  #endif

  x_now += 0.1 * x_err * AnimYX_SpeedGain(x_err) * Fps_Gain;
  y_now += 0.1 * y_err * AnimYX_SpeedGain(y_err) * Fps_Gain;
  w_now += 0.1 * w_err * AnimYX_SpeedGain(w_err) * Fps_Gain;
  h_now += 0.1 * h_err * AnimYX_SpeedGain(h_err) * Fps_Gain;
}


bool ElementYX::At_Targ_State()
{
  return ((fabs(x_now - x_targ) < 0.35) && (fabs(y_now - y_targ) < 0.35) && (fabs(w_now - w_targ) < 0.35) && (fabs(h_now - h_targ) < 0.35) && !NeedUpdate);
}

bool ElementYX::IsExPanel()
{
  return elem_type == Panel && panel_expanded;
}

void ElementYX::Init_BarVar(float* TargVarP, float MaxValue)
{
  LinkedFloatP = TargVarP;
  value_1 = MaxValue;
}

void ElementYX::Update_BarVar()
{
  value_0 = *LinkedFloatP;
}

void ElementYX::Set_ChildID(int ElemId)
{
  childElemId = ElemId;
}


///////////////////////////////////////////////////////////////////////////////




void PageYX::Add_NonLinkElem(ElementYX &targ_elem)
{
  Elems_List[Elem_Nums] = &targ_elem;    // 把指针加入控件列表
  
  Elem_Nums++;    // 控件数量+1
}

/**
 * @name Add_Elem
 * @brief 链式地向页面中加入元素
 * @note 像按钮一类的元素就适合 链式地加入，它会在菜单中顺序地显示出来
 */
void PageYX::Add_Elem(ElementYX &targ_elem)
{
  Elems_List[Elem_Nums] = &targ_elem;    // 把指针加入控件列表

  if (Elem_Nums > 0)
  {
    targ_elem.last_elem = Elems_List[Elem_Nums - 1];
    Elems_List[Elem_Nums - 1]->next_elem = &targ_elem;
  }

  targ_elem.next_elem = Elems_List[0];
  Elems_List[0]->last_elem = &targ_elem;
  
  Elem_Nums++;    // 控件数量+1
  targ_elem.at_page = this;
}

/**
 * @name Get_TargID_Elem
 * @brief 获取页面中的元素的指针
 * @note 根据你提供的元素ID，获取页面中的元素的指针
 */
ElementYX* PageYX::Get_TargID_Elem(int targ_id)
{
  for (int i = 0; i < Elem_Nums; i++)
  {
    if (Elems_List[i]->Elem_id == targ_id)
    {
      return Elems_List[i];
    }
  }
  
  return NULL;
}

/**
 * @name Init_SysYX
 * @brief 显示器系统驱动初始化
 * @note 你组织的控件，都在这里完成初始化
 */
void Init_SysYX()
{
  tft.setTextSize(3);
  new_bar8.Init_BarVar(&DynamicFpsSetting, 50);       // 将 滑动条 与 变量 练习起来，并设置一个变量的最大值
  new_button0.Set_ChildID(new_bar8.Elem_id);     // 告诉按钮0，它的子控件是 ID为100的那个
  
  new_button0.Set_Size(100, 50);    // 设置开局三个按钮的 三个大小
  new_button1.Set_Size(190, 85);
  new_button2.Set_Size(100, 50);
  new_button0.visible = 1;          // 让开局三个按钮变得可见
  new_button1.visible = 1;
  new_button2.visible = 1;
  
  MainMenu_Page.Page_id = 0;    // 把已经注册的页面加入页面列表（用户如果新添加页面需要改这里），如果想新增页面，这里必须改
  Anim_Page.Page_id = 1;
  Config_Page.Page_id = 2;
  Calib_Page.Page_id = 3;
  RmtCtrl_Page.Page_id = 4;
  Connect_Page.Page_id = 5;

  Calib_Page.openAnimType = RmtCtrlANIM;        // 将非菜单类页面的出场动画和类型自定义，如果想新增页面，这里必须改
  Calib_Page.page_type = CustomPage;
  RmtCtrl_Page.openAnimType = RmtCtrlANIM;
  RmtCtrl_Page.page_type = CustomPage;
  Connect_Page.openAnimType = RmtCtrlANIM;
  Connect_Page.page_type = CustomPage;
  
  MainMenu_Page.Add_Elem(new_button0);    // 向页面中添加 链式控件，如果想新增页面，这里必须改
  MainMenu_Page.Add_Elem(new_button1);
  MainMenu_Page.Add_Elem(new_button2);
  MainMenu_Page.Add_Elem(new_button3);
  MainMenu_Page.Add_Elem(new_button4);
  MainMenu_Page.Add_Elem(new_button5);
  
  MainMenu_Page.Add_NonLinkElem(new_bar8);    // 向页面中添加 非链式控件

  Anim_Page.Add_Elem(anim_button0);
  Anim_Page.Add_Elem(anim_button1);
  Anim_Page.Add_Elem(anim_button2);
  Anim_Page.Add_Elem(anim_button3back);

  Config_Page.Add_Elem(config_button0);
  Config_Page.Add_Elem(config_button1);
  Config_Page.Add_Elem(config_button2);
  Config_Page.Add_Elem(config_button3back);

  button_focoused = MainMenu_Page.Get_TargID_Elem(1);
  PageYX_Now = &MainMenu_Page;
}






void ScreenRefresh_Handler()
{
  // 根据当前所在的页面，刷新当前页面的内容

}









void Page_Handler()
{
  // 遍历页面内的元素，刷新他们
  /*
    1. 覆盖前一帧
    2.  刷新元素状态
    3.  按顺序刷新本帧
  */

  if (BlackRefresh)
  {
    tft.fillScreen(TFT_BLACK);
    BlackRefresh = false;
  }


  // 优先播放动画
  if (PageYX_Now->openAnimType != NULLANIM)
  {
    if(!LoadingAnim_Rotate_Rings(PageYX_Now->pageReady))
    {
      goto FPS_REFRESH;
    }
  }
  // Serial.printf("anim:%dready:%d", AnimOK, PageYX_Now->pageReady);

  
  /******     遍历页面中的所有元素，并按一定规律刷新他们（只针对菜单类界面）     ******/
  if (PageYX_Now->page_type == MenuPage)
  {
    for(int i = 0; i < PageYX_Now->Elem_Nums; i++)
    {
      ElementYX* elem = PageYX_Now->Elems_List[i];

      switch (elem->elem_type)
      {
        case Bar:
        {
          if (elem->superElemId != DefualtID && !PageYX_Now->Get_TargID_Elem(elem->superElemId)->At_Targ_State())   // 父控件如果需要刷新，子控件一定也需要
          {
            elem->NeedUpdate = true;
          }

          if (!elem->At_Targ_State())
          {
            // Serial.println("Its a Bar");
            if (elem->superElemId != DefualtID)       // 如果这个Bar它有父控件
            {
              float lft_x = elem->x_now - (elem->w_now / 2.0);
              float lft_y = elem->y_now - (elem->h_now / 2.0);

                tft.drawRoundRect(lft_x, lft_y,
                elem->w_now, elem->h_now,
                elem->h_now / 2.5, TFT_BLACK);

                tft.fillRoundRect(lft_x, lft_y,
                (elem->w_now) * (elem->value_0 / elem->value_1), elem->h_now,
                elem->h_now / 2.5, TFT_BLACK);


              /////////////////    实现跟随   //////////////////////////
              auto SuperElem = PageYX_Now->Get_TargID_Elem(elem->superElemId);
              if (!SuperElem->panel_expanded)
              {
                elem->Set_Size(0, 0);
              }
              else
              {
                elem->Set_Size(min(SuperElem->w_now * 0.67f, elem->w_targ), min(SuperElem->h_now * 0.4f, elem->h_targ));
              }
              elem->Set_Pos(SuperElem->x_now, SuperElem->y_now);

              ///////////////////////////////////////////////////
                elem->Update_BarVar();
                elem->To_Targ_State(1.75f);
              ///////////////////////////////////////////////////

              int Targ_Color;
              if (elem->color == Widget_White)
                  {
                    Targ_Color = TFT_WHITE;
                  }
                  else
                  {
                    Targ_Color = TFT_DARKGREY;
                  }


              tft.drawRoundRect(elem->x_now - (elem->w_now / 2.0), elem->y_now - (elem->h_now / 2.0),
              elem->w_now, elem->h_now,
              elem->h_now / 2.5, Targ_Color);

              tft.fillRoundRect(elem->x_now - (elem->w_now / 2.0), elem->y_now - (elem->h_now / 2.0),
                (elem->w_now) * (elem->value_0 / elem->value_1) - 1, elem->h_now - 1,
                elem->h_now / 2.5, Targ_Color);
            }
            else        // 如果这个Bar它 没有父控件：直接刷新一个框
            {
              float lft_x = elem->x_now - (elem->w_now / 2.0);
              float lft_y = elem->y_now - (elem->h_now / 2.0);

              tft.drawRoundRect(lft_x, lft_y,
              elem->w_now, elem->h_now,
              elem->h_now / 3.0, TFT_BLACK);

              ///////////////////////////////////////////////////
                elem->To_Targ_State();
              ///////////////////////////////////////////////////

              int Targ_Color;
              if (elem->color == Widget_White)
              {
                Targ_Color = TFT_WHITE;
              }
              else
              {
                Targ_Color = TFT_DARKGREY;
              }
              tft.drawRoundRect(elem->x_now - (elem->w_now / 2.0), elem->y_now - (elem->h_now / 2.0),
              elem->w_now, elem->h_now,
              elem->h_now / 3.0, Targ_Color);
            }

            if (elem->NeedUpdate)
            {
              elem->NeedUpdate = false;
            }
          }
          break;
        }

        case Panel:
        case Button:
        {
          if (!elem->At_Targ_State())
          {
            if (elem->childElemId != DefualtID)   // 父控件如果需要刷新，子控件一定也需要
            {
              PageYX_Now->Get_TargID_Elem(elem->childElemId)->NeedUpdate = true;
            }
            
            /*                            先覆盖上一帧                                    */
            float lft_x = elem->x_now - (elem->w_now / 2.0);
            float lft_y = elem->y_now - (elem->h_now / 2.0);

            tft.drawRoundRect(lft_x, lft_y,
            elem->w_now, elem->h_now,
            elem->h_now / 4.0, TFT_BLACK);

            // 字体高度是8 ，实际上是
            if (strcmp(elem->title, "") != 0)   // 如果文本标题存在
            {
              // 先判定elem大小
              float text_size = 0;
              float text_width = 0;

              int i = 8;
              while (i--)
              {
                text_width = text_size * 6 + min(text_size, 3.0f) * 6 * strlen(elem->title + 1);
                if (text_size * 8 < 0.45 * elem->h_now)
                {
                  text_size++;
                }
                else
                {
                  break;
                }
              }
              i = 8;
              while (i--)
              {
                text_width = text_size * 6 + min(text_size, 3.0f) * 6 * strlen(elem->title + 1);
                if (text_size * 8 > 0.45 * elem->h_now)
                {
                  text_size--;
                }
                else
                {
                  break;
                }
              }
              float text_height = (int)(text_size) * 8;

              char cuted_title[10];
              int TextLen = strlen(elem->title);
              while (text_width > 0.9 * elem->w_now && TextLen > 1)
              {
                TextLen--;
                text_width -= min(text_size, 3.0f) * 6;
              }

              strcpy(cuted_title, elem->title);
              cuted_title[TextLen] = 0;               // 截断字符串

              
              if (text_size > 0.5)
              {
                tft.setTextColor(TFT_BLACK);
                char text_head = cuted_title[0];
                char text_after[9];
                strcpy(text_after, cuted_title+1);

                // 根据是否是打开的面板，决定字符刷新的位置；
                if (elem->elem_type == Panel && elem->PanelUpdate)
                {
                  tft.setTextSize(min(text_size, 3.0f));
                  tft.drawString(elem->title, elem->x_now - (text_width / 2.0) + min(text_size, 3.0f) * 6,
                  elem->y_now - (text_height / 2.0) - elem->h_now / 2.0 + (text_height / 2.0) + (text_size - min(text_size, 3.0f)) * 3);

                  elem->PanelUpdate = false;
                }
                else
                {
                  tft.setTextSize(text_size);
                  tft.drawChar(text_head, elem->x_now - (text_width / 2.0), elem->y_now - (text_height / 2.0));

                  tft.setTextSize(min(text_size, 3.0f));
                  tft.drawString(text_after, elem->x_now - (text_width / 2.0) + text_size * 6,
                  elem->y_now - (text_height / 2.0) + (text_size - min(text_size, 3.0f)) * 3);
                }
                
              }

            }

            ///////////////////////////////////////////////////
            elem->To_Targ_State();
            ///////////////////////////////////////////////////

            int Targ_Color;
            if (elem->color == Widget_White)
                {
                  Targ_Color = TFT_WHITE;
                }
                else
                {
                  Targ_Color = TFT_DARKGREY;
                }


            tft.drawRoundRect(elem->x_now - (elem->w_now / 2.0), elem->y_now - (elem->h_now / 2.0),
            elem->w_now, elem->h_now,
            elem->h_now / 4.0, Targ_Color);

            /*
                刷新标题的部分
            */
            if (strcmp(elem->title, "") != 0)
            {
              // 先判定elem大小
              float text_size = 0;
              float text_width = 0;

              int i = 8;
              while (i--)
              {
                text_width = text_size * 6 + min(text_size, 3.0f) * 6 * strlen(elem->title + 1);
                if (text_size * 8 < 0.45 * elem->h_now)
                {
                  text_size++;
                }
                else
                {
                  break;
                }
              }
              i = 8;
              while (i--)
              {
                text_width = text_size * 6 + min(text_size, 3.0f) * 6 * strlen(elem->title + 1);
                if (text_size * 8 > 0.45 * elem->h_now)
                {
                  text_size--;
                }
                else
                {
                  break;
                }
              }
              
              float text_height = (int)(text_size) * 8;
              char cuted_title[10];
              int TextLen = strlen(elem->title);
              while (text_width > 0.9 * elem->w_now && TextLen > 1)
              {
                TextLen--;
                text_width -= min(text_size, 3.0f) * 6;
              }

              strcpy(cuted_title, elem->title);
              cuted_title[TextLen] = 0;                 // 截断字符串
              
              if (text_size > 0.5)
              {
                tft.setTextColor(Targ_Color);

                char text_head = cuted_title[0];
                char text_after[9];
                strcpy(text_after, cuted_title+1);

                // 根据是否是打开的面板，决定字符刷新的位置； 在这里还会看一看自己有没有子控件，如果有的话，会组织子控件的刷新
                if (elem->elem_type == Panel && (elem->panel_expanded || PanelOutAnimFlag))
                {

                  tft.setTextSize(min(text_size, 3.0f));
                  tft.drawString(elem->title, elem->x_now - (text_width / 2.0) + min(text_size, 3.0f) * 6,
                  elem->y_now - (text_height / 2.0) - elem->h_now / 2.0 + (text_height / 2.0) + (text_size - min(text_size, 3.0f)) * 3);

                  elem->PanelUpdate = true;         // 这个标志位置位之后，Panel将不再以Button形式刷新，而是被我们的刷新管理器以Panel形式刷新

                  if (elem->childElemId != DefualtID)   //    如果自己有子控件的话
                  {
                    auto my_child_widgets = PageYX_Now->Get_TargID_Elem(elem->childElemId);    // 获得自己的子控件，并根据子控件的不同，刷新子控件。
                    switch (my_child_widgets->elem_type)
                    {
                      case Bar:           // 如果子控件是滑条，如此刷新
                      {
                        my_child_widgets->Set_Size(240, 24);
                        my_child_widgets->superElemId = elem->Elem_id;
                        break;
                      }
                    }
                    
                  }
                }
                else
                {
                  tft.setTextSize(text_size);
                  tft.drawChar(text_head, elem->x_now - (text_width / 2.0), elem->y_now - (text_height / 2.0));

                  tft.setTextSize(min(text_size, 3.0f));
                  tft.drawString(text_after, elem->x_now - (text_width / 2.0) + text_size * 6,
                  elem->y_now - (text_height / 2.0) + (text_size - min(text_size, 3.0f)) * 3);
                }
              }
            }
            elem->NeedUpdate = false;
          }
          break;
        }
      default:
        break;
      }
    }
  }
  /******     自定义刷新逻辑（针对自定义界面）     ******/
  else  
  {
    switch (PageYX_Now->Page_id)
    {
      case 3:{Calib_Refresh();break;}
      case 4:{RmtCtrl_Refresh();break;}
      // case 5:{Connect_Refresh();break;}
    }
  }

  /******     打印FPS 和 电池 和 状态码    ******/
  FPS_REFRESH:
  static bool chargingprint = 0;
  if (fps_print_flag)
    {
      tft.setTextSize(1);

      char temp[16], temp2[16], temp3[16], temp4[16];
      sprintf(temp, "fps:%d", fps_for_print_preserve * 2);
      tft.setTextColor(TFT_BLACK);
      tft.drawString(temp, 0, 0);
      sprintf(temp2, "fps:%d", fps_for_print * 2);
      tft.setTextColor(TFT_WHITE);
      tft.drawString(temp2, 0, 0);

      tft.setTextColor(TFT_BLACK);
      tft.drawString(temp5, 0, 12);
      sprintf(temp5, "state:%s", stm_real_state);
      tft.setTextColor(TFT_WHITE);
      tft.drawString(temp5, 0, 12);
      
      // bat_volt_precent = ((BatVolt - 3.75) / 0.45);
      if (IsCharging == 0)
      {
        // sprintf(temp3, "bat:%d%%", (int)(bat_volt_print * 100));
        sprintf(temp3, "bat:%.2fV", bat_volt_print);
        tft.setTextColor(TFT_BLACK);
        tft.drawString(temp3, 0, 24);
        // sprintf(temp4, "bat:%d%%", (int)(bat_volt_precent * 100));
        sprintf(temp4, "bat:%.2fV", BatVolt);
        tft.setTextColor(TFT_WHITE);
        tft.drawString(temp4, 0, 24);
      }
      
      


      if (IsCharging)
      {
        sprintf(temp3, "bat:%.2fV", bat_volt_print);
        tft.setTextColor(TFT_BLACK);
        tft.drawString(temp3, 0, 24);

        tft.setTextSize(2);
        tft.setTextColor(TFT_RED);
        tft.drawString("Charging", 0, 24);
        chargingprint = 1;
      }

      if (chargingprint && IsCharging == 0)
      {
        tft.setTextSize(2);
        tft.setTextColor(TFT_BLACK);
        tft.drawString("Charging", 0, 24);
        chargingprint = 0;
      }
      

      fps_for_print_preserve = fps_for_print;
      bat_volt_print = BatVolt;

      fps_print_flag = 0;
    }
}






/*    以下为 Calibration 页面的刷新逻辑*/
float bias_y;
float bias_x;
float last_bias_y;
float last_bias_x;
AnimArcBlock arc_blocks[8];
AnimArcBlock arc_blocks_outer[4];
AnimArcBlock arc_originRing;
void Calib_Refresh()
{
  static int time_ticks;    // 切换状态时，这个必须刷新，给下一个状态用
  static float anim_ticks;    // 切换状态时，这个必须刷新，给下一个状态用
  switch (PageYX_Now->page_state_ID)
  {
    case 0:     // 刚进入Calibrate页面，播放动画，捕捉左摇杆静态位置
    {
      if (time_ticks == 0)
      {
        time_ticks = millis();
        arc_originRing.inr = 75;
        arc_originRing.outr = 80;
        arc_originRing.color = TFT_WHITE;
      }
      tft.setTextSize(3);
      tft.setTextColor(TFT_WHITE);
      tft.drawString("Capturing Static", 100, 0);
      tft.drawString("Js Origins", 120, 40);
      
      Draw_Anim_Arcs_heng(&arc_originRing, 0, ((millis() - time_ticks) / 3000.0) * 360);

      // tft.drawArc(240, 180, 80, 75, 0, ((millis() - time_ticks) / 3000.0) * 360, TFT_WHITE, TFT_BLACK);

      if(millis() - time_ticks > 3000)    // 等待三秒
      {
        PageYX_Now->page_state_ID++;
        time_ticks = 0;
        NeedAllRefresh
      }
      break;
    }
    
    case 1:     // 进入左摇杆校准页面，初始化页面
    {
      tft.setTextSize(3);
      tft.setTextColor(TFT_WHITE);
      tft.drawString("Push Your Left Joystic", 40, 0);
      tft.drawString("Up,Down,Left and Right", 20, 32);

      for (int i = 0; i < 8; i++)
      {
        arc_blocks[i].angle_start = (174 + 45 * i);
        arc_blocks[i].angle_end = (186 + 45 * i);
        arc_blocks[i].inr = 0;
        arc_blocks[i].outr = 0;
        arc_blocks[i].color = TFT_WHITE;
      }
      for (int i = 0; i < 4; i++)
      {
        arc_blocks_outer[i].angle_start = (180 + 90 * i);
        arc_blocks_outer[i].angle_end = (180 + 90 * i);
        arc_blocks_outer[i].inr = 98;
        arc_blocks_outer[i].outr = 102;
        arc_blocks_outer[i].color = TFT_WHITE;
      }
      PageYX_Now->page_state_ID++;
      break;
    }
    
    case 2:     // 进入左摇杆校准页面，不断刷新
    {
      Draw_Anim_Arcs(&arc_originRing, 0, 361, 120, 121);

      for (int i = 0; i < 4; i++)       // 如果外环好了，刷新外环
      {
        if (JoyStickPushingCounting[i] >= 301)    // 已经采集好了
        {
          arc_blocks_outer[i].color = TFT_LIGHTGREEN;
          Draw_Anim_Arcs   // 不用取余360了，函数里取了
          (arc_blocks_outer + i, 
          (int)(181 - (30 * (JoyStickPushingCounting[i] / 300.0)) + 90 * i), 
          (int)(182 + (30 * (JoyStickPushingCounting[i] / 300.0)) + 90 * i),
          108, 116);
        }
        else    // 没采集好的话
        {
          Draw_Anim_Arcs_heng   // 不用取余360了，函数里取了
          (arc_blocks_outer + i, 
          (int)(181 - (30 * (JoyStickPushingCounting[i] / 300.0)) + 90 * i), 
          (int)(182 + (30 * (JoyStickPushingCounting[i] / 300.0)) + 90 * i));
        }
      }


      // 刷新我的光标所在位置
      tft.drawCircle(240 - last_bias_x, 180 - last_bias_y, 9, TFT_BLACK);
      
      bias_x = ((my_joystick.L_X - Left_JoyStick_X_Origin) / (float)Left_JoyStick_X_Origin) * 100.0;
      bias_y = ((my_joystick.L_Y - Left_JoyStick_Y_Origin) / (float)Left_JoyStick_Y_Origin) * 100.0;
      
      if(Geoge_Ave(bias_x, bias_y) > 100)
      {
        bias_y = bias_y / Geoge_Ave(bias_x, bias_y) * 100.0;
        bias_x = bias_x / Geoge_Ave(bias_x, bias_y) * 100.0;
      }
      
      tft.drawCircle(240 - bias_x, 180 - bias_y, 9, TFT_WHITE);
      
      last_bias_x = bias_x;
      last_bias_y = bias_y;
      
      
      if (JoyStickPushingCounting[0] >= 301 && JoyStickPushingCounting[1] >= 301
      && JoyStickPushingCounting[2] >= 301 && JoyStickPushingCounting[3] >= 301)
      {
        for (int i = 0; i < 4; i++)
        {
          arc_blocks_outer[i].angle_start = (180 + 90 * i);
          arc_blocks_outer[i].angle_end = (180 + 90 * i);
          arc_blocks_outer[i].inr = 59;
          arc_blocks_outer[i].outr = 61;
          arc_blocks_outer[i].color = TFT_WHITE;
        }
        NeedAllRefresh
        PageYX_Now->page_state_ID++;
      }
      break;
    }

    case 3:     // 左摇杆校准好了，消除圆圈
    {
      Draw_Anim_Arcs(&arc_originRing, 0, 0, 90, 91);

      for (int i = 0; i < 4; i++)       // 如果外环好了，刷新外环
      {
          Draw_Anim_Arcs_heng   // 不用取余360了，函数里取了
          (arc_blocks_outer + i,  (int)(181 - 50 + 90 * i), (int)(182 + 50 + 90 * i));
      }

      if (Check_Anim_Arcs(&arc_originRing, 0, 0, 90, 91))
      {
        // NeedAllRefresh
        PageYX_Now->page_state_ID++;
      }
      break;
    }

    case 4:     // 左摇杆校准好了，播放勾号动画
    {
      //  缓入
      anim_ticks = anim_ticks + 0.02 * (1.05 - anim_ticks);
      switch (time_ticks)
      {
        case 0:
        {
          tft.drawWideLine(220, 180, 220 + anim_ticks * 50, 180 + anim_ticks * 100, 8, TFT_WHITE, TFT_TRANSPARENT);
          
          if (anim_ticks > 0.2)
          {
            time_ticks++;
          }
          break;
        }
        case 1:
        {
          float base_anim_tick = anim_ticks - 0.2;
          tft.drawWideLine(230, 200, 230 + base_anim_tick * 40, 200 - base_anim_tick * 40, 8, TFT_WHITE, TFT_TRANSPARENT);
          if (anim_ticks > 1)
          {
            time_ticks++;
          }
          break;
        }
        case 2:
        {
          if (anim_ticks > 1.025)
          {
            time_ticks++;
            break;
          }
        }
      }
      if (time_ticks >= 3)
      {
        NeedAllRefresh
        PageYX_Now->page_state_ID++;
        time_ticks = 0;
        anim_ticks = 0;
      }
      break;
    }

    case 5:     // 播放动画，捕捉右摇杆静态位置
    {
      if (time_ticks == 0)
      {
        time_ticks = millis();
        arc_originRing.inr = 75;
        arc_originRing.outr = 80;
        arc_originRing.color = TFT_WHITE;
      }
      tft.setTextSize(2);
      tft.setTextColor(TFT_WHITE);
      tft.drawString("Capturing Static", 150, 0);
      tft.setTextSize(3);
      tft.drawString("Right Origins", 120, 30);
      
      Draw_Anim_Arcs_heng(&arc_originRing, 0, ((millis() - time_ticks) / 3000.0) * 360);

      // tft.drawArc(240, 180, 80, 75, 0, ((millis() - time_ticks) / 3000.0) * 360, TFT_WHITE, TFT_BLACK);

      if(millis() - time_ticks > 3000)    // 等待三秒
      {
        PageYX_Now->page_state_ID++;
        time_ticks = 0;
        NeedAllRefresh
      }
      break;
    }

    case 6:     // 进入右摇杆校准页面，初始化页面
    {
      tft.setTextSize(2);
      tft.setTextColor(TFT_WHITE);
      tft.drawString("Push Your Right Joystic", 40, 0);
      tft.setTextSize(3);
      tft.drawString("Up,Down,Left and Right", 20, 28);

      for (int i = 0; i < 8; i++)
      {
        arc_blocks[i].angle_start = (174 + 45 * i);
        arc_blocks[i].angle_end = (186 + 45 * i);
        arc_blocks[i].inr = 0;
        arc_blocks[i].outr = 0;
        arc_blocks[i].color = TFT_WHITE;
      }
      for (int i = 0; i < 4; i++)
      {
        arc_blocks_outer[i].angle_start = (180 + 90 * i);
        arc_blocks_outer[i].angle_end = (180 + 90 * i);
        arc_blocks_outer[i].inr = 98;
        arc_blocks_outer[i].outr = 102;
        arc_blocks_outer[i].color = TFT_WHITE;
      }
      PageYX_Now->page_state_ID++;
      break;
    }

    case 7:     // 进入右摇杆校准页面，不断刷新
    {
      Draw_Anim_Arcs(&arc_originRing, 0, 361, 120, 121);

      for (int i = 0; i < 4; i++)       // 如果外环好了，刷新外环
      {
        if (JoyStickPushingCounting[i] >= 301)    // 已经采集好了
        {
          arc_blocks_outer[i].color = TFT_LIGHTGREEN;
          Draw_Anim_Arcs   // 不用取余360了，函数里取了
          (arc_blocks_outer + i, 
          (int)(181 - (30 * (JoyStickPushingCounting[i] / 300.0)) + 90 * i), 
          (int)(182 + (30 * (JoyStickPushingCounting[i] / 300.0)) + 90 * i),
          108, 116);
        }
        else    // 没采集好的话
        {
          Draw_Anim_Arcs_heng   // 不用取余360了，函数里取了
          (arc_blocks_outer + i, 
          (int)(181 - (30 * (JoyStickPushingCounting[i] / 300.0)) + 90 * i), 
          (int)(182 + (30 * (JoyStickPushingCounting[i] / 300.0)) + 90 * i));
        }
      }


      // 刷新我的光标所在位置
      tft.drawCircle(240 + last_bias_x, 180 + last_bias_y, 9, TFT_BLACK);
      
      bias_x = ((my_joystick.R_X - Right_JoyStick_X_Origin) / (float)Right_JoyStick_X_Origin) * 100.0;
      bias_y = ((my_joystick.R_Y - Right_JoyStick_Y_Origin) / (float)Right_JoyStick_Y_Origin) * 100.0;
      
      if(Geoge_Ave(bias_x, bias_y) > 100)
      {
        bias_y = bias_y / Geoge_Ave(bias_x, bias_y) * 100.0;
        bias_x = bias_x / Geoge_Ave(bias_x, bias_y) * 100.0;
      }
      
      tft.drawCircle(240 + bias_x, 180 + bias_y, 9, TFT_WHITE);
      
      last_bias_x = bias_x;
      last_bias_y = bias_y;
      
      
      if (JoyStickPushingCounting[0] >= 301 && JoyStickPushingCounting[1] >= 301
      && JoyStickPushingCounting[2] >= 301 && JoyStickPushingCounting[3] >= 301)
      {
        for (int i = 0; i < 4; i++)
        {
          arc_blocks_outer[i].angle_start = (180 + 90 * i);
          arc_blocks_outer[i].angle_end = (180 + 90 * i);
          arc_blocks_outer[i].inr = 59;
          arc_blocks_outer[i].outr = 61;
          arc_blocks_outer[i].color = TFT_WHITE;
        }
        NeedAllRefresh
        PageYX_Now->page_state_ID++;
      }
      break;
    }

    case 8:     // 右摇杆校准好了，消除圆圈
    {
      Draw_Anim_Arcs(&arc_originRing, 0, 0, 90, 91);

      for (int i = 0; i < 4; i++)       // 如果外环好了，刷新外环
      {
          Draw_Anim_Arcs_heng   // 不用取余360了，函数里取了
          (arc_blocks_outer + i,  (int)(181 - 50 + 90 * i), (int)(182 + 50 + 90 * i));
      }

      if (Check_Anim_Arcs(&arc_originRing, 0, 0, 90, 91))
      {
        // NeedAllRefresh
        PageYX_Now->page_state_ID++;
      }
      break;
    }

    case 9:     // 右摇杆校准好了，播放勾号动画
    {
      //  缓入
      anim_ticks = anim_ticks + 0.02 * (1.05 - anim_ticks);
      switch (time_ticks)
      {
        case 0:
        {
          tft.drawWideLine(220, 180, 220 + anim_ticks * 50, 180 + anim_ticks * 100, 8, TFT_WHITE, TFT_TRANSPARENT);
          
          if (anim_ticks > 0.2)
          {
            time_ticks++;
          }
          break;
        }
        case 1:
        {
          float base_anim_tick = anim_ticks - 0.2;
          tft.drawWideLine(230, 200, 230 + base_anim_tick * 40, 200 - base_anim_tick * 40, 8, TFT_WHITE, TFT_TRANSPARENT);
          if (anim_ticks > 1)
          {
            time_ticks++;
          }
          break;
        }
        case 2:
        {
          if (anim_ticks > 1.025)
          {
            time_ticks++;
            break;
          }
        }
      }
      if (time_ticks >= 3)
      {
        NeedAllRefresh
        PageYX_Now->page_state_ID++;
        time_ticks = 0;
        anim_ticks = 0;
      }
      break;
    }
  }
}


float l_x;
float l_y;
float last_l_x;
float last_l_y;
float r_x;
float r_y;
float last_r_x;
float last_r_y;

void RmtCtrl_Refresh()
{
  switch (PageYX_Now->page_state_ID)
  {
    case 0:   // 默认进入，刷新两个向量
    {
      // 使用全新的 矫正过的数据来计算摇杆位置
      l_x = (my_joystick.L_X - Left_JoyStick_X_Origin) / (1400.0) * 90;
      l_y = (my_joystick.L_Y - Left_JoyStick_Y_Origin) / (1400.0) * 90;

      r_x = -(my_joystick.R_X - Right_JoyStick_X_Origin) / (1600.0) * 80;
      r_y = (my_joystick.R_Y - Right_JoyStick_Y_Origin) / (1600.0) * 80;


      // 覆盖上一次的
      tft.drawCircle(240 - last_r_x, 140 + last_r_y, 6, TFT_BLACK);
      tft.drawLine(240, 140, 240 - last_r_x, 140 + last_r_y, TFT_BLACK);

      if (last_l_x >= 0)
      {
        tft.drawCircle(240 - sin(last_l_x * PI / 180.0) * 105, 140 + cos(last_l_x * PI / 180.0) * 105, 5, TFT_BLACK);
        tft.drawCircle(240 - sin((180 + last_l_x) * PI / 180.0) * 105, 140 + cos((180 + last_l_x) * PI / 180.0) * 105, 5, TFT_BLACK);

        tft.drawArc(240, 140, 107, 105, 0, (int)last_l_x, TFT_BLACK, TFT_TRANSPARENT);
        tft.drawArc(240, 140, 107, 105, 180, 180 + (int)last_l_x, TFT_BLACK, TFT_TRANSPARENT);
      }
      else
      {
        tft.drawCircle(240 - sin(last_l_x * PI / 180.0) * 105, 140 + cos(last_l_x * PI / 180.0) * 105, 5, TFT_BLACK);
        tft.drawCircle(240 - sin((180 + last_l_x) * PI / 180.0) * 105, 140 + cos((180 + last_l_x) * PI / 180.0) * 105, 5, TFT_BLACK);

        tft.drawArc(240, 140, 107, 105, (int)(360 + last_l_x), 360, TFT_BLACK, TFT_TRANSPARENT);
        tft.drawArc(240, 140, 107, 105, (int)(180 + last_l_x), 180, TFT_BLACK, TFT_TRANSPARENT);
      }
      
      
      
      // tft.drawLine(120, 140, 120 - last_l_x, 140, TFT_BLACK);

      // 刷新本次的
      // 画右摇杆的圈和线
      tft.drawCircle(240 - r_x, 140 + r_y, 6, TFT_WHITE);
      tft.drawLine(240, 140, 240 - r_x, 140 + r_y, TFT_WHITE);

      // 画左摇杆的线和圈
      if (l_x >= 0)
      {
        tft.drawCircle(240 - sin(l_x * PI / 180.0) * 105, 140 + cos(l_x * PI / 180.0) * 105, 5, TFT_WHITE);
        tft.drawCircle(240 - sin((180 + l_x) * PI / 180.0) * 105, 140 + cos((180 + l_x) * PI / 180.0) * 105, 5, TFT_WHITE);

        tft.drawArc(240, 140, 107, 105, 0, (int)l_x, TFT_WHITE, TFT_TRANSPARENT);
        tft.drawArc(240, 140, 107, 105, 180, 180 + (int)l_x, TFT_WHITE, TFT_TRANSPARENT);
      }
      else
      {
        tft.drawCircle(240 - sin(l_x * PI / 180.0) * 105, 140 + cos(l_x * PI / 180.0) * 105, 5, TFT_WHITE);
        tft.drawCircle(240 - sin((180 + l_x) * PI / 180.0) * 105, 140 + cos((180 + l_x) * PI / 180.0) * 105, 5, TFT_WHITE);

        tft.drawArc(240, 140, 107, 105, (int)(360 + l_x), 360, TFT_WHITE, TFT_TRANSPARENT);
        tft.drawArc(240, 140, 107, 105, (int)(180 + l_x), 180, TFT_WHITE, TFT_TRANSPARENT);
      }
      // tft.drawCircle(120 - l_x, 140, 8 * (fabs(l_x) / 60.0 + 0.75), TFT_WHITE);
      // tft.drawLine(120, 140, 120 - l_x, 140, TFT_WHITE);

      // 画左右摇杆的圈
      tft.drawCircle(240, 140, 100, TFT_WHITE);
      tft.drawCircle(240, 140, 20, TFT_WHITE);
      // tft.drawCircle(120, 140, 100, TFT_WHITE);

      last_r_x = r_x;
      last_r_y = r_y;
      last_l_x = l_x;
      last_l_y = l_y;
    }
    break;  

  }
}


bool Keys_preserve[24];
/**
 * 
 */
void PureRmtCtrl_Refresh()
{
  // 使用全新的 矫正过的数据来计算摇杆位置
  l_x = (my_joystick.L_X - Left_JoyStick_X_Origin) / (1400.0) * 90;
  l_y = (my_joystick.L_Y - Left_JoyStick_Y_Origin) / (1400.0) * 90;

  r_x = -(my_joystick.R_X - Right_JoyStick_X_Origin) / (1600.0) * 60;
  r_y = (my_joystick.R_Y - Right_JoyStick_Y_Origin) / (1600.0) * 60;



  /************************     覆盖上一次的    *********************************/
  // 覆盖摇杆信息
  uint8_t pushed_keys = 0;
  for (int i = 0; i < 14; i++)
  {
    if(Keys_preserve[i])
    {
      tft.setTextColor(TFT_BLACK);
      tft.setTextSize(1);
      tft.drawChar( (i < 10 ? i + '0' : (i-10) + 'a'), 5, 5 + 12 * pushed_keys);
      pushed_keys++;
    }
  }

  tft.drawCircle(240 - last_r_x, 140 + last_r_y, 5, TFT_BLACK);
  tft.drawLine(240, 140, 240 - last_r_x, 140 + last_r_y, TFT_BLACK);

  if (last_l_x >= 0)
  {
    tft.drawCircle(240 - sin(last_l_x * PI / 180.0) * 80, 140 + cos(last_l_x * PI / 180.0) * 80, 5, TFT_BLACK);
    tft.drawCircle(240 - sin((180 + last_l_x) * PI / 180.0) * 80, 140 + cos((180 + last_l_x) * PI / 180.0) * 80, 5, TFT_BLACK);

    tft.drawArc(240, 140, 82, 80, 0, (int)last_l_x, TFT_BLACK, TFT_TRANSPARENT);
    tft.drawArc(240, 140, 82, 80, 180, 180 + (int)last_l_x, TFT_BLACK, TFT_TRANSPARENT);
  }
  else
  {
    tft.drawCircle(240 - sin(last_l_x * PI / 180.0) * 80, 140 + cos(last_l_x * PI / 180.0) * 80, 5, TFT_BLACK);
    tft.drawCircle(240 - sin((180 + last_l_x) * PI / 180.0) * 80, 140 + cos((180 + last_l_x) * PI / 180.0) * 80, 5, TFT_BLACK);

    tft.drawArc(240, 140, 82, 80, (int)(360 + last_l_x), 360, TFT_BLACK, TFT_TRANSPARENT);
    tft.drawArc(240, 140, 82, 80, (int)(180 + last_l_x), 180, TFT_BLACK, TFT_TRANSPARENT);
  }




  /************************   刷新本次的    *********************************/ 
  // 画出摇杆信息
  pushed_keys = 0;
  for (int i = 0; i < 14; i++)
  {
    if(my_joystick.Keys[i])
    {
      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(1);
      tft.drawChar( (i < 10 ? i + '0' : (i-10) + 'a'), 5, 5 + 12 * pushed_keys);
      pushed_keys++;
    }
  }
  memcpy(Keys_preserve, my_joystick.Keys, sizeof(my_joystick.Keys));
  

  // 画右摇杆的圈和线
  tft.drawCircle(240 - r_x, 140 + r_y, 5, TFT_WHITE);
  tft.drawLine(240, 140, 240 - r_x, 140 + r_y, TFT_WHITE);

  // 画左摇杆的线和圈
  if (l_x >= 0)
  {
    tft.drawCircle(240 - sin(l_x * PI / 180.0) * 80, 140 + cos(l_x * PI / 180.0) * 80, 5, TFT_WHITE);
    tft.drawCircle(240 - sin((180 + l_x) * PI / 180.0) * 80, 140 + cos((180 + l_x) * PI / 180.0) * 80, 5, TFT_WHITE);

    tft.drawArc(240, 140, 82, 80, 0, (int)l_x, TFT_WHITE, TFT_TRANSPARENT);
    tft.drawArc(240, 140, 82, 80, 180, 180 + (int)l_x, TFT_WHITE, TFT_TRANSPARENT);
  }
  else
  {
    tft.drawCircle(240 - sin(l_x * PI / 180.0) * 80, 140 + cos(l_x * PI / 180.0) * 80, 5, TFT_WHITE);
    tft.drawCircle(240 - sin((180 + l_x) * PI / 180.0) * 80, 140 + cos((180 + l_x) * PI / 180.0) * 80, 5, TFT_WHITE);

    tft.drawArc(240, 140, 82, 80, (int)(360 + l_x), 360, TFT_WHITE, TFT_TRANSPARENT);
    tft.drawArc(240, 140, 82, 80, (int)(180 + l_x), 180, TFT_WHITE, TFT_TRANSPARENT);
  }

  // 画左右摇杆的圈
  tft.drawCircle(240, 140, 75, TFT_WHITE);
  tft.drawCircle(240, 140, 15, TFT_WHITE);
  // tft.drawCircle(120, 140, 100, TFT_WHITE);

  last_r_x = r_x;
  last_r_y = r_y;
  last_l_x = l_x;
  last_l_y = l_y;
}



/**
 * @name Connect_Refresh
 * @brief 连接页面的刷新逻辑
 */
void Connect_Refresh()
{
  switch (PageYX_Now->page_state_ID)
  {
    case 0:   // 默认进入，刷新两个向量
    {
      // 使用全新的 矫正过的数据来计算摇杆位置
      l_x = (my_joystick.L_X - Left_JoyStick_X_Origin) / 2048.0 * 80;
      l_y = (my_joystick.L_Y - Left_JoyStick_Y_Origin) / 2048.0 * 80;

      r_x = (my_joystick.R_X - 2048) / 2048.0 * 80;
      r_y = (my_joystick.R_Y - 2048) / 2048.0 * 80;


      // 覆盖上一次的
      tft.drawCircle(360 - last_r_y, 160 + last_r_x, 8, TFT_BLACK);
      tft.drawLine(360, 160, 360 - last_r_y, 160 + last_r_x, TFT_BLACK);
      tft.drawCircle(120 - last_l_y, 160, 8 * (fabs(last_l_y) / 60.0 + 0.75), TFT_BLACK);
      tft.drawLine(120, 160, 120 - last_l_y, 160, TFT_BLACK);

      // 刷新本次的
      // 画右摇杆的圈和线
      tft.drawCircle(360 - r_y, 160 + r_x, 8, TFT_WHITE);
      tft.drawLine(360, 160, 360 - r_y, 160 + r_x, TFT_WHITE);

      // 画左摇杆的线和圈
      tft.drawCircle(120 - l_y, 160, 8 * (fabs(l_y) / 60.0 + 0.75), TFT_WHITE);
      tft.drawLine(120, 160, 120 - l_y, 160, TFT_WHITE);

      // 画左右摇杆的圈
      tft.drawCircle(360, 160, 100, TFT_WHITE);
      tft.drawCircle(120, 160, 100, TFT_WHITE);

      last_r_x = r_x;
      last_r_y = r_y;
      last_l_x = l_x;
      last_l_y = l_y;
    }
    break;

  }
}

















































/******************     以下是一堆预定义好的动画      ********************/
bool LoadingAnim_Rotate_Rings(bool PageReady)
{
  static int loading_ticks;
  static float radios = 2;
  static float radios_before = radios;

  if (radios < 50 && !PageReady)
  {
    radios += (50 - radios) * 0.1;
  }

  if (PageReady)
  {
    radios += (-2 - radios) * 0.1;
  }
  

  tft.drawArc(240, 160, radios_before, (radios_before - 8 > 8 ? radios_before - 8 : radios_before * 0.25), 
  (20 + loading_ticks - 2) % 360, (100 + loading_ticks - 2) % 360, TFT_BLACK, TFT_BLACK, true);
  tft.drawArc(240, 160, radios, (radios - 8 > 8 ? radios - 8 : radios * 0.25), (20 + loading_ticks) % 360, 
  (100 + loading_ticks) % 360, TFT_WHITE, TFT_BLACK, true);

  tft.drawArc(240, 160, radios_before, (radios_before - 8 > 8 ? radios_before - 8 : radios_before * 0.25), 
  (140 + loading_ticks - 2) % 360, (220 + loading_ticks - 2) % 360, TFT_BLACK, TFT_BLACK, true);
  tft.drawArc(240, 160, radios, (radios - 8 > 8 ? radios - 8 : radios * 0.25), (140 + loading_ticks) % 360, 
  (220 + loading_ticks) % 360, TFT_WHITE, TFT_BLACK, true);

  tft.drawArc(240, 160, radios_before, (radios_before - 8 > 8 ? radios_before - 8 : radios_before * 0.25), 
  (260 + loading_ticks - 2) % 360, (340 + loading_ticks - 2) % 360, TFT_BLACK, TFT_BLACK, true);
  tft.drawArc(240, 160, radios, (radios - 8 > 8 ? radios - 8 : radios * 0.25), (260 + loading_ticks) % 360, 
  (340 + loading_ticks) % 360, TFT_WHITE, TFT_BLACK, true);

  loading_ticks += 2;
  if (loading_ticks > 360)
  {
    loading_ticks = 0;
  }
  
  radios_before = radios;
  if(radios < 0) radios = 0;
  return radios == 0;
}


void Draw_Anim_Arcs(AnimArcBlock* targ_arcblock, float targ_ir, float targ_or)
{
  tft.drawArc(240, 180, targ_arcblock->last_or, targ_arcblock->last_ir, (int)targ_arcblock->angle_start % 360, (int)targ_arcblock->angle_end % 360, TFT_BLACK, TFT_BLACK, true);
  tft.drawArc(240, 180, targ_arcblock->outr, targ_arcblock->inr, (int)targ_arcblock->angle_start % 360, (int)targ_arcblock->angle_end % 360, TFT_WHITE, TFT_BLACK, true);
  targ_arcblock->last_ir = targ_arcblock->inr;
  targ_arcblock->last_or = targ_arcblock->outr;
  targ_arcblock->inr = targ_arcblock->inr + (targ_ir - targ_arcblock->inr) * 0.15;
  targ_arcblock->outr = targ_arcblock->outr + (targ_or - targ_arcblock->outr) * 0.15;
}

void Draw_Anim_Arcs_heng(AnimArcBlock* targ_arcblock, float targ_sangel, float targ_eangel)
{
  tft.drawArc(240, 180, targ_arcblock->outr, targ_arcblock->inr,
   (int)(targ_arcblock->last_angle_start) % 360, (int)(targ_arcblock->last_angle_end) % 360,
    TFT_BLACK, TFT_BLACK, true);


  tft.drawArc(240, 180, targ_arcblock->outr, targ_arcblock->inr,
   (int)(targ_arcblock->angle_start) % 360, (int)(targ_arcblock->angle_end) % 360,
    targ_arcblock->color, TFT_BLACK, true);

  targ_arcblock->last_angle_start = targ_arcblock->angle_start;
  targ_arcblock->last_angle_end = targ_arcblock->angle_end;

  targ_arcblock->angle_start = targ_arcblock->angle_start + (targ_sangel - targ_arcblock->angle_start) * 0.15;
  targ_arcblock->angle_end = targ_arcblock->angle_end + (targ_eangel - targ_arcblock->angle_end) * 0.15;
}

void Draw_Anim_Arcs(AnimArcBlock* targ_arcblock, float targ_sangel, float targ_eangel, float targ_ir, float targ_or)
{

  if ((int)(targ_arcblock->angle_start) == 0 && (int)(targ_arcblock->angle_end) == 360)
  {
    tft.drawArc(240, 180, targ_arcblock->last_or + 1, targ_arcblock->last_ir - 1,
   (int)(targ_arcblock->last_angle_start) % 360, (int)(targ_arcblock->last_angle_end) % 360,
    TFT_BLACK, TFT_BLACK, true);
    
    tft.drawArc(240, 180, targ_arcblock->last_or + 1, targ_arcblock->last_ir - 1,
    0, 360,
    TFT_BLACK, TFT_BLACK, true);


    tft.drawArc(240, 180, targ_arcblock->outr, targ_arcblock->inr,
    0, 360,
    targ_arcblock->color, TFT_BLACK, true);

    targ_arcblock->last_angle_start = 0;
    targ_arcblock->last_angle_end = 360;
  }
  else
  {
    if ((int)(targ_arcblock->last_angle_start) == 0 && (int)(targ_arcblock->last_angle_end) == 360)
    {
      tft.drawArc(240, 180, targ_arcblock->last_or + 1, targ_arcblock->last_ir - 1,
      (int)(targ_arcblock->last_angle_start) % 360, (int)(targ_arcblock->last_angle_end) % 360,
      TFT_BLACK, TFT_BLACK, true);
      tft.drawArc(240, 180, targ_arcblock->last_or + 1, targ_arcblock->last_ir - 1,
      0, 360,
      TFT_BLACK, TFT_BLACK, true);
    }

    tft.drawArc(240, 180, targ_arcblock->last_or + 1, targ_arcblock->last_ir - 1,
   (int)(targ_arcblock->last_angle_start) % 360, (int)(targ_arcblock->last_angle_end) % 360,
    TFT_BLACK, TFT_BLACK, true);

    tft.drawArc(240, 180, targ_arcblock->outr, targ_arcblock->inr,
   (int)(targ_arcblock->angle_start) % 360, (int)(targ_arcblock->angle_end) % 360,
    targ_arcblock->color, TFT_BLACK, true);

    targ_arcblock->last_angle_start = targ_arcblock->angle_start;
    targ_arcblock->last_angle_end = targ_arcblock->angle_end;
  }
  

  targ_arcblock->last_ir = targ_arcblock->inr;
  targ_arcblock->last_or = targ_arcblock->outr;
  
  targ_arcblock->inr = targ_arcblock->inr + (targ_ir - targ_arcblock->inr) * 0.15;
  targ_arcblock->outr = targ_arcblock->outr + (targ_or - targ_arcblock->outr) * 0.15;

  
  targ_arcblock->angle_start = targ_arcblock->angle_start + (targ_sangel - targ_arcblock->angle_start) * 0.15;
  targ_arcblock->angle_end = targ_arcblock->angle_end + (targ_eangel - targ_arcblock->angle_end) * 0.15;
}


bool Check_Anim_Arcs(AnimArcBlock* targ_arcblock, float targ_ir, float targ_or)
{
  return (targ_or - targ_arcblock->outr < 0.35) && (targ_ir - targ_arcblock->inr < 0.35);
}

bool Check_Anim_Arcs(AnimArcBlock* targ_arcblock, float targ_sa, float targ_ea, float targ_ir, float targ_or)
{
  return (targ_or - targ_arcblock->outr < 0.35) && (targ_ir - targ_arcblock->inr < 0.35) 
  && (fabs(targ_sa - targ_arcblock->angle_start) < 0.35) && (fabs(targ_ea - targ_arcblock->angle_end) < 0.35);
}







// 左：上下左右。右：上下左右。左按钮右按钮
void estimate_userInput(bool user_actions[], JoyStick_State* my_stick)
{
  for (int i = 0; i < 16; i++)
  {
    user_actions[i] = false;
  }
  
  if (my_stick->L_X < 1000) user_actions[LJs_Right] = true;    // L右
  if (my_stick->L_X > 3000) user_actions[LJs_Left] = true;  // L左
  if (my_stick->L_Y < 1000) user_actions[LJs_Down] = true;  
  if (my_stick->L_Y > 3000) user_actions[LJs_Up] = true;
  

  if (my_stick->R_X < 1000) user_actions[RJs_Left] = true;          // R左
  if (my_stick->R_X > 3000) user_actions[RJs_Right] = true;         // R右
  if (my_stick->R_Y < 1000) user_actions[RJs_Up] = true;            
  if (my_stick->R_Y > 3000) user_actions[RJs_Down] = true;          


  if (my_stick->L_But == 0) user_actions[LJs_Pressed] = true;
  if (my_stick->R_But == 0) user_actions[RJs_Pressed] = true;
}



ElementYX* Menu_SlideUpButton(ElementYX* button_now)
{
  button_now->last_elem->Set_Pos(240, 60);
  button_now->last_elem->Set_Size(0, 0);
  vTaskDelay(48);
  button_now->Set_Pos(240, 60);
  button_now->Set_Size(89, 40);
  vTaskDelay(48);
  button_now->next_elem->Set_Pos(240, 160);
  button_now->next_elem->Set_Size(220, 100);
  vTaskDelay(48);

  if (button_now->next_elem->next_elem->h_now <= 0.49)
  {
    button_now->next_elem->next_elem->Tele_To_Pos(240, 260);
  }
  else
  {
    button_now->next_elem->next_elem->Set_Pos(240, 260);
  }
  button_now->next_elem->next_elem->Set_Size(89, 40);

  return button_now->next_elem;
}



ElementYX* Menu_SlideDownButton(ElementYX* button_now)
{
  button_now->next_elem->Set_Pos(240, 260);
  button_now->next_elem->Set_Size(0, 0);
  vTaskDelay(48);
  button_now->Set_Pos(240, 260);
  button_now->Set_Size(89, 40);
  vTaskDelay(48);
  button_now->last_elem->Set_Pos(240, 160);
  button_now->last_elem->Set_Size(220, 100);
  vTaskDelay(48);

  if (button_now->last_elem->last_elem->h_now <= 0.49)
  {
    button_now->last_elem->last_elem->Tele_To_Pos(240, 60);
  }
  else
  {
    button_now->last_elem->last_elem->Set_Pos(240, 60);
  }

  button_now->last_elem->last_elem->Set_Size(89, 40);
  return button_now->last_elem;
}


void Menu_PressingButton(ElementYX* button_now)
{
  button_now->color = Widget_Grey;      // 灰色（被按下了）
  button_now->widget_pressed = true;
  button_now->NeedUpdate = true;
  vTaskDelay(48);
}

ElementYX* Menu_ReleaseButtonOpen(ElementYX* button_now)
{
  if (button_now->widget_pressed)     // 说明是释放的那个情况
  {
    button_now->color = Widget_White;
    button_now->widget_pressed = false;
    button_now->NeedUpdate = true;
    
    /******************   如果这是一个面板，让他展开     *******************/
    if (button_now->elem_type == Panel)
    {
      button_now->last_elem->Set_Pos(240, 60);
      button_now->last_elem->Set_Size(0, 0);
      button_now->next_elem->Set_Pos(240, 260);
      button_now->next_elem->Set_Size(0, 0);

      button_now->Set_Pos(240, 160);
      button_now->Set_Size(45, 20);
      vTaskDelay(80);
      button_now->Set_Pos(240, 160);
      button_now->Set_Size(460, 300);

      button_now->panel_expanded = true;
      vTaskDelay(48);
    }

    /******************   如果这是一个按钮，进入新的页面     *******************/
    else if (button_now->elem_type == Button)
    {
      button_now->Tele_To_Size(0, 0);
      button_now->next_elem->Tele_To_Size(0, 0);
      button_now->last_elem->Tele_To_Size(0, 0);

      PageYX_Now = button_now->targ_page;

      NeedAllRefresh;

      if (PageYX_Now->Get_TargID_Elem(1) != NULL)
      {
        button_now = PageYX_Now->Get_TargID_Elem(1);
        button_now->last_elem->Tele_To_Pos(240, 60);
        button_now->last_elem->Set_Size(89, 40);

        button_now->Tele_To_Pos(240, 160);
        button_now->Set_Size(220, 100);

        button_now->next_elem->Tele_To_Pos(240, 260);
        button_now->next_elem->Set_Size(89, 40);
      }
      vTaskDelay(500);
      PageYX_Now->pageReady = true;
    }
  }

  return button_now;
}


void Menu_ReleaseButtonClose(ElementYX* button_now)
{
  if (button_now->widget_pressed)     // 说明是释放的那个情况
  {
    button_now->color = Widget_White;
    button_now->widget_pressed = false;
    button_now->NeedUpdate = true;
    
    /******************   如果这是一个面板，让他关闭     *******************/
    if (button_now->elem_type == Panel)
    {
      button_now->Set_Pos(240, 160);
      button_now->Set_Size(560, 360);
      vTaskDelay(80);
      button_now->Set_Pos(240, 160);
      button_now->Set_Size(220, 100);

      button_now->last_elem->Set_Pos(240, 60);
      button_now->last_elem->Set_Size(89, 40);
      button_now->next_elem->Set_Pos(240, 260);
      button_now->next_elem->Set_Size(89, 40);

      button_now->panel_expanded = false;
      vTaskDelay(48);
    }
  }
}

int panel_x_intg = 0;
int panel_y_intg = 0;
bool DragedButton;
float Geoge_Ave(float a, float b)
{
  return sqrt(a*a+b*b);
}

void Menu_DragDownButton(ElementYX* button_now)
{
  DragedButton = true;
  panel_y_intg += 1;
  vTaskDelay(15);
}

void Menu_DragUpButton(ElementYX* button_now)
{
  DragedButton = true;
  panel_y_intg -= 1;
  
  vTaskDelay(15);
}

void Menu_DragLeftButton(ElementYX* button_now)
{
  DragedButton = true;
  panel_x_intg -= 1;
  vTaskDelay(15);
}

bool Menu_IsDragButtonOK(ElementYX* button_now)
{
  return (panel_x_intg *  panel_x_intg) + (panel_y_intg * panel_y_intg) > 625;
}

ElementYX* Menu_DragButtonClose(ElementYX* button_now)
{
  
  int state;
  if(abs(panel_x_intg) > abs(panel_y_intg))
  {
    state = 0;    // 左退出
  }
  else if (abs(panel_x_intg) <= abs(panel_y_intg) && panel_y_intg > 0)
  {
    state = 1;    // 下退出
  }
  else
  {
    state = 2;    // 上退出
  }
  

  panel_x_intg = 0;
  panel_y_intg = 0;
  button_now->panel_expanded = false;

  switch (state)
  {
    case 0:
    {
      button_now->last_elem->Set_Pos(240, 60);
      button_now->last_elem->Set_Size(89, 40);
      vTaskDelay(48);
      button_now->Set_Pos(240, 160);
      button_now->Set_Size(220, 100);
      vTaskDelay(48);
      button_now->next_elem->Set_Pos(240, 260);
      button_now->next_elem->Set_Size(89, 40);
      return button_now;
      break;
    }

    case 1:
    {
      button_now->next_elem->Set_Pos(240, 260);
      button_now->next_elem->Set_Size(0, 0);

      vTaskDelay(48);
      button_now->Set_Pos(240, 260);
      button_now->Set_Size(89, 40);
      vTaskDelay(48);
      button_now->last_elem->Set_Pos(240, 160);
      button_now->last_elem->Set_Size(220, 100);
      vTaskDelay(48);

      button_now->last_elem->last_elem->Tele_To_Pos(240, 60);
      button_now->last_elem->last_elem->Set_Size(89, 40);
      return button_now->last_elem;
      break;
    }

    case 2:
    {
      button_now->last_elem->Set_Pos(240, 260);
      button_now->last_elem->Set_Size(0, 0);

      vTaskDelay(48);
      button_now->Set_Pos(240, 60);
      button_now->Set_Size(89, 40);
      vTaskDelay(48);
      button_now->next_elem->Set_Pos(240, 160);
      button_now->next_elem->Set_Size(220, 100);
      vTaskDelay(48);
      button_now->next_elem->next_elem->Tele_To_Pos(240, 260);
      button_now->next_elem->next_elem->Set_Size(89, 40);
      return button_now->next_elem;
      break;
    }
  }
}

bool Menu_DragedButtonThisTime(ElementYX* button_now)
{
  if (DragedButton && button_now->panel_expanded)
  {
    button_now->Set_Pos(240 + panel_x_intg * 2, 160 + panel_y_intg * 2);
    button_now->Set_Size(460 - Geoge_Ave(panel_x_intg, panel_y_intg) * 2, 300 - Geoge_Ave(panel_x_intg, panel_y_intg) * 2);
  }
  
  if (DragedButton)
  {
    DragedButton = false;
    return true;
  }
  return false;
}

void Menu_ClearDragButton(ElementYX* button_now)
{
  if (button_now->panel_expanded)
  {
    button_now->Set_Pos(240, 160);
    button_now->Set_Size(460, 300);
    panel_x_intg = 0;
    panel_y_intg = 0;
    vTaskDelay(15);
  }
}

ElementYX* To_MainMenu()
{
  ElementYX* button_now = MainMenu_Page.Get_TargID_Elem(0);
  button_now->last_elem->Tele_To_Pos(240, 60);
  button_now->last_elem->Set_Size(89, 40);
  button_now->Tele_To_Pos(240, 160);
  button_now->Set_Size(220, 100);
  button_now->next_elem->Tele_To_Pos(240, 260);
  button_now->next_elem->Set_Size(89, 40);
  PageYX_Now = &MainMenu_Page;
  NeedAllRefresh;
  return button_now;
}

void Add_ZigbeeNode(NodeInfo NodeList[], NodeInfo newNode)
{
  // 遍历列表，确认没有这个节点存在
  for (int i = 0; i < 4; i++)
  {
    if(NodeList[i].addr == newNode.addr)    // 出现重复，直接退出
    {
      return;
    }
  }
   
  // 找到第一个为 0的节点，把新节点载入
  int z_i = 0;
  for(z_i = 0; z_i < 4; z_i++)
  {
    if (NodeList[z_i].addr == 0)
    {
      break;
    }
  }
  if(z_i < 4)
  {
    NodeList[z_i].addr = newNode.addr;
    strcpy(NodeList[z_i].name, newNode.name);
  }
}