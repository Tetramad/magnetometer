/*
 * io.c
 *
 *      Author: Tetramad
 */

#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx_hal.h>

int __io_putchar(int ch) {
    ITM_SendChar(ch > 0 ? ch : 0);

    return ch;
}

void HAL_Delay(uint32_t Delay) {
    uint32_t tickstart = HAL_GetTick();
    uint32_t wait = Delay;

    /* Add a freq to guarantee minimum wait */
    if (wait < HAL_MAX_DELAY) {
        wait += (uint32_t)(uwTickFreq);
    }

    while ((HAL_GetTick() - tickstart) < wait) {
        __WFE();
    }
}
