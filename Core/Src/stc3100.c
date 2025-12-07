#include <stm32f4xx_hal.h>

#include <log.h>
#include <stc3100.h>
#include <stc3100_reg.h>

/** 11.77uV * 0.5s * (1h / 3600s) * (2^12 LSB_ADC / 1 LSB_ACC) */
#define CHARGE_RESOLUTION 6.695822221312
#define VOLTAGE_RESOLUTION 2.44
#define RSHUNT_OHM 0.932084

LOG_LEVEL_SET(LOG_LEVEL_INF);

HAL_StatusTypeDef STC3100_Init(STC3100_HandleTypeDef *hstc3100) {
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t buffer[2] = {0};

    hstc3100->DevAddress = SLAVE_ADDRESS;
    hstc3100->RShunt_ohm = (float)RSHUNT_OHM;

    for (size_t i = 0; i < 5; ++i) {
        status = HAL_I2C_Mem_Read(hstc3100->I2CInstance,
                                  hstc3100->DevAddress,
                                  REG_CTRL,
                                  I2C_MEMADD_SIZE_8BIT,
                                  &buffer[0],
                                  1,
                                  10);
        if (status == HAL_OK) {
            break;
        }
    }
    if (status != HAL_OK) {
        return HAL_ERROR;
    }

    if (buffer[0] & REG_CTRL_PORDET_Msk) {
        LOG_DBG("STC3100 POR detected");

        buffer[REG_MODE - REG_MODE] = REG_MODE_SEL_EXT_CLK_AUTO |
                                      REG_MODE_GG_RES_14BITS | REG_MODE_GG_RUN;
        buffer[REG_CTRL - REG_MODE] =
            REG_CTRL_IO0DATA_LOW | REG_CTRL_GG_RST | REG_CTRL_PORDET_RELEASE;

        status = HAL_I2C_Mem_Write(hstc3100->I2CInstance,
                                   hstc3100->DevAddress,
                                   REG_MODE,
                                   I2C_MEMADD_SIZE_8BIT,
                                   &buffer[0],
                                   REG_CTRL - REG_MODE + 1,
                                   10);
        if (status != HAL_OK) {
            return HAL_ERROR;
        }
    }

    status = STC3100_CheckSanity(hstc3100);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }

    LOG_DBG("STC3100 initialization done");

    return HAL_OK;
}

HAL_StatusTypeDef STC3100_CheckSanity(STC3100_HandleTypeDef *hstc3100) {
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t buffer[2] = {0};

    status = HAL_I2C_Mem_Read(hstc3100->I2CInstance,
                              hstc3100->DevAddress,
                              REG_MODE,
                              I2C_MEMADD_SIZE_8BIT,
                              &buffer[0],
                              REG_CTRL - REG_MODE + 1,
                              10);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }
    LOG_DBG("STC3100: MODE 0x%02X CTRL 0x%02X",
            buffer[REG_MODE - REG_MODE],
            buffer[REG_CTRL - REG_MODE]);

    if ((buffer[REG_MODE - REG_MODE] & REG_MODE_GG_RUN_Msk) !=
            REG_MODE_GG_RUN ||
        (buffer[REG_CTRL - REG_MODE] & REG_CTRL_PORDET_Msk) !=
            REG_CTRL_PORDET_RELEASE) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

uint32_t STC3100_GetError(STC3100_HandleTypeDef *hstc3100) {
    return hstc3100->ErrorCode;
}

float STC3100_ChargeUsed_uAh(STC3100_HandleTypeDef *hstc3100) {
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t buffer[2] = {0};

    status = HAL_I2C_Mem_Read(hstc3100->I2CInstance,
                              hstc3100->DevAddress,
                              REG_CHARGE_LOW,
                              I2C_MEMADD_SIZE_8BIT,
                              &buffer[0],
                              REG_CHARGE_HIGH - REG_CHARGE_LOW + 1,
                              10);
    if (status != HAL_OK) {
        return 0.0f;
    }

    const int16_t charge_code = (int16_t)((buffer[1] << 8U) | buffer[0]);
    LOG_DBG("STC3100: read charge data %d(0x%02X%02X)",
            charge_code,
            buffer[1],
            buffer[0]);

    if (charge_code >= 0) {
        return 0.0f;
    } else {
        return -((float)charge_code * (float)CHARGE_RESOLUTION /
                 hstc3100->RShunt_ohm);
    }
}

float STC3100_Voltage_mV(STC3100_HandleTypeDef *hstc3100) {
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t buffer[2] = {0};

    status = HAL_I2C_Mem_Read(hstc3100->I2CInstance,
                              hstc3100->DevAddress,
                              REG_VOLTAGE_LOW,
                              I2C_MEMADD_SIZE_8BIT,
                              &buffer[0],
                              REG_VOLTAGE_HIGH - REG_VOLTAGE_LOW + 1,
                              10);
    if (status != HAL_OK) {
        return 0.0f;
    }

    const int16_t voltage_code = (int16_t)((buffer[1] << 8U) | buffer[0]);
    LOG_DBG("STC3100: read voltage data %d(0x%02X%02X)",
            voltage_code,
            buffer[1],
            buffer[0]);

    return (float)voltage_code * (float)VOLTAGE_RESOLUTION;
}
