#include <stm32f4xx_hal.h>

#include "log.h"

#include "mode_selector.h"

LOG_LEVEL_SET(LOG_LEVEL_INF);

HAL_StatusTypeDef MOD_Init(MOD_HandleTypeDef *hmod) {
    hmod->LatestADCCode = 0;
    hmod->CurrentModeState = MOD_MODE_STATE_UNKNOWN;

    hmod->RangeX = (MOD_RangeTypeDef){.begin = 3278U, .end = 3923U};

    hmod->RangeY = (MOD_RangeTypeDef){.begin = 2536U, .end = 3277U};

    hmod->RangeZ = (MOD_RangeTypeDef){.begin = 2001U, .end = 2535U};

    hmod->RangeMagnitude = (MOD_RangeTypeDef){.begin = 1583U, .end = 2000U};

    hmod->RangeBearing = (MOD_RangeTypeDef){.begin = 1251U, .end = 1582U};

    hmod->RangeSOH = (MOD_RangeTypeDef){.begin = 1046U, .end = 1250U};

    return HAL_OK;
}

HAL_StatusTypeDef MOD_UpdateMode(MOD_HandleTypeDef *hmod) {
    HAL_StatusTypeDef status = HAL_OK;
    int is_adc_running = 0;

    status = HAL_ADC_Start(hmod->ADCInstance);
    if (status != HAL_OK) {
        LOG_ERR("failed to start ADC");
        goto error;
    }
    is_adc_running = !0;

    status = HAL_ADC_PollForConversion(hmod->ADCInstance, 3);
    if (status != HAL_OK) {
        LOG_ERR("failed to poll ADC");
        goto error;
    }

    const uint32_t val = HAL_ADC_GetValue(hmod->ADCInstance);
    LOG_DBG("ADC result: %d", val);

    status = HAL_ADC_Stop(hmod->ADCInstance);
    if (status != HAL_OK) {
        LOG_ERR("failed to stop ADC");
        goto error;
    }
    is_adc_running = 0;

    if (val >= hmod->RangeX.begin && val <= hmod->RangeX.end) {
        hmod->CurrentModeState = MOD_MODE_STATE_X;
        return HAL_OK;
    }

    if (val >= hmod->RangeY.begin && val <= hmod->RangeY.end) {
        hmod->CurrentModeState = MOD_MODE_STATE_Y;
        return HAL_OK;
    }

    if (val >= hmod->RangeZ.begin && val <= hmod->RangeZ.end) {
        hmod->CurrentModeState = MOD_MODE_STATE_Z;
        return HAL_OK;
    }

    if (val >= hmod->RangeMagnitude.begin && val <= hmod->RangeMagnitude.end) {
        hmod->CurrentModeState = MOD_MODE_STATE_MAGNITUDE;
        return HAL_OK;
    }

    if (val >= hmod->RangeBearing.begin && val <= hmod->RangeBearing.end) {
        hmod->CurrentModeState = MOD_MODE_STATE_BEARING;
        return HAL_OK;
    }

    if (val >= hmod->RangeSOH.begin && val <= hmod->RangeSOH.end) {
        hmod->CurrentModeState = MOD_MODE_STATE_SOH;
        return HAL_OK;
    }

    hmod->CurrentModeState = MOD_MODE_STATE_UNKNOWN;
    return HAL_OK;

error:
    if (is_adc_running) {
        HAL_ADC_Stop(hmod->ADCInstance);
    }
    return HAL_ERROR;
}

MOD_ModeStateTypedef MOD_ReadMode(MOD_HandleTypeDef *hmod) {
    return hmod->CurrentModeState;
}
