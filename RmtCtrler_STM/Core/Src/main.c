/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_hal_dma.h"
#include "string.h"
#include "rmt_stick.h"
#include "dl_ln.h"
#include "std_msg.h"

uint8_t rx_Buffer[32];
uint8_t esp_rx_Buffer[12];
char state_code[4] = {'?', '?', '?', '?'};
DL_LN_Msg my_msg; 

int uart_recv_times = 0;
int espuart_recv_times = 0;

float valid_rate = 0.0;
int valid_info = 0;


int32_t SEND_INT = 0;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance == ADC1) {
        // 处理ADC DMA完成后的操作
    }
}

/**
 * @details 本机的ZIGBEE ID 为11 45 ，PANID 为 45 22
 * 
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart == &huart2)   // 来自底盘的消息：串口2
    {
			uart_recv_times++;    // 计数：收到一次
      DL_LN_decode(rx_Buffer, &my_msg);

      if(my_msg.source_port == DL_LN_REPLY_PORT)    // 如果是返回回来的节点信息，就把节点信息送给ESP32
      {
        uint8_t esp32_sd_buf[12] = {'#', 'n', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '#'};
        esp32_sd_buf[2] = my_msg.addr >> 8;
        esp32_sd_buf[3] = my_msg.addr & 0xff;
        memcpy(esp32_sd_buf + 4, my_msg.data, 7);
        HAL_UART_Transmit_DMA(&huart1, esp32_sd_buf, sizeof(esp32_sd_buf));
      }

      memset(rx_Buffer, 0, sizeof(rx_Buffer));
			HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_Buffer, sizeof(rx_Buffer));
			HAL_UARTEx_ReceiveToIdle_DMA(&huart1, esp_rx_Buffer, sizeof(esp_rx_Buffer));
    }

    

    if (huart == &huart1)   // 来自ESP32的消息：串口1
    {
			espuart_recv_times++;

      switch (esp_rx_Buffer[0])
      {
        case 'm':    // 说明是 状态码消息，给ESP32反馈回去
        {
          memcpy(state_code, esp_rx_Buffer + 1, 3);
					uint8_t state_reply_buf[4];
					
					memcpy(state_reply_buf, esp_rx_Buffer, 4);
					HAL_UART_Transmit_DMA(&huart1, state_reply_buf, sizeof(state_reply_buf));
          break;
        }
        case 1:    // 说明是 摇杆校准 消息
        {
          rmt_decode_joystick_msg(&My_js_origins, esp_rx_Buffer, esp_rx_Buffer[1]);
          break;
        }
        
      default:
        break;
      }
      
			
      memset(esp_rx_Buffer, 0, sizeof(esp_rx_Buffer));
			HAL_UARTEx_ReceiveToIdle_DMA(&huart1, esp_rx_Buffer, sizeof(esp_rx_Buffer));
			HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_Buffer, sizeof(rx_Buffer));
    }
}
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_Buffer, sizeof(rx_Buffer));
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, esp_rx_Buffer, sizeof(esp_rx_Buffer));
	
  __HAL_DMA_DISABLE_IT(&hdma_usart1_rx,DMA_IT_HT); 		//	关闭DMA传输过半中断
  __HAL_DMA_DISABLE_IT(&hdma_usart2_rx,DMA_IT_HT); 		//	关闭DMA传输过半中断

//  DL_LN_set(&huart2, &huart2, BPS, 230400, 0);
//  DL_LN_read(&huart2, 0x04);
//  DL_LN_restart(&huart2);
  
	// HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_Value, 2);
  
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  { 
    // HAL_Delay(20);    // 每秒上报50次
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
