#include <stm32f4xx_hal.h>

#include <log.h>
#include <macros.h>
#include <input.h>

LOG_LEVEL_SET(LOG_LEVEL_INF);

int Input_Init(void *payload) {
    ARG_UNUSED(payload);

    LOG_INF("INPUT: initialize succeeded");

    return 0;
}

struct InputContext Input_NewContext(void) {
    return (struct InputContext){.left = 0, .right = 0, .select = 0};
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
int Input_UpdateContext(struct InputContext *ctx) {
    __disable_irq();
    const int left = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_RESET;
    const int right = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_RESET;
    const int select = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_RESET;
    __enable_irq();

    LOG_DBG(left, "INPUT: left=");
    LOG_DBG(right, "INPUT: right=");
    LOG_DBG(select, "INPUT: select=");

    ctx->left = left == 0 ? 0 : -(2 * !!ctx->left - 1);
    ctx->right = right == 0 ? 0 : -(2 * !!ctx->right - 1);
    ctx->select = select == 0 ? 0 : -(2 * !!ctx->select - 1);

    return 0;
}
