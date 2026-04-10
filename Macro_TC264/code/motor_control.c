#include "motor_control.h"

// 规则表
static const float fuzzy_pid_rule_table_kp[7][7] = {
    {0.0,	0.0,	0.1,	0.15,	0.2,	0.25,	0.3f},
    {0.0,	0.1,	0.15,	0.2,	0.25,	0.3,	0.35f},
    {-0.1,	0.0,	0.1,	0.15,	0.2,	0.25,	0.3f},
    {-0.15,	-0.1,	0.0,	0.0,	0.1,	0.15,	0.2f},
    {-0.2,	-0.15,	-0.1,	0.0,	0.0,	0.1,	0.15f},
    {-0.25,	-0.2,	-0.15,	-0.1,	0.0,	0.0,	0.1f},
    {-0.3,	-0.25,	-0.2,	-0.15,	-0.1,	0.0,	0.0f}
};

static const float fuzzy_pid_rule_table_ki[7][7] = {
    {0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0},
    {0.0,	0.1,	0.2,	0.3,	0.4,	0.5,	0.6},
    {0.0,	0.2,	0.4,	0.6,	0.8,	1.0,	1.2},
    {0.0,	0.3,	0.6,	0.9,	1.2,	1.5,	1.8},
    {0.0,	0.4,	0.8,	1.2,	1.6,	2.0,	2.4},
    {0.0,	0.5,	1.0,	1.5,	2.0,	2.5,	3.0},
    {0.0,	0.6,	1.2,	1.8,	2.4,	3.0,	3.6}
};

static const float fuzzy_pid_rule_table_kd[7][7] = {
    {0.4,	0.35,	0.3,	0.25,	0.2,	0.15,	0.1f},	
    {0.35,	0.3,	0.25,	0.2,	0.15,	0.1,	0.0f},	
    {0.3,	0.25,	0.2,	0.15,	0.1,	0.0,	-0.1f},	
    {0.25,	0.2,	0.15,	0.0,	-0.1,	-0.15,	-0.2f},	
    {0.2,	0.15,	0.1,	-0.1,	-0.15,	-0.2,	-0.25f},	
    {0.15,	0.1,	0.0,	-0.15,	-0.2,	-0.25,	-0.3f},	
    {0.1,	0.0,	-0.1,	-0.2,	-0.25,	-0.3,	-0.35f}
};

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

// 图像误差要求的转向pid   //6.2   1.0   1.0   0.0
// 6.7 1.5 1.1 0.05 // 科目1
// 6.75 2.0 1.1 0.1 // 科目2
// 6.4 1.7 1.1 0.08 // 科目3
//7.2 1.9 1.1 0.1
PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 6.7f, .ki = 0.0f, .kd = 1.8f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f
};

// 实际转向pid
PIDParam motor_steering_pid = {
    .type = PID_POS,
    .kp = 1.05f, .ki = 0.0f, .kd = 0.12f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f
};

uint8 motion_control_run_flag = 0;      // 作用于单电机速度环，让速度=0
uint8 motion_control_pit_run_flag = 0;  // 作用于PIT回调，让行进电机PIT回调函数不执行

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

uint8 motor_soft_start_flag = 0; // 电机软启动标志位 0 表示需要软启动,1表示软启动完成

