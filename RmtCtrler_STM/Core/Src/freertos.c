/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "adc.h"
#include "rmt_stick.h"
#include "dl_ln.h"
#include "algorithm.h"

uint8_t arrayToByte(uint8_t keys[8]);

#define Lef_Down_But 10
#define Rig_Down_But 13

#define Lef_Up_But 0
#define Rig_Up_But 2

#define L_2 14
#define L_1_A 15
#define L_1_B 16

#define R_2 17
#define R_1_A 18
#define R_1_B 19


char JsSends_datas[20] = {0};

int rmt_delay_time = 0;
int targ_connect_addr = 0xaaaa;   // 自己要连接的目标地址

// j 代表摇杆数据
#define R1_ZIGBEE_ADDR 0x1202

uint8_t esp32_send_buf[16] = {'#', 'j', 0x08, 0x00, 0x07, 0x00, 0x06, 0x00, 0x05, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, '#'};
const uint8_t no_pure_buf[12] = {'#', 'd', 'e', 'b', 'u', 'g', '#'};
uint16_t ADC_Value[4] = {0};
uint16_t ADC_Value_Filtered[4] = {0};
float filter_rate = 0.20;
uint8_t Keys[24];

int uart_send_times = 0;
uint8_t can_send_flag = 0;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
void send_JS_dma();
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for adc_read */
osThreadId_t adc_readHandle;
const osThreadAttr_t adc_read_attributes = {
  .name = "adc_read",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for uart_1_send */
osThreadId_t uart_1_sendHandle;
const osThreadAttr_t uart_1_send_attributes = {
  .name = "uart_1_send",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void adc_read_Task(void *argument);
void uart_1_send_Task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of adc_read */
  adc_readHandle = osThreadNew(adc_read_Task, NULL, &adc_read_attributes);

  /* creation of uart_1_send */
  uart_1_sendHandle = osThreadNew(uart_1_send_Task, NULL, &uart_1_send_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  
  
  
  /* Infinite loop */
  for (;;)
  {
    
    // My_joystick.signal_delay_ms_temp++;
    osDelay(500);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_adc_read_Task */
/**
 * @name adc_read_Task
 * @brief 读取ADC，各种按钮的值，并处理他们
 * @details 这些值会在这里被 发送到ESP32 以及被装进数组，在另一个进程中 被发送到ZIGBEE
 */
/* USER CODE END Header_adc_read_Task */
void adc_read_Task(void *argument)
{
  /* USER CODE BEGIN adc_read_Task */

  /* Infinite loop */
  for (;;)
  {
    /*******************    读取按钮和摇杆    **********************/
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC_Value, 4);
    Keys[Lef_Up_But] = HAL_GPIO_ReadPin(Key_0_GPIO_Port, Key_0_Pin);
    Keys[Rig_Up_But] = HAL_GPIO_ReadPin(Key_1_GPIO_Port, Key_1_Pin);
    
    Keys[2] = HAL_GPIO_ReadPin(Key_2_GPIO_Port, Key_2_Pin);
    Keys[3] = HAL_GPIO_ReadPin(Key_3_GPIO_Port, Key_3_Pin);
    Keys[4] = HAL_GPIO_ReadPin(Key_4_GPIO_Port, Key_4_Pin);
    Keys[5] = HAL_GPIO_ReadPin(Key_5_GPIO_Port, Key_5_Pin);
    Keys[6] = HAL_GPIO_ReadPin(Key_6_GPIO_Port, Key_6_Pin);
    Keys[7] = HAL_GPIO_ReadPin(Key_7_GPIO_Port, Key_7_Pin);
    Keys[8] = HAL_GPIO_ReadPin(Key_8_GPIO_Port, Key_8_Pin);
    Keys[9] = HAL_GPIO_ReadPin(Key_9_GPIO_Port, Key_9_Pin);
    Keys[Lef_Down_But] = HAL_GPIO_ReadPin(Key_10_GPIO_Port, Key_10_Pin);
    Keys[11] = HAL_GPIO_ReadPin(Key_11_GPIO_Port, Key_11_Pin);
    Keys[12] = HAL_GPIO_ReadPin(Key_12_GPIO_Port, Key_12_Pin);
    Keys[Rig_Down_But] = HAL_GPIO_ReadPin(Key_13_GPIO_Port, Key_13_Pin);

    /*****************    读取拨杆    ***************/
    Keys[L_1_A] = HAL_GPIO_ReadPin(L_1_A_GPIO_Port, L_1_A_Pin);
    Keys[L_1_B] = HAL_GPIO_ReadPin(L_1_B_GPIO_Port, L_1_B_Pin);
    Keys[L_2] = HAL_GPIO_ReadPin(L_2_A_GPIO_Port, L_2_A_Pin);

    Keys[R_1_A] = HAL_GPIO_ReadPin(R_1_A_GPIO_Port, R_1_A_Pin);
    Keys[R_1_B] = HAL_GPIO_ReadPin(R_1_B_GPIO_Port, R_1_B_Pin);
    Keys[R_2] = HAL_GPIO_ReadPin(R_2_A_GPIO_Port, R_2_A_Pin);


    osDelay(5);

    ADC_Value_Filtered[0] = ADC_Value_Filtered[0] + (ADC_Value[0] - ADC_Value_Filtered[0]) * filter_rate;
    ADC_Value_Filtered[1] = ADC_Value_Filtered[1] + (ADC_Value[1] - ADC_Value_Filtered[1]) * filter_rate;
    ADC_Value_Filtered[2] = ADC_Value_Filtered[2] + (ADC_Value[2] - ADC_Value_Filtered[2]) * filter_rate;
    ADC_Value_Filtered[3] = ADC_Value_Filtered[3] + (ADC_Value[3] - ADC_Value_Filtered[3]) * filter_rate;

    if (rmt_Has_origins)
    {
      My_joystick.left_stick_x = ADC_Value_Filtered[L_x_adc_Channel] - My_js_origins.left_stick_x_mid;
      My_joystick.left_stick_y = ADC_Value_Filtered[L_y_adc_Channel] - My_js_origins.left_stick_y_mid;

      My_joystick.right_stick_x = ADC_Value_Filtered[R_x_adc_Channel] - My_js_origins.right_stick_x_mid;
      My_joystick.right_stick_y = ADC_Value_Filtered[R_y_adc_Channel] - My_js_origins.right_stick_y_mid;
      
    }
    else
    {
      My_joystick.left_stick_x = ADC_Value_Filtered[L_x_adc_Channel] - 2048;
      My_joystick.left_stick_y = ADC_Value_Filtered[L_y_adc_Channel] - 2048;

      My_joystick.right_stick_x = ADC_Value_Filtered[R_x_adc_Channel] - 2048;
      My_joystick.right_stick_y = ADC_Value_Filtered[R_y_adc_Channel] - 2048;
    }
    rmt_Reverse(&My_joystick);

    My_joystick.L_But = Keys[Lef_Up_But];
    My_joystick.R_But = Keys[Rig_Up_But];

    Vec2 right_ = {My_joystick.right_stick_x, My_joystick.right_stick_y};
    
    rmt_Joystick_GetPercent(&My_joystick);
    rmt_Joystick_Reflect(&My_joystick);
    can_send_flag = 0;
    // rmt_load_JsBuffer(JsSends_datas, My_joystick);
    rmt_load_JsBuffer_addr(JsSends_datas, My_joystick, R1_ZIGBEE_ADDR);
    can_send_flag = 1;

    // LX
    esp32_send_buf[2] = ADC_Value_Filtered[L_x_adc_Channel] >> 8;
    esp32_send_buf[3] = ADC_Value_Filtered[L_x_adc_Channel] & 0xff;
    // LY
    esp32_send_buf[4] = ADC_Value_Filtered[L_y_adc_Channel] >> 8;
    esp32_send_buf[5] = ADC_Value_Filtered[L_y_adc_Channel] & 0xff;
    // RX
    esp32_send_buf[6] = ADC_Value_Filtered[R_x_adc_Channel] >> 8;
    esp32_send_buf[7] = ADC_Value_Filtered[R_x_adc_Channel] & 0xff;
    
    // RY
    esp32_send_buf[8] = ADC_Value_Filtered[R_y_adc_Channel] >> 8;
    esp32_send_buf[9] = ADC_Value_Filtered[R_y_adc_Channel] & 0xff;

    // 每八个按钮变为一个字节
    uint8_t Keys_byte_0, Keys_byte_1, Keys_byte_2;
    Keys_byte_0 = arrayToByte(Keys);
    Keys_byte_1 = arrayToByte(Keys + 8);
    Keys_byte_2 = arrayToByte(Keys + 16);

    esp32_send_buf[10] = Keys_byte_0;
    esp32_send_buf[11] = Keys_byte_1;
    esp32_send_buf[12] = Keys_byte_2;
    esp32_send_buf[13] = Keys_byte_0 ^ Keys_byte_1 ^ Keys_byte_2;   // 异或校验位
    

    if (Keys[Lef_Down_But] == 1 && Keys[Rig_Down_But] == 1 && rmt_check_rmts_status(state_code, RMTS_UNINIT))
    {
      HAL_UART_Transmit_DMA(&huart1, no_pure_buf, sizeof(no_pure_buf));
    }
    else
    {
      HAL_UART_Transmit_DMA(&huart1, esp32_send_buf, sizeof(esp32_send_buf));
    }
    

    
    
    osDelay(15);
  }
  /* USER CODE END adc_read_Task */
}

/* USER CODE BEGIN Header_uart_1_send_Task */
/**
 * @name uart_1_send_Task 串口一发送任务
 * @brief 串口一（发送到远端的串口）的各种任务
 * @details 在发送之前，需要验证状态。如果还没有连接，就需要寻找节点连接；如果已经连接了，进入RMTCTRL模式时，才能遥控
 */
/* USER CODE END Header_uart_1_send_Task */
void uart_1_send_Task(void *argument)
{
  /* USER CODE BEGIN uart_1_send_Task */
  /* Infinite loop */
  for (;;)
  {

    // 首先验证状态是否正确
    if (rmt_check_rmts_status(state_code, RMTS_CONTROL) && can_send_flag)
    {
      send_JS_dma();
			uart_send_times++;
      can_send_flag = 0;
			osDelay(10);
		}


    // 频率 125Hz（之后会可变）
			osDelay(10);
  }
  /* USER CODE END uart_1_send_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void send_JS_dma()
{
  HAL_UART_Transmit_DMA(&huart2, (uint8_t *)JsSends_datas, sizeof(JsSends_datas));
}


// 函数用于将 uint8_t 数组转换为一个字节
uint8_t arrayToByte(uint8_t keys[8]) 
{
  uint8_t result = 0;
  for (int i = 0; i < 8; i++) {
      if (keys[i]) {
          result |= (1 << i);
      }
  }
  return result;
}
/* USER CODE END Application */

