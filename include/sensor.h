#ifndef SENSOR_H
#define SENSOR_H

#include <work_queue.h>

int sensor_init(void *payload);
int sensor_measure(void *payload);

extern int temperature_celsius;
extern int magnetic_field_x_axis_mt;
extern int magnetic_field_y_axis_mt;
extern int magnetic_field_z_axis_mt;
extern int magnetic_field_magnitude_mt;

#endif /* SENSOR_H */
