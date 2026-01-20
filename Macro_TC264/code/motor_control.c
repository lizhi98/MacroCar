#include "motor_control.h"

// 算法部分
/* 
 *  PID计算器
 *  pid_param: PID参数结构体指针
 *  target: 目标值
 *  current: 当前值
 *  返回值: 位置式PID返回输出，增量式PID返回增量
*/
float PID_calculate(PIDParam* pid_param, float target, float current){
    float output = 0.0f;

    float error = target - current;

    if(pid_param->type == PID_POS){
        // 位置式PID
        pid_param->integral += error;
        // 积分限幅
        pid_param->integral =   (pid_param->integral > pid_param->integral_limit) ? 
                                (pid_param->integral_limit) : ((pid_param->integral < -pid_param->integral_limit) ? -pid_param->integral_limit : pid_param->integral);
        output = pid_param->kp * error
               + pid_param->ki * pid_param->integral
               + pid_param->kd * (error - pid_param->previous_error);
    }else if(pid_param->type == PID_INC){
        // 增量式PID
        output = pid_param->kp * (error - pid_param->previous_error)
               + pid_param->ki * error
               + pid_param->kd * (error - 2 * pid_param->previous_error + pid_param->previous_previous_error);
        pid_param->previous_previous_error = pid_param->previous_error;
    }
    pid_param->previous_error = error;
    return output;
}

void PID_clear(PIDParam* pid_param){
    pid_param->previous_error = 0;
    pid_param->previous_previous_error = 0;
    pid_param->integral = 0;
}

// 单电机速度控制
PIDParam motor_left_speed_pid   = {
    .type = PID_INC,
    .kp = 0.0f, .ki = 0.0f, .kd = 0.00f,
    .integral_limit = 100.0f,   .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f
};

PIDParam motor_right_speed_pid  = {
    .type = PID_INC,
    .kp = 0.0f, .ki = 0.0f, .kd = 0.00f,
    .integral_limit = 100.0f,   .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f
};

// 转向控制
PIDParam motion_steering_pid = {
    .type = PID_POS,
    .kp = 0.0f, .ki = 0.0f, .kd = 0.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f
};

int16 motor_steering_speed = 0;

uint8 motion_control_run_flag = 0;
// 电机
int16 motor_left_current_pwm_duty = 0;
int16 motor_right_current_pwm_duty = 0;
int32 motor_left_speed = 0;
int32 motor_right_speed = 0;

void motion_control_pit_callback(){
    // 获取电机速度
    motor_get_speed(&motor_left_speed, &motor_right_speed);
    // 转向环
    motor_steering_speed = (int16)PID_calculate(&motion_steering_pid, 0.0f, 0.0f); // 这里的目标值和当前值需要根据具体应用修改
    
    // 单电机速度环
    if(motor_left_speed_pid.type == PID_INC){
        motor_left_current_pwm_duty  += (int16)PID_calculate(&motor_left_speed_pid,  (float)(motion_control_run_flag ? motor_steering_speed + 0 : 0), (float)motor_left_speed);
        motor_right_current_pwm_duty += (int16)PID_calculate(&motor_right_speed_pid, (float)(motion_control_run_flag ? -motor_steering_speed + 0 : 0), (float)motor_right_speed);
    }else if(motor_left_speed_pid.type == PID_POS){
        motor_left_current_pwm_duty  = (int16)PID_calculate(&motor_left_speed_pid,  (float)(motion_control_run_flag ? 0 : 0), (float)motor_left_speed);
        motor_right_current_pwm_duty = (int16)PID_calculate(&motor_right_speed_pid, (float)(motion_control_run_flag ? 0 : 0), (float)motor_right_speed);
    }
    // 应用PWM
    motor_set_pwm(motor_left_current_pwm_duty, motor_right_current_pwm_duty);
}

void motion_control_pit_init(void){
    pit_ms_init(MOTION_CONTROL_PIT_INDEX, MOTION_CONTROL_PIT_TIME);
}

void motion_control_init(void){
    // 初始化电机接口
    motor_interface_init(BRUSHED_MOTOR);
    // 清理PID参数
    PID_clear(&motor_left_speed_pid);
    PID_clear(&motor_right_speed_pid);
    PID_clear(&motion_steering_pid);
    // 初始化计算定时器中断
    motion_control_pit_init();
}
