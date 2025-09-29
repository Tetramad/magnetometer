/*
 * busy_wait.c
 *
 *      Author: Tetramad
 */

#include <stdint.h>
#include <limits.h>

#include <stm32f4xx_hal.h>

int BusyWait_ms(uint32_t waitTime_ms) {
	if (HAL_RCC_GetHCLKFreq() / 1000U == 0) {
		__NOP();
		return 0;
	}

	if (waitTime_ms / (HAL_RCC_GetHCLKFreq() / 1000U) > UINT32_MAX) {
		return -__LINE__;
	}

	volatile uint32_t timeout = 1U + waitTime_ms * (HAL_RCC_GetHCLKFreq() / 1000U)
			/ 7U;

	while (timeout--) {
	}

	return 0;
}

int BusyWait_us(uint32_t waitTime_us) {
	if (HAL_RCC_GetHCLKFreq() / 1000000U == 0) {
		__NOP();
		return 0;
	}

	if (waitTime_us / (HAL_RCC_GetHCLKFreq() / 1000000U) > UINT32_MAX) {
		return -__LINE__;
	}

	volatile uint32_t timeout = 1U + waitTime_us * (HAL_RCC_GetHCLKFreq() / 1000000U)
			/ 7U;

	while (timeout--) {
	}

	return 0;
}
