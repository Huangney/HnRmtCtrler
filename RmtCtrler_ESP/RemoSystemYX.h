#ifndef REMOSYS_YX_H
#define REMOSYS_YX_H

#include <string>
#include <list>
#include <TFT_eSPI.h>
#include "UartTransmitYX.h"
using namespace std;

// #define fpsFree
#define fps60
// #define fps30
// #define fps15
#define MYTFT_WIDTH 480
#define MYTFT_HEIGHT 320

#define DefualtID 9213187

#define HEIGHT 240
#define WIDTH 320
#define NeedAllRefresh BlackRefresh = true;

#define SCRCENTER_X WIDTH / 2
#define SCRCENTER_Y HEIGHT / 2

#define PIX4_BLACK 0
#define PIX4_WHITE 9
#define PIX4_TRANSPARENT 14
#define TFT_LIGHTGREEN 0xa7f4

class PageYX;
class ElementYX;

typedef enum ElementType
{
  Button,
  Panel,
  Bar,
  Switcher,       // 多选项滑条
  Booler,     // 二元按键
  Text,
  ParamText,
}ElementType;

typedef enum Joys_4Div
{
  Joys_4Div_Up,
  Joys_4Div_Right,
  Joys_4Div_Down,
  Joys_4Div_Left,

  RJoys_4Div_Up = 2,
  RJoys_4Div_Right = 3,
  RJoys_4Div_Down = 0,
  RJoys_4Div_Left = 1,
}Joys_4Div;

typedef enum Joys_8Div
{
  Joys_8Div_Up,
  Joys_8Div_UpR,
  Joys_8Div_Right,
  Joys_8Div_DownR,
  Joys_8Div_Down,
  Joys_8Div_DownL,
  Joys_8Div_Left,
  Joys_8Div_UpL,
}Joys_8Div;

typedef enum InnerPanelType
{
  PanelSwitch,
  PanelLevel,
  PanelSlider,
}InnerPanelType;

typedef enum Widget_Color
{
  Widget_White,
  Widget_Grey,
  Widget_Red,
  Widget_Green,
}Widget_Color;

typedef enum PageType
{
  MenuPage,
  CustomPage,
}Page_Type;

typedef enum AnimType
{
  NULLANIM,
  RmtCtrlANIM,
}AnimType;

class ElementYX
{
  public:
    int Elem_id;

    ElementYX(int id);

    ElementYX()
    {
    }

    ElementYX(int id, int x, int y);
    ElementYX(int id, int x, int y, char targ_title[]);
    ElementYX(int id, int x, int y, char targ_title[], PageYX* page);
    ElementYX(int id, int x, int y, char targ_title[], ElementType elemtype);

    void Set_Pos(int x, int y);   // 中心位置
    void Tele_To_Pos(int x, int y);   // 中心位置
    void Set_Size(int w, int h);
    void Tele_To_Size(int w, int h);
    void Set_ChildID(int childElemId);
    void Init_BarVar(float* TargVarP, float MaxValue);
    void Update_BarVar();

    void To_Targ_State(float Fps_Gain);
    bool At_Targ_State();
    bool IsExPanel();

    ElementYX* last_elem;
    ElementYX* next_elem;

    float x_now;
    float y_now;
    float w_now;
    float h_now;

    bool visible = false;
    Widget_Color color = Widget_White;
    bool NeedUpdate;
    bool PanelUpdate;

    int superElemId = DefualtID;      // 父控件的ID
    int childElemId = DefualtID;      // 子控件的ID

    float* LinkedFloatP;

    float value_0;
    float value_1;

    bool panel_expanded;
    bool widget_pressed;
    
    PageYX* targ_page;
    PageYX* at_page;

    ElementType elem_type;

    char title[10];
    string text;

    float x_targ;
    float y_targ;
    float w_targ;
    float h_targ;
};


class PageYX
{
  public:
    int Page_id;
    Page_Type page_type;
    int Elem_Nums = 0;
    ElementYX* Elems_List[12];    // 一个页面最多12个控件
    AnimType openAnimType = NULLANIM;        // 进入页面的初始动画
    bool pageReady;               // 页面是否准备好
    int page_state_ID;        // 页面的状态码，没有规定使用方式，可根据自己喜好使用（比如一个页面有很多个步骤，或者多个控件，可以用这个来刷新）

    void Add_Elem(ElementYX &targ_elem);
    void Add_NonLinkElem(ElementYX &targ_elem);
    ElementYX* Get_TargID_Elem(int targ_id);
    
};

