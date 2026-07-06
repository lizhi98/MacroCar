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

// 1200
#define MOTOR_FORWARD_LINEAR_SPEED      1200    // 前进直线速度 1500    1450
#define MOTOR_FORWARD_CURVE_SPEED       1100    // 前进转角速度 1200    1250
PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 12.0f, .ki = 0.0f, .kd = 0.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 94.0f, .error_min  = -93.0f, .error_delta_max = 100.0f, .error_delta_min = -100.0f,
};
PIDParam motor_steering_pid = {
    .type = PID_POS,
    .kp = 1.1f, .ki = 0.0f, .kd = 0.1f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 1000.0f,     .error_min = -1000.0f, .error_delta_max = 800.0f, .error_delta_min = -800.0f,
};




#define MOTOR_FORWARD_LINEAR_SPEED      1500    // 前进直线速度 1500    1450
#define MOTOR_FORWARD_CURVE_SPEED       1400    // 前进转角速度 1200    1250

PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 16.0f, .ki = 0.0f, .kd = 0.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 94.0f, .error_min  = -93.0f, .error_delta_max = 100.0f, .error_delta_min = -100.0f,
};



#define MOTOR_FORWARD_LINEAR_SPEED      1600    // 前进直线速度 1500    1450
#define MOTOR_FORWARD_CURVE_SPEED       1400    // 前进转角速度 1200    1250

PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 18.0f, .ki = 0.0f, .kd = 3.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 94.0f, .error_min  = -93.0f, .error_delta_max = 100.0f, .error_delta_min = -100.0f,
};

PIDParam motor_steering_pid = {
    .type = PID_POS,
    .kp = 1.38f, .ki = 0.0f, .kd = 0.1f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 1000.0f,     .error_min = -1000.0f, .error_delta_max = 800.0f, .error_delta_min = -800.0f,
};




// 进行速度决策
#define MOTOR_FORWARD_LINEAR_SPEED      1800    // 前进直线速度 1500    1450
#define MOTOR_FORWARD_CURVE_SPEED       1600    // 前进转角速度 1200    1250

PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 25.0f, .ki = 0.0f, .kd = 5.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 94.0f, .error_min  = -93.0f, .error_delta_max = 100.0f, .error_delta_min = -100.0f,
};

PIDParam motor_steering_pid = {
    .type = PID_POS,
    .kp = 1.4f, .ki = 0.0f, .kd = 0.5f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 1000.0f,     .error_min = -1000.0f, .error_delta_max = 800.0f, .error_delta_min = -800.0f,
};
