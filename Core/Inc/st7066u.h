#ifndef ST7066U_H_
#define ST7066U_H_

#include <stdint.h>

#include <stm32f4xx_hal.h>

#include <udelay.h>

enum {
    ST7066U_BUS_MODE_4,
    ST7066U_BUS_MODE_8,
};

typedef unsigned int ST7066U_BusModeTypeDef;

typedef struct {
    ST7066U_BusModeTypeDef BusMode;
    GPIO_TypeDef *RS_Port;
    uint16_t RS_Pin;
    GPIO_TypeDef *RW_Port;
    uint16_t RW_Pin;
    GPIO_TypeDef *E_Port;
    uint16_t E_Pin;
    GPIO_TypeDef *DB_Port[8];
    uint16_t DB_Pin[8];
} ST7066U_InitTypeDef;

typedef struct {
    UDELAY_HandleTypeDef *MICROWAITInstance;
    ST7066U_InitTypeDef Init;
    ST7066U_BusModeTypeDef BusMode;
    GPIO_TypeDef *RS_Port;
    uint16_t RS_Pin;
    GPIO_TypeDef *RW_Port;
    uint16_t RW_Pin;
    GPIO_TypeDef *E_Port;
    uint16_t E_Pin;
    GPIO_TypeDef *DB_Port[8];
    uint16_t DB_Pin[8];
} ST7066U_HandleTypeDef;

HAL_StatusTypeDef ST7066U_Init(ST7066U_HandleTypeDef *hdisplay);
HAL_StatusTypeDef
ST7066U_Print(ST7066U_HandleTypeDef *hdisplay, const char *format, ...);
HAL_StatusTypeDef ST7066U_CheckSanity(ST7066U_HandleTypeDef *hdisplay);

#endif /* ST7066U_H_ */