// 算法部分
// 模糊PID参数计算
FuzzyKPID fuzzy_pid_update(PIDParam* pid_param){
    // 计算隶属度
    // 先确保误差和误差变化率在设定的范围内
    float error_i = (pid_param->error > pid_param->error_max) ? (pid_param->error_max) : 
                    ((pid_param->error < pid_param->error_min) ? (pid_param->error_min) : (pid_param->error));
    float error_delta_i =   (pid_param->error_delta > pid_param->error_delta_max) ? (pid_param->error_delta_max) : 
                            ((pid_param->error_delta < pid_param->error_delta_min) ? (pid_param->error_delta_min) : (pid_param->error_delta));
    // 计算相对误差等级
    // -3 负大  -2 负中   -1 负小    0 零    1 正小    2 正中    3 正大
    float error_level = (error_i > 0) ? ((error_i * 3 / pid_param->error_max) ) : ((error_i * 3 / (-pid_param->error_min))); // -3 ~ 3
    float error_delta_level = (error_delta_i > 0) ? ((error_delta_i * 3 / pid_param->error_delta_max) ) : ((error_delta_i * 3 / (-pid_param->error_delta_min))); // -3 ~ 3

    float error_level_down  = floorf(error_level);  // 计算误差等级的下界
    float error_level_up    = error_level_down + 1; // 计算误差等级的上界
    float error_delta_level_down  = floorf(error_delta_level);  // 计算误差变化率等级的下界
    float error_delta_level_up    = error_delta_level_down + 1; // 计算误差变化率等级的上界
    // 计算隶属度
    float error_level_up_mu   = 1.0 - (error_level_up - error_level);
    float error_level_down_mu = 1.0 - (error_level - error_level_down);
    float error_delta_level_up_mu   = 1.0 - (error_delta_level_up - error_delta_level);
    float error_delta_level_down_mu = 1.0 - (error_delta_level - error_delta_level_down);
    // 计算激活规则权重
    float w1 = fminf(error_level_down_mu, error_delta_level_down_mu);
    float w2 = fminf(error_level_down_mu, error_delta_level_up_mu);
    float w3 = fminf(error_level_up_mu,   error_delta_level_down_mu);
    float w4 = fminf(error_level_up_mu,   error_delta_level_up_mu);
    // 查表得激活规则对应增量
    float dkp1 = fuzzy_pid_rule_table_kp[(int)error_level_down + 3][(int)error_delta_level_down + 3];
    float dkp2 = fuzzy_pid_rule_table_kp[(int)error_level_down + 3][(int)error_delta_level_up   + 3];
    float dkp3 = fuzzy_pid_rule_table_kp[(int)error_level_up   + 3][(int)error_delta_level_down + 3];
    float dkp4 = fuzzy_pid_rule_table_kp[(int)error_level_up   + 3][(int)error_delta_level_up   + 3];

    float dki1 = fuzzy_pid_rule_table_ki[(int)error_level_down + 3][(int)error_delta_level_down + 3];
    float dki2 = fuzzy_pid_rule_table_ki[(int)error_level_down + 3][(int)error_delta_level_up   + 3];
    float dki3 = fuzzy_pid_rule_table_ki[(int)error_level_up   + 3][(int)error_delta_level_down + 3];
    float dki4 = fuzzy_pid_rule_table_ki[(int)error_level_up   + 3][(int)error_delta_level_up   + 3];

    float dkd1 = fuzzy_pid_rule_table_kd[(int)error_level_down + 3][(int)error_delta_level_down + 3];
    float dkd2 = fuzzy_pid_rule_table_kd[(int)error_level_down + 3][(int)error_delta_level_up   + 3];
    float dkd3 = fuzzy_pid_rule_table_kd[(int)error_level_up   + 3][(int)error_delta_level_down + 3];
    float dkd4 = fuzzy_pid_rule_table_kd[(int)error_level_up   + 3][(int)error_delta_level_up   + 3];
    // 计算权重和（防止除 0）
    float sum_w = w1 + w2 + w3 + w4;
    if(sum_w < 0.001f) sum_w = 1.0f;

    // 解模糊（重心法）
    float delta_kp = (w1 * dkp1 + w2 * dkp2 + w3 * dkp3 + w4 * dkp4) / sum_w;
    float delta_ki = (w1 * dki1 + w2 * dki2 + w3 * dki3 + w4 * dki4) / sum_w;
    float delta_kd = (w1 * dkd1 + w2 * dkd2 + w3 * dkd3 + w4 * dkd4) / sum_w;
    // 更新PID参数
    FuzzyKPID fuzzy_kpid;
    fuzzy_kpid.kp = pid_param->kp + delta_kp;
    fuzzy_kpid.ki = pid_param->ki + delta_ki;
    fuzzy_kpid.kd = pid_param->kd + delta_kd;
    
    return fuzzy_kpid;
}

