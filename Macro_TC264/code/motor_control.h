#ifndef __MOTOR_CONTROL_H__
#define __MOTOR_CONTROL_H__

#include "motor_interface.h"
#include "image_process.h"
#include "gyroscope_interface.h"
#include "zf_common_clock.h"
#include "zf_driver_delay.h"
#include <math.h>
#include "zf_driver_timer.h"

// 算法部分
// PID算法
typedef enum _PIDType{
    PID_INC, // 增量式PID
    PID_POS, // 位置式PID
    FUZZY_PID_POS, // 模糊位置式PID
    FUZZY_PID_INC, // 模糊增量式PID
    SEGMENT_PID_POS, // 分段位置式PID
} PIDType;

typedef struct _PIDParam{
    PIDType type;

    float kp;
    float ki;
    float kd;

    // error
    float error;
    float error_delta; // 误差变化率

    // 限幅
    float integral_limit;

    // 模糊PID用到的
    float error_max;
    float error_min;
    float error_delta_max;
    float error_delta_min;

    float fuzzy_kp;
    float fuzzy_ki;
    float fuzzy_kd;
    
    // 分段PID用到的
    float segment_kp;
    float segment_ki;
    float segment_kd;

    // 记录
    float previous_error;
    float previous_previous_error;
    float integral;
    float output;

    // 限幅
    float output_max;
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
#define MOTION_CONTROL_PIT_TIME         5  // 单位ms
#define MOTION_CONTROL_PIT_INDEX        CCU61_CH0

#define MOTOR_SOFT_START_PWM            3500// 电机软启动PWM占空比

// 不进行速度决策
#define MOTOR_FORWARD_NORMAL_SPEED      1500  // 前进正常速度

// 进行速度决策
#define MOTOR_FORWARD_LINEAR_SPEED      1600    // 前进直线速度 1500    1450
#define MOTOR_FORWARD_CURVE_SPEED       1500    // 前进转角速度 1200    1250

// 负压风扇直线行驶开度
#define MOTOR_FUN_LINEAR_OPEN_PERCENT   45

// #define CURVE_SPEED_EXIT_ANGLE_TH       70.0f // 转弯速度锁定解除转角阈值

#define RUN_PROTECT_IMG_TH_MAX      220 // 运行保护图像阈值最大值，超过这个值认为图像异常，进行保护措施
#define RUN_PROTECT_IMG_TH_MIN      145  // 运行保护图像阈值最小值，低于这个值认为图像异常，进行保护措施

extern int32 motor_forward_linear_speed;
extern int32 motor_forward_curve_speed;

extern uint8 motion_control_run_flag; // 作用于单电机速度环，让速度=0
extern uint8 motor_traveling_pid_run_flag; // 作用于行进电机速度环，不让速度环运行

extern int16 motor_left_current_pwm_duty;
extern int16 motor_right_current_pwm_duty;
extern uint16 motor_fun_pwm_duty;

extern int32 motor_left_speed;
extern int32 motor_right_speed;

extern int32 motor_forward_speed;

extern int16 motor_traveling_left_target_speed;
extern int16 motor_traveling_right_target_speed;

extern uint8 forward_speed_decision_enable; // 0表示不执行速度决策，1表示执行速度决策

void fuzzy_pid_update(PIDParam* pid_param);
void pid_param_check(PIDParam* pid_param);
void motion_control_pit_callback(void);
void motion_control_pit_init(void);
void motion_control_init(void);

void motor_fun_soft_start(void); // 负压风扇软启动
void motor_traveling_soft_start(void); // 行进电机软启动

void forward_speed_decision(void); // 前进速度决策，基于图像处理结果

void run_control_protect(void); // 运行保护，基于图像处理结果

#endif
