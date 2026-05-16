#include "motor_control.h"

// 规则表
static const float fuzzy_pid_rule_table_kp[7][7] = {
    { 0.0f,  0.0f,  0.1f,  0.15f,  0.2f,  0.25f,  0.3f },
    { 0.0f,  0.1f,  0.15f, 0.2f,   0.25f, 0.3f,   0.35f},
    {-0.1f,  0.0f,  0.1f,  0.15f,  0.2f,  0.25f,  0.3f },
    {-0.15f,-0.1f,  0.0f,  0.0f,   0.1f,  0.15f,  0.2f },
    {-0.2f, -0.15f,-0.1f,  0.0f,   0.0f,  0.1f,   0.15f},
    {-0.25f,-0.2f, -0.15f,-0.1f,   0.0f,  0.0f,   0.1f },
    {-0.3f, -0.25f,-0.2f, -0.15f, -0.1f,  0.0f,   0.0f }
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
    { 0.15f, 0.12f, 0.10f, 0.08f, 0.05f, 0.0f,  -0.05f},
    { 0.12f, 0.10f, 0.08f, 0.05f, 0.0f,  -0.05f,-0.10f},
    { 0.10f, 0.08f, 0.05f, 0.0f,  -0.05f,-0.10f,-0.15f},
    { 0.08f, 0.05f, 0.0f,  -0.05f,-0.10f,-0.15f,-0.20f},
    { 0.05f, 0.0f,  -0.05f,-0.10f,-0.15f,-0.20f,-0.25f},
    { 0.0f,  -0.05f,-0.10f,-0.15f,-0.20f,-0.25f,-0.30f},
    {-0.05f,-0.10f,-0.15f,-0.20f,-0.25f,-0.30f,-0.35f}
};

// 单电机速度控制
// PIDParam motor_left_speed_pid   = {
//     .type = PID_INC,
//     .kp = 8.0f, .ki = 0.56f, .kd = 0.0f,
//     .integral_limit = 3000.0f,   .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f
// };
// PIDParam motor_right_speed_pid  = {
//     .type = PID_INC,
//     .kp = 8.0f, .ki = 0.56f, .kd = 0.0f,
//     .integral_limit = 3000.0f,   .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f
// };

PIDParam motor_left_speed_pid   = {
    .type = PID_INC,
    .kp = 4.0f, .ki = 0.50f, .kd = 0.1f,
    .integral_limit = 3000.0f,   .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f
};

PIDParam motor_right_speed_pid  = {
    .type = PID_INC,
    .kp = 4.0f, .ki = 0.50f, .kd = 0.1f,
    .integral_limit = 3000.0f,   .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f
};

// 图像误差要求的转向pid   //6.2   1.0   1.0   0.0
// 6.7 1.5 1.1 0.05 // 科目1
// 6.75 2.0 1.1 0.1 // 科目2
// 6.4 1.7 1.1 0.08 // 科目3
//7.2 1.9 1.1 0.1
//6.9 2.8 1.1  0.12
PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 7.1f, .ki = 0.0f, .kd = 1.4f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 94.0f, .error_min = -93.0f, .error_delta_max = 100.0f, .error_delta_min = -100.0f,
};

// 实际转向pid
PIDParam motor_steering_pid = {
    .type = PID_POS,
    .kp = 1.25f, .ki = 0.0f, .kd = 0.15f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f
};

uint8 motion_control_run_flag = 0;      // 作用于单电机速度环，让速度=0
uint8 motor_traveling_pid_run_flag  = 0; // 作用于行进电机速度环，不让速度环运行

int16 motion_image_steering_speed = 0;  // 图像要求的转向速度
int16 motor_steering_speed = 0;         // 实际转向速度

// 电机
int16 motor_left_current_pwm_duty = 0;
int16 motor_right_current_pwm_duty = 0;

uint16 motor_fun_open_percent = 0;

int32 motor_left_speed = 0;
int32 motor_right_speed = 0;

int32 motor_forward_speed = 0;

uint32 motor_pit_count = 0; // PIT中断计数

uint8 motor_soft_start_flag = 0; // 电机软启动标志位 0 表示需要软启动,1表示软启动完成

