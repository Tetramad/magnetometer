#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>

#include <stm32f4xx_hal.h>

#include <micro_wait.h>

enum {
    DISPLAY_BUS_MODE_4,
    DISPLAY_BUS_MODE_8,
};

typedef unsigned int DISPLAY_BusModeTypeDef;

typedef struct {
    DISPLAY_BusModeTypeDef BusMode;
    GPIO_TypeDef *RS_Port;
    uint16_t RS_Pin;
    GPIO_TypeDef *RW_Port;
    uint16_t RW_Pin;
    GPIO_TypeDef *E_Port;
    uint16_t E_Pin;
    GPIO_TypeDef *DB_Port[8];
    uint16_t DB_Pin[8];
} DISPLAY_InitTypeDef;

typedef struct {
    MICROWAIT_HandleTypeDef *MICROWAITInstance;
    DISPLAY_InitTypeDef Init;
    DISPLAY_BusModeTypeDef BusMode;
    GPIO_TypeDef *RS_Port;
    uint16_t RS_Pin;
    GPIO_TypeDef *RW_Port;
    uint16_t RW_Pin;
    GPIO_TypeDef *E_Port;
    uint16_t E_Pin;
    GPIO_TypeDef *DB_Port[8];
    uint16_t DB_Pin[8];
} DISPLAY_HandleTypeDef;

HAL_StatusTypeDef DISPLAY_Init(DISPLAY_HandleTypeDef *hdisplay);
HAL_StatusTypeDef
DISPLAY_Print(DISPLAY_HandleTypeDef *hdisplay, const char *format, ...);
HAL_StatusTypeDef DISPLAY_CheckSanity(DISPLAY_HandleTypeDef *hdisplay);


#endif /* DISPLAY_H_ */
