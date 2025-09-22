#include <stm32f4xx.h>

#include <gpio.h>
#include <log.h>
#include <macros.h>
#include <input.h>

LOG_LEVEL_SET(LOG_LEVEL_INF);

int input_init(void *payload) {
    ARG_UNUSED(payload);
    int err = 0;

    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN);

    err = gpio_configure((struct gpio_config){.port = GPIO_CONFIG_PORT_B,
                                              .pin = GPIO_CONFIG_PIN_5,
                                              .mode = GPIO_CONFIG_MODE_INPUT,
                                              .pupd = GPIO_CONFIG_PUPD_PU});
    if (err < 0) {
        return -1;
    }

    err = gpio_configure((struct gpio_config){.port = GPIO_CONFIG_PORT_B,
                                              .pin = GPIO_CONFIG_PIN_6,
                                              .mode = GPIO_CONFIG_MODE_INPUT,
                                              .pupd = GPIO_CONFIG_PUPD_PU});
    if (err < 0) {
        return -1;
    }

    err = gpio_configure((struct gpio_config){.port = GPIO_CONFIG_PORT_B,
                                              .pin = GPIO_CONFIG_PIN_7,
                                              .mode = GPIO_CONFIG_MODE_INPUT,
                                              .pupd = GPIO_CONFIG_PUPD_PU});
    if (err < 0) {
        return -1;
    }

    LOG_INF("INPUT: initialize succeeded");

    return 0;
}

struct input_context input_new_context(void) {
    return (struct input_context){.left = 0, .right = 0, .select = 0};
}

/*
 * LATEST | INPUT |>| RESULT
 * 0        0         0
 * 0        1         1
 * 1        0         0
 * 1        1         -1
 * -1       0         0
 * -1       1         -1
 */
int input_update_context(struct input_context *ctx) {
    __disable_irq();
    const int left = !READ_BIT(GPIOB->IDR, GPIO_IDR_ID5);
    const int right = !READ_BIT(GPIOB->IDR, GPIO_IDR_ID6);
    const int select = !READ_BIT(GPIOB->IDR, GPIO_IDR_ID7);
    __enable_irq();

    LOG_DBG(left, "INPUT: left=");
    LOG_DBG(right, "INPUT: right=");
    LOG_DBG(select, "INPUT: select=");

    ctx->left = left == 0 ? 0 : -(2 * !!ctx->left - 1);
    ctx->right = right == 0 ? 0 : -(2 * !!ctx->right - 1);
    ctx->select = select == 0 ? 0 : -(2 * !!ctx->select - 1);

    return 0;
}
