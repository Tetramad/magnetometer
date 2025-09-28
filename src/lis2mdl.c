#include <math.h>
#include <stdint.h>

#include <endianess.h>
#include <i2c.h>
#include <lis2mdl_reg.h>
#include <log.h>
#include <sensor.h>
#include <sleep_and_wait.h>

LOG_LEVEL_SET(LOG_LEVEL_DBG);

static int configure(void);
static int is_configured(void);
static int sample_single(void);
static int is_data_available(void);

int lis2mdl_init(void) {
    return 0;
}

int lis2mdl_measure(void) {
    int err = 0;
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

    LOOP(3) {
        busy_wait_us(500);
        err = sample_single();
        if (err < 0) {
            return -__LINE__;
        }

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

    err = standard_reg_read(SLAVE_ADDRESS, STATUS_REG, buffer, sizeof(buffer) - 2U);
    if (err < 0) {
        return -__LINE__;
    }

    err = standard_reg_read(SLAVE_ADDRESS, TEMP_OUT_L_REG, &buffer[7], 2U);
    if (err < 0) {
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

    magnetic_field_magnitude_mt = (signed)mag_meas;
    magnetic_field_x_axis_mt = (signed)mag_x_meas;
    magnetic_field_y_axis_mt = (signed)mag_y_meas;
    magnetic_field_z_axis_mt = (signed)mag_z_meas;
    temperature_celsius = (signed)t_meas;

    LOG_DBG(magnetic_field_magnitude_mt, "LIS2MDL: Magnetic Field (uT)");
    LOG_DBG(magnetic_field_x_axis_mt, "LIS2MDL: Magnetic Field X(uT)");
    LOG_DBG(magnetic_field_y_axis_mt, "LIS2MDL: Magnetic Field Y(uT)");
    LOG_DBG(magnetic_field_z_axis_mt, "LIS2MDL: Magnetic Field Z(uT)");
    LOG_DBG(temperature_celsius, "LIS2MDL: Temperature(m'C)");

    return 0;
}

static int configure(void) {
    int err = 0;
    unsigned char buffer[CFG_REG_C - CFG_REG_A + 1 + 1] = {0};

    buffer[0] = CFG_REG_A;
    buffer[CFG_REG_A - CFG_REG_A + 1] = CFG_REG_A_COMP_TEMP_EN;
    buffer[CFG_REG_B - CFG_REG_A + 1] = CFG_REG_B_LPF;
    buffer[CFG_REG_C - CFG_REG_A + 1] = CFG_REG_C_BDU;

    err = standard_write(SLAVE_ADDRESS, buffer, sizeof(buffer));
    if (err < 0) {
        return -__LINE__;
    }

    LOG_INF("LIS2MDL: configureation succeeded");

    return 0;
}

static int is_configured(void) {
    int err = 0;
    unsigned char cfg_reg_a = 0x00U;

    err = standard_reg_read(SLAVE_ADDRESS, CFG_REG_A, &cfg_reg_a, 1U);
    if (err < 0) {
        return -__LINE__;
    }

    return !!(cfg_reg_a & CFG_REG_A_COMP_TEMP_EN);
}

static int sample_single(void) {
    int err = 0;
    unsigned char buffer[2] = {0};

    buffer[0] = CFG_REG_A;
    buffer[1] = CFG_REG_A_COMP_TEMP_EN | CFG_REG_A_MD_0;

    err = standard_write(SLAVE_ADDRESS, buffer, sizeof(buffer));
    if (err < 0) {
        return -__LINE__;
    }

    return 0;
}

static int is_data_available(void) {
    int err = 0;
    unsigned char status_reg = 0x00U;

    err = standard_reg_read(SLAVE_ADDRESS, STATUS_REG, &status_reg, 1U);
    if (err < 0) {
        return -__LINE__;
    }

    return !!(status_reg & STATUS_REG_ZYXDA);
}
