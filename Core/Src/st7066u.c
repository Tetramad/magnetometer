#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <stm32f4xx_hal.h>

#include <st7066u.h>
#include <log.h>
#include <macros.h>
#include <udelay.h>

#define ST7066U_RS_WRITE(Handle, PinState)                                     \
    HAL_GPIO_WritePin(hdisplay->RS_Port, hdisplay->RS_Pin, PinState)
#define ST7066U_RW_WRITE(Handle, PinState)                                     \
    HAL_GPIO_WritePin(hdisplay->RW_Port, hdisplay->RW_Pin, PinState)
#define ST7066U_E_WRITE(Handle, PinState)                                      \
    HAL_GPIO_WritePin(hdisplay->E_Port, hdisplay->E_Pin, PinState)
#define ST7066U_DB_WRITE(Handle, N, PinState)                                  \
    HAL_GPIO_WritePin((Handle)->DB_Port[(N)], (Handle)->DB_Pin[(N)], (PinState))

#define ST7066U_DB_READ(Handle, N)                                             \
    HAL_GPIO_ReadPin((Handle)->DB_Port[(N)], (Handle)->DB_Pin[(N)]);

#define ST7066U_DB_INIT(Handle, N, OpMode)                                     \
    HAL_GPIO_Init((Handle)->DB_Port[(N)],                                      \
                  &(GPIO_InitTypeDef){.Pin = (Handle)->DB_Pin[(N)],            \
                                      .Mode = OpMode,                          \
                                      .Pull = GPIO_NOPULL,                     \
                                      .Speed = GPIO_SPEED_FREQ_LOW})

static int CheckBusyFlag(ST7066U_HandleTypeDef *hdisplay);
static uint8_t ReadAddress(ST7066U_HandleTypeDef *hdisplay);
static void CommandInitial(ST7066U_HandleTypeDef *hdisplay, uint8_t i);
static void Command(ST7066U_HandleTypeDef *hdisplay, uint8_t i);
static void Clear(ST7066U_HandleTypeDef *hdisplay);
static void Write(ST7066U_HandleTypeDef *hdisplay, char c);

LOG_LEVEL_SET(LOG_LEVEL_INF);

HAL_StatusTypeDef ST7066U_Init(ST7066U_HandleTypeDef *hdisplay) {
    if (hdisplay->Init.BusMode != ST7066U_BUS_MODE_4 &&
        hdisplay->Init.BusMode != ST7066U_BUS_MODE_8) {
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
    if (hdisplay->BusMode == ST7066U_BUS_MODE_8) {
        for (size_t i = 4; i < 8; ++i) {
            hdisplay->DB_Port[i] = hdisplay->Init.DB_Port[i];
            hdisplay->DB_Pin[i] = hdisplay->Init.DB_Pin[i];
        }
    }

    uint32_t tick = HAL_GetTick();
    if (tick < 40) {
        HAL_Delay(40 - tick);
    }

    CommandInitial(hdisplay, 0x30);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 37);
    CommandInitial(hdisplay, 0x30);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 37);
    CommandInitial(hdisplay, 0x30);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 37);
    CommandInitial(hdisplay, 0x20);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 37);

    Command(hdisplay, 0x28);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 37);
    Command(hdisplay, 0x10);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 37);
    Command(hdisplay, 0x0C);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 37);
    Command(hdisplay, 0x06);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 37);
    Command(hdisplay, 0x01);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1520);

    ST7066U_Print(hdisplay, "%s", "Init...");

    if (ST7066U_CheckSanity(hdisplay) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef
ST7066U_Print(ST7066U_HandleTypeDef *hdisplay, const char *format, ...) {
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

HAL_StatusTypeDef ST7066U_CheckSanity(ST7066U_HandleTypeDef *hdisplay) {
    const uint8_t old = ReadAddress(hdisplay);
    uint8_t address = 0x00U;

    Command(hdisplay, 0x70 | 0x80);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 37);
    address = ReadAddress(hdisplay);
    if ((address & 0x7F) != (0x70 & 0x7F)) {
        return HAL_ERROR;
    }

    Command(hdisplay, 0x4C | 0x80);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 37);
    address = ReadAddress(hdisplay);
    if ((address & 0x7F) != (0x4C & 0x7F)) {
        return HAL_ERROR;
    }

    Command(hdisplay, 0x01 | 0x80);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 37);
    address = ReadAddress(hdisplay);
    if ((address & 0x7F) != (0x01 & 0x7F)) {
        return HAL_ERROR;
    }

    Command(hdisplay, old | 0x80);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 37);

    return HAL_OK;
}

