#ifndef STC3100_H_
#define STC3100_H_

#include <stdint.h>

#include <stm32f4xx_hal.h>

typedef struct {
    I2C_HandleTypeDef *I2CInstance;
    uint32_t ErrorCode;
    uint16_t DevAddress;
    float RShunt_ohm;
} STC3100_HandleTypeDef;

HAL_StatusTypeDef STC3100_Init(STC3100_HandleTypeDef *hstc3100);
HAL_StatusTypeDef STC3100_CheckSanity(STC3100_HandleTypeDef *hstc3100);
uint32_t STC3100_GetError(STC3100_HandleTypeDef *hstc3100);
float STC3100_ChargeUsed_uAh(STC3100_HandleTypeDef *hstc3100);
float STC3100_Voltage_mV(STC3100_HandleTypeDef *hstc3100);

#endif /* STC3100_H_ */
