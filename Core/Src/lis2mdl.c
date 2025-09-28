#include <math.h>
#include <stdint.h>

#include <stm32f4xx_hal.h>

#include <endianess.h>
#include <lis2mdl_reg.h>
#include <log.h>
#include <sensor.h>
#include <busy_wait.h>

extern I2C_HandleTypeDef hi2c1;

static int configure(void);
static int is_configured(void);
static int sample_single(void);
static int is_data_available(void);

LOG_LEVEL_SET(LOG_LEVEL_DBG);

int LIS2MDL_Init(void) {
    return 0;
}

int LIS2MDL_Measure(void) {
    int err = 0;
    HAL_StatusTypeDef status = HAL_OK;
    unsigned char buffer[TEMP_OUT_H_REG - STATUS_REG + 1] = {0};

    err = is_configured();
    if (err < 0) {
        return -__LINE__;
    }

    const int flag_configured = err > 0;
    if (!flag_configured) {
        err = configure();
        if (err < 0) {
            return -__LINE__;
        }
    }

    err = sample_single();
    if (err < 0) {
        return -__LINE__;
    }
    LOOP(3) {
        BusyWait_us(500);
        err = is_data_available();
        if (err < 0) {
            return -__LINE__;
        }

        const int data_available = err > 0;
        if (data_available) {
            break;
        }
    }
    const int data_available = err > 0;
    if (!data_available) {
        return -__LINE__;
    }

    status = HAL_I2C_Mem_Read(&hi2c1, SLAVE_ADDRESS, STATUS_REG, I2C_MEMADD_SIZE_8BIT, buffer, sizeof(buffer) - 2U, 25U);
    if (status != HAL_OK) {
    	return -__LINE__;
    }

    status = HAL_I2C_Mem_Read(&hi2c1, SLAVE_ADDRESS, TEMP_OUT_L_REG, I2C_MEMADD_SIZE_8BIT, &buffer[7], 2U, 25U);
    if (status != HAL_OK) {
    	return -__LINE__;
    }

    LOG_DBG(buffer[1], "LIS2MDL: STATUS_REG");

    const float mag_x_adc = (int16_t)get_le16(&buffer[1]);
    const float mag_x_meas = mag_x_adc * (1.5f / 1e1f);

    const float mag_y_adc = (int16_t)get_le16(&buffer[3]);
    const float mag_y_meas = mag_y_adc * (1.5f / 1e1f);

    const float mag_z_adc = (int16_t)get_le16(&buffer[5]);
    const float mag_z_meas = mag_z_adc * (1.5f / 1e1f);

    const float mag_meas =
        sqrtf(powf(mag_x_meas, 2.0f) + powf(mag_y_meas, 2.0f) +
              powf(mag_z_meas, 2.0f));

    LOG_DBG(buffer[7], "LIS2MDL: TEMP_OUT_L_REG=");
    LOG_DBG(buffer[8], "LIS2MDL: TEMP_OUT_H_REG=");
    const float t_adc = (int16_t)get_le16(&buffer[7]);
    const float t_meas = 25000.0f + t_adc / 8.0f * 1e3f;

    magneticFieldMagnitude_mT = (signed)mag_meas;
    magneticFieldXAxis_mT = (signed)mag_x_meas;
    magneticFieldYAxis_mT = (signed)mag_y_meas;
    magneticFieldZAxis_mT = (signed)mag_z_meas;
    temperature_Celsius = (signed)t_meas;

    LOG_DBG(magneticFieldMagnitude_mT, "LIS2MDL: Magnetic Field (uT)");
    LOG_DBG(magneticFieldXAxis_mT, "LIS2MDL: Magnetic Field X(uT)");
    LOG_DBG(magneticFieldYAxis_mT, "LIS2MDL: Magnetic Field Y(uT)");
    LOG_DBG(magneticFieldZAxis_mT, "LIS2MDL: Magnetic Field Z(uT)");
    LOG_DBG(temperature_Celsius, "LIS2MDL: Temperature(m'C)");

    return 0;
}

static int configure(void) {
    HAL_StatusTypeDef status = HAL_OK;
    unsigned char buffer[CFG_REG_C - CFG_REG_A + 1 + 1] = {0};

    buffer[0] = CFG_REG_A;
    buffer[CFG_REG_A - CFG_REG_A + 1] = CFG_REG_A_COMP_TEMP_EN;
    buffer[CFG_REG_B - CFG_REG_A + 1] = CFG_REG_B_LPF;
    buffer[CFG_REG_C - CFG_REG_A + 1] = CFG_REG_C_BDU;

    status = HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS, buffer, sizeof(buffer), 25U);
    if (status != HAL_OK) {
    	return -__LINE__;
    }

    LOG_INF("LIS2MDL: configureation succeeded");

    return 0;
}

static int is_configured(void) {
	HAL_StatusTypeDef status = HAL_OK;
	unsigned char cfg_reg_a = 0x00U;

	status = HAL_I2C_Mem_Read(&hi2c1, SLAVE_ADDRESS, CFG_REG_A, I2C_MEMADD_SIZE_8BIT, &cfg_reg_a, 1U, 25U);
    if (status != HAL_OK) {
        return -__LINE__;
    }

    return !!(cfg_reg_a & CFG_REG_A_COMP_TEMP_EN);
}

static int sample_single(void) {
	HAL_StatusTypeDef status = HAL_OK;
	unsigned char buffer[2] = {0};

	buffer[0] = CFG_REG_A;
	buffer[1] = CFG_REG_A_COMP_TEMP_EN | CFG_REG_A_MD_0;

    status = HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS, buffer, sizeof(buffer), 25U);
    if (status != HAL_OK) {
    	return -__LINE__;
    }

    return 0;
}

static int is_data_available(void) {
	HAL_StatusTypeDef status = HAL_OK;
	unsigned char status_reg = 0x00U;

	status = HAL_I2C_Mem_Read(&hi2c1, SLAVE_ADDRESS, STATUS_REG, I2C_MEMADD_SIZE_8BIT, &status_reg, 1U, 25U);
	if (status != HAL_OK) {
		return -__LINE__;
	}

    return !!(status_reg & STATUS_REG_ZYXDA);
}
