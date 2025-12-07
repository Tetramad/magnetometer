#ifndef UDELAY_H_
#define UDELAY_H_

#include <stm32f4xx_hal.h>

typedef struct {
    TIM_HandleTypeDef *TIMInstance;
} UDELAY_HandleTypeDef;

HAL_StatusTypeDef UDELAY_Init(UDELAY_HandleTypeDef *hmicrowait);
void UDELAY_DelayMicro(UDELAY_HandleTypeDef *hmicrowait, unsigned Delay);

#endif /* UDELAY_H_ */
