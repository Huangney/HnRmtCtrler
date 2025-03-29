#include "rmt_stick.h"

RmtJsOriginInfo My_js_origins;
RmtJoystickInfo My_joystick;

uint16_t l_stick_x_origin = 2048;
uint16_t l_stick_y_origin = 2048;
uint16_t r_stick_x_origin = 2048;
uint16_t r_stick_y_origin = 2048;

uint8_t reverse_r_y = 1;
uint8_t reverse_r_x = 0;
uint8_t reverse_l_y = 0;
uint8_t reverse_l_x = 1;

uint8_t rmt_Has_origins = 0;

float rmt_pow_float(float a, float pownum);

void encode_joystick_msg(RmtJoystickInfo *js_info, uint8_t *encode_result_msg)
{
    // js_info->left_stick_x_float = js_info->left_stick_x / 2048.0;
    encode_result_msg[0] = 'J'; // 开始标识符

    encode_result_msg[1] = js_info->msg_id_mark >> 8;
    encode_result_msg[2] = js_info->msg_id_mark & 0xff;

    encode_result_msg[3] = js_info->left_stick_x >> 8;
    encode_result_msg[4] = js_info->left_stick_x & 0xff;
    encode_result_msg[5] = js_info->left_stick_y >> 8;
    encode_result_msg[6] = js_info->left_stick_y & 0xff;

    encode_result_msg[7] = js_info->right_stick_x >> 8;
    encode_result_msg[8] = js_info->right_stick_x & 0xff;
    encode_result_msg[9] = js_info->right_stick_y >> 8;
    encode_result_msg[10] = js_info->right_stick_y & 0xff;


    encode_result_msg[MSG_LENGTH - 1] = 'S'; // 结束标识符

    js_info->msg_id_mark++;
}

void encode_reply_msg(uint8_t *msg_recved, uint8_t *encode_result_msg)
{
    encode_result_msg[0] = msg_recved[1];
    encode_result_msg[1] = msg_recved[2];
}

int rmt_check_rmts_status(char status_code[], RMTS_STATUS targ_status)
{
    switch (targ_status)
    {
        case RMTS_UNINIT:
        {
            if (status_code[0] == '?' && status_code[1] == '?' && status_code[2] == '?')
            {
                return 1;
            }
            else return 0;
        }
        break;
        case RMTS_CONTROL:
        {
            if (status_code[0] == '1' && status_code[1] == '0' && status_code[2] == '1')
            {
                return 1;
            }
            else return 0;
        }
        break;
        case RMTS_NORMAL:
        {
            if (status_code[0] == '2' && status_code[1] == '1' && status_code[2] == '2')
            {
                return 1;
            }
            else return 0;
        }
        case RMTS_SEARCH:
        {
            if (status_code[0] == '5' && status_code[1] == '0' && status_code[2] == '1')
            {
                return 1;
            }
            else return 0;
        }
        break;
    }
    return 0;
}

int rmt_recv_complete_msg(uint8_t *msg_recv)
{
    if (msg_recv[MSG_LENGTH - 1] == 'S')
    {
        return 1;
    }
    else
        return 0;
}

void rmt_decode_joystick_msg(RmtJsOriginInfo *js_info, uint8_t msg_to_decode[], uint8_t id)
{
    switch (id)
    {
        case 0:
        {
            js_info->left_stick_x_low = msg_to_decode[2] * 256 + msg_to_decode[3];
            break;
        }
        case 1:
        {
            js_info->left_stick_x_high = msg_to_decode[2] * 256 + msg_to_decode[3];
            break;
        }
        case 2:
        {
            js_info->left_stick_y_low = msg_to_decode[2] * 256 + msg_to_decode[3];
            break;
        }
        case 3:
        {
            js_info->left_stick_y_high = msg_to_decode[2] * 256 + msg_to_decode[3];
            break;
        }

/********************************************************************************************** */

        case 4:
        {
            js_info->right_stick_x_low = msg_to_decode[2] * 256 + msg_to_decode[3];
            break;
        }
        case 5:
        {
            js_info->right_stick_x_high = msg_to_decode[2] * 256 + msg_to_decode[3];
            break;
        }
        case 6:
        {
            js_info->right_stick_y_low = msg_to_decode[2] * 256 + msg_to_decode[3];
            break;
        }
        case 7:
        {
            js_info->right_stick_y_high = msg_to_decode[2] * 256 + msg_to_decode[3];
            break;
        }

/********************************************************************************************** */

        case 8:
        {
            js_info->left_stick_x_mid = msg_to_decode[2] * 256 + msg_to_decode[3];
            break;
        }
        case 9:
        {
            js_info->left_stick_y_mid = msg_to_decode[2] * 256 + msg_to_decode[3];
            break;
        }
        case 10:
        {
            js_info->right_stick_x_mid = msg_to_decode[2] * 256 + msg_to_decode[3];
            break;
        }
        case 11:
        {
            js_info->right_stick_y_mid = msg_to_decode[2] * 256 + msg_to_decode[3];
            rmt_Has_origins = 1;
            break;
        }

    default:
        break;
    }
    
}

