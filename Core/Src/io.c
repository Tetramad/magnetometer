/*
 * io.c
 *
 *      Author: Tetramad
 */

#include <stdio.h>
#include <stdint.h>
#include <stm32f4xx_hal.h>

int __io_putchar(int ch) {
	ITM_SendChar(ch > 0 ? ch : 0);

	return ch;
}
