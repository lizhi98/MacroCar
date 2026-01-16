#ifndef CODE_INERTIAL_NAVIGATION_H_
#define CODE_INERTIAL_NAVIGATION_H_

#include "zf_common_typedef.h"
#include "quaternion.h"
#include "zf_driver_encoder.h"
#include "motor_interface.h"

#define PI 3.1415926f
#define radius 15
#define data_num 5000

void Inertial_Navigation_Getdate(void);
float get_target_yaw(void);

#endif /* CODE_INERTIAL_NAVIGATION_H_ */