static int CheckBusyFlag(ST7066U_HandleTypeDef *hdisplay) {
    assert(GPIO_PIN_SET == 1 && GPIO_PIN_RESET == 0);

    ST7066U_DB_WRITE(hdisplay, 0, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 1, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 2, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 3, GPIO_PIN_RESET);

    ST7066U_DB_INIT(hdisplay, 0, GPIO_MODE_INPUT);
    ST7066U_DB_INIT(hdisplay, 1, GPIO_MODE_INPUT);
    ST7066U_DB_INIT(hdisplay, 2, GPIO_MODE_INPUT);
    ST7066U_DB_INIT(hdisplay, 3, GPIO_MODE_INPUT);

    ST7066U_RS_WRITE(hdisplay, GPIO_PIN_RESET);
    ST7066U_RW_WRITE(hdisplay, GPIO_PIN_SET);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 0);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_SET);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);

    const GPIO_PinState busy_flag = ST7066U_DB_READ(hdisplay, 3);

    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_RESET);

    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 2);

    ST7066U_RS_WRITE(hdisplay, GPIO_PIN_RESET);
    ST7066U_RW_WRITE(hdisplay, GPIO_PIN_SET);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 0);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_SET);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);

    __NOP();

    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_RESET);

    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 2);

    ST7066U_RS_WRITE(hdisplay, GPIO_PIN_RESET);
    ST7066U_RW_WRITE(hdisplay, GPIO_PIN_RESET);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 0, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 1, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 2, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 3, GPIO_PIN_RESET);

    ST7066U_DB_INIT(hdisplay, 0, GPIO_MODE_OUTPUT_PP);
    ST7066U_DB_INIT(hdisplay, 1, GPIO_MODE_OUTPUT_PP);
    ST7066U_DB_INIT(hdisplay, 2, GPIO_MODE_OUTPUT_PP);
    ST7066U_DB_INIT(hdisplay, 3, GPIO_MODE_OUTPUT_PP);

    return busy_flag == GPIO_PIN_SET;
}

static uint8_t ReadAddress(ST7066U_HandleTypeDef *hdisplay) {
    assert(GPIO_PIN_SET == 1 && GPIO_PIN_RESET == 0);

    uint8_t address = 0;

    ST7066U_DB_WRITE(hdisplay, 0, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 1, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 2, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 3, GPIO_PIN_RESET);

    ST7066U_DB_INIT(hdisplay, 0, GPIO_MODE_INPUT);
    ST7066U_DB_INIT(hdisplay, 1, GPIO_MODE_INPUT);
    ST7066U_DB_INIT(hdisplay, 2, GPIO_MODE_INPUT);
    ST7066U_DB_INIT(hdisplay, 3, GPIO_MODE_INPUT);

    ST7066U_RS_WRITE(hdisplay, GPIO_PIN_RESET);
    ST7066U_RW_WRITE(hdisplay, GPIO_PIN_SET);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 0);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_SET);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);

    address <<= 1;
    address |= ST7066U_DB_READ(hdisplay, 3);
    address <<= 1;
    address |= ST7066U_DB_READ(hdisplay, 2);
    address <<= 1;
    address |= ST7066U_DB_READ(hdisplay, 1);
    address <<= 1;
    address |= ST7066U_DB_READ(hdisplay, 0);

    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_RESET);

    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 2);

    ST7066U_RS_WRITE(hdisplay, GPIO_PIN_RESET);
    ST7066U_RW_WRITE(hdisplay, GPIO_PIN_SET);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 0);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_SET);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);

    address <<= 1;
    address |= ST7066U_DB_READ(hdisplay, 3);
    address <<= 1;
    address |= ST7066U_DB_READ(hdisplay, 2);
    address <<= 1;
    address |= ST7066U_DB_READ(hdisplay, 1);
    address <<= 1;
    address |= ST7066U_DB_READ(hdisplay, 0);

    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_RESET);

    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 2);

    ST7066U_RS_WRITE(hdisplay, GPIO_PIN_RESET);
    ST7066U_RW_WRITE(hdisplay, GPIO_PIN_RESET);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 0, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 1, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 2, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 3, GPIO_PIN_RESET);

    ST7066U_DB_INIT(hdisplay, 0, GPIO_MODE_OUTPUT_PP);
    ST7066U_DB_INIT(hdisplay, 1, GPIO_MODE_OUTPUT_PP);
    ST7066U_DB_INIT(hdisplay, 2, GPIO_MODE_OUTPUT_PP);
    ST7066U_DB_INIT(hdisplay, 3, GPIO_MODE_OUTPUT_PP);

    return address & 0x7F;
}

