/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f411_hal_ex.h"
#include "mode_selector.h"
#include "lis2mdl.h"
#include "stc3100.h"
#include "udelay.h"
#include "st7066u.h"

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
#define NOTUSED_Pin GPIO_PIN_13
#define NOTUSED_GPIO_Port GPIOC
#define NOTUSEDC14_Pin GPIO_PIN_14
#define NOTUSEDC14_GPIO_Port GPIOC
#define NOTUSEDC15_Pin GPIO_PIN_15
#define NOTUSEDC15_GPIO_Port GPIOC
#define NOTUSEDH0_Pin GPIO_PIN_0
#define NOTUSEDH0_GPIO_Port GPIOH
#define NOTUSEDH1_Pin GPIO_PIN_1
#define NOTUSEDH1_GPIO_Port GPIOH
#define NOTUSEDC0_Pin GPIO_PIN_0
#define NOTUSEDC0_GPIO_Port GPIOC
#define NOTUSEDC1_Pin GPIO_PIN_1
#define NOTUSEDC1_GPIO_Port GPIOC
#define NOTUSEDC2_Pin GPIO_PIN_2
#define NOTUSEDC2_GPIO_Port GPIOC
#define NOTUSEDC3_Pin GPIO_PIN_3
#define NOTUSEDC3_GPIO_Port GPIOC
#define NOTUSEDA0_Pin GPIO_PIN_0
#define NOTUSEDA0_GPIO_Port GPIOA
#define NOTUSEDA2_Pin GPIO_PIN_2
#define NOTUSEDA2_GPIO_Port GPIOA
#define NOTUSEDA3_Pin GPIO_PIN_3
#define NOTUSEDA3_GPIO_Port GPIOA
#define NOTUSEDA4_Pin GPIO_PIN_4
#define NOTUSEDA4_GPIO_Port GPIOA
#define NOTUSEDA5_Pin GPIO_PIN_5
#define NOTUSEDA5_GPIO_Port GPIOA
#define NOTUSEDA6_Pin GPIO_PIN_6
#define NOTUSEDA6_GPIO_Port GPIOA
#define NOTUSEDA7_Pin GPIO_PIN_7
#define NOTUSEDA7_GPIO_Port GPIOA
#define NOTUSEDC4_Pin GPIO_PIN_4
#define NOTUSEDC4_GPIO_Port GPIOC
#define NOTUSEDC5_Pin GPIO_PIN_5
#define NOTUSEDC5_GPIO_Port GPIOC
#define NOTUSEDB0_Pin GPIO_PIN_0
#define NOTUSEDB0_GPIO_Port GPIOB
#define NOTUSEDB1_Pin GPIO_PIN_1
#define NOTUSEDB1_GPIO_Port GPIOB
#define NOTUSEDB2_Pin GPIO_PIN_2
#define NOTUSEDB2_GPIO_Port GPIOB
#define NOTUSEDB10_Pin GPIO_PIN_10
#define NOTUSEDB10_GPIO_Port GPIOB
#define LCD_DB7_Pin GPIO_PIN_12
#define LCD_DB7_GPIO_Port GPIOB
#define LCD_DB6_Pin GPIO_PIN_13
#define LCD_DB6_GPIO_Port GPIOB
#define LCD_DB5_Pin GPIO_PIN_14
#define LCD_DB5_GPIO_Port GPIOB
#define LCD_DB4_Pin GPIO_PIN_15
#define LCD_DB4_GPIO_Port GPIOB
#define LCD_DB3_Pin GPIO_PIN_6
#define LCD_DB3_GPIO_Port GPIOC
#define LCD_DB2_Pin GPIO_PIN_7
#define LCD_DB2_GPIO_Port GPIOC
#define LCD_DB1_Pin GPIO_PIN_8
#define LCD_DB1_GPIO_Port GPIOC
#define LCD_DB0_Pin GPIO_PIN_9
#define LCD_DB0_GPIO_Port GPIOC
#define LCD_E_Pin GPIO_PIN_8
#define LCD_E_GPIO_Port GPIOA
#define LCD_RW_Pin GPIO_PIN_9
#define LCD_RW_GPIO_Port GPIOA
#define LCD_RS_Pin GPIO_PIN_10
#define LCD_RS_GPIO_Port GPIOA
#define NOTUSEDA11_Pin GPIO_PIN_11
#define NOTUSEDA11_GPIO_Port GPIOA
#define NOTUSEDA12_Pin GPIO_PIN_12
#define NOTUSEDA12_GPIO_Port GPIOA
#define NOTUSEDA15_Pin GPIO_PIN_15
#define NOTUSEDA15_GPIO_Port GPIOA
#define NOTUSEDC10_Pin GPIO_PIN_10
#define NOTUSEDC10_GPIO_Port GPIOC
#define NOTUSEDC11_Pin GPIO_PIN_11
#define NOTUSEDC11_GPIO_Port GPIOC
#define NOTUSEDC12_Pin GPIO_PIN_12
#define NOTUSEDC12_GPIO_Port GPIOC
#define NOTUSEDD2_Pin GPIO_PIN_2
#define NOTUSEDD2_GPIO_Port GPIOD
#define NOTUSEDB4_Pin GPIO_PIN_4
#define NOTUSEDB4_GPIO_Port GPIOB
#define Sensor_INT_Pin GPIO_PIN_5
#define Sensor_INT_GPIO_Port GPIOB
#define NOTUSEDB8_Pin GPIO_PIN_8
#define NOTUSEDB8_GPIO_Port GPIOB
#define NOTUSEDB9_Pin GPIO_PIN_9
#define NOTUSEDB9_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
