#include "motor_interface.h"

static int32 motor_left_speed = 0;
static int32 motor_right_speed = 0;

int32 motor_left_pwm = 0;
int32 motor_right_pwm = 0;

uint8 motor_traveling_power_flag = 0;

uint16 motor_get_speed_pit_time = 1;

void motor_interface_init(uint16 pit_time){
    gpio_init(P20_9, GPO, 1, GPO_PUSH_PULL); // 关闭8701睡眠

    gpio_init(MOTOR_LEFT_PH_PIN,  GPO, 1, GPO_PUSH_PULL);
    gpio_init(MOTOR_RIGHT_PH_PIN, GPO, 1, GPO_PUSH_PULL);
    pwm_init(MOTOR_LEFT_EN_PIN,  MOTOR_PWM_FREQUENCY, MOTOR_PWM_INIT_DUTY);
    pwm_init(MOTOR_RIGHT_EN_PIN, MOTOR_PWM_FREQUENCY, MOTOR_PWM_INIT_DUTY);
    // 负压风扇
    pwm_init(MOTOR_FUN_PWM_PIN, MOTOR_FUN_FREQUENCY,  MOTOR_PWM_INIT_DUTY);
    motor_fun_set_open_percent(0); // 负压风扇初始关闭
    // 编码器初始化
    encoder_dir_init(MOTOR_LEFT_ENCODER_INDEX, MOTOR_LEFT_ENCODER_PIN_1,    MOTOR_LEFT_ENCODER_PIN_2);
    encoder_dir_init(MOTOR_RIGHT_ENCODER_INDEX, MOTOR_RIGHT_ENCODER_PIN_1,   MOTOR_RIGHT_ENCODER_PIN_2);
    motor_get_speed_pit_time = pit_time;
}

void motor_get_speed(int32 * left_speed, int32 * right_speed){
    *left_speed  = motor_left_speed;
    *right_speed = motor_right_speed;
}
void motor_traveling_set_pwm(int16 * left_pwm, int16 *right_pwm){
    // PWM限幅
    if(abs(*left_pwm) > MOTOR_PWM_MAX_DUTY){
        *left_pwm = (*left_pwm >=0) ? MOTOR_PWM_MAX_DUTY : -MOTOR_PWM_MAX_DUTY;
    }
    if(abs(*right_pwm) > MOTOR_PWM_MAX_DUTY){
        *right_pwm = (*right_pwm >=0) ? MOTOR_PWM_MAX_DUTY : -MOTOR_PWM_MAX_DUTY;
    }
    motor_left_pwm  = *left_pwm;
    motor_right_pwm = *right_pwm;

    if(!motor_traveling_power_flag){
        motor_left_pwm  = 0;
        motor_right_pwm = 0;
    }
    gpio_set_level(MOTOR_LEFT_PH_PIN,  (motor_left_pwm  > 0) ? 1 : 0);
    gpio_set_level(MOTOR_RIGHT_PH_PIN, (motor_right_pwm > 0) ? 1 : 0);
    pwm_set_duty(MOTOR_LEFT_EN_PIN,  abs(motor_left_pwm));
    pwm_set_duty(MOTOR_RIGHT_EN_PIN, abs(motor_right_pwm));
}

void motor_fun_set_open_percent(uint16 percent){
    // 检查percent范围
    if(percent > 100){
        percent = 100;
    }
    // 负压风扇开度百分比转化为PWM值
    // 50Hz 1ms高电平时间对应0%开度，2ms高电平时间对应100%开度，线性关系
    uint16 duty = (uint16)((1.0 + percent / 100.0) / (1000.0 / MOTOR_FUN_FREQUENCY) * PWM_DUTY_MAX);
    pwm_set_duty(MOTOR_FUN_PWM_PIN, duty);
    // pwm_set_duty(MOTOR_FUN_PWM_PIN, 0);
}

// 主要是用于获取速度
void motor_interface_pit_callback(void){
    // 获取速度
    // 清零计数
    motor_right_speed  = encoder_get_count(MOTOR_LEFT_ENCODER_INDEX);
    encoder_clear_count(MOTOR_LEFT_ENCODER_INDEX);
    motor_left_speed = encoder_get_count(MOTOR_RIGHT_ENCODER_INDEX) * -1;
    encoder_clear_count(MOTOR_RIGHT_ENCODER_INDEX);
}

