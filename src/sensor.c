#include <stddef.h>

#include <stm32f4xx.h>

#include <i2c.h>
#include <log.h>
#include <macros.h>
#include <sensor.h>
#include <sleep_and_wait.h>
#include <tim3.h>
#include <work_queue.h>

#define SENSOR lis2mdl

#define SENSOR_HEADER <SENSOR.h>
#define SENSOR_INIT CONCAT(SENSOR, _init)
#define SENSOR_MEASURE CONCAT(SENSOR, _measure)

#include SENSOR_HEADER

LOG_LEVEL_SET(LOG_LEVEL_INF);

int temperature_celsius = 0;
int magnetic_field_x_axis_mt = 0;
int magnetic_field_y_axis_mt = 0;
int magnetic_field_z_axis_mt = 0;
int magnetic_field_magnitude_mt = 0;

static void tim3_callback(void *payload);

int sensor_init(void *payload) {
    ARG_UNUSED(payload);
    int err = 0;

    err = i2c_configure();
    if (err < 0) {
        LOG_ERR("SENSOR: failed to configure I2C");
        return -1;
    }

    err = SENSOR_INIT();
    if (err < 0) {
        LOG_ERR("SENSOR: failed to initialize TMAG5173");
        return -1;
    }

    tim3_initialize();
    tim3_register_isr(tim3_callback, NULL);
    tim3_enable();

    LOG_INF("SENSOR: initialize succeeded");

    return 0;
}

int sensor_measure(void *payload) {
    ARG_UNUSED(payload);
    int err = 0;

    err = SENSOR_MEASURE();
    if (err < 0) {
        LOG_ERR("SENSOR: failed to measure sensor readings");
        LOG_ERR(err, "SENSOR: err=");
        return -1;
    }

    return 0;
}

static void tim3_callback(void *payload) {
    ARG_UNUSED(payload);
    int err = 0;

    err = work_queue_enqueue(WORK_PRIORITY_DEFAULT, sensor_measure, NULL);
    if (err < 0) {
        LOG_ERR("SENSOR: failed to enqueue work \"sensor_measure\"");
    }
}
