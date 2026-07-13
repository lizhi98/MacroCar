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


PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 24.0f, .ki = 0.0f, .kd = 0.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 94.0f, .error_min  = -93.0f, .error_delta_max = 100.0f, .error_delta_min = -100.0f,
};


PIDParam motor_steering_pid = {
    .type = PID_POS,
    .kp = 1.4f, .ki = 0.0f, .kd = 0.5f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 1000.0f,     .error_min = -1000.0f, .error_delta_max = 800.0f, .error_delta_min = -800.0f,
};
#define MOTOR_FORWARD_LINEAR_SPEED      1550    // 前进直线速度 1500    1450
#define MOTOR_FORWARD_CURVE_SPEED       1450    // 前进转角速度 1200    1250

// 负压风扇直线行驶开度
#define MOTOR_FUN_LINEAR_OPEN_PERCENT   48



// 进行速度决策
#define MOTOR_FORWARD_LINEAR_SPEED      1700    // 前进直线速度 1500    1450
#define MOTOR_FORWARD_CURVE_SPEED       1600    // 前进转角速度 1200    1250
// 负压风扇直线行驶开度
#define MOTOR_FUN_LINEAR_OPEN_PERCENT   50

PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 30.0f, .ki = 0.0f, .kd = 0.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 94.0f, .error_min  = -93.0f, .error_delta_max = 100.0f, .error_delta_min = -100.0f,
};

/*/
//第二版
*/

#define MOTOR_FORWARD_LINEAR_SPEED      1800    // 前进直线速度 1500    1450
#define MOTOR_FORWARD_CURVE_SPEED       1650    // 前进转角速度 1200    1250

// 负压风扇直线行驶开度
#define MOTOR_FUN_LINEAR_OPEN_PERCENT   52


PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 28.0f, .ki = 0.0f, .kd = 0.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 93.0f, .error_min  = -93.0f, .error_delta_max = 100.0f, .error_delta_min = -100.0f,
};

PIDParam motor_steering_pid = {
    .type = PID_POS,
    .kp = 1.4f, .ki = 0.0f, .kd = 0.5f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 1000.0f,     .error_min = -1000.0f, .error_delta_max = 800.0f, .error_delta_min = -800.0f,
};


// 进行速度决策
#define MOTOR_FORWARD_LINEAR_SPEED      1850    // 前进直线速度 1500    1450
#define MOTOR_FORWARD_CURVE_SPEED       1700    // 前进转角速度 1200    1250

// 负压风扇直线行驶开度
#define MOTOR_FUN_LINEAR_OPEN_PERCENT   53
// 前瞻 42 
PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 30.0f, .ki = 0.0f, .kd = 0.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 93.0f, .error_min  = -93.0f, .error_delta_max = 100.0f, .error_delta_min = -100.0f,
};


// 进行速度决策
#define MOTOR_FORWARD_LINEAR_SPEED      1800    // 前进直线速度 1500    1450
#define MOTOR_FORWARD_CURVE_SPEED       1700    // 前进转角速度 1200    1250

#define ERROR_IMAGE_LINE_2 42
#define MOTOR_FUN_LINEAR_OPEN_PERCENT   60

PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 34.0f, .ki = 0.0f, .kd = 0.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 93.0f, .error_min  = -93.0f, .error_delta_max = 100.0f, .error_delta_min = -100.0f,
};



// =========
#define ERROR_IMAGE_LINE_2 48
PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 26.0f, .ki = 0.0f, .kd = 0.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 93.0f, .error_min  = -93.0f, .error_delta_max = 100.0f,    .error_delta_min = -100.0f,
};

#define MOTOR_FORWARD_LINEAR_SPEED      1850    // 前进直线速度 1500    1450
#define MOTOR_FORWARD_CURVE_SPEED       1650    // 前进转角速度 1200    1250

// 负压风扇直线行驶开度
#define MOTOR_FUN_LINEAR_OPEN_PERCENT   55


