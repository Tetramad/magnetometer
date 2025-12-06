#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <stm32f4xx_hal.h>

#include <display.h>
#include <log.h>
#include <macros.h>
#include <micro_wait.h>

/* LCD Driver ST7066U */

#define DISPLAY_RS_WRITE(Handle, PinState)                                     \
    HAL_GPIO_WritePin(hdisplay->RS_Port, hdisplay->RS_Pin, PinState)
#define DISPLAY_RW_WRITE(Handle, PinState)                                     \
    HAL_GPIO_WritePin(hdisplay->RW_Port, hdisplay->RW_Pin, PinState)
#define DISPLAY_E_WRITE(Handle, PinState)                                      \
    HAL_GPIO_WritePin(hdisplay->E_Port, hdisplay->E_Pin, PinState)
#define DISPLAY_DB_WRITE(Handle, N, PinState)                                  \
    HAL_GPIO_WritePin((Handle)->DB_Port[(N)], (Handle)->DB_Pin[(N)], (PinState))

#define DISPLAY_DB_INIT(Handle, N, OpMode)                                     \
    HAL_GPIO_Init((Handle)->DB_Port[(N)],                                      \
                  &(GPIO_InitTypeDef){.Pin = (Handle)->DB_Pin[(N)],            \
                                      .Mode = OpMode,                          \
                                      .Pull = GPIO_NOPULL,                     \
                                      .Speed = GPIO_SPEED_FREQ_LOW})

static int CheckBusyFlag(DISPLAY_HandleTypeDef *hdisplay);
static void CommandInitial(DISPLAY_HandleTypeDef *hdisplay, uint8_t i);
static void Command(DISPLAY_HandleTypeDef *hdisplay, uint8_t i);
static void Clear(DISPLAY_HandleTypeDef *hdisplay);
static void Write(DISPLAY_HandleTypeDef *hdisplay, char c);

LOG_LEVEL_SET(LOG_LEVEL_DBG);

HAL_StatusTypeDef DISPLAY_Init(DISPLAY_HandleTypeDef *hdisplay) {
    if (hdisplay->Init.BusMode != DISPLAY_BUS_MODE_4 &&
        hdisplay->Init.BusMode != DISPLAY_BUS_MODE_8) {
        return HAL_ERROR;
    }

    hdisplay->BusMode = hdisplay->Init.BusMode;

    hdisplay->RS_Port = hdisplay->Init.RS_Port;
    hdisplay->RS_Pin = hdisplay->Init.RS_Pin;

    hdisplay->RW_Port = hdisplay->Init.RW_Port;
    hdisplay->RW_Pin = hdisplay->Init.RW_Pin;

    hdisplay->E_Port = hdisplay->Init.E_Port;
    hdisplay->E_Pin = hdisplay->Init.E_Pin;

    for (size_t i = 0; i < 4; ++i) {
        hdisplay->DB_Port[i] = hdisplay->Init.DB_Port[i];
        hdisplay->DB_Pin[i] = hdisplay->Init.DB_Pin[i];
    }
    if (hdisplay->BusMode == DISPLAY_BUS_MODE_8) {
        for (size_t i = 4; i < 8; ++i) {
            hdisplay->DB_Port[i] = hdisplay->Init.DB_Port[i];
            hdisplay->DB_Pin[i] = hdisplay->Init.DB_Pin[i];
        }
    }

    const uint32_t tick = HAL_GetTick();
    if (tick < 40U) {
        HAL_Delay(40U - tick);
    }

    CommandInitial(hdisplay, 0x30);
    CommandInitial(hdisplay, 0x30);
    CommandInitial(hdisplay, 0x30);
    CommandInitial(hdisplay, 0x20);

    Command(hdisplay, 0x28);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 37);
    Command(hdisplay, 0x10);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 37);
    Command(hdisplay, 0x0C);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 37);
    Command(hdisplay, 0x06);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 37);
    Command(hdisplay, 0x01);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 1520);

    DISPLAY_Print(hdisplay, "%s", "Init...");

    return HAL_OK;
}

