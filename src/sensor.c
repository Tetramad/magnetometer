#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <stm32f4xx.h>

#include <TMAG5173-Q1.h>
#include <display_internal.h>
#include <endianess.h>
#include <gpio.h>
#include <i2c.h>
#include <log.h>
#include <macros.h>
#include <sensor.h>
#include <sleep_and_wait.h>
#include <tim3.h>
#include <work_queue.h>

LOG_LEVEL_SET(LOG_LEVEL_INF);

int temperature_celsius = 0;
int magnetic_field_x_axis_mt = 0;
int magnetic_field_y_axis_mt = 0;
int magnetic_field_z_axis_mt = 0;
int magnetic_field_magnitude_mt = 0;

static void tim3_callback(void *payload);
static int configure(void);

int sensor_init(void *payload) {
    ARG_UNUSED(payload);
    int err = 0;

    err = i2c_configure();
    if (err < 0) {
        LOG_ERR("MAG: failed to configure I2C");
        return -1;
    }

    tim3_initialize();
    tim3_register_isr(tim3_callback, NULL);
    tim3_enable();

    LOG_INF("MAG: initialize succeeded");

    return 0;
}

int sensor_measure(void *payload) {
    ARG_UNUSED(payload);
    int err = 0;
    tick_t timeout = 0U;

    for (size_t _ = 0; _ < 3; ++_) {
        err = standard_write(SLAVE_ADDRESS, NULL, 0U);
        if (!(err < 0)) {
            break;
        }
        busy_wait_us(50);
    }

    unsigned char buffer[10] = {0};
    err = standard_reg_read(SLAVE_ADDRESS, CONV_STATUS, &buffer[0], 1U);
    if (err < 0) {
        return -1;
    }
    busy_wait_us(2);

    if (buffer[0] & CONV_STATUS_POR_Msk) {
        err = configure();
        if (err < 0) {
            return -1;
        }
    }

    timeout = get_ticks() + 50 * 2000;
    do {
        err = standard_reg_read(SLAVE_ADDRESS,
                                CONV_STATUS | 0x80U,
                                &buffer[0],
                                1U);
        if (err < 0) {
            return -1;
        }
        busy_wait_us(2);
    } while (!(buffer[0] & 0x01U) && (get_ticks() < timeout));
    if (!(get_ticks() < timeout)) {
        return -1;
    }

    err = standard_reg_read(SLAVE_ADDRESS, T_MSB_RESULT, &buffer[0], 9U);
    if (err < 0) {
        LOG_ERR("MAG: failed to get mearsurements");
        return -1;
    }
    LOG_DBG(buffer[8], "MAG: CONV_STATUS");

    const float t_sens_t0 = 25.0f;
    const float t_adc_t0 = 17508.0f;
    const float t_adc_res = 58.0f;
    const float t_adc_t = (int16_t)get_be16(&buffer[0]);
    const float t_meas =
        t_sens_t0 * 1e3f + (t_adc_t - t_adc_t0) * 1e3f / t_adc_res;
    LOG_DBG((signed)t_meas, "MAG: Temperature(m'C)");
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

    LOG_DBG((signed)mag_meas, "MAG: Magnetic Field (uT)");
    LOG_DBG((signed)mag_x_meas, "MAG: Magnetic Field X(uT)");
    LOG_DBG((signed)mag_y_meas, "MAG: Magnetic Field Y(uT)");
    LOG_DBG((signed)mag_z_meas, "MAG: Magnetic Field Z(uT)");
    magnetic_field_magnitude_mt = (signed)mag_meas;
    magnetic_field_x_axis_mt = (signed)mag_x_meas;
    magnetic_field_y_axis_mt = (signed)mag_y_meas;
    magnetic_field_z_axis_mt = (signed)mag_z_meas;

    buffer[0] = DEVICE_CONFIG_2;
    buffer[1] =
        DEVICE_CONFIG_2_RESET_VALUE | DEVICE_CONFIG_2_OPERATING_MODE_SLEEP;
    err = standard_write(SLAVE_ADDRESS, buffer, 2U);
    if (err < 0) {
        LOG_ERR("MAG: failed to configure sleep mode");
        return -1;
    }

    return 0;
}