typedef struct AnimArcBlock
{
  float last_ir;
  float last_or;
  float inr;
  float outr;
  float last_angle_start;
  float last_angle_end;
  float angle_start;
  float angle_end;
  int color;
}AnimArcBlock;


typedef enum User_Inputs
{
  LJs_Up,
  LJs_Down,
  LJs_Left,
  LJs_Right,
  RJs_Up,
  RJs_Down,
  RJs_Left,
  RJs_Right,
  LJs_Pressed,
  RJs_Pressed,
  EmptyInput,
}User_Inputs;



void Init_SysYX();
void ScreenRefresh_Handler();

/*****    自定义工具函数    *****/
float Geoge_Ave(float a, float b);
int RGB888_2_565(uint8_t R, uint8_t G, uint8_t B);


/*****    自定义刷新函数    *****/
void Page_Handler();
void Calib_Refresh();
void RmtCtrl_Refresh();
void PureRmtCtrl_Refresh();


bool LoadingAnim_Rotate_Rings(bool PageReady);
void estimate_userInput(bool user_actions[], JoyStick_State* my_stick);
void Draw_Anim_Arcs(AnimArcBlock* targ_arcblock, float targ_ir, float targ_or);
bool Check_Anim_Arcs(AnimArcBlock* targ_arcblock, float targ_ir, float targ_or);
bool Check_Anim_Arcs(AnimArcBlock* targ_arcblock, float targ_sa, float targ_ea, float targ_ir, float targ_or);
void Draw_Anim_Arcs_heng(AnimArcBlock* targ_arcblock, float targ_sangel, float targ_eangel);
void Draw_Anim_Arcs(AnimArcBlock* targ_arcblock, float targ_sangel, float targ_eangel, float targ_ir, float targ_or);
/****************       系统响应用函数        ****************/
ElementYX* Menu_SlideUpButton(ElementYX* button_now);
ElementYX* Menu_SlideDownButton(ElementYX* button_now);
void Menu_PressingButton(ElementYX* button_now);
ElementYX* Menu_ReleaseButtonOpen(ElementYX* button_now);
void Menu_ReleaseButtonClose(ElementYX* button_now);

void Menu_DragDownButton(ElementYX* button_now);
void Menu_DragUpButton(ElementYX* button_now);
void Menu_DragLeftButton(ElementYX* button_now);
bool Menu_IsDragButtonOK(ElementYX* button_now);
ElementYX* Menu_DragButtonClose(ElementYX* button_now);
bool Menu_DragedButtonThisTime(ElementYX* button_now);
void Menu_ClearDragButton(ElementYX* button_now);

ElementYX* To_MainMenu();

/****************       扩展变量        ****************/
extern bool BlackRefresh;
extern int PanelOutIntg;

extern int fps_print_flag;
extern int fps_for_print;
extern int fps_for_print_preserve;
extern int FpsDelayTimeBias;
extern float DynamicFpsSetting;

extern TFT_eSPI tft;
extern PageYX Test_Page;

extern TFT_eSprite white_spr;
extern TFT_eSprite black_spr;

extern Page_Type page_now;
extern User_Inputs input_now;
extern ElementYX* button_focoused;
extern PageYX* PageYX_Now;
extern PageYX* PagePointers[24];

extern float BatVolt;   // 电池电压
extern float VBUSVolt;   // 电池电压
extern float bat_volt_precent;   // 电池电压
extern float bat_volt_print;   // 电池电压
extern bool IsCharging;   // 充电中

extern char stm_real_state[5];
extern char stm_pure[16];

extern uint32_t Left_JoyStick_4_Divisions[4];
extern uint32_t Right_JoyStick_4_Divisions[4];
extern uint16_t JoyStickPushingCounting[4];    // 推动摇杆的计时
extern bool JoyStick_8_SampleOK[8];

extern uint16_t Left_JoyStick_X_Origin;
extern uint16_t Left_JoyStick_Y_Origin;
extern uint16_t Left_JoyStick_X_Wave;
extern uint16_t Left_JoyStick_Y_Wave;

extern uint16_t Right_JoyStick_X_Origin;
extern uint16_t Right_JoyStick_Y_Origin;
extern uint16_t Right_JoyStick_X_Wave;
extern uint16_t Right_JoyStick_Y_Wave;


void Add_ZigbeeNode(NodeInfo NodeList[], NodeInfo newNode);
extern NodeInfo MyFavNode;
extern NodeInfo FindNodes[4];   // 最多显示4个节点

#endif