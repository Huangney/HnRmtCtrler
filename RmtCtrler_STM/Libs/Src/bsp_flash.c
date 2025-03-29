#include "bsp_flash.h"

/**
 *@功能：从内部Flash读取指定字节数据
 *@参数1：ReadAddress：数据起始地址
 *@参数2：*data：      读取到的数据缓存首地址
 *@参数3：length：     读取字节个数
 */
void bsp_flash_read_data(uint32_t ReadAddress, uint8_t *data, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        data[i] = *(uint8_t *)(ReadAddress + i); // 读取数据
    }
}


/**
 *@功能：向内部Flash写入数据
 *@参数1：WriteAddress：数据要写入的目标地址（偏移地址）
 *@参数2：*data： 写入的数据首地址
 *@参数3：length：写入数据的个数
 */
void bsp_flash_write_data(uint32_t write_addr, uint8_t *data, uint32_t length)
{
    FLASH_EraseInitTypeDef FlashEraseInit;
    HAL_StatusTypeDef FlashStatus = HAL_OK;
    uint32_t SectorError = 0;
    uint32_t addrx = 0;                 // 当前操作的内存地址
    uint32_t start_addr = 0;            // 写入的起始地址
    uint32_t end_addr = 0;              // 写入的结束地址

    // 判定是否内存越界，是的话禁止读写，直接返回
    if ((write_addr < FMC_FLASH_BASE) || (write_addr + length >= FMC_FLASH_END) || (length <= 0))
        return;

    // 判定内存是否在十一区，否则直接返回
    if (write_addr < BSP_FLASH_SEC_11)
        return;

    HAL_FLASH_Unlock();                     // 解锁内存
    start_addr = write_addr;
    addrx = start_addr;
    end_addr = write_addr + length;       
    
    /********       准备擦除扇区11    *********/
    FlashEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;     // 擦除类型，扇区擦除
    FlashEraseInit.Sector = FLASH_SECTOR_11;                // 要擦除的扇区（扇区11）
    FlashEraseInit.NbSectors = 1;                           // 一次只擦除一个扇区
    FlashEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;    // 电压范围，VCC=2.7~3.6V之间!!
    if (HAL_FLASHEx_Erase(&FlashEraseInit, &SectorError) != HAL_OK)
    {
        return; // 发生错误了
    }


    /********       准备写入扇区11    *********/
    FlashStatus = FLASH_WaitForLastOperation(FLASH_WAITETIME); // 等待上次操作完成
    if (FlashStatus == HAL_OK)
    {
        while (write_addr < end_addr) // 写数据
        {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, write_addr, *data) != HAL_OK) // 写入数据
            {
                break; // 写入异常
            }
            write_addr += 1;
            data++;
        }
    }

    HAL_FLASH_Lock(); // 上锁
}
