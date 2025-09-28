#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <stm32f4xx_hal.h>

#include <input.h>
#include <log.h>
#include <macros.h>
#include <busy_wait.h>
#include <work_queue.h>

extern TIM_HandleTypeDef htim4;

extern signed magneticFieldMagnitude_mT;
extern signed magneticFieldXAxis_mT;
extern signed magneticFieldYAxis_mT;
extern signed magneticFieldZAxis_mT;
extern signed temperature_Celsius;

static void Display_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
static int CheckBusyFlag(void);
static void CommandInitial(uint8_t i);
static void Command(uint8_t i);
static int DisplayString(const char *str);
static int DisplayMagMag(void *payload);
static int DisplayMagX(void *payload);
static int DisplayMagY(void *payload);
static int DisplayMagZ(void *payload);
static int DisplayTemperature(void *payload);

int (*displayables[])(void*) = {
	DisplayMagMag,
	DisplayMagX,
	DisplayMagY,
	DisplayMagZ,
	DisplayTemperature
};
size_t displayablesIndex = 0U;

struct InputContext inputContext = { 0 };

LOG_LEVEL_SET(LOG_LEVEL_INF);

int Display_Init(void *payload) {
	ARG_UNUSED(payload);
	HAL_StatusTypeDef status = HAL_OK;

	const uint32_t tick = HAL_GetTick();
	if (tick < 40U) {
		BusyWait_ms(40U - tick);
	}

	CommandInitial(0x30);
	CommandInitial(0x30);
	CommandInitial(0x30);
	CommandInitial(0x20);

	Command(0x28);
	BusyWait_us(37);
	Command(0x0C);
	BusyWait_us(37);
	Command(0x01);
	BusyWait_us(1520);
	Command(0x06);
	BusyWait_us(37);

	status = HAL_TIM_RegisterCallback(&htim4, HAL_TIM_PERIOD_ELAPSED_CB_ID,
			Display_TIM_PeriodElapsedCallback);
	if (status != HAL_OK) {
		return -__LINE__;
	}

	status = HAL_TIM_Base_Start_IT(&htim4);
	if (status != HAL_OK) {
		return -__LINE__;
	}

	inputContext = Input_NewContext();

	LOG_INF("display: initialize succeeded");

	return 0;
}

static void Display_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	ARG_UNUSED(htim);
	int err = 0;

	err = Input_UpdateContext(&inputContext);
	if (err < 0) {
		LOG_ERR("display: failed to update input");
	} else {
		if (inputContext.left == 1 && inputContext.right != 1) {
			displayablesIndex =
					(displayablesIndex + (ARRAY_SIZE(displayables) - 1))
							% ARRAY_SIZE(displayables);
		}
		if (inputContext.right == 1 && inputContext.left != 1) {
			displayablesIndex = (displayablesIndex + 1)
					% ARRAY_SIZE(displayables);
		}
	}

	err = WorkQueue_Enqueue(WORK_PRIORITY_DEFAULT,
			displayables[displayablesIndex],
			NULL);
	if (err < 0) {
		LOG_ERR("display: failed to enqeue work");
	}
}

static int CheckBusyFlag(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);

	GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
	BusyWait_us(0);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	BusyWait_us(1);

	const GPIO_PinState busy_flag = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);

	BusyWait_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);

	BusyWait_us(2);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
	BusyWait_us(0);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	BusyWait_us(1);

	__NOP();

	BusyWait_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);

	BusyWait_us(2);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);

	GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	return busy_flag;
}

static void CommandInitial(uint8_t i) {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
	BusyWait_us(0);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	BusyWait_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4,
			i & (1 << 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5,
			i & (1 << 5) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6,
			i & (1 << 6) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7,
			i & (1 << 7) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	BusyWait_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);

	BusyWait_us(2);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);

	BusyWait_us(160);
}

