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
#include <math.h>
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
#define MAGNETIC_FLUX_OFFSET_X ((int16_t)(-26))
#define MAGNETIC_FLUX_OFFSET_Y ((int16_t)(256))
#define MAGNETIC_FLUX_OFFSET_Z ((int16_t)(10))
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
LOG_LEVEL_SET(LOG_LEVEL_INF);

MOD_HandleTypeDef hmod;
LIS2MDL_HandleTypeDef hlis2mdl;
STC3100_HandleTypeDef hstc3100;
MICROWAIT_HandleTypeDef hmicrowait;
DISPLAY_HandleTypeDef hdisplay;

MOD_ModeStateTypedef mode_state = MOD_MODE_STATE_UNKNOWN;

float magnetic_flux_x_mgauss = 0.0f;
float magnetic_flux_y_mgauss = 0.0f;
float magnetic_flux_z_mgauss = 0.0f;
float magnetic_flux_magnitude_mgauss = 0.0f;
float magnetic_flux_bearing_degree = 0.0f;

float gas_gauge_charge_used_uah = 0.0f;
float gas_gauge_voltage_mv = 0.0f;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
static void NotMX_MOD_Init(void);
static void NotMX_LIS2MDL_Init(void);
static void NotMX_STC3100_Init(void);
static void NotMX_MICROWAIT_Init(void);
static void NotMX_DISPLAY_Init(void);
static HAL_StatusTypeDef Task_ModeUpdate(void);
static HAL_StatusTypeDef Task_MagneticMeasurementUpdate(void);
static HAL_StatusTypeDef Task_SOHUpdate(void);
static HAL_StatusTypeDef Task_DisplayUpdate(void);
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
    HAL_Delay(50);
    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();
    MX_RTC_Init();
    MX_TIM2_Init();
    /* USER CODE BEGIN 2 */
    HAL_Delay(50);

    NotMX_MOD_Init();
    NotMX_LIS2MDL_Init();
    NotMX_STC3100_Init();
    NotMX_MICROWAIT_Init();
    NotMX_DISPLAY_Init();

    HAL_Delay(50);

    LOG_INF("Magnetometer");
    LOG_INF("H/W Rev.2");
    LOG_INF("F/W Ver.1.2.0");

    HAL_Delay(50);

    struct tick_aligned_task {
        uint32_t target_tick;
        const uint32_t tick_alignment;
        HAL_StatusTypeDef (*const task)(void);
        const char *const name;
    };

    struct tick_aligned_task tasks[] = {
        [0] = {.target_tick = 0,
               .tick_alignment = 1000,
               .task = Task_ModeUpdate,
               .name = "ModeUpdate"},
        [1] = {.target_tick = 0,
               .tick_alignment = 200,
               .task = Task_MagneticMeasurementUpdate,
               .name = "MagneticMeasurementUpdate"},
        [2] = {.target_tick = 0,
               .tick_alignment = 1000,
               .task = Task_SOHUpdate,
               .name = "SOHUpdate"},
        [3] = {.target_tick = 0,
               .tick_alignment = 200,
               .task = Task_DisplayUpdate,
               .name = "DisplayUpdate"},
    };
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
                    LOG_ERR("task(%s) failed", task->name);
                    task->target_tick = UINT32_MAX;
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

        /**
         * TODO: Cleanup these things.
         */
        HAL_SuspendTick();
        HAL_RTCEx_SetWakeUpTimer_IT(&hrtc,
                                    (task->target_tick - current_tick) * 2,
                                    RTC_WAKEUPCLOCK_RTCCLK_DIV16);
        HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
        HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
        uwTick += task->target_tick - current_tick;
        HAL_ResumeTick();
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
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
    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
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
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void) {

    /* USER CODE BEGIN RTC_Init 0 */

    /* USER CODE END RTC_Init 0 */

    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    /* USER CODE BEGIN RTC_Init 1 */

    /* USER CODE END RTC_Init 1 */

    /** Initialize RTC Only
  */
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(&hrtc) != HAL_OK) {
        Error_Handler();
    }

    /* USER CODE BEGIN Check_RTC_BKUP */

    /* USER CODE END Check_RTC_BKUP */

    /** Initialize RTC and set the Time and Date
  */
    sTime.Hours = 0x0;
    sTime.Minutes = 0x0;
    sTime.Seconds = 0x0;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK) {
        Error_Handler();
    }
    sDate.WeekDay = RTC_WEEKDAY_MONDAY;
    sDate.Month = RTC_MONTH_JANUARY;
    sDate.Date = 0x1;
    sDate.Year = 0x0;

    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK) {
        Error_Handler();
    }

    /** Enable the WakeUp
  */
    if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 0, RTC_WAKEUPCLOCK_RTCCLK_DIV16) !=
        HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN RTC_Init 2 */
    if (HAL_RTCEx_DeactivateWakeUpTimer(&hrtc) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE END RTC_Init 2 */
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void) {

    /* USER CODE BEGIN TIM2_Init 0 */

    /* USER CODE END TIM2_Init 0 */

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    /* USER CODE BEGIN TIM2_Init 1 */

    /* USER CODE END TIM2_Init 1 */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 15;
    htim2.Init.CounterMode = TIM_COUNTERMODE_DOWN;
    htim2.Init.Period = 4294967295;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM2_Init 2 */

    /* USER CODE END TIM2_Init 2 */
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
    HAL_GPIO_WritePin(GPIOA, LCD_E_Pin | LCD_RW_Pin | LCD_RS_Pin, GPIO_PIN_SET);

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

    /*Configure GPIO pins : LCD_E_Pin LCD_RW_Pin LCD_RS_Pin */
    GPIO_InitStruct.Pin = LCD_E_Pin | LCD_RW_Pin | LCD_RS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
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

static void NotMX_LIS2MDL_Init(void) {
    hlis2mdl.I2CInstance = &hi2c1;

    if (LIS2MDL_Init(&hlis2mdl) != HAL_OK) {
        LOG_ERR("failed to initialize LIS2MDL");
        return;
        /* TODO: fix */
        Error_Handler();
    }
}

static void NotMX_STC3100_Init(void) {
    hstc3100.I2CInstance = &hi2c1;

    if (STC3100_Init(&hstc3100) != HAL_OK) {
        LOG_ERR("failed to initialize STC3100");
        return;
        /* TODO: fix */
        Error_Handler();
    }
}

static void NotMX_MICROWAIT_Init(void) {
    hmicrowait.TIMInstance = &htim2;

    if (MICROWAIT_Init(&hmicrowait) != HAL_OK) {
        LOG_ERR("failed to initialize MICROWAIT");
        return;
        /* TODO: fix */
        Error_Handler();
    }
}

static void NotMX_DISPLAY_Init(void) {
    hdisplay.MICROWAITInstance = &hmicrowait;
    hdisplay.Init.BusMode = DISPLAY_BUS_MODE_4;

    hdisplay.Init.RS_Port = LCD_RS_GPIO_Port;
    hdisplay.Init.RS_Pin = LCD_RS_Pin;
    hdisplay.Init.RW_Port = LCD_RW_GPIO_Port;
    hdisplay.Init.RW_Pin = LCD_RW_Pin;
    hdisplay.Init.E_Port = LCD_E_GPIO_Port;
    hdisplay.Init.E_Pin = LCD_E_Pin;

    hdisplay.Init.DB_Port[0] = LCD_DB0_GPIO_Port;
    hdisplay.Init.DB_Pin[0] = LCD_DB0_Pin;
    hdisplay.Init.DB_Port[1] = LCD_DB1_GPIO_Port;
    hdisplay.Init.DB_Pin[1] = LCD_DB1_Pin;
    hdisplay.Init.DB_Port[2] = LCD_DB2_GPIO_Port;
    hdisplay.Init.DB_Pin[2] = LCD_DB2_Pin;
    hdisplay.Init.DB_Port[3] = LCD_DB3_GPIO_Port;
    hdisplay.Init.DB_Pin[3] = LCD_DB3_Pin;
    hdisplay.Init.DB_Port[4] = LCD_DB4_GPIO_Port;
    hdisplay.Init.DB_Pin[4] = LCD_DB4_Pin;
    hdisplay.Init.DB_Port[5] = LCD_DB5_GPIO_Port;
    hdisplay.Init.DB_Pin[5] = LCD_DB5_Pin;
    hdisplay.Init.DB_Port[6] = LCD_DB6_GPIO_Port;
    hdisplay.Init.DB_Pin[6] = LCD_DB6_Pin;
    hdisplay.Init.DB_Port[7] = LCD_DB7_GPIO_Port;
    hdisplay.Init.DB_Pin[7] = LCD_DB7_Pin;

    if (DISPLAY_Init(&hdisplay) != HAL_OK) {
        LOG_ERR("failed to initialize DISPLAY");
        return;
        /* TODO: fix */
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

    mode_state = MOD_ReadMode(&hmod);

    switch (mode_state) {
    default:
    case MOD_MODE_STATE_UNKNOWN:
        LOG_DBG("Mode [Unknown]");
        return HAL_OK;
    case MOD_MODE_STATE_X:
        LOG_DBG("Mode [X]");
        return HAL_OK;
    case MOD_MODE_STATE_Y:
        LOG_DBG("Mode [Y]");
        return HAL_OK;
    case MOD_MODE_STATE_Z:
        LOG_DBG("Mode [Z]");
        return HAL_OK;
    case MOD_MODE_STATE_MAGNITUDE:
        LOG_DBG("Mode [Magnitude]");
        return HAL_OK;
    case MOD_MODE_STATE_BEARING:
        LOG_DBG("Mode [Bearing]");
        return HAL_OK;
    case MOD_MODE_STATE_SOH:
        LOG_DBG("Mode [SOH]");
        return HAL_OK;
    }
}

HAL_StatusTypeDef Task_MagneticMeasurementUpdate(void) {
    HAL_StatusTypeDef status = HAL_OK;

    status = LIS2MDL_CheckSanity(&hlis2mdl);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }

    status = LIS2MDL_StartSingleMode(&hlis2mdl);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }

    uint16_t x_code = 0U;
    uint16_t y_code = 0U;
    uint16_t z_code = 0U;

    status = LIS2MDL_OUTXYZ(&hlis2mdl, &x_code, &y_code, &z_code);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }

    const float x_mgauss =
        (float)((int16_t)x_code - MAGNETIC_FLUX_OFFSET_X) * 1.5f;
    const float y_mgauss =
        (float)((int16_t)y_code - MAGNETIC_FLUX_OFFSET_Y) * 1.5f;
    const float z_mgauss =
        (float)((int16_t)z_code - MAGNETIC_FLUX_OFFSET_Z) * 1.5f;

    /*
	 * TODO: should be tidy out
	 */
    const float magnitude_mgauss =
        sqrtf(x_mgauss * x_mgauss + y_mgauss * y_mgauss + z_mgauss * z_mgauss);

    const float bearing_degree = ({
        float theta = 0.0f;
        if (isless(fabsf(x_mgauss), 0.001f) &&
            isless(fabsf(y_mgauss), 0.001f)) {
            theta = 0.0f;
        } else if (isless(fabsf(x_mgauss), 0.001f) && signbit(y_mgauss)) {
            theta = (float)M_PI_2 * 3.0f;
        } else if (isless(fabsf(x_mgauss), 0.001f) && !signbit(y_mgauss)) {
            theta = (float)M_PI_2 * 1.0f;
        } else {
            theta =
                atanf(y_mgauss / -x_mgauss) +
                (!signbit(x_mgauss) * (float)M_PI) +
                ((signbit(x_mgauss) && signbit(y_mgauss)) * 2.0f * (float)M_PI);
        }
        theta * 180.0f / (float)M_PI;
    });

    magnetic_flux_x_mgauss = x_mgauss;
    magnetic_flux_y_mgauss = y_mgauss;
    magnetic_flux_z_mgauss = z_mgauss;
    magnetic_flux_magnitude_mgauss = magnitude_mgauss;
    magnetic_flux_bearing_degree = bearing_degree;

    return HAL_OK;
}

