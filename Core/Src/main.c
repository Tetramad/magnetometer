/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <log.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */
LOG_LEVEL_SET(LOG_LEVEL_INF);

MOD_HandleTypeDef hmod;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM1_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
static void NotMX_MOD_Init(void);
static HAL_StatusTypeDef Task_ModeUpdate(void);
static HAL_StatusTypeDef task_print_ticks(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

    /* USER CODE BEGIN 1 */
    HAL_StatusTypeDef status = HAL_OK;
    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */
    HAL_Delay(200);
    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_TIM1_Init();
    MX_I2C1_Init();
    /* USER CODE BEGIN 2 */
    NotMX_MOD_Init();

    LOG_INF("Magnetometer");
    LOG_INF("H/W Rev.2");
    LOG_INF("F/W Ver.1.1.0");

    struct tick_aligned_task {
        uint32_t target_tick;
        const uint32_t tick_alignment;
        HAL_StatusTypeDef (*const task)(void);
        const void *user_data;
    };

    struct tick_aligned_task tasks[] = {[0] = {.target_tick = 0,
                                               .tick_alignment = 1000,
                                               .task = Task_ModeUpdate},
                                        [1] = {.target_tick = 0,
                                               .tick_alignment = 1000,
                                               .task = task_print_ticks}};
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    assert(sizeof(tasks) / sizeof(*tasks) > 0);
    while (1) {
        /**
		 * TODO: Tick Overflow dectection logic needed. it will be broken after nearly 49 days.
		 */
        const uint32_t current_tick = HAL_GetTick();

        struct tick_aligned_task *task = NULL;
        for (struct tick_aligned_task *t = &tasks[0];
             t < &tasks[sizeof(tasks) / sizeof(*tasks)];
             ++t) {
            if (t->target_tick <= current_tick) {
                task = t;
                break;
            }
        }

        if (task != NULL) {
            task->target_tick = current_tick -
                                (current_tick % task->tick_alignment) +
                                task->tick_alignment;
            if (task->task != NULL) {
                status = task->task();
                if (status != HAL_OK) {
                    LOG_ERR("task(%p) returns error (%d)", task, status);
                    goto error;
                }
            }
            continue;
        }

        task = &tasks[0];
        for (struct tick_aligned_task *t = &tasks[0];
             t < &tasks[sizeof(tasks) / sizeof(*tasks)];
             ++t) {
            if (task->target_tick > t->target_tick) {
                task = t;
            }
        }

        HAL_Delay(task->target_tick - current_tick);
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }

error:
    for (;;) {
        __WFE();
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
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
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
	 */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

    /* USER CODE BEGIN ADC1_Init 0 */

    /* USER CODE END ADC1_Init 0 */

    ADC_ChannelConfTypeDef sConfig = {0};

    /* USER CODE BEGIN ADC1_Init 1 */

    /* USER CODE END ADC1_Init 1 */

    /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        Error_Handler();
    }

    /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN ADC1_Init 2 */

    /* USER CODE END ADC1_Init 2 */
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

    /* USER CODE BEGIN I2C1_Init 0 */

    /* USER CODE END I2C1_Init 0 */

    /* USER CODE BEGIN I2C1_Init 1 */

    /* USER CODE END I2C1_Init 1 */
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN I2C1_Init 2 */

    /* USER CODE END I2C1_Init 2 */
}

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void) {

    /* USER CODE BEGIN TIM1_Init 0 */

    /* USER CODE END TIM1_Init 0 */

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    /* USER CODE BEGIN TIM1_Init 1 */

    /* USER CODE END TIM1_Init 1 */
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 65535;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM1_Init 2 */

    /* USER CODE END TIM1_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* USER CODE BEGIN MX_GPIO_Init_1 */

    /* USER CODE END MX_GPIO_Init_1 */

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, LCD_E_Pin | LCD_RS_Pin | LCD_RW_Pin, GPIO_PIN_SET);

    /*Configure GPIO pins : NOTUSED_Pin NOTUSEDC14_Pin NOTUSEDC15_Pin NOTUSEDC0_Pin
	 NOTUSEDC1_Pin NOTUSEDC2_Pin NOTUSEDC3_Pin NOTUSEDC4_Pin
	 NOTUSEDC5_Pin NOTUSEDC10_Pin NOTUSEDC11_Pin NOTUSEDC12_Pin */
    GPIO_InitStruct.Pin = NOTUSED_Pin | NOTUSEDC14_Pin | NOTUSEDC15_Pin |
                          NOTUSEDC0_Pin | NOTUSEDC1_Pin | NOTUSEDC2_Pin |
                          NOTUSEDC3_Pin | NOTUSEDC4_Pin | NOTUSEDC5_Pin |
                          NOTUSEDC10_Pin | NOTUSEDC11_Pin | NOTUSEDC12_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pins : NOTUSEDH0_Pin NOTUSEDH1_Pin */
    GPIO_InitStruct.Pin = NOTUSEDH0_Pin | NOTUSEDH1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    /*Configure GPIO pins : NOTUSEDA0_Pin NOTUSEDA2_Pin NOTUSEDA3_Pin NOTUSEDA4_Pin
	 NOTUSEDA5_Pin NOTUSEDA6_Pin NOTUSEDA7_Pin NOTUSEDA11_Pin
	 NOTUSEDA12_Pin NOTUSEDA15_Pin */
    GPIO_InitStruct.Pin = NOTUSEDA0_Pin | NOTUSEDA2_Pin | NOTUSEDA3_Pin |
                          NOTUSEDA4_Pin | NOTUSEDA5_Pin | NOTUSEDA6_Pin |
                          NOTUSEDA7_Pin | NOTUSEDA11_Pin | NOTUSEDA12_Pin |
                          NOTUSEDA15_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pins : NOTUSEDB0_Pin NOTUSEDB1_Pin NOTUSEDB2_Pin NOTUSEDB10_Pin
	 NOTUSEDB4_Pin NOTUSEDB8_Pin NOTUSEDB9_Pin */
    GPIO_InitStruct.Pin = NOTUSEDB0_Pin | NOTUSEDB1_Pin | NOTUSEDB2_Pin |
                          NOTUSEDB10_Pin | NOTUSEDB4_Pin | NOTUSEDB8_Pin |
                          NOTUSEDB9_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : LCD_E_Pin LCD_RS_Pin LCD_RW_Pin */
    GPIO_InitStruct.Pin = LCD_E_Pin | LCD_RS_Pin | LCD_RW_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pin : NOTUSEDD2_Pin */
    GPIO_InitStruct.Pin = NOTUSEDD2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(NOTUSEDD2_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : Sensor_INT_Pin */
    GPIO_InitStruct.Pin = Sensor_INT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(Sensor_INT_GPIO_Port, &GPIO_InitStruct);

    /* USER CODE BEGIN MX_GPIO_Init_2 */
    /*Configure GPIO pins : LCD_DB7_Pin LCD_DB6_Pin LCD_DB5_Pin LCD_DB4_Pin */
    GPIO_InitStruct.Pin = LCD_DB7_Pin | LCD_DB6_Pin | LCD_DB5_Pin | LCD_DB4_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : LCD_DB3_Pin LCD_DB2_Pin LCD_DB1_Pin LCD_DB0_Pin */
    GPIO_InitStruct.Pin = LCD_DB3_Pin | LCD_DB2_Pin | LCD_DB1_Pin | LCD_DB0_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static void NotMX_MOD_Init(void) {
    hmod.ADCInstance = &hadc1;

    if (MOD_Init(&hmod) != HAL_OK) {
        Error_Handler();
    }
}

HAL_StatusTypeDef Task_ModeUpdate(void) {
    HAL_StatusTypeDef status = HAL_OK;

    status = MOD_UpdateMode(&hmod);
    if (status != HAL_OK) {
        LOG_ERR("failed to update mode");
        return HAL_ERROR;
    }

    const MOD_ModeStateTypedef mode_state = MOD_ReadMode(&hmod);

    switch (mode_state) {
    default:
    case MOD_MODE_STATE_UNKNOWN:
        LOG_INF("Mode [Unknown]");
        return HAL_OK;
    case MOD_MODE_STATE_X:
        LOG_INF("Mode [X]");
        return HAL_OK;
    case MOD_MODE_STATE_Y:
        LOG_INF("Mode [Y]");
        return HAL_OK;
    case MOD_MODE_STATE_Z:
        LOG_INF("Mode [Z]");
        return HAL_OK;
    case MOD_MODE_STATE_MAGNITUDE:
        LOG_INF("Mode [Magnitude]");
        return HAL_OK;
    case MOD_MODE_STATE_BEARING:
        LOG_INF("Mode [Bearing]");
        return HAL_OK;
    case MOD_MODE_STATE_SOH:
        LOG_INF("Mode [SOH]");
        return HAL_OK;
    }
}

HAL_StatusTypeDef task_print_ticks(void) {
    return HAL_OK;
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line) {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
