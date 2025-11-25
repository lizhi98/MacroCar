#ifndef __MOTOR_INTERFACE_H__
#define __MOTOR_INTERFACE_H__

#include "zf_common_typedef.h"
#include "zf_driver_encoder.h"
#include "zf_driver_pwm.h"
#include "zf_driver_pit.h"
#include "zf_driver_gpio.h"
#include "zf_common_debug.h"

// 有刷电机接口
// 有刷电机PWM输出
#define MOTOR_LEFT_PWM_PIN              0
#define MOTOR_RIGHT_PWM_PIN             0

#define MOTOR_PWM_FREQUENCY             20000  // 20kHz
#define MOTOR_PWM_MAX_DUTY              1000   // 最大占空比
#define MOTOR_PWM_INIT_DUTY             0  // 最小占空比

// 编码器引脚
#define MOTOR_LEFT_ENCODER_PIN_1        0
#define MOTOR_LEFT_ENCODER_PIN_2        0
#define MOTOR_RIGHT_ENCODER_PIN_1       0
#define MOTOR_RIGHT_ENCODER_PIN_2       0

#define MOTOR_LEFT_DIR_PIN              0
#define MOTOR_RIGHT_DIR_PIN             0

#define MOTOR_LEFT_ENCODER_INDEX        TIM2_ENCODER
#define MOTOR_RIGHT_ENCODER_INDEX       TIM3_ENCODER

// 定时中断
#define MOTOR_INTERFACE_PIT_CHANNEL     CCU60_CH1
#define MOTOR_INTERFACE_PIT_TIME        5     // 5ms

typedef enum {
    BRUSHLESS_MOTOR, // 无刷电机
    BRUSHED_MOTOR    // 有刷电机
} MotorType;

void motor_interface_pit_callback(void);
void motor_interface_pit_init(void);

void motor_get_speed(int32 * left_speed, int32 * right_speed);
void motor_set_pwm(int16 left_pwm, int16 right_pwm);
void motor_interface_init(MotorType motor_type);

#endif