HAL_StatusTypeDef
DISPLAY_Print(DISPLAY_HandleTypeDef *hdisplay, const char *format, ...) {
    char buffer[32] = "";
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    const size_t len = strlen(buffer);
    const size_t maxlen = len > 8 ? 8 : len;

    Clear(hdisplay);
    for (size_t i = 0; i < maxlen; ++i) {
        Write(hdisplay, buffer[i]);
    }

    return HAL_OK;
}

static int CheckBusyFlag(DISPLAY_HandleTypeDef *hdisplay) {
    DISPLAY_DB_WRITE(hdisplay, 0, GPIO_PIN_RESET);
    DISPLAY_DB_WRITE(hdisplay, 1, GPIO_PIN_RESET);
    DISPLAY_DB_WRITE(hdisplay, 2, GPIO_PIN_RESET);
    DISPLAY_DB_WRITE(hdisplay, 3, GPIO_PIN_RESET);

    DISPLAY_DB_INIT(hdisplay, 0, GPIO_MODE_INPUT);
    DISPLAY_DB_INIT(hdisplay, 1, GPIO_MODE_INPUT);
    DISPLAY_DB_INIT(hdisplay, 2, GPIO_MODE_INPUT);
    DISPLAY_DB_INIT(hdisplay, 3, GPIO_MODE_INPUT);

    DISPLAY_RS_WRITE(hdisplay, GPIO_PIN_RESET);
    DISPLAY_RW_WRITE(hdisplay, GPIO_PIN_SET);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 0);
    DISPLAY_E_WRITE(hdisplay, GPIO_PIN_SET);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 1);

    /* TODO: extract out HAL_GPIO_* */
    const GPIO_PinState busy_flag = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);

    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 1);
    DISPLAY_E_WRITE(hdisplay, GPIO_PIN_RESET);

    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 2);

    DISPLAY_RS_WRITE(hdisplay, GPIO_PIN_RESET);
    DISPLAY_RW_WRITE(hdisplay, GPIO_PIN_SET);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 0);
    DISPLAY_E_WRITE(hdisplay, GPIO_PIN_SET);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 1);

    __NOP();

    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 1);
    DISPLAY_E_WRITE(hdisplay, GPIO_PIN_RESET);

    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 2);

    DISPLAY_RS_WRITE(hdisplay, GPIO_PIN_RESET);
    DISPLAY_RW_WRITE(hdisplay, GPIO_PIN_RESET);
    DISPLAY_E_WRITE(hdisplay, GPIO_PIN_RESET);
    DISPLAY_DB_WRITE(hdisplay, 0, GPIO_PIN_RESET);
    DISPLAY_DB_WRITE(hdisplay, 1, GPIO_PIN_RESET);
    DISPLAY_DB_WRITE(hdisplay, 2, GPIO_PIN_RESET);
    DISPLAY_DB_WRITE(hdisplay, 3, GPIO_PIN_RESET);

    DISPLAY_DB_INIT(hdisplay, 0, GPIO_MODE_OUTPUT_PP);
    DISPLAY_DB_INIT(hdisplay, 1, GPIO_MODE_OUTPUT_PP);
    DISPLAY_DB_INIT(hdisplay, 2, GPIO_MODE_OUTPUT_PP);
    DISPLAY_DB_INIT(hdisplay, 3, GPIO_MODE_OUTPUT_PP);

    return busy_flag == GPIO_PIN_SET;
}

static void CommandInitial(DISPLAY_HandleTypeDef *hdisplay, uint8_t i) {
    assert(GPIO_PIN_SET == 1 && GPIO_PIN_RESET == 0);

    DISPLAY_RS_WRITE(hdisplay, GPIO_PIN_RESET);
    DISPLAY_RW_WRITE(hdisplay, GPIO_PIN_RESET);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 0);
    DISPLAY_E_WRITE(hdisplay, GPIO_PIN_SET);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 1);
    DISPLAY_DB_WRITE(hdisplay, 0, !!(i & (1 << 4)));
    DISPLAY_DB_WRITE(hdisplay, 1, !!(i & (1 << 5)));
    DISPLAY_DB_WRITE(hdisplay, 2, !!(i & (1 << 6)));
    DISPLAY_DB_WRITE(hdisplay, 3, !!(i & (1 << 7)));
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 1);
    DISPLAY_E_WRITE(hdisplay, GPIO_PIN_RESET);

    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 2);

    DISPLAY_DB_WRITE(hdisplay, 0, GPIO_PIN_RESET);
    DISPLAY_DB_WRITE(hdisplay, 1, GPIO_PIN_RESET);
    DISPLAY_DB_WRITE(hdisplay, 2, GPIO_PIN_RESET);
    DISPLAY_DB_WRITE(hdisplay, 3, GPIO_PIN_RESET);

    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 160);
}

