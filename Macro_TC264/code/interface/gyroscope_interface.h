#ifndef __GYROSCOPE_H__
#define __GYROSCOPE_H__

#include "zf_device_imu660ra.h"
#include "zf_driver_pit.h"

#include "quaternion.h"

#define GYRO_PIT_TIME         2         // 单位ms
#define GYRO_PIT_INDEX        CCU61_CH1

typedef struct _GyroData
{
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float angle_x;
    float angle_y;
    float angle_z;
}GyroData;

extern GyroData gyro_current_data;

void gyro_init(void);
// void gyro_get_data();
void gyro_pit_init(void);
void gyro_pit_callback(void);

#endif