//++++++++

PIDParam motor_steering_pid = {
    .type = PID_POS,
    .kp = 1.1f, .ki = 0.0f, .kd = 0.5f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 1000.0f,     .error_min = -1000.0f, .error_delta_max = 800.0f, .error_delta_min = -800.0f,
};

PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 24.0f, .ki = 0.0f, .kd = 0.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 93.0f, .error_min  = -93.0f, .error_delta_max = 100.0f,    .error_delta_min = -100.0f,
};
#define MOTOR_FORWARD_LINEAR_SPEED      1900    // 前进直线速度 1500    1450
#define MOTOR_FORWARD_CURVE_SPEED       1700    // 前进转角速度 1200    1250

// 负压风扇直线行驶开度
#define MOTOR_FUN_LINEAR_OPEN_PERCENT   60
#define ERROR_IMAGE_LINE_2 43



// 3ms
#define MOTOR_FORWARD_LINEAR_SPEED      400    // 前进直线速度
#define MOTOR_FORWARD_CURVE_SPEED       400    // 前进转角速度
#define MOTOR_FUN_LINEAR_OPEN_PERCENT   50

#define ERROR_IMAGE_LINE 70
#define ERROR_IMAGE_LINE_2 53

PIDParam motor_left_speed_pid   = {
    .type = PID_INC,
    .kp = 25.0f, .ki = 3.5f, .kd = 0.0f,
    .integral_limit = 3000.0f,   .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f
};

PIDParam motor_right_speed_pid  = {
    .type = PID_INC,
    .kp = 25.0f, .ki = 3.5f, .kd = 0.0f,
    .integral_limit = 3000.0f,   .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f
};


PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 18.0f, .ki = 0.0f, .kd = 0.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 93.0f, .error_min  = -93.0f, .error_delta_max = 100.0f,    .error_delta_min = -100.0f,
};

PIDParam motor_steering_pid = {
    .type = PID_POS,
    .kp = 0.12f, .ki = 0.0f, .kd = 0.05f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 1000.0f,     .error_min = -1000.0f, .error_delta_max = 800.0f, .error_delta_min = -800.0f,
};



// +++
PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 19.5f, .ki = 0.0f, .kd = 0.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 93.0f, .error_min  = -93.0f, .error_delta_max = 100.0f,    .error_delta_min = -100.0f,
};

PIDParam motor_steering_pid = {
    .type = PID_POS,
    .kp = 0.12f, .ki = 0.0f, .kd = 0.06f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 1000.0f,     .error_min = -1000.0f, .error_delta_max = 800.0f, .error_delta_min = -800.0f,
};

#define ERROR_IMAGE_LINE_2 48
#define MOTOR_FUN_LINEAR_OPEN_PERCENT   60

#define MOTOR_FORWARD_LINEAR_SPEED      420    // 前进直线速度
#define MOTOR_FORWARD_CURVE_SPEED       420    // 前进转角速度


// +++
#define MOTOR_FORWARD_LINEAR_SPEED      450    // 前进直线速度
#define MOTOR_FORWARD_CURVE_SPEED       450    // 前进转角速度

#define MOTOR_FUN_LINEAR_OPEN_PERCENT   60

#define ERROR_IMAGE_LINE 50
PIDParam motion_image_steering_pid = {
    .type = PID_POS,
    .kp = 19.5f, .ki = 0.0f, .kd = 0.0f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 93.0f, .error_min  = -93.0f, .error_delta_max = 100.0f,    .error_delta_min = -100.0f,
};

PIDParam motor_steering_pid = {
    .type = PID_POS,
    .kp = 0.12f, .ki = 0.0f, .kd = 0.06f,
    .integral_limit = 0.0f,    .integral = 0.0f,   .previous_error = 0.0f,   .previous_previous_error = 0.0f,
    .error_max = 1000.0f,     .error_min = -1000.0f, .error_delta_max = 800.0f, .error_delta_min = -800.0f,
};

