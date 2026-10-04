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
#define LED_GPIO_Port GPIOA                           

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
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET); \
  }while(0)

  #define KEY_INIT()                             \
  do{                                            \
    GPIO_InitTypeDef GPIO_InitStruct = {0};      \
    __HAL_RCC_GPIOB_CLK_ENABLE();                \
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_11;\
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;      \
    GPIO_InitStruct.Pull = GPIO_PULLUP;          \
    HAL_GPIO_Init(GPIOB,&GPIO_InitStruct);       \
  }while(0)

  #define ALL_INIT()        \
  do{                       \
    LED_INIT();             \
    KEY_INIT();             \
  }while(0)                 
  /*LED状态函数*/
  /*点亮LED*/
  #define  LED_ON(pin) HAL_GPIO_WritePin(GPIOA,pin,GPIO_PIN_RESET)
  /*切换LED状态*/
  #define  LED_SWITCH(pin) HAL_GPIO_TogglePin(GPIOA,pin)
  /*关闭LED*/
  #define  LED_OFF(pin) HAL_GPIO_WritePin(GPIOA,pin,GPIO_PIN_SET)

  /* 按键消抖延时(ms)，用于跨越机械抖动区 */
  #define KEY_DEBOUNCE_MS          15u
  /* 等待松手的最大时间(ms)，超时后判为无效，避免异常时永久阻塞 */
  #define KEY_RELEASE_TIMEOUT_MS   3000u

  /**
    * @brief  检测 GPIOB 上的按键是否发生一次“有效按下并释放”
    * @param  pin: 按键引脚，如 GPIO_PIN_1 / GPIO_PIN_11（上拉输入，按下为低电平）
    * @retval 1: 有效按下且已松手；0: 未按下、抖动或超时未松手
    * @note   使用前后两次采样 + 延时消抖；等待释放带超时，接口是非阻塞返回的错误安全版本
    */
  static inline uint8_t KEY_IsPressed(uint16_t pin)
  {
    uint32_t timeout;

    /* 第一次采样：高电平说明未按下，直接返回，无任何延时开销 */
    if (HAL_GPIO_ReadPin(GPIOB, pin) != GPIO_PIN_RESET)
                                                          /*GPIO_RESET等价于0*/
    { 
      return 0;
    }

    /* 跳过前沿抖动区后再次确认，避免毛刺被误判为有效按键 */
    HAL_Delay(KEY_DEBOUNCE_MS);
    if (HAL_GPIO_ReadPin(GPIOB, pin) != GPIO_PIN_RESET)
    {
      return 0;
    }

    /* 等待松手：带超时，防止引脚短路/卡键时系统被永久挂死 */
    timeout = KEY_RELEASE_TIMEOUT_MS / KEY_DEBOUNCE_MS;/*超时所需循环轮数*/
    while (HAL_GPIO_ReadPin(GPIOB, pin) == GPIO_PIN_RESET)
    {
      HAL_Delay(KEY_DEBOUNCE_MS);/*每轮delay基础时间*/
      if (timeout-- == 0u)
      {
        return 0;
      }
    }

    /* 跳过后沿抖动区 */
    HAL_Delay(KEY_DEBOUNCE_MS);
    return 1;
  }
  /*这是这个函数设计得好的地方：timeout 表示"还要循环多少轮"，每轮消耗 KEY_DEBOUNCE_MS 毫秒。
  这样**"超时时间"和"消抖步长"就解耦了** —— 你改 KEY_DEBOUNCE_MS 不用去动循环次数，
  改 KEY_RELEASE_TIMEOUT_MS 也不影响消抖。*/

  /* 兼容原有 KEY_PRESSED(pin) 写法：现为表达式，可安全参与 == 比较 */
  #define KEY_PRESSED(pin) KEY_IsPressed((pin))
/* USER CODE END EM */

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