/**
 * @name Joystick_Reflect
 * @brief 摇杆映射
 */
void rmt_Joystick_Reflect(RmtJoystickInfo *js_info)
{
    if (rmt_Has_origins)
    {
        js_info->left_stick_x_perc = rmt_pow_float(js_info->left_stick_x_perc, 1.4);
        js_info->left_stick_y_perc = rmt_pow_float(js_info->left_stick_y_perc, 1.4);
        js_info->right_stick_x_perc = rmt_pow_float(js_info->right_stick_x_perc, 1.4);
        js_info->right_stick_y_perc = rmt_pow_float(js_info->right_stick_y_perc, 1.4);
    }
    // js_info->left_stick_x = rmt_pow_int16(js_info->left_stick_x, 1.5);
    // js_info->left_stick_y = rmt_pow_int16(js_info->left_stick_y, 1.5);
    // js_info->right_stick_x = rmt_pow_int16(js_info->right_stick_x, 1.5);
    // js_info->right_stick_y = rmt_pow_int16(js_info->right_stick_y, 1.5);
}

void rmt_Reverse(RmtJoystickInfo *js_info)
{
    if (reverse_l_x)
    {
        js_info->left_stick_x *= -1;
    }
    if (reverse_l_y)
    {
        js_info->left_stick_y *= -1;
    }
    if (reverse_r_x)
    {
        js_info->right_stick_x *= -1;
    }
    if (reverse_r_y)
    {
        js_info->right_stick_y *= -1;
    }
}

void rmt_load_JsBuffer(uint8_t JsSends_datas[], RmtJoystickInfo My_joystick)
{ 
    int16_t temp_lx;
    int16_t temp_ly;
    int16_t temp_rx;
    int16_t temp_ry;

    if (rmt_Has_origins)        // 放缩到-5000 ~ 5000
    {
        temp_lx = (int16_t)(My_joystick.left_stick_x_perc * 5000);
        temp_ly = (int16_t)(My_joystick.left_stick_y_perc * 5000);
        temp_rx = (int16_t)(My_joystick.right_stick_x_perc * 5000);
        temp_ry = (int16_t)(My_joystick.right_stick_y_perc * 5000);
    }
    else
    {
        temp_lx = (int16_t)(My_joystick.left_stick_x / 2048.0 * 5000);
        temp_ly = (int16_t)(My_joystick.left_stick_y / 2048.0 * 5000);
        temp_rx = (int16_t)(My_joystick.right_stick_x / 2048.0 * 5000);
        temp_ry = (int16_t)(My_joystick.right_stick_y / 2048.0 * 5000);
    }

    // LX
    JsSends_datas[0] = temp_lx >> 8;
    JsSends_datas[1] = temp_lx & 0xff;
    // LY
    JsSends_datas[2] = temp_ly >> 8;
    JsSends_datas[3] = temp_ly & 0xff;
    // RX
    JsSends_datas[4] = temp_rx >> 8;
    JsSends_datas[5] = temp_rx & 0xff;
    // RY
    JsSends_datas[6] = temp_ry >> 8;
    JsSends_datas[7] = temp_ry & 0xff;
    // Buttons
    JsSends_datas[8] = (My_joystick.L_But == 0 ? 0x00 : 0xf0) | (My_joystick.R_But == 0 ? 0x00 : 0x0f);
    // 验证位
    JsSends_datas[9] = JsSends_datas[1] + JsSends_datas[3] + JsSends_datas[5] + JsSends_datas[7];
    JsSends_datas[10] = JsSends_datas[0] + JsSends_datas[2] + JsSends_datas[4] + JsSends_datas[6];
    // 标识符
    JsSends_datas[11] = 0x01;       // 摇杆数据标识符：0x01
}

void rmt_load_JsBuffer_addr(uint8_t JsSends_datas[], RmtJoystickInfo My_joystick, uint16_t addr)
{ 
    int16_t temp_lx;
    int16_t temp_ly;
    int16_t temp_rx;
    int16_t temp_ry;

    if (rmt_Has_origins)        // 放缩到-5000 ~ 5000
    {
        temp_lx = (int16_t)(My_joystick.left_stick_x_perc * 5000);
        temp_ly = (int16_t)(My_joystick.left_stick_y_perc * 5000);
        temp_rx = (int16_t)(My_joystick.right_stick_x_perc * 5000);
        temp_ry = (int16_t)(My_joystick.right_stick_y_perc * 5000);
    }
    else
    {
        temp_lx = (int16_t)(My_joystick.left_stick_x / 2048.0 * 5000);
        temp_ly = (int16_t)(My_joystick.left_stick_y / 2048.0 * 5000);
        temp_rx = (int16_t)(My_joystick.right_stick_x / 2048.0 * 5000);
        temp_ry = (int16_t)(My_joystick.right_stick_y / 2048.0 * 5000);
    }

    JsSends_datas[0] = 0xec;    // 代表地址传输模式
    JsSends_datas[1] = 0x10;    // 数据长度：16
    JsSends_datas[2] = addr >> 8;       // 地址高八位
    JsSends_datas[3] = addr & 0xff;     // 地址低八位

    /************       以下是数据      *************/
    // LX
    JsSends_datas[4] = temp_lx >> 8;
    JsSends_datas[5] = temp_lx & 0xff;
    // LY
    JsSends_datas[6] = temp_ly >> 8;
    JsSends_datas[7] = temp_ly & 0xff;
    // RX
    JsSends_datas[8] = temp_rx >> 8;
    JsSends_datas[9] = temp_rx & 0xff;
    // RY
    JsSends_datas[10] = temp_ry >> 8;
    JsSends_datas[11] = temp_ry & 0xff;
    // Buttons
    JsSends_datas[12] = (My_joystick.L_But == 0 ? 0x00 : 0xf0) | (My_joystick.R_But == 0 ? 0x00 : 0x0f);
    // 验证位
    JsSends_datas[13] = JsSends_datas[5] + JsSends_datas[7] + JsSends_datas[9] + JsSends_datas[11];
    JsSends_datas[14] = JsSends_datas[4] + JsSends_datas[6] + JsSends_datas[8] + JsSends_datas[10];
    // 标识符
    JsSends_datas[15] = 0x01;       // 摇杆数据标识符：0x01
}


void rmt_Joystick_GetPercent(RmtJoystickInfo *js_info)
{
    if (rmt_Has_origins)        // 放缩到-1 ~ 1
    {
        if (js_info->left_stick_x > 0)
        {
            js_info->left_stick_x_perc = js_info->left_stick_x * 1.0 / (My_js_origins.left_stick_x_high - My_js_origins.left_stick_x_mid);
        }
        else
        {
            js_info->left_stick_x_perc = js_info->left_stick_x * 1.0 / (My_js_origins.left_stick_x_mid - My_js_origins.left_stick_x_low);
        }
        
        if (js_info->left_stick_y > 0)
        {
            js_info->left_stick_y_perc = js_info->left_stick_y * 1.0 / (My_js_origins.left_stick_y_high - My_js_origins.left_stick_y_mid);
        }
        else
        {
            js_info->left_stick_y_perc = js_info->left_stick_y * 1.0 / (My_js_origins.left_stick_y_mid - My_js_origins.left_stick_y_low);
        }
        
        if (js_info->right_stick_x > 0)
        {
            js_info->right_stick_x_perc = js_info->right_stick_x * 1.0 / (My_js_origins.right_stick_x_high - My_js_origins.right_stick_x_mid);
        }
        else
        {
            js_info->right_stick_x_perc = js_info->right_stick_x * 1.0 / (My_js_origins.right_stick_x_mid - My_js_origins.right_stick_x_low);
        }

        if (js_info->right_stick_y > 0)
        {
            js_info->right_stick_y_perc = js_info->right_stick_y * 1.0 / (My_js_origins.right_stick_y_high - My_js_origins.right_stick_y_mid);
        }
        else
        {
            js_info->right_stick_y_perc = js_info->right_stick_y * 1.0 / (My_js_origins.right_stick_y_mid - My_js_origins.right_stick_y_low);
        }
    }
}


int16_t rmt_pow_int16(int16_t a, float pownum)
{
    if (a > 0)
    {
        return (int16_t)pow(a, pownum);
    }
    else if (a < 0)
    {
        return -(int16_t)pow(-a, pownum);
    }
    else
    return 0;
}

float rmt_pow_float(float a, float pownum)
{
    if (a > 0)
    {
        return (float)pow(a, pownum);
    }
    else if (a < 0)
    {
        return -(float)pow(-a, pownum);
    }
    else
    return 0;
}
