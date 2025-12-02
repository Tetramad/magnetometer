#include <stdint.h>

#include <stm32f4xx_hal.h>

#include <lis2mdl.h>
#include <lis2mdl_reg.h>
#include <log.h>

LOG_LEVEL_SET(LOG_LEVEL_INF);

HAL_StatusTypeDef LIS2MDL_Init(LIS2MDL_HandleTypeDef *hlis2mdl) {
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t buffer[3] = {0};

    hlis2mdl->DevAddress = SLAVE_ADDRESS;
    hlis2mdl->ErrorCode = 0;

    buffer[0] = CFG_REG_A_COMP_TEMP_EN | CFG_REG_A_MD_1 | CFG_REG_A_MD_0;
    buffer[1] = CFG_REG_B_LPF;
    buffer[2] = CFG_REG_C_BDU;

    status = HAL_I2C_Mem_Write(hlis2mdl->I2CInstance,
                               hlis2mdl->DevAddress,
                               CFG_REG_A,
                               I2C_MEMADD_SIZE_8BIT,
                               &buffer[0],
                               3,
                               10);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }

    status = LIS2MDL_CheckSanity(hlis2mdl);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef LIS2MDL_CheckSanity(LIS2MDL_HandleTypeDef *hlis2mdl) {
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t buffer = 0;

    status = HAL_I2C_Mem_Read(hlis2mdl->I2CInstance,
                              hlis2mdl->DevAddress,
                              CFG_REG_A,
                              I2C_MEMADD_SIZE_8BIT,
                              &buffer,
                              1,
                              10);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }

    if (!(buffer & CFG_REG_A_COMP_TEMP_EN_Msk)) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

uint32_t LIS2MDL_GetError(LIS2MDL_HandleTypeDef *hlis2mdl) {
    return hlis2mdl->ErrorCode;
}

HAL_StatusTypeDef LIS2MDL_StartSingleMode(LIS2MDL_HandleTypeDef *hlis2mdl) {
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t buffer = 0;

    status = LIS2MDL_CheckSanity(hlis2mdl);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }

    status = HAL_I2C_Mem_Read(hlis2mdl->I2CInstance,
                              hlis2mdl->DevAddress,
                              CFG_REG_A,
                              I2C_MEMADD_SIZE_8BIT,
                              &buffer,
                              1,
                              10);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }

    buffer = (uint8_t)((buffer & ~CFG_REG_A_MD_Msk) | CFG_REG_A_MD_0);

    status = HAL_I2C_Mem_Write(hlis2mdl->I2CInstance,
                               hlis2mdl->DevAddress,
                               CFG_REG_A,
                               I2C_MEMADD_SIZE_8BIT,
                               &buffer,
                               1,
                               10);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef LIS2MDL_IsDataReady(LIS2MDL_HandleTypeDef *hlis2mdl) {
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t buffer = 0;

    status = LIS2MDL_CheckSanity(hlis2mdl);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }

    status = HAL_I2C_Mem_Read(hlis2mdl->I2CInstance,
                              hlis2mdl->DevAddress,
                              STATUS_REG,
                              I2C_MEMADD_SIZE_8BIT,
                              &buffer,
                              1,
                              10);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }

    if (!(buffer & STATUS_REG_ZYXDA_Msk)) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

uint16_t LIS2MDL_OUTX(LIS2MDL_HandleTypeDef *hlis2mdl) {
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t buffer[2] = {0};

    status = LIS2MDL_CheckSanity(hlis2mdl);
    if (status != HAL_OK) {
        return 0U;
    }

    status = HAL_I2C_Mem_Read(hlis2mdl->I2CInstance,
                              hlis2mdl->DevAddress,
                              OUTX_L_REG | AUTO_INCREMENT,
                              I2C_MEMADD_SIZE_8BIT,
                              &buffer[0],
                              2,
                              10);
    if (status != HAL_OK) {
        return 0U;
    }

    return (uint16_t)((buffer[1] << 8U) | buffer[0]);
}

uint16_t LIS2MDL_OUTY(LIS2MDL_HandleTypeDef *hlis2mdl) {
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t buffer[2] = {0};

    status = LIS2MDL_CheckSanity(hlis2mdl);
    if (status != HAL_OK) {
        return 0U;
    }

    status = HAL_I2C_Mem_Read(hlis2mdl->I2CInstance,
                              hlis2mdl->DevAddress,
                              OUTY_L_REG | AUTO_INCREMENT,
                              I2C_MEMADD_SIZE_8BIT,
                              &buffer[0],
                              2,
                              10);
    if (status != HAL_OK) {
        return 0U;
    }

    return (uint16_t)((buffer[1] << 8U) | buffer[0]);
}

uint16_t LIS2MDL_OUTZ(LIS2MDL_HandleTypeDef *hlis2mdl) {
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t buffer[2] = {0};

    status = LIS2MDL_CheckSanity(hlis2mdl);
    if (status != HAL_OK) {
        return 0U;
    }

    status = HAL_I2C_Mem_Read(hlis2mdl->I2CInstance,
                              hlis2mdl->DevAddress,
                              OUTZ_L_REG | AUTO_INCREMENT,
                              I2C_MEMADD_SIZE_8BIT,
                              &buffer[0],
                              2,
                              10);
    if (status != HAL_OK) {
        return 0U;
    }

    return (uint16_t)((buffer[1] << 8U) | buffer[0]);
}