static void Command(uint8_t i) {
	for (size_t _ = 0; _ < 8 && CheckBusyFlag(); ++_) {
		BusyWait_us(37);
	}

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
	BusyWait_us(0);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	BusyWait_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4,
			i & (1 << 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5,
			i & (1 << 5) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6,
			i & (1 << 6) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7,
			i & (1 << 7) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	BusyWait_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);

	BusyWait_us(2);

	i <<= 4;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	BusyWait_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4,
			i & (1 << 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5,
			i & (1 << 5) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6,
			i & (1 << 6) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7,
			i & (1 << 7) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	BusyWait_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);

	BusyWait_us(2);
}

static void write(char c) {
	for (size_t i = 0; i < 8 && CheckBusyFlag(); ++i) {
		BusyWait_us(37);
	}

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
	BusyWait_us(0);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	BusyWait_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4,
			c & (1 << 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5,
			c & (1 << 5) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6,
			c & (1 << 6) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7,
			c & (1 << 7) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	BusyWait_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);

	BusyWait_us(2);

	c <<= 4;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	BusyWait_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4,
			c & (1 << 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5,
			c & (1 << 5) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6,
			c & (1 << 6) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7,
			c & (1 << 7) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	BusyWait_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);

	BusyWait_us(2);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
}

static int DisplayString(const char *str) {
	Command(0x01);
	BusyWait_us(1520);
	Command(0x80);
	BusyWait_us(37);
	for (size_t i = 0; str[i]; ++i) {
		write(str[i]);
		BusyWait_us(37);
	}

	return 0;
}

static int DisplayMagMag(void *payload) {
	ARG_UNUSED(payload);

	char buffer[32] = "";

	if (magneticFieldMagnitude_mT > 999999
			|| magneticFieldMagnitude_mT < -999999) {
		sprintf(buffer, "M%cOvrflw", magneticFieldMagnitude_mT > 0 ? '+' : '-');
	} else {
		sprintf(buffer, "M% 4d.%03d", magneticFieldMagnitude_mT / 1000,
				abs(magneticFieldMagnitude_mT % 1000));
	}

	DisplayString(buffer);

	return 0;
}

static int DisplayMagX(void *payload) {
	ARG_UNUSED(payload);

	char buffer[32] = "";

	if (magneticFieldXAxis_mT > 99999 || magneticFieldXAxis_mT < -99999) {
		sprintf(buffer, "X%cOvrflw", magneticFieldXAxis_mT > 0 ? '+' : '-');
	} else {
		sprintf(buffer, "X% 3d.%03d", magneticFieldXAxis_mT / 1000,
				abs(magneticFieldXAxis_mT % 1000));
	}

	DisplayString(buffer);

	return 0;
}

static int DisplayMagY(void *payload) {
	ARG_UNUSED(payload);

	char buffer[32] = "";

	if (magneticFieldYAxis_mT > 99999 || magneticFieldYAxis_mT < -99999) {
		sprintf(buffer, "Y%cOvrflw", magneticFieldYAxis_mT > 0 ? '+' : '-');
	} else {
		sprintf(buffer, "Y% 3d.%03d", magneticFieldYAxis_mT / 1000,
				abs(magneticFieldYAxis_mT % 1000));
	}

	DisplayString(buffer);

	return 0;
}

static int DisplayMagZ(void *payload) {
	ARG_UNUSED(payload);

	char buffer[32] = "";

	if (magneticFieldZAxis_mT > 99999 || magneticFieldZAxis_mT < -99999) {
		sprintf(buffer, "Z%cOvrflw", magneticFieldZAxis_mT > 0 ? '+' : '-');
	} else {
		sprintf(buffer, "Z% 3d.%03d", magneticFieldZAxis_mT / 1000,
				abs(magneticFieldZAxis_mT % 1000));
	}

	DisplayString(buffer);

	return 0;
}

static int DisplayTemperature(void *payload) {
	ARG_UNUSED(payload);

	char buffer[32] = "";

	if (temperature_Celsius > 999999 || temperature_Celsius < -999999) {
		sprintf(buffer, "T%cOvrflw", temperature_Celsius > 0 ? '+' : '-');
	} else {
		sprintf(buffer, "T% 4d.%02d", temperature_Celsius / 1000,
				abs(temperature_Celsius % 1000) / 10);
	}

	DisplayString(buffer);

	return 0;
}