static HAL_StatusTypeDef Task_SOHUpdate(void) {
    HAL_StatusTypeDef status = HAL_OK;

    status = STC3100_CheckSanity(&hstc3100);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }

    gas_gauge_charge_used_uah = STC3100_ChargeUsed_uAh(&hstc3100);

    LOG_DBG("Charge Used: %d uAh", (int)gas_gauge_charge_used_uah);

    return HAL_OK;
}

static HAL_StatusTypeDef Task_DisplayUpdate(void) {
    assert(abs((int)magnetic_flux_x_mgauss) <= 99999);
    assert(abs((int)magnetic_flux_y_mgauss) <= 99999);
    assert(abs((int)magnetic_flux_z_mgauss) <= 99999);
    assert(abs((int)magnetic_flux_magnitude_mgauss) <= 99999);

    if (DISPLAY_CheckSanity(&hdisplay) != HAL_OK) {
        return HAL_ERROR;
    }

    switch (mode_state) {
    default:
    case MOD_MODE_STATE_UNKNOWN:
        return DISPLAY_Print(&hdisplay, "%s", "Unknown");
    case MOD_MODE_STATE_X:
        return DISPLAY_Print(&hdisplay, "X %6d", (int)magnetic_flux_x_mgauss);
    case MOD_MODE_STATE_Y:
        return DISPLAY_Print(&hdisplay, "Y %6d", (int)magnetic_flux_y_mgauss);
    case MOD_MODE_STATE_Z:
        return DISPLAY_Print(&hdisplay, "Z %6d", (int)magnetic_flux_z_mgauss);
    case MOD_MODE_STATE_MAGNITUDE:
        return DISPLAY_Print(
            &hdisplay, "M %6d", (int)magnetic_flux_magnitude_mgauss);
    case MOD_MODE_STATE_BEARING:
        return DISPLAY_Print(
            &hdisplay, "B %6d", (int)magnetic_flux_bearing_degree);
    case MOD_MODE_STATE_SOH:
        return DISPLAY_Print(
            &hdisplay, "S %6d", (int)gas_gauge_charge_used_uah);
    }
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