// 算法部分
// 模糊PID参数计算
void fuzzy_pid_update(PIDParam* pid_param){
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
    if(fabs(pid_param->kp) >= 0.01f){    
        pid_param->fuzzy_kp = pid_param->kp + delta_kp;
    }
    if(fabs(pid_param->ki) >= 0.01f){    
        pid_param->fuzzy_ki = pid_param->ki + delta_ki;
    }
    if(fabs(pid_param->kd) >= 0.01f){    
        pid_param->fuzzy_kd = pid_param->kd + delta_kd;
    }
}

void pid_param_check(PIDParam* pid_param){
    // 确保模糊参数合理
    if( (pid_param->type == FUZZY_PID_POS || pid_param->type == FUZZY_PID_INC) && 
        (pid_param->error_max <= 0 || pid_param->error_min >= 0 || pid_param->error_delta_max <= 0 || pid_param->error_delta_min >= 0)){
        zf_assert(0); // 模糊PID参数正负错误
    }
}


/* PID计算器
 *  pid_param: PID参数结构体指针
 *  target: 目标值
 *  current: 当前值
 *  返回值: 位置式PID返回输出，增量式PID返回增量 
 * */
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
        fuzzy_pid_update(pid_param);
        
        // 积分计算和限幅
        pid_param->integral +=  pid_param->fuzzy_kp * pid_param->error;
        pid_param->integral =   (pid_param->integral > pid_param->integral_limit) ? 
                                (pid_param->integral_limit) : 
                                ((pid_param->integral < -pid_param->integral_limit) ? -pid_param->integral_limit : pid_param->integral);
        output = pid_param->fuzzy_kp * pid_param->error
               + pid_param->fuzzy_ki * pid_param->integral
               + pid_param->fuzzy_kd * pid_param->error_delta;
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


int16 motor_traveling_left_target_speed = 0;
int16 motor_traveling_right_target_speed = 0;

void motion_control_pit_callback(){

    motor_pit_count++;

    // 电机接口PIT，获取速度
    motor_interface_pit_callback();
    motor_get_speed(&motor_left_speed, &motor_right_speed);

    // 如果车不运动
    if(!motion_control_run_flag){
        motor_soft_start_flag = 0; // 需要软启动
        // motor_fun_open_percent = 0; // 负压风扇关闭
    }

    motor_fun_set_open_percent(motor_fun_open_percent); // 开度

    // 前进速度决策
    forward_speed_decision();

    // 运行保护
    run_control_protect();

    // 图像要求的转向环
    if(motor_pit_count % 2 == 0){ // 转向环10ms运行一次
        motion_image_steering_pid.previous_error = (float)error_image_last;
        motion_image_steering_speed = (int16)PID_calculate(&motion_image_steering_pid, 0.0f, (float)error_image); // 这里的目标值和当前值需要根据具体应用修改
        // motion_image_steering_speed = (int16)PID_calculate(&motion_image_steering_pid, 0.0f, (float)0.0); // 这里的目标值和当前值需要根据具体应用修改
    }
    // 转向闭环
    motor_steering_speed = (int16)PID_calculate(&motor_steering_pid, (float)motion_image_steering_speed, gyro_current_data.gyro_z); // 这里的当前值需要根据具体应用修改
    
    if(motor_traveling_pid_run_flag){   
        // 单电机速度环
        // if(motor_left_speed_pid.type == PID_INC){
        //     motor_left_current_pwm_duty  += (int16)PID_calculate(&motor_left_speed_pid,  (float)(motion_control_run_flag ? motor_forward_speed : 0), (float)motor_left_speed);
        //     motor_right_current_pwm_duty += (int16)PID_calculate(&motor_right_speed_pid, (float)(motion_control_run_flag ? motor_forward_speed : 0), (float)motor_right_speed);
        // }else if(motor_left_speed_pid.type == PID_POS){
        //     motor_left_current_pwm_duty  = (int16)PID_calculate(&motor_left_speed_pid,  (float)(motion_control_run_flag ? motor_forward_speed : 0), (float)motor_left_speed);
        //     motor_right_current_pwm_duty = (int16)PID_calculate(&motor_right_speed_pid, (float)(motion_control_run_flag ? motor_forward_speed : 0), (float)motor_right_speed);
        // }
        
        motor_traveling_left_target_speed  =    motion_control_run_flag ? motor_steering_speed +   motor_forward_speed : 0;
        motor_traveling_right_target_speed =    motion_control_run_flag ? -motor_steering_speed  + motor_forward_speed : 0;
        if(motor_left_speed_pid.type == PID_INC){
            motor_left_current_pwm_duty  += (int16)PID_calculate(&motor_left_speed_pid,  (float)(motor_traveling_left_target_speed), (float)motor_left_speed);
            motor_right_current_pwm_duty += (int16)PID_calculate(&motor_right_speed_pid, (float)(motor_traveling_right_target_speed), (float)motor_right_speed);
        }else if(motor_left_speed_pid.type == PID_POS){
            motor_left_current_pwm_duty  = (int16)PID_calculate(&motor_left_speed_pid,  (float)(motor_traveling_left_target_speed), (float)motor_left_speed);
            motor_right_current_pwm_duty = (int16)PID_calculate(&motor_right_speed_pid, (float)(motor_traveling_right_target_speed), (float)motor_right_speed);
        }
        motor_traveling_soft_start(); // 行进电机软启动
        
        // 应用PWM
        motor_traveling_set_pwm(&motor_left_current_pwm_duty, &motor_right_current_pwm_duty);
    }
}

