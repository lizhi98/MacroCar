#include "motor_interface.h"

static int32 motor_left_speed = 0;
static int32 motor_right_speed = 0;

static int32 motor_left_pwm = 0;
static int32 motor_right_pwm = 0;

static uint16 motor_fun_pwm = 0;

static MotorType motor_type;

uint8 motor_interface_power_flag = 0;

uint16 motor_get_speed_pit_time = 1;

void motor_interface_init(MotorType type, uint16 pit_time){
    switch (type) {
        case BRUSHLESS_MOTOR:
            // 初始化无刷电机相关设置
            break;
        case BRUSHED_MOTOR:
            // 初始化有刷电机相关设置
            // PWM输出初始化
            gpio_init(MOTOR_LEFT_DIR_PIN,  GPO, 0, GPO_PUSH_PULL);
            gpio_init(MOTOR_RIGHT_DIR_PIN, GPO, 0, GPO_PUSH_PULL);
            pwm_init(MOTOR_LEFT_PWM_PIN,  MOTOR_PWM_FREQUENCY, MOTOR_PWM_INIT_DUTY);
            pwm_init(MOTOR_RIGHT_PWM_PIN, MOTOR_PWM_FREQUENCY, MOTOR_PWM_INIT_DUTY);
            // 负压风扇
            pwm_init(MOTOR_FUN_PWM_PIN, MOTOR_PWM_FREQUENCY, MOTOR_PWM_INIT_DUTY);
            // 编码器初始化
            encoder_dir_init(MOTOR_LEFT_ENCODER_INDEX, MOTOR_LEFT_ENCODER_PIN_1,    MOTOR_LEFT_ENCODER_PIN_2);
            encoder_dir_init(MOTOR_RIGHT_ENCODER_INDEX, MOTOR_RIGHT_ENCODER_PIN_1,   MOTOR_RIGHT_ENCODER_PIN_2);
            break;
        default:
            zf_assert(0); // 不支持的电机类型
            break;
    }
    motor_type = type;
    motor_get_speed_pit_time = pit_time;
}

void motor_get_speed(int32 * left_speed, int32 * right_speed){
    *left_speed  = motor_left_speed;
    *right_speed = motor_right_speed;
}
void motor_set_pwm(int16 * left_pwm, int16 *right_pwm){

    // PWM限幅
    if(abs(*left_pwm) > MOTOR_PWM_MAX_DUTY){
        *left_pwm = (*left_pwm >=0) ? MOTOR_PWM_MAX_DUTY : -MOTOR_PWM_MAX_DUTY;
    }
    if(abs(*right_pwm) > MOTOR_PWM_MAX_DUTY){
        *right_pwm = (*right_pwm >=0) ? MOTOR_PWM_MAX_DUTY : -MOTOR_PWM_MAX_DUTY;
    }
    motor_left_pwm  = *left_pwm;
    motor_right_pwm = *right_pwm;

    if(!motor_interface_power_flag){
        motor_left_pwm  = 0;
        motor_right_pwm = 0;
    }
    
    switch (motor_type)
    {
        case BRUSHLESS_MOTOR:
            // 无刷电机PWM设置代码
            break;
        case BRUSHED_MOTOR:
            gpio_set_level(MOTOR_LEFT_DIR_PIN,  (motor_left_pwm  >= 0) ? 1 : 0);
            gpio_set_level(MOTOR_RIGHT_DIR_PIN, (motor_right_pwm >= 0) ? 1 : 0);
            pwm_set_duty(MOTOR_LEFT_PWM_PIN,  abs(motor_left_pwm));
            pwm_set_duty(MOTOR_RIGHT_PWM_PIN, abs(motor_right_pwm));
            break;
        default:
            zf_assert(0); // 不支持的电机类型
            break;
    }
}

/*
    功能：设置负压风扇PWM PWM只有正值
*/
void motor_fun_set_pwm(uint16 * target_pwm){
    if(*target_pwm > MOTOR_PWM_MAX_DUTY){
        *target_pwm = MOTOR_PWM_MAX_DUTY;
    }
    motor_fun_pwm = *target_pwm;
    if(!motor_interface_power_flag){
        motor_fun_pwm = 0;
    }
    pwm_set_duty(MOTOR_FUN_PWM_PIN, motor_fun_pwm);
}

// 主要是用于获取速度
void motor_interface_pit_callback(void){
    // 获取速度
    motor_left_speed  = encoder_get_count(MOTOR_LEFT_ENCODER_INDEX)  * -10 / motor_get_speed_pit_time;
    motor_right_speed = encoder_get_count(MOTOR_RIGHT_ENCODER_INDEX) * 10  / motor_get_speed_pit_time;  // 转换速度不受PIT时间影响
    // 清零计数
    encoder_clear_count(MOTOR_LEFT_ENCODER_INDEX);
    encoder_clear_count(MOTOR_RIGHT_ENCODER_INDEX);
}

