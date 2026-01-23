#ifndef __MOTOR_CONTROL_H__
#define __MOTOR_CONTROL_H__

#include "motor_interface.h"
#include "image_process.h"
#include "gyroscope_interface.h"

// 算法部分
// PID算法
typedef enum _PIDType{
    PID_INC, // 增量式PID
    PID_POS, // 位置式PID
} PIDType;

typedef struct _PIDParam{
    PIDType type;

    float kp;
    float ki;
    float kd;

    // 限幅
    float integral_limit;

    float previous_error;
    float previous_previous_error;
    float integral;
} PIDParam;

float   PID_calculate(PIDParam* pid_param, float target, float current);
void    PID_clear(PIDParam* pid_param);

// 单电机速度控制
extern PIDParam motor_left_speed_pid;
extern PIDParam motor_right_speed_pid;

// 转向控制
extern PIDParam motion_image_steering_pid;
extern int16 motion_image_steering_speed;
extern PIDParam motor_steering_pid;
extern int16 motor_steering_speed;

// 运动控制
#define MOTION_CONTROL_PIT_TIME         10  // 单位ms
#define MOTION_CONTROL_PIT_INDEX        CCU61_CH0

extern uint8 motion_control_run_flag; // 作用于单电机速度环，让速度=0
extern int16 motor_left_current_pwm_duty;
extern int16 motor_right_current_pwm_duty;
extern int32 motor_left_speed;
extern int32 motor_right_speed;

extern int32 motor_forward_speed;

void motion_control_pit_callback(void);
void motion_control_pit_init(void);
void motion_control_init(void);

#endif