void motion_control_pit_init(void){
    pit_ms_init(MOTION_CONTROL_PIT_INDEX, MOTION_CONTROL_PIT_TIME);
}

void motion_control_init(void){
    // 初始化电机接口
    motor_interface_init(MOTION_CONTROL_PIT_TIME);
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

// 主循环中调用的函数，不要在中断中调用
void motor_fun_soft_start(void){
    motor_fun_open_percent = 25;
    system_delay_ms(1500);
    motor_fun_open_percent = MOTOR_FUN_LINEAR_OPEN_PERCENT;
    system_delay_ms(2000);
}

void    motor_traveling_soft_start(void){
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
        if(motor_left_speed > (motor_forward_speed / 2) && motor_right_speed > (motor_forward_speed / 2)){
            motor_soft_start_flag = 1;
        }
    }
}

uint8 forward_speed_decision_enable = 0; // 0表示不执行速度决策，1表示执行速度决策

static uint8    curve_speed_lock = 0;
static float    curve_speed_lock_angle = 0.0f; // 锁定转弯速度时的转向角
static uint32   turn_time_start = 0; // 转弯开始时间

// 速度决策
void forward_speed_decision(void){
    if(!forward_speed_decision_enable){
        motor_forward_speed = MOTOR_FORWARD_NORMAL_SPEED;
        return;
    }else{
        if(deceleration_label && !curve_speed_lock){ // 进行速度决策
            curve_speed_lock_angle = attitude.yaw; // 记录当前转向角
            turn_time_start = system_getval_ms(); // 记录转弯开始时间
            curve_speed_lock = 1;
        }
        if(!deceleration_label){
            if( feature_T_index == 1 || feature_T_index == 2 || feature_T_index == 4 ||
                feature_T_index == 5   ||  feature_T_index == 6 || feature_T_index == 10 || feature_T_index == 11 || 
                feature_T_index == 18 || feature_T_index == 19 || 
                feature_T_index == 20)
            {
            // if( feature_T_index == 0    || feature_T_index == 2 || feature_T_index == 3 || feature_T_index == 5 ||
            //     feature_T_index == 6   ||  feature_T_index >= 8)
            // {
                motor_forward_speed = MOTOR_FORWARD_LINEAR_SPEED + 150;
            }else{
                motor_forward_speed = MOTOR_FORWARD_LINEAR_SPEED; // 直线行驶时正常速度
            }
            curve_speed_lock = 0; // 解除转弯速度锁定
        }else{
            motor_forward_speed = MOTOR_FORWARD_CURVE_SPEED; // 转弯时降低速度
            // 转弯超时停车
            if(system_getval_ms() > turn_time_start){ // 防止系统时钟溢出导致车辆误停车
                if(system_getval_ms() - turn_time_start > 2000){ // 转弯超过2秒
                    motor_traveling_power_flag = 0; // 停车
                }
            }
        }
    }

}

void run_control_protect(){
    if(img_threshold > RUN_PROTECT_IMG_TH_MAX || img_threshold < RUN_PROTECT_IMG_TH_MIN || run_once_flag){
        motor_traveling_power_flag = 0; // 停车
        motor_fun_open_percent = 0; // 负压风扇关闭
    }
}
