#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include <endianess.h>
#include <i2c.h>
#include <log.h>
#include <macros.h>
#include <sensor.h>
#include <sleep_and_wait.h>
#include <tmag5173_reg.h>

LOG_LEVEL_SET(LOG_LEVEL_INF);

static int configure(void);
static int is_configured(void);
static int wakeup(void);

int tmag5173_init(void) {
    return 0;
}

int tmag5173_measure(void) {
    int err = 0;

    err = wakeup();
    if (err < 0) {
        return -__LINE__;
    }
    busy_wait_us(50);

    err = is_configured();
    if (err < 0) {
        return -__LINE__;
    }
    busy_wait_us(50);

    const int flag_configured = err > 0;
    if (!flag_configured) {
        err = configure();
        if (err < 0) {
            return -__LINE__;
        }
    }
    busy_wait_us(50);

    unsigned char buffer[10] = {0};
    const tick_t timeout = get_timeout_tick_ms(50);
    do {
        err = standard_reg_read(SLAVE_ADDRESS,
                                CONV_STATUS | 0x80U,
                                &buffer[0],
                                1U);
        if (err < 0) {
            return err;
            return -__LINE__;
        }
        busy_wait_us(2);
    } while (!(buffer[0] & 0x01U) && (get_ticks() < timeout));
    if (!(get_ticks() < timeout)) {
        return -__LINE__;
    }

    err = standard_reg_read(SLAVE_ADDRESS, T_MSB_RESULT, &buffer[0], 9U);
    if (err < 0) {
        LOG_ERR("TMAG5173: failed to get mearsurements");
        return -__LINE__;
    }
    LOG_DBG(buffer[8], "TMAG5173: CONV_STATUS");

    const float t_sens_t0 = 25.0f;
    const float t_adc_t0 = 17508.0f;
    const float t_adc_res = 58.0f;
    const float t_adc_t = (int16_t)get_be16(&buffer[0]);
    const float t_meas =
        t_sens_t0 * 1e3f + (t_adc_t - t_adc_t0) * 1e3f / t_adc_res;
    LOG_DBG((signed)t_meas, "TMAG5173: Temperature(m'C)");
    temperature_celsius = (signed)t_meas;

    const float mag_adc_res = 844.0f;
    const float mag_x_adc = (int16_t)get_be16(&buffer[2]);
    const float mag_x_meas = mag_x_adc * 1e3f / mag_adc_res;
    const float mag_y_adc = (int16_t)get_be16(&buffer[4]);
    const float mag_y_meas = mag_y_adc * 1e3f / mag_adc_res;
    const float mag_z_adc = (int16_t)get_be16(&buffer[6]);
    const float mag_z_meas = mag_z_adc * 1e3f / mag_adc_res;

    const float mag_meas =
        sqrtf(powf(mag_x_meas, 2.0f) + powf(mag_y_meas, 2.0f) +
              powf(mag_z_meas, 2.0f));

    LOG_DBG((signed)mag_meas, "TMAG5173: Magnetic Field (uT)");
    LOG_DBG((signed)mag_x_meas, "TMAG5173: Magnetic Field X(uT)");
    LOG_DBG((signed)mag_y_meas, "TMAG5173: Magnetic Field Y(uT)");
    LOG_DBG((signed)mag_z_meas, "TMAG5173: Magnetic Field Z(uT)");
    magnetic_field_magnitude_mt = (signed)mag_meas;
    magnetic_field_x_axis_mt = (signed)mag_x_meas;
    magnetic_field_y_axis_mt = (signed)mag_y_meas;
    magnetic_field_z_axis_mt = (signed)mag_z_meas;

    buffer[0] = DEVICE_CONFIG_2;
    buffer[1] =
        DEVICE_CONFIG_2_RESET_VALUE | DEVICE_CONFIG_2_OPERATING_MODE_SLEEP;
    err = standard_write(SLAVE_ADDRESS, buffer, 2U);
    if (err < 0) {
        LOG_ERR("TMAG5173: failed to configure sleep mode");
        return -__LINE__;
    }

    return 0;
}

static int configure(void) {
    int err = 0;
    unsigned char buffer[1 + DEVICE_STATUS + 1] = {0};

    for (size_t _ = 0; _ < 3; ++_) {
        err = standard_write(SLAVE_ADDRESS, NULL, 0U);
        if (!(err < 0)) {
            break;
        }
        busy_wait_us(50);
    }

    buffer[0] = DEVICE_CONFIG_1;
    err = standard_reg_read(SLAVE_ADDRESS,
                            buffer[0],
                            buffer + 1,
                            MAG_OFFSET_CONFIG_2 + 1);
    if (err < 0) {
        return -__LINE__;
    }

    LOG_DBG("TMAG5173: before configuration");
    LOG_DBG(buffer[1 + DEVICE_CONFIG_1], "TMAG5173: DEVICE_CONFIG_1");
    LOG_DBG(buffer[1 + DEVICE_CONFIG_2], "TMAG5173: DEVICE_CONFIG_2");
    LOG_DBG(buffer[1 + SENSOR_CONFIG_1], "TMAG5173: SENSOR_CONFIG_1");
    LOG_DBG(buffer[1 + SENSOR_CONFIG_2], "TMAG5173: SENSOR_CONFIG_2");
    LOG_DBG(buffer[1 + X_THR_CONFIG], "TMAG5173: X_THR_CONFIG");
    LOG_DBG(buffer[1 + Y_THR_CONFIG], "TMAG5173: Y_THR_CONFIG");
    LOG_DBG(buffer[1 + Z_THR_CONFIG], "TMAG5173: Z_THR_CONFIG");
    LOG_DBG(buffer[1 + T_CONFIG], "TMAG5173: T_CONFIG");
    LOG_DBG(buffer[1 + INT_CONFIG_1], "TMAG5173: INT_CONFIG_1");
    LOG_DBG(buffer[1 + MAG_GAIN_CONFIG], "TMAG5173: MAG_GAIN_CONFIG");
    LOG_DBG(buffer[1 + MAG_OFFSET_CONFIG_1], "TMAG5173: MAG_OFFSET_CONFIG_1");
    LOG_DBG(buffer[1 + MAG_OFFSET_CONFIG_2], "TMAG5173: MAG_OFFSET_CONFIG_2");
    LOG_DBG(buffer[1 + I2C_ADDRESS], "TMAG5173: I2C_ADDRESS");
    LOG_DBG(buffer[1 + DEVICE_ID], "TMAG5173: DEVICE_ID");
    LOG_DBG(buffer[1 + MANUFACTURER_ID_LSB], "TMAG5173: MANUFACTURER_ID_LSB");
    LOG_DBG(buffer[1 + MANUFACTURER_ID_MSB], "TMAG5173: MANUFACTURER_ID_MSB");
    LOG_DBG(buffer[1 + T_MSB_RESULT], "TMAG5173: T_MSB_RESULT");
    LOG_DBG(buffer[1 + T_LSB_RESULT], "TMAG5173: T_LSB_RESULT");
    LOG_DBG(buffer[1 + X_MSB_RESULT], "TMAG5173: X_MSB_RESULT");
    LOG_DBG(buffer[1 + X_LSB_RESULT], "TMAG5173: X_LSB_RESULT");
    LOG_DBG(buffer[1 + Y_MSB_RESULT], "TMAG5173: Y_MSB_RESULT");
    LOG_DBG(buffer[1 + Y_LSB_RESULT], "TMAG5173: Y_LSB_RESULT");
    LOG_DBG(buffer[1 + Z_MSB_RESULT], "TMAG5173: Z_MSB_RESULT");
    LOG_DBG(buffer[1 + Z_LSB_RESULT], "TMAG5173: Z_LSB_RESULT");
    LOG_DBG(buffer[1 + CONV_STATUS], "TMAG5173: CONV_STATUS");
    LOG_DBG(buffer[1 + ANGLE_RESULT_MSB], "TMAG5173: ANGLE_RESULT_MSB");
    LOG_DBG(buffer[1 + ANGLE_RESULT_LSB], "TMAG5173: ANGLE_RESULT_LSB");
    LOG_DBG(buffer[1 + MAGNITUDE_RESULT], "TMAG5173: MAGNITUDE_RESULT");
    LOG_DBG(buffer[1 + DEVICE_STATUS], "TMAG5173: DEVICE_STATUS");

    buffer[0] = DEVICE_CONFIG_1;
    buffer[1 + DEVICE_CONFIG_1] = DEVICE_CONFIG_1_CONV_AVG_1X;
    buffer[1 + DEVICE_CONFIG_2] = DEVICE_CONFIG_2_OPERATING_MODE_STANDBY;
    buffer[1 + SENSOR_CONFIG_1] = SENSOR_CONFIG_1_MAG_CH_EN_XYZ;
    buffer[1 + SENSOR_CONFIG_2] = SENSOR_CONFIG_2_RESET_VALUE;
    buffer[1 + X_THR_CONFIG] = X_THR_CONFIG_RESET_VALUE;
    buffer[1 + Y_THR_CONFIG] = Y_THR_CONFIG_RESET_VALUE;
    buffer[1 + Z_THR_CONFIG] = Z_THR_CONFIG_RESET_VALUE;
    buffer[1 + T_CONFIG] = T_CONFIG_T_CH_EN;
    buffer[1 + INT_CONFIG_1] = INT_CONFIG_1_MASK_INTB_INT_DISABLED;
    buffer[1 + MAG_GAIN_CONFIG] = MAG_GAIN_CONFIG_RESET_VALUE;
    buffer[1 + MAG_OFFSET_CONFIG_1] = MAG_OFFSET_CONFIG_1_RESET_VALUE;
    buffer[1 + MAG_OFFSET_CONFIG_2] = MAG_OFFSET_CONFIG_2_RESET_VALUE;

    err = standard_write(SLAVE_ADDRESS, buffer, 1 + T_CONFIG + 1);
    if (err < 0) {
        LOG_ERR("TMAG5173: failed to configure default setting");
        return -__LINE__;
    }

    buffer[0] = CONV_STATUS;
    buffer[1] = CONV_STATUS_POR;
    err = standard_write(SLAVE_ADDRESS, buffer, 2U);
    if (err < 0) {
        LOG_ERR("TMAG5173: failed to reset POR");
        return -__LINE__;
    }

    LOG_INF("TMAG5173: configuration succeeded");

    return 0;
}

static int is_configured(void) {
    int err = 0;
    unsigned char reg_status = 0x00U;

    err = standard_reg_read(SLAVE_ADDRESS, CONV_STATUS, &reg_status, 1U);
    if (err < 0) {
        return -__LINE__;
    }
    busy_wait_us(2);

    return !(reg_status & CONV_STATUS_POR_Msk);
}

static int wakeup(void) {
    int err = 0;

    for (size_t _ = 0; _ < 3; ++_) {
        err = standard_write(SLAVE_ADDRESS, NULL, 0U);
        if (!(err < 0)) {
            return 0;
        }
        busy_wait_us(50);
    }

    return -__LINE__;
}