void pid_param_check(PIDParam* pid_param){
    // 确保模糊参数合理
    if((pid_param->type == FUZZY_PID_POS || pid_param->type == FUZZY_PID_INC) && 
       (pid_param->error_max <= 0 || pid_param->error_min >= 0 || pid_param->error_delta_max <= 0 || pid_param->error_delta_min >= 0)){
        zf_assert(0); // 模糊PID参数正负错误
    }
}


/* 
 *  PID计算器
 *  pid_param: PID参数结构体指针
 *  target: 目标值
 *  current: 当前值
 *  返回值: 位置式PID返回输出，增量式PID返回增量
*/
float PID_calculate(PIDParam* pid_param, float target, float current){
    float output = 0.0f;
    // 算出误差和误差变化率
    pid_param->error = target - current;
    pid_param->error_delta = pid_param->error - pid_param->previous_error;

    if(pid_param->type == PID_POS){
        // 位置式PID
        pid_param->integral += pid_param->error;
        // 积分限幅
        pid_param->integral =   (pid_param->integral > pid_param->integral_limit) ? 
                                (pid_param->integral_limit) : 
                                ((pid_param->integral < -pid_param->integral_limit) ? -pid_param->integral_limit : pid_param->integral);
        output = pid_param->kp * pid_param->error
               + pid_param->ki * pid_param->integral
               + pid_param->kd * pid_param->error_delta;
    }else if(pid_param->type == PID_INC){
        // 增量式PID
        output = pid_param->kp * pid_param->error_delta
               + pid_param->ki * pid_param->error
               + pid_param->kd * (pid_param->error - 2 * pid_param->previous_error + pid_param->previous_previous_error);
        pid_param->previous_previous_error = pid_param->previous_error;
    }else if(pid_param->type == FUZZY_PID_POS){
        // 模糊位置式PID
        FuzzyKPID fuzzy_kpid = fuzzy_pid_update(pid_param);
        // 积分计算和限幅
        pid_param->integral +=  fuzzy_kpid.kp * pid_param->error;
        pid_param->integral =   (pid_param->integral > pid_param->integral_limit) ? 
                                (pid_param->integral_limit) : 
                                ((pid_param->integral < -pid_param->integral_limit) ? -pid_param->integral_limit : pid_param->integral);
        output = fuzzy_kpid.kp * pid_param->error
               + fuzzy_kpid.ki * pid_param->integral
               + fuzzy_kpid.kd * pid_param->error_delta;
    }else{
        // 未写
        zf_assert(0); // PID类型错误
    }
    pid_param->previous_error = pid_param->error; // PID计算器记录上次误差，可以在计算前手动修改此值
    // 限幅，不超过占空比最大值，也不超过int16最大值
    if(output > 10000.0f){
        output = 10000.0f;
    }else if(output < -10000.0f){
        output = -10000.0f;
    }
    pid_param->output = output; // 记录输出值，方便调试
    return output;
}

void PID_clear(PIDParam* pid_param){
    pid_param->previous_error = 0;
    pid_param->previous_previous_error = 0;
    pid_param->integral = 0;
    pid_param->output = 0;
    pid_param->error = 0;
}

