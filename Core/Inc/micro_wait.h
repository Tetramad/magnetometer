#ifndef MICRO_WAIT_H_
#define MICRO_WAIT_H_

#include <stm32f4xx_hal.h>

typedef struct {
    TIM_HandleTypeDef *TIMInstance;
} MICROWAIT_HandleTypeDef;

HAL_StatusTypeDef MICROWAIT_Init(MICROWAIT_HandleTypeDef *hmicrowait);
void MICROWAIT_DelayMilli(MICROWAIT_HandleTypeDef *hmicrowait, unsigned Delay);
void MICROWAIT_DelayMicro(MICROWAIT_HandleTypeDef *hmicrowait, unsigned Delay);

#endif /* MICRO_WAIT_H_ */