static void CommandInitial(ST7066U_HandleTypeDef *hdisplay, uint8_t i) {
    assert(GPIO_PIN_SET == 1 && GPIO_PIN_RESET == 0);

    ST7066U_RS_WRITE(hdisplay, GPIO_PIN_RESET);
    ST7066U_RW_WRITE(hdisplay, GPIO_PIN_RESET);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 0, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 1, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 2, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 3, GPIO_PIN_RESET);

    ST7066U_DB_INIT(hdisplay, 0, GPIO_MODE_OUTPUT_PP);
    ST7066U_DB_INIT(hdisplay, 1, GPIO_MODE_OUTPUT_PP);
    ST7066U_DB_INIT(hdisplay, 2, GPIO_MODE_OUTPUT_PP);
    ST7066U_DB_INIT(hdisplay, 3, GPIO_MODE_OUTPUT_PP);

    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 0);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_SET);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);
    ST7066U_DB_WRITE(hdisplay, 0, !!(i & (1 << 4)));
    ST7066U_DB_WRITE(hdisplay, 1, !!(i & (1 << 5)));
    ST7066U_DB_WRITE(hdisplay, 2, !!(i & (1 << 6)));
    ST7066U_DB_WRITE(hdisplay, 3, !!(i & (1 << 7)));
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_RESET);

    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 2);

    ST7066U_DB_WRITE(hdisplay, 0, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 1, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 2, GPIO_PIN_RESET);
    ST7066U_DB_WRITE(hdisplay, 3, GPIO_PIN_RESET);

    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 160);
}

static void Command(ST7066U_HandleTypeDef *hdisplay, uint8_t i) {
    assert(GPIO_PIN_SET == 1 && GPIO_PIN_RESET == 0);

    for (size_t _ = 0; _ < 8 && CheckBusyFlag(hdisplay); ++_) {
        UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 37);
    }

    ST7066U_RS_WRITE(hdisplay, GPIO_PIN_RESET);
    ST7066U_RW_WRITE(hdisplay, GPIO_PIN_RESET);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 0);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_SET);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);
    ST7066U_DB_WRITE(hdisplay, 0, !!(i & (1 << 4)));
    ST7066U_DB_WRITE(hdisplay, 1, !!(i & (1 << 5)));
    ST7066U_DB_WRITE(hdisplay, 2, !!(i & (1 << 6)));
    ST7066U_DB_WRITE(hdisplay, 3, !!(i & (1 << 7)));
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_RESET);

    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 2);

    i <<= 4;

    ST7066U_E_WRITE(hdisplay, GPIO_PIN_SET);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);
    ST7066U_DB_WRITE(hdisplay, 0, !!(i & (1 << 4)));
    ST7066U_DB_WRITE(hdisplay, 1, !!(i & (1 << 5)));
    ST7066U_DB_WRITE(hdisplay, 2, !!(i & (1 << 6)));
    ST7066U_DB_WRITE(hdisplay, 3, !!(i & (1 << 7)));
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_RESET);

    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 2);
}

static void Write(ST7066U_HandleTypeDef *hdisplay, char c) {
    assert(GPIO_PIN_SET == 1 && GPIO_PIN_RESET == 0);

    for (size_t i = 0; i < 8 && CheckBusyFlag(hdisplay); ++i) {
        UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 37);
    }

    ST7066U_RS_WRITE(hdisplay, GPIO_PIN_SET);
    ST7066U_RW_WRITE(hdisplay, GPIO_PIN_RESET);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 0);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_SET);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);
    ST7066U_DB_WRITE(hdisplay, 0, !!(c & (1 << 4)));
    ST7066U_DB_WRITE(hdisplay, 1, !!(c & (1 << 5)));
    ST7066U_DB_WRITE(hdisplay, 2, !!(c & (1 << 6)));
    ST7066U_DB_WRITE(hdisplay, 3, !!(c & (1 << 7)));
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_RESET);

    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 2);

    c <<= 4;

    ST7066U_E_WRITE(hdisplay, GPIO_PIN_SET);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);
    ST7066U_DB_WRITE(hdisplay, 0, !!(c & (1 << 4)));
    ST7066U_DB_WRITE(hdisplay, 1, !!(c & (1 << 5)));
    ST7066U_DB_WRITE(hdisplay, 2, !!(c & (1 << 6)));
    ST7066U_DB_WRITE(hdisplay, 3, !!(c & (1 << 7)));
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1);
    ST7066U_E_WRITE(hdisplay, GPIO_PIN_RESET);

    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 2);

    ST7066U_RS_WRITE(hdisplay, GPIO_PIN_RESET);
    ST7066U_RW_WRITE(hdisplay, GPIO_PIN_RESET);
}

static void Clear(ST7066U_HandleTypeDef *hdisplay) {
    Command(hdisplay, 0x01);
    UDELAY_DelayMicro(hdisplay->MICROWAITInstance, 1520);
}
