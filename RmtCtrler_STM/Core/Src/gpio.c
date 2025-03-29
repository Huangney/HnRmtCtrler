/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pins : Key_2_Pin Key_3_Pin Key_4_Pin Key_5_Pin
                           Key_6_Pin Key_7_Pin Key_8_Pin Key_9_Pin
                           Key_10_Pin Key_11_Pin Key_12_Pin Key_13_Pin
                           Key_0_Pin Key_1_Pin */
  GPIO_InitStruct.Pin = Key_2_Pin|Key_3_Pin|Key_4_Pin|Key_5_Pin
                          |Key_6_Pin|Key_7_Pin|Key_8_Pin|Key_9_Pin
                          |Key_10_Pin|Key_11_Pin|Key_12_Pin|Key_13_Pin
                          |Key_0_Pin|Key_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : L_1_A_Pin L_1_B_Pin L_2_A_Pin R_1_A_Pin
                           R_1_B_Pin R_2_A_Pin */
  GPIO_InitStruct.Pin = L_1_A_Pin|L_1_B_Pin|L_2_A_Pin|R_1_A_Pin
                          |R_1_B_Pin|R_2_A_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
