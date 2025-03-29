#ifndef UART_TRANSMIT_YX_H
#define UART_TRANSMIT_YX_H
#include "Arduino.h"

typedef enum
{
  
};

void Decode_Uart_Msg(uint8_t rxBuffs[]);

class JoyStick_State
{
    public:
        uint16_t L_X;
        uint16_t L_Y;
        uint16_t R_X;
        uint16_t R_Y;
        bool L_But;
        bool R_But;
        bool Keys[24];
};

class NodeInfo
{
    public:
        uint16_t addr;
        char name[7];
        bool drawed;
};

extern JoyStick_State my_joystick;
extern bool Pure_Control_Mode;

#endif