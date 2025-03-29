/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
extern int32_t SEND_INT;
extern char state_code[4];

extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Key_2_Pin GPIO_PIN_2
#define Key_2_GPIO_Port GPIOE
#define Key_3_Pin GPIO_PIN_3
#define Key_3_GPIO_Port GPIOE
#define Key_4_Pin GPIO_PIN_4
#define Key_4_GPIO_Port GPIOE
#define Key_5_Pin GPIO_PIN_5
#define Key_5_GPIO_Port GPIOE
#define Key_6_Pin GPIO_PIN_6
#define Key_6_GPIO_Port GPIOE
#define Key_7_Pin GPIO_PIN_7
#define Key_7_GPIO_Port GPIOE
#define Key_8_Pin GPIO_PIN_8
#define Key_8_GPIO_Port GPIOE
#define Key_9_Pin GPIO_PIN_9
#define Key_9_GPIO_Port GPIOE
#define Key_10_Pin GPIO_PIN_10
#define Key_10_GPIO_Port GPIOE
#define Key_11_Pin GPIO_PIN_11
#define Key_11_GPIO_Port GPIOE
#define Key_12_Pin GPIO_PIN_12
#define Key_12_GPIO_Port GPIOE
#define Key_13_Pin GPIO_PIN_13
#define Key_13_GPIO_Port GPIOE
#define L_1_A_Pin GPIO_PIN_9
#define L_1_A_GPIO_Port GPIOD
#define L_1_B_Pin GPIO_PIN_10
#define L_1_B_GPIO_Port GPIOD
#define L_2_A_Pin GPIO_PIN_11
#define L_2_A_GPIO_Port GPIOD
#define R_1_A_Pin GPIO_PIN_12
#define R_1_A_GPIO_Port GPIOD
#define R_1_B_Pin GPIO_PIN_13
#define R_1_B_GPIO_Port GPIOD
#define R_2_A_Pin GPIO_PIN_14
#define R_2_A_GPIO_Port GPIOD
#define Key_0_Pin GPIO_PIN_0
#define Key_0_GPIO_Port GPIOE
#define Key_1_Pin GPIO_PIN_1
#define Key_1_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
