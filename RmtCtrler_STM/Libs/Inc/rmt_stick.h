#ifndef RMT_STICK_H
#define RMT_STICK_H

#include "stm32f4xx_hal.h"
#include "math.h"
#define MSG_LENGTH 12

#define L_x_adc_Channel 0
#define L_y_adc_Channel 1

#define R_x_adc_Channel 2
#define R_y_adc_Channel 3



/// @brief 遥控器数据结构体
/// @param left_stick_x 左摇杆X值，取值为 -2048~2048
/// @param left_stick_x_float 左摇杆X值，取值换算为-1~1的float（不会实际在串口中传输）
typedef struct rmt_stick
{
    uint16_t msg_id_mark;       // 发送信息的记号ID

    int16_t left_stick_x;
    int16_t left_stick_y;

    int16_t right_stick_x;
    int16_t right_stick_y;

    float left_stick_x_perc;
    float left_stick_y_perc;
    float right_stick_x_perc;
    float right_stick_y_perc;

    uint8_t L_But;
    uint8_t R_But;
    

    // int signal_delay_ms;        // 遥控延迟ms
    // int signal_delay_ms_temp;        // 遥控延迟ms_temp
    // uint16_t signal_id;              // 用于计算延迟的特殊帧的帧id
}RmtJoystickInfo;

typedef struct rmt_stick_origins
{
    uint16_t left_stick_x_low;
    uint16_t left_stick_x_high;
    uint16_t left_stick_y_low;
    uint16_t left_stick_y_high;

    uint16_t right_stick_x_low;
    uint16_t right_stick_x_high;
    uint16_t right_stick_y_low;
    uint16_t right_stick_y_high;

    uint16_t left_stick_x_mid;
    uint16_t left_stick_y_mid;
    uint16_t right_stick_x_mid;
    uint16_t right_stick_y_mid;

}RmtJsOriginInfo;


typedef enum
{
    RMTS_UNINIT,            // 未初始化
    RMTS_NORMAL,            // 普通状态
    RMTS_CONTROL,           // 遥控器遥控状态  
    RMTS_CONNECT,           // 遥控器遥控状态  
    RMTS_STOP,              // 遥控器急停状态
    RMTS_SEARCH,            // 遥控器搜索状态
}RMTS_STATUS;

void encode_joystick_msg(RmtJoystickInfo* js_info, uint8_t* encode_result_msg);
void encode_reply_msg(uint8_t* msg_recved, uint8_t* encode_result_msg);
int rmt_recv_complete_msg(uint8_t *msg_recv);

void rmt_decode_joystick_msg(RmtJsOriginInfo* js_info, uint8_t msg_to_decode[], uint8_t id);

int rmt_check_rmts_status(char status_code[], RMTS_STATUS targ_status);
void rmt_Joystick_Reflect(RmtJoystickInfo *js_info);
void rmt_Joystick_GetPercent(RmtJoystickInfo *js_info);

void rmt_Reverse(RmtJoystickInfo *js_info);

int16_t rmt_pow_int16(int16_t a, float pownum);
void rmt_load_JsBuffer(uint8_t JsSends_datas[],RmtJoystickInfo My_joystick);
void rmt_load_JsBuffer_addr(uint8_t JsSends_datas[], RmtJoystickInfo My_joystick, uint16_t addr);

extern int valid_info;
extern int lost_info;

extern RmtJoystickInfo My_joystick;
extern RmtJsOriginInfo My_js_origins;

extern uint8_t rmt_Has_origins;

extern uint8_t reverse_r_y;
extern uint8_t reverse_r_x;
extern uint8_t reverse_l_y;
extern uint8_t reverse_l_x;

extern uint16_t l_stick_x_origin;
extern uint16_t l_stick_y_origin;
extern uint16_t r_stick_x_origin;
extern uint16_t r_stick_y_origin;

#endif