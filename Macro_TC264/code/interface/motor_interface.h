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
#define MOTOR_LEFT_PWM_PIN              ATOM2_CH6_P11_11
#define MOTOR_RIGHT_PWM_PIN             ATOM2_CH4_P11_9

#define MOTOR_FUN_PWM_PIN               ATOM1_CH0_P21_2

#define MOTOR_PWM_FREQUENCY             17000  // 17kHz
#define MOTOR_PWM_MAX_DUTY              5000   // 最大占空比
#define MOTOR_PWM_INIT_DUTY             0  // 最小占空比

// 编码器引脚
#define MOTOR_LEFT_ENCODER_PIN_1        TIM4_ENCODER_CH1_P02_8
#define MOTOR_LEFT_ENCODER_PIN_2        TIM4_ENCODER_CH2_P00_9
#define MOTOR_RIGHT_ENCODER_PIN_1       TIM6_ENCODER_CH1_P20_3
#define MOTOR_RIGHT_ENCODER_PIN_2       TIM6_ENCODER_CH2_P20_0

#define MOTOR_LEFT_DIR_PIN              P11_12
#define MOTOR_RIGHT_DIR_PIN             P11_10

#define MOTOR_LEFT_ENCODER_INDEX        TIM4_ENCODER
#define MOTOR_RIGHT_ENCODER_INDEX       TIM6_ENCODER

typedef enum {
    BRUSHLESS_MOTOR, // 无刷电机
    BRUSHED_MOTOR    // 有刷电机
} MotorType;

extern uint8 motor_interface_power_flag; // 作用于PWM输出，让PWM=0

void motor_interface_pit_callback(void);

void motor_get_speed(int32 * left_speed, int32 * right_speed);
void motor_set_pwm(int16 * left_pwm, int16 * right_pwm);
void motor_fun_set_pwm(uint16 * target_pwm);
void motor_interface_init(MotorType type, uint16 pit_time);

#endif