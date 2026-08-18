#ifndef CODE_QUATERNION_H_
#define CODE_QUATERNION_H_

#include "zf_device_imu660rc.h"
#include <math.h>

typedef struct {
    float q0;
    float q1;
    float q2;
    float q3;
    float roll;
    float pitch;
    float yaw;
} Quaternion;

extern Quaternion attitude;

extern float angle_z;

void quaternion_init(void);
void quaternion_update(void);

// tools
float get_angle_err(float angle);

#endif
