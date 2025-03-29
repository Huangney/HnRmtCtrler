#ifndef BSP_FLASH_H
#define BSP_FLASH_H

// 注！！！：本FLASH设置是为F407VET6设计的！
#define FMC_FLASH_BASE      0x08000000   // 总的FLASH的起始地址
#define FMC_FLASH_END       0x08080000   // 总的FLASH的结束地址

#define FLASH_WAITETIME     50000        // FLASH读写等待超时时间(ms)
#define BSP_FLASH_SEC_11    0x80EFFFF       // 使用扇区11存储数据

#include "stm32f4xx_hal.h"



/**
 * @note 这个是给遥控器存储数据用的结构体，包含了一些遥控器的配置参数
 * 
 */
typedef struct rmt_ctrl_config_infos
{   
    // 上下左右，中点（二维）
    uint16_t L_JS_4_divs[6];
    uint16_t R_JS_4_divs[6];

    // 左右摇杆的死区
    uint16_t L_JS_dead_band;
    uint16_t R_JS_dead_band;
    
    
}RmtCtrlConfigInfos;







// 读取flash上的数据到数组中
void bsp_flash_read_data(uint32_t ReadAddress, uint8_t *data, uint32_t length);
// 擦除flash
void bsp_flash_write_data(uint32_t WriteAddress, uint8_t *data, uint32_t length);


#endif