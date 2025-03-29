#include "UartTransmitYX.h"
#include "RemoSystemYX.h"
JoyStick_State my_joystick;
bool Pure_Control_Mode = true;


// 反向转换函数，将一个 uint8_t 类型的字节转换为长度为 8 的 uint8_t 数组
void byteToArray(uint8_t byte, bool array[8])
{
    for (int i = 0; i < 8; i++) {
        array[i] = (byte >> i) & 1;
    }
} 


/// @brief 
/// @param rxBuffs 默认长度为 12 
void Decode_Uart_Msg(uint8_t rxBuffs[])
{
    if (rxBuffs[0] == '#' && rxBuffs[1] == 'j' && rxBuffs[15] == '#')     // 传来的是有效的摇杆数据
    {
        my_joystick.L_X = rxBuffs[2] * 256 + rxBuffs[3];
        my_joystick.L_Y = rxBuffs[4] * 256 + rxBuffs[5];
        my_joystick.R_X = rxBuffs[6] * 256 + rxBuffs[7];
        my_joystick.R_Y = rxBuffs[8] * 256 + rxBuffs[9];
        uint8_t byte_0, byte_1, byte_2;
        byte_0 = rxBuffs[10];
        byte_1 = rxBuffs[11];
        byte_2 = rxBuffs[12];

        byteToArray(byte_0, my_joystick.Keys);
        byteToArray(byte_1, my_joystick.Keys + 8);
        byteToArray(byte_2, my_joystick.Keys + 16);
    }
    else if (rxBuffs[0] == '#' && rxBuffs[1] == 'n' && rxBuffs[11] == '#')     // 传来的是有效的节点
    {
        NodeInfo the_node;
        the_node.addr = rxBuffs[2] << 8 | rxBuffs[3];
        memcpy(the_node.name, rxBuffs + 4, 7);
        Add_ZigbeeNode(FindNodes, the_node);        // 加入到节点列表
    }
    else if (rxBuffs[0] == 'm')     // 传来的是 状态码反馈
    {
        memcpy(stm_real_state, rxBuffs, 4);
        // Serial.printf("state:%s\n", stm_real_state);
    }
    else if (rxBuffs[0] == '#' && rxBuffs[1] == 'd')     // 传来的是 调试模式启动
    {
        memcpy(stm_pure, rxBuffs, sizeof(stm_pure));
        // Serial.printf("%s\n", rxBuffs);
    }
}

   