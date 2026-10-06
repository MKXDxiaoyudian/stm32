/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

#define LED_INIT()                                    \
  do{                                                 \
    GPIO_InitTypeDef GPIO_InitStruct = {0};           \
    __HAL_RCC_GPIOA_CLK_ENABLE();                     \
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2;    \
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;       \
    GPIO_InitStruct.Pull = GPIO_NOPULL;               \
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;      \
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);     \
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);\
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);\
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);\
  }while(0)

  /*LED状态函数*/
  /*点亮LED*/
  #define  LED_ON(pin) HAL_GPIO_WritePin(GPIOA,pin,GPIO_PIN_RESET)
  /*切换LED状态*/
  #define  LED_SWITCH(pin) HAL_GPIO_TogglePin(GPIOA,pin)
  /*关闭LED*/
  #define  LED_OFF(pin) HAL_GPIO_WritePin(GPIOA,pin,GPIO_PIN_SET)

  /* ---- LED PWM（PA1 = TIM2_CH2）---- */
  #define PWM_ARR         999u
  #define BRIGHT_MAX      1000u      /* 0 = 灭, 1000 = 最亮 */
  #define BRIGHT_STEP     50u        /* 每转一格改变 5% */
  #define ENC_COUNTS_PER_DETENT   2u

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
