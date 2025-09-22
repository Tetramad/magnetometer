#include <stddef.h>
#include <stdint.h>

#include <stm32f4xx.h>

#include <display_internal.h>
#include <input.h>
#include <log.h>
#include <macros.h>
#include <sensor.h>
#include <sleep_and_wait.h>
#include <work_queue.h>

LOG_LEVEL_SET(LOG_LEVEL_INF);

static int init(void);
static int loop(void);

int main(void) {
    if (!(init() < 0)) {
        while (!(loop() < 0)) {
            __WFE();
        }
    }

    for (;;) {
        __WFE();
    }
}

static int init(void) {
    int err = 0;

    LOG_INF("Digital Magnetometer");
    LOG_INF("Hardware Version: v0.1.0");
    LOG_INF("Firmware Version: v0.1.0");

    err = work_queue_init();
    if (err < 0) {
        LOG_ERR("failed to initialize work queue");
        return -1;
    }

    err = sleep_and_wait_init();
    if (err < 0) {
        LOG_ERR("failed to initialize sleep and wait");
        return -1;
    }

    err = display_internal_init(NULL);
    if (err < 0) {
        LOG_ERR("failed to initialize display");
        return -1;
    }

    err = input_init(NULL);
    if (err < 0) {
        LOG_ERR("failed to initialize input");
        return -1;
    }

    err = sensor_init(NULL);
    if (err < 0) {
        LOG_ERR("failed to initialize sensor");
        return -1;
    }

    return 0;
}

static int loop(void) {
    int err = 0;
    work_t work = {0};

    if (work_queue_dequeue(&work) < 0) {
        return 0;
    }

    err = work.procedure(work.payload);
    if (err < 0) {
        LOG_ERR("work procedure returns error");
    } else {
        LOG_DBG("work procedure successed");
    }

    return 0;
}
