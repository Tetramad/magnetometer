#include <limits.h>

#include <stm32f4xx_hal.h>

#include <udelay.h>

/* TODO: maybe change name to Micro Delay? */

HAL_StatusTypeDef UDELAY_Init(UDELAY_HandleTypeDef *hmicrowait) {
    UNUSED(hmicrowait);

    return HAL_OK;
}

void UDELAY_DelayMicro(UDELAY_HandleTypeDef *hmicrowait, unsigned Delay) {
    if (Delay < UINT_MAX) {
        ++Delay;
    }

    __HAL_TIM_SET_COUNTER(hmicrowait->TIMInstance, Delay);
    __HAL_TIM_CLEAR_FLAG(hmicrowait->TIMInstance, TIM_FLAG_UPDATE);
    HAL_TIM_Base_Start(hmicrowait->TIMInstance);
    while (!__HAL_TIM_GET_FLAG(hmicrowait->TIMInstance, TIM_FLAG_UPDATE)) {
    }
    HAL_TIM_Base_Stop(hmicrowait->TIMInstance);
}
