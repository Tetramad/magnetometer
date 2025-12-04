#include <stdint.h>
#include <stdio.h>

#include <stm32f4xx_hal.h>

#include <log.h>

LOG_LEVEL_SET(LOG_LEVEL_DBG);

void HAL_Delay(uint32_t Delay) {
    uint32_t tickstart = HAL_GetTick();
    uint32_t wait = Delay;

    /* Add a freq to guarantee minimum wait */
    if (wait < HAL_MAX_DELAY) {
        wait += (uint32_t)(uwTickFreq);
    }

    while ((HAL_GetTick() - tickstart) < wait) {
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFE);
    }
}

void HAL_DelayEx(uint32_t Delay) {
    __IO uint32_t wait = Delay * (HAL_RCC_GetSysClockFreq() / 1000000U);
    wait += 1;

    while (wait--) {
    }
}