static void Command(DISPLAY_HandleTypeDef *hdisplay, uint8_t i) {
    assert(GPIO_PIN_SET == 1 && GPIO_PIN_RESET == 0);

    for (size_t _ = 0; _ < 8 && CheckBusyFlag(hdisplay); ++_) {
        MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 37);
    }

    DISPLAY_RS_WRITE(hdisplay, GPIO_PIN_RESET);
    DISPLAY_RW_WRITE(hdisplay, GPIO_PIN_RESET);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 0);
    DISPLAY_E_WRITE(hdisplay, GPIO_PIN_SET);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 1);
    DISPLAY_DB_WRITE(hdisplay, 0, !!(i & (1 << 4)));
    DISPLAY_DB_WRITE(hdisplay, 1, !!(i & (1 << 5)));
    DISPLAY_DB_WRITE(hdisplay, 2, !!(i & (1 << 6)));
    DISPLAY_DB_WRITE(hdisplay, 3, !!(i & (1 << 7)));
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 1);
    DISPLAY_E_WRITE(hdisplay, GPIO_PIN_RESET);

    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 2);

    i <<= 4;

    DISPLAY_E_WRITE(hdisplay, GPIO_PIN_SET);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 1);
    DISPLAY_DB_WRITE(hdisplay, 0, !!(i & (1 << 4)));
    DISPLAY_DB_WRITE(hdisplay, 1, !!(i & (1 << 5)));
    DISPLAY_DB_WRITE(hdisplay, 2, !!(i & (1 << 6)));
    DISPLAY_DB_WRITE(hdisplay, 3, !!(i & (1 << 7)));
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 1);
    DISPLAY_E_WRITE(hdisplay, GPIO_PIN_RESET);

    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 2);
}

static void Write(DISPLAY_HandleTypeDef *hdisplay, char c) {
    assert(GPIO_PIN_SET == 1 && GPIO_PIN_RESET == 0);

    for (size_t i = 0; i < 8 && CheckBusyFlag(hdisplay); ++i) {
        MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 37);
    }

    DISPLAY_RS_WRITE(hdisplay, GPIO_PIN_SET);
    DISPLAY_RW_WRITE(hdisplay, GPIO_PIN_RESET);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 0);
    DISPLAY_E_WRITE(hdisplay, GPIO_PIN_SET);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 1);
    DISPLAY_DB_WRITE(hdisplay, 0, !!(c & (1 << 4)));
    DISPLAY_DB_WRITE(hdisplay, 1, !!(c & (1 << 5)));
    DISPLAY_DB_WRITE(hdisplay, 2, !!(c & (1 << 6)));
    DISPLAY_DB_WRITE(hdisplay, 3, !!(c & (1 << 7)));
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 1);
    DISPLAY_E_WRITE(hdisplay, GPIO_PIN_RESET);

    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 2);

    c <<= 4;

    DISPLAY_E_WRITE(hdisplay, GPIO_PIN_SET);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 1);
    DISPLAY_DB_WRITE(hdisplay, 0, !!(c & (1 << 4)));
    DISPLAY_DB_WRITE(hdisplay, 1, !!(c & (1 << 5)));
    DISPLAY_DB_WRITE(hdisplay, 2, !!(c & (1 << 6)));
    DISPLAY_DB_WRITE(hdisplay, 3, !!(c & (1 << 7)));
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 1);
    DISPLAY_E_WRITE(hdisplay, GPIO_PIN_RESET);

    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 2);

    DISPLAY_RS_WRITE(hdisplay, GPIO_PIN_RESET);
    DISPLAY_RW_WRITE(hdisplay, GPIO_PIN_RESET);
}

static void Clear(DISPLAY_HandleTypeDef *hdisplay) {
    Command(hdisplay, 0x02);
    MICROWAIT_DelayMicro(hdisplay->MICROWAITInstance, 1520);
}
