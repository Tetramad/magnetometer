#ifndef MODE_SELECTOR_H_
#define MODE_SELECTOR_H_

#include <stdint.h>

#include <stm32f4xx_hal.h>

#define MOD_MODE_STATE_UNKNOWN 0U
#define MOD_MODE_STATE_X 1U
#define MOD_MODE_STATE_Y 2U
#define MOD_MODE_STATE_Z 3U
#define MOD_MODE_STATE_MAGNITUDE 4U
#define MOD_MODE_STATE_BEARING 5U
#define MOD_MODE_STATE_SOH 6U

typedef int MOD_ModeStateTypedef;

typedef struct {
	uint32_t begin;
	uint32_t end;
} MOD_RangeTypeDef;

typedef struct {
	ADC_HandleTypeDef *ADCInstance;
	uint32_t LatestADCCode;
	MOD_ModeStateTypedef CurrentModeState;
	MOD_RangeTypeDef RangeX;
	MOD_RangeTypeDef RangeY;
	MOD_RangeTypeDef RangeZ;
	MOD_RangeTypeDef RangeMagnitude;
	MOD_RangeTypeDef RangeBearing;
	MOD_RangeTypeDef RangeSOH;
} MOD_HandleTypeDef;

HAL_StatusTypeDef MOD_Init(MOD_HandleTypeDef *hmod);
HAL_StatusTypeDef MOD_UpdateMode(MOD_HandleTypeDef *hmod);
MOD_ModeStateTypedef MOD_ReadMode(MOD_HandleTypeDef *hmod);

#endif /* MODE_SELECTOR_H_ */
