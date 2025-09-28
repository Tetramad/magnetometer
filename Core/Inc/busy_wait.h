/*
 * busy_wait.h
 *
 *      Author: Tetramad
 */

#ifndef __BUSY_WAIT_H
#define __BUSY_WAIT_H

#include <stdint.h>

int BusyWait_ms(uint32_t waitTime_ms);
int BusyWait_us(uint32_t waitTime_us);

#endif /* __BUSY_WAIT_H */
