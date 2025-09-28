/*
 * sensor.h
 *
 *      Author: Tetramad
 */

#ifndef __SENSOR_H
#define __SENSOR_H

#include <work_queue.h>

int Sensor_Init(void *payload);
int Sensor_Measure(void *payload);

extern int temperature_Celsius;
extern int magneticFieldXAxis_mT;
extern int magneticFieldYAxis_mT;
extern int magneticFieldZAxis_mT;
extern int magneticFieldMagnitude_mT;

#endif /* __SENSOR_H */
