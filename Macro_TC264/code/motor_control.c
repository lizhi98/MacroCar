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
    // 限幅，不超过占空比最大值，也不超过int16最大值
    if(output > 10000.0f){
        output = 10000.0f;
    }else if(output < -10000.0f){
        output = -10000.0f;
    }
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
    .kp = 8.0f, .ki = 0.56f, .kd = 0.0f,
    .integral_limit = 3000.0f,   .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f
};

PIDParam motor_right_speed_pid  = {
    .type = PID_INC,
    .kp = 8.0f, .ki = 0.56f, .kd = 0.0f,
    .integral_limit = 3000.0f,   .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f
};

// 图像误差要求的转向pid
PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 6.0f, .ki = 0.0f, .kd = 1.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f
};

// 实际转向pid
PIDParam motor_steering_pid = {
    .type = PID_POS,
    .kp = 1.0f, .ki = 0.0f, .kd = 0.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f
};

uint8 motion_control_run_flag = 0;      // 作用于单电机速度环，让速度=0
uint8 motion_control_pit_run_flag = 0;  // 作用于PIT回调，让电机PIT回调函数不执行

int16 motion_image_steering_speed = 0;  // 图像要求的转向速度
int16 motor_steering_speed = 0;         // 实际转向速度
// 电机
int16 motor_left_current_pwm_duty = 0;
int16 motor_right_current_pwm_duty = 0;
uint16 motor_fun_pwm_duty = 0;

int32 motor_left_speed = 0;
int32 motor_right_speed = 0;

int32 motor_forward_speed = 0;

uint32 motor_pit_count = 0; // PIT中断计数

void motion_control_pit_callback(){

    motor_pit_count++;

    // 电机接口PIT，获取速度
    motor_interface_pit_callback();
    
    // 负压风扇PWM设置
    if(!motion_control_run_flag){
        motor_fun_pwm_duty = 0;
    }
    motor_fun_set_pwm(&motor_fun_pwm_duty);

    // 获取电机速度
    motor_get_speed(&motor_left_speed, &motor_right_speed);

    if(!motion_control_pit_run_flag){
        return;
    }
    // 图像要求的转向环
    if(motor_pit_count % 4 == 0){ // 转向环20ms运行一次
        motion_image_steering_pid.previous_error = (float)error_image_last;
        motion_image_steering_speed = (int16)PID_calculate(&motion_image_steering_pid, 0.0f, (float)error_image); // 这里的目标值和当前值需要根据具体应用修改
        // motion_image_steering_speed = (int16)PID_calculate(&motion_image_steering_pid, 0.0f, (float)0.0); // 这里的目标值和当前值需要根据具体应用修改
    }
    // 转向闭环
    motor_steering_speed = (int16)PID_calculate(&motor_steering_pid, (float)motion_image_steering_speed, gyro_current_data.gyro_z); // 这里的当前值需要根据具体应用修改
    
    //单电机速度环
    // if(motor_left_speed_pid.type == PID_INC){
    //     motor_left_current_pwm_duty  += (int16)PID_calculate(&motor_left_speed_pid,  (float)(motion_control_run_flag ? motor_forward_speed : 0), (float)motor_left_speed);
    //     motor_right_current_pwm_duty += (int16)PID_calculate(&motor_right_speed_pid, (float)(motion_control_run_flag ? motor_forward_speed : 0), (float)motor_right_speed);
    // }else if(motor_left_speed_pid.type == PID_POS){
    //     motor_left_current_pwm_duty  = (int16)PID_calculate(&motor_left_speed_pid,  (float)(motion_control_run_flag ? motor_forward_speed : 0), (float)motor_left_speed);
    //     motor_right_current_pwm_duty = (int16)PID_calculate(&motor_right_speed_pid, (float)(motion_control_run_flag ? motor_forward_speed : 0), (float)motor_right_speed);
    // } 

    if(motor_left_speed_pid.type == PID_INC){
        motor_left_current_pwm_duty  += (int16)PID_calculate(&motor_left_speed_pid,  (float)(motion_control_run_flag ? motor_steering_speed  + motor_forward_speed : 0), (float)motor_left_speed);
        motor_right_current_pwm_duty += (int16)PID_calculate(&motor_right_speed_pid, (float)(motion_control_run_flag ? -motor_steering_speed + motor_forward_speed : 0), (float)motor_right_speed);
    }else if(motor_left_speed_pid.type == PID_POS){
        motor_left_current_pwm_duty  = (int16)PID_calculate(&motor_left_speed_pid,  (float)(motion_control_run_flag ? motor_steering_speed  + motor_forward_speed : 0), (float)motor_left_speed);
        motor_right_current_pwm_duty = (int16)PID_calculate(&motor_right_speed_pid, (float)(motion_control_run_flag ? -motor_steering_speed + motor_forward_speed : 0), (float)motor_right_speed);
    }
    // 应用PWM
    motor_set_pwm(&motor_left_current_pwm_duty, &motor_right_current_pwm_duty);
}

void motion_control_pit_init(void){
    pit_ms_init(MOTION_CONTROL_PIT_INDEX, MOTION_CONTROL_PIT_TIME);
}

void motion_control_init(void){
    // 初始化电机接口
    motor_interface_init(BRUSHED_MOTOR, MOTION_CONTROL_PIT_TIME);
    // 清理PID参数
    PID_clear(&motor_left_speed_pid);
    PID_clear(&motor_right_speed_pid);
    PID_clear(&motion_image_steering_pid);
    PID_clear(&motor_steering_pid);
    // 初始化计算定时器中断
    motion_control_pit_init();
}