void motion_control_pit_callback(){

    motor_pit_count++;

    // 电机接口PIT，获取速度
    motor_interface_pit_callback();
    
    // 负压风扇PWM设置
    if(!motion_control_run_flag){
        motor_fun_pwm_duty = 0;
        motor_soft_start_flag = 0; // 需要软启动
    }
    motor_fun_set_pwm(&motor_fun_pwm_duty);

    // 获取电机速度
    motor_get_speed(&motor_left_speed, &motor_right_speed);

    if(!motion_control_pit_run_flag){
        motor_soft_start_flag = 0; // 需要软启动
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
    
    /*单电机速度环
    // if(motor_left_speed_pid.type == PID_INC){
    //     motor_left_current_pwm_duty  += (int16)PID_calculate(&motor_left_speed_pid,  (float)(motion_control_run_flag ? motor_forward_speed : 0), (float)motor_left_speed);
    //     motor_right_current_pwm_duty += (int16)PID_calculate(&motor_right_speed_pid, (float)(motion_control_run_flag ? motor_forward_speed : 0), (float)motor_right_speed);
    // }else if(motor_left_speed_pid.type == PID_POS){
    //     motor_left_current_pwm_duty  = (int16)PID_calculate(&motor_left_speed_pid,  (float)(motion_control_run_flag ? motor_forward_speed : 0), (float)motor_left_speed);
    //     motor_right_current_pwm_duty = (int16)PID_calculate(&motor_right_speed_pid, (float)(motion_control_run_flag ? motor_forward_speed : 0), (float)motor_right_speed);
    // }
    */ 

    if(motor_left_speed_pid.type == PID_INC){
        motor_left_current_pwm_duty  += (int16)PID_calculate(&motor_left_speed_pid,  (float)(motion_control_run_flag ? motor_steering_speed  + motor_forward_speed : 0), (float)motor_left_speed);
        motor_right_current_pwm_duty += (int16)PID_calculate(&motor_right_speed_pid, (float)(motion_control_run_flag ? -motor_steering_speed + motor_forward_speed : 0), (float)motor_right_speed);
    }else if(motor_left_speed_pid.type == PID_POS){
        motor_left_current_pwm_duty  = (int16)PID_calculate(&motor_left_speed_pid,  (float)(motion_control_run_flag ? motor_steering_speed  + motor_forward_speed : 0), (float)motor_left_speed);
        motor_right_current_pwm_duty = (int16)PID_calculate(&motor_right_speed_pid, (float)(motion_control_run_flag ? -motor_steering_speed + motor_forward_speed : 0), (float)motor_right_speed);
    }
    motor_traveling_soft_start(); // 行进电机软启动
    // 应用PWM
    // motor_left_current_pwm_duty     = -2000;
    // motor_right_current_pwm_duty    = -2000;
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
    // 检查PID参数
    pid_param_check(&motor_left_speed_pid);
    pid_param_check(&motor_right_speed_pid);
    pid_param_check(&motion_image_steering_pid);
    pid_param_check(&motor_steering_pid);
    // 初始化计算定时器中断
    motion_control_pit_init();
}

void motor_fun_soft_start(void){
    for(uint16 i = MOTOR_FUN_MIN_PWM_DUTY; i < MOTOR_FUN_NORMAL_PWM_DUTY; i++){
        motor_fun_pwm_duty = i;
        motor_fun_set_pwm(&motor_fun_pwm_duty);
        system_delay_ms(2); // 每2ms增加一次PWM占空比
    }
}

void motor_traveling_soft_start(void){
    // 电机软启动限幅
    if(!motor_soft_start_flag){
        // 限制PWM最大值
        if(abs(motor_left_current_pwm_duty) > MOTOR_SOFT_START_PWM){
            motor_left_current_pwm_duty = (motor_left_current_pwm_duty > 0) ? MOTOR_SOFT_START_PWM : -MOTOR_SOFT_START_PWM;
        }
        if(abs(motor_right_current_pwm_duty) > MOTOR_SOFT_START_PWM){
            motor_right_current_pwm_duty = (motor_right_current_pwm_duty > 0) ? MOTOR_SOFT_START_PWM : -MOTOR_SOFT_START_PWM;
        }
        // 当电机转速大于前进速度时，认为软启动完成
        if(motor_left_speed > motor_forward_speed && motor_right_speed > motor_forward_speed){
            motor_soft_start_flag = 1;
        }
    }
}
