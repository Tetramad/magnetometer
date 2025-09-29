/*
 * sensor.c
 *
 *      Author: Tetramad
 */

#include <stddef.h>

#include <stm32f4xx_hal.h>

#include <log.h>
#include <macros.h>
#include <sensor.h>
#include <busy_wait.h>
#include <work_queue.h>

#define SENSOR LIS2MDL
#define SENSOR_HEADER <lis2mdl.h>

#define SENSOR_INIT CONCAT(SENSOR, _Init)
#define SENSOR_MEASURE CONCAT(SENSOR, _Measure)

#include SENSOR_HEADER

LOG_LEVEL_SET(LOG_LEVEL_INF);

extern TIM_HandleTypeDef htim3;

int temperature_Celsius = 0;
int magneticFieldXAxis_mT = 0;
int magneticFieldYAxis_mT = 0;
int magneticFieldZAxis_mT = 0;
int magneticFieldMagnitude_mT = 0;

static void Sensor_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

int Sensor_Init(void *payload) {
	ARG_UNUSED(payload);
	HAL_StatusTypeDef status = HAL_OK;

	status = HAL_TIM_RegisterCallback(&htim3, HAL_TIM_PERIOD_ELAPSED_CB_ID,
			Sensor_TIM_PeriodElapsedCallback);
	if (status != HAL_OK) {
		return -__LINE__;
	}

	status = HAL_TIM_Base_Start_IT(&htim3);
	if (status != HAL_OK) {
		return -__LINE__;
	}

	LOG_INF("sensor: initialize succeeded");

	return 0;
}

int Sensor_Measure(void *payload) {
	ARG_UNUSED(payload);
	int err = 0;

	err = SENSOR_MEASURE();
	if (err < 0) {
		LOG_ERR("sensor: failed to measure sensor readings");
		LOG_ERR("sensor: err=: %d", err);
		return -1;
	}

	return 0;
}

static void Sensor_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	ARG_UNUSED(htim);
	int err = 0;

	err = WorkQueue_Enqueue(WORK_PRIORITY_DEFAULT, Sensor_Measure, NULL);
	if (err < 0) {
		LOG_ERR("sensor: failed to enqueue work \"Sensor_Measure\"");
	}
}
