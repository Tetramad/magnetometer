#ifndef LIS2MDL_H_
#define LIS2MDL_H_

#include <stdint.h>

#include <stm32f4xx_hal.h>

typedef struct {
    I2C_HandleTypeDef *I2CInstance;
    uint16_t DevAddress;
    uint32_t ErrorCode;
} LIS2MDL_HandleTypeDef;

HAL_StatusTypeDef LIS2MDL_Init(LIS2MDL_HandleTypeDef *hlis2mdl);
HAL_StatusTypeDef LIS2MDL_CheckSanity(LIS2MDL_HandleTypeDef *hlis2mdl);
uint32_t LIS2MDL_GetError(LIS2MDL_HandleTypeDef *hlis2mdl);
HAL_StatusTypeDef LIS2MDL_StartSingleMode(LIS2MDL_HandleTypeDef *hlis2mdl);
HAL_StatusTypeDef LIS2MDL_IsDataReady(LIS2MDL_HandleTypeDef *hlis2mdl);
uint16_t LIS2MDL_OUTX(LIS2MDL_HandleTypeDef *hlis2mdl);
uint16_t LIS2MDL_OUTY(LIS2MDL_HandleTypeDef *hlis2mdl);
uint16_t LIS2MDL_OUTZ(LIS2MDL_HandleTypeDef *hlis2mdl);
HAL_StatusTypeDef LIS2MDL_OUTXYZ(LIS2MDL_HandleTypeDef *hlis2mdl,
                                 uint16_t *outx,
                                 uint16_t *outy,
                                 uint16_t *outz);

#endif /* LIS2MDL_H_ */
