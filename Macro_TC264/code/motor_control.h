#ifndef __MOTOR_CONTROL_H__
#define __MOTOR_CONTROL_H__

#define SPEED_CHOICE 1 // 预赛1 决赛2

#include "motor_interface.h"
#include "image_process.h"
#include "gyroscope_interface.h"
#include "zf_common_clock.h"
#include "zf_driver_delay.h"
#include "menu_interface.h"
#include <math.h>
#include "zf_driver_timer.h"
#include "inertial_navigation.h"

typedef enum _CarRunState
{
    INIT_WAIT, // 初始化等待状态，等待所有核心初始化完毕
    WAIT_START, // 等待开始状态，等待按键或发车指令开始运行
    STARTING, // 启动状态，等待几秒，让图像稳定
    RUNNING_IMG, // 运行状态，电机速度环和负压风扇开度受到算法控制
    RUNNING_INS, // 运行状态，电机速度环和负压风扇开度受到算法控制
    ERR_STOP, // 错误停止状态，电机速度环和负压风扇开度为0
    OVER_STOP, // 结束停止状态，电机速度环和负压风扇开度为0
    Test, // 测试状态
} CarRunState;

typedef struct _SpeedDecisionParam
{
    const int8    index; // 速度决策条件，索引
    const int8    condition; // 速度决策条件，索引锁
    const uint32  motor_forward_speed; // 前进基础速度
    const uint32  motor_average_distance; // 前进速度对应的平均距离计数和
    const uint32  motor_forward_speed_exit; // 前进速度退出，平均距离计数和
} SpeedDecisionParam;

#define SPEED_DECISION_PARAM_LIST_SIZE 23

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

extern CarRunState car_run_state;

// 单电机速度控制
extern PIDParam motor_left_speed_pid;
extern PIDParam motor_right_speed_pid;

// 转向控制
extern PIDParam motion_image_steering_pid;
extern int16 motion_image_steering_speed;
extern PIDParam motor_steering_pid;
extern int16 motor_steering_speed;

// 运动控制
#define MOTION_CONTROL_PIT_TIME         3  // 单位ms
#define MOTION_CONTROL_PIT_INDEX        CCU61_CH0

#define MOTOR_SOFT_START_PWM            3000// 电机软启动PWM占空比

// 不进行速度决策
#define MOTOR_FORWARD_NORMAL_SPEED      500  // 前进正常速度

// 进行速度决策

// 4.2
// #define MOTOR_FORWARD_LINEAR_SPEED      640    // 前进直线速度
// #define MOTOR_FORWARD_CURVE_SPEED       630    // 前进转角速度

// 3.6
// #define MOTOR_FORWARD_LINEAR_SPEED      630    // 前进直线速度
// #define MOTOR_FORWARD_CURVE_SPEED       620    // 前进转角速度

// 3.5
// #define MOTOR_FORWARD_LINEAR_SPEED      570    // 前进直线速度
// #define MOTOR_FORWARD_CURVE_SPEED       570    // 前进转角速度

// 3.4
// #define MOTOR_FORWARD_LINEAR_SPEED      550    // 前进直线速度
// #define MOTOR_FORWARD_CURVE_SPEED       550    // 前进转角速度

// 3.2
#define MOTOR_FORWARD_LINEAR_SPEED      450    // 前进直线速度
#define MOTOR_FORWARD_CURVE_SPEED       450    // 前进转角速度

// #define MOTOR_FORWARD_LINEAR_SPEED      635    // 前进直线速度
// #define MOTOR_FORWARD_CURVE_SPEED       615    // 前进转角速度

// 负压风扇直线行驶开度
#define MOTOR_FUN_LINEAR_OPEN_PERCENT   60

// #define CURVE_SPEED_EXIT_ANGLE_TH       70.0f // 转弯速度锁定解除转角阈值

#define RUN_PROTECT_IMG_TH_MAX      70 // 运行保护图像阈值最大值，超过这个值认为图像异常，进行保护措施
#define RUN_PROTECT_IMG_TH_MIN      25  // 运行保护图像阈值最小值，低于这个值认为图像异常，进行保护措施

extern int32 motor_forward_linear_speed;
extern int32 motor_forward_curve_speed;

extern uint8 motion_control_run_flag; // 作用于单电机速度环，让速度=0
extern uint8 motor_traveling_pid_run_flag; // 作用于行进电机速度环，不让速度环运行

extern int16 motor_left_current_pwm_duty;
extern int16 motor_right_current_pwm_duty;

extern int32 motor_left_speed;
extern int32 motor_right_speed;

extern int32 motor_forward_speed;

extern uint16 motor_fun_open_percent;

extern int16 motor_traveling_left_target_speed;
extern int16 motor_traveling_right_target_speed;

extern uint8 forward_speed_decision_enable; // 0表示不执行速度决策，1表示执行速度决策

extern volatile uint32 run_protect_trigger_time;        // 保护触发时间
extern volatile uint8 run_control_protect_trigger_flag; // 运行保护触发标志位

extern volatile uint8 run_control_protect_enable;

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