static void tim3_callback(void *payload) {
    ARG_UNUSED(payload);
    int err = 0;

    err = work_queue_enqueue(WORK_PRIORITY_DEFAULT, sensor_measure, NULL);
    if (err < 0) {
        LOG_ERR("failed to enqueue work \"sensor_measure\"");
    }
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
        return -1;
    }

    LOG_DBG("MAG: before configuration");
    LOG_DBG(buffer[1 + DEVICE_CONFIG_1], "MAG: DEVICE_CONFIG_1");
    LOG_DBG(buffer[1 + DEVICE_CONFIG_2], "MAG: DEVICE_CONFIG_2");
    LOG_DBG(buffer[1 + SENSOR_CONFIG_1], "MAG: SENSOR_CONFIG_1");
    LOG_DBG(buffer[1 + SENSOR_CONFIG_2], "MAG: SENSOR_CONFIG_2");
    LOG_DBG(buffer[1 + X_THR_CONFIG], "MAG: X_THR_CONFIG");
    LOG_DBG(buffer[1 + Y_THR_CONFIG], "MAG: Y_THR_CONFIG");
    LOG_DBG(buffer[1 + Z_THR_CONFIG], "MAG: Z_THR_CONFIG");
    LOG_DBG(buffer[1 + T_CONFIG], "MAG: T_CONFIG");
    LOG_DBG(buffer[1 + INT_CONFIG_1], "MAG: INT_CONFIG_1");
    LOG_DBG(buffer[1 + MAG_GAIN_CONFIG], "MAG: MAG_GAIN_CONFIG");
    LOG_DBG(buffer[1 + MAG_OFFSET_CONFIG_1], "MAG: MAG_OFFSET_CONFIG_1");
    LOG_DBG(buffer[1 + MAG_OFFSET_CONFIG_2], "MAG: MAG_OFFSET_CONFIG_2");
    LOG_DBG(buffer[1 + I2C_ADDRESS], "MAG: I2C_ADDRESS");
    LOG_DBG(buffer[1 + DEVICE_ID], "MAG: DEVICE_ID");
    LOG_DBG(buffer[1 + MANUFACTURER_ID_LSB], "MAG: MANUFACTURER_ID_LSB");
    LOG_DBG(buffer[1 + MANUFACTURER_ID_MSB], "MAG: MANUFACTURER_ID_MSB");
    LOG_DBG(buffer[1 + T_MSB_RESULT], "MAG: T_MSB_RESULT");
    LOG_DBG(buffer[1 + T_LSB_RESULT], "MAG: T_LSB_RESULT");
    LOG_DBG(buffer[1 + X_MSB_RESULT], "MAG: X_MSB_RESULT");
    LOG_DBG(buffer[1 + X_LSB_RESULT], "MAG: X_LSB_RESULT");
    LOG_DBG(buffer[1 + Y_MSB_RESULT], "MAG: Y_MSB_RESULT");
    LOG_DBG(buffer[1 + Y_LSB_RESULT], "MAG: Y_LSB_RESULT");
    LOG_DBG(buffer[1 + Z_MSB_RESULT], "MAG: Z_MSB_RESULT");
    LOG_DBG(buffer[1 + Z_LSB_RESULT], "MAG: Z_LSB_RESULT");
    LOG_DBG(buffer[1 + CONV_STATUS], "MAG: CONV_STATUS");
    LOG_DBG(buffer[1 + ANGLE_RESULT_MSB], "MAG: ANGLE_RESULT_MSB");
    LOG_DBG(buffer[1 + ANGLE_RESULT_LSB], "MAG: ANGLE_RESULT_LSB");
    LOG_DBG(buffer[1 + MAGNITUDE_RESULT], "MAG: MAGNITUDE_RESULT");
    LOG_DBG(buffer[1 + DEVICE_STATUS], "MAG: DEVICE_STATUS");

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
        LOG_ERR("MAG: failed to configure default setting");
        return -1;
    }

    buffer[0] = CONV_STATUS;
    buffer[1] = CONV_STATUS_POR;
    err = standard_write(SLAVE_ADDRESS, buffer, 2U);
    if (err < 0) {
        LOG_ERR("MAG: failed to reset POR");
        return -1;
    }

    buffer[0] = DEVICE_CONFIG_2;
    buffer[1] = DEVICE_CONFIG_2_OPERATING_MODE_SLEEP;
    err = standard_write(SLAVE_ADDRESS, buffer, 2U);
    if (err < 0) {
        LOG_ERR("MAG: failed to configure sleep mode");
        return -1;
    }

    LOG_INF("MAG: configuration succeeded");

    return 0;
}
