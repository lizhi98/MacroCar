#include "menu_pro.h"

#ifdef SMARTCAR_DEBUG_IPS_PRO

KeyCmd current_key_cmd = KEY_NO_CMD; // 当前按键命令

void menu_key_init(void){
    key_init(KEY_PIT_TIME);   // 按键初始化 5ms扫描一次 // 改这个需要改所处中断时间
    pit_ms_init(MENU_KEY_PIT_INDEX, KEY_PIT_TIME); // 按键扫描PIT初始化
}

void menu_pro_init(void){
    ips200pro_init("测试", IPS200PRO_TITLE_BOTTOM, 30);
}


void no_screen_key_event_handle(){
    if((key_get_state(KEY_1) == KEY_SHORT_PRESS) || (key_get_state(KEY_1) == KEY_LONG_PRESS)){
        menu_run_1(); // 直接运行科目1
        key_clear_state(KEY_1);
    }
    if((key_get_state(KEY_2) == KEY_SHORT_PRESS) || (key_get_state(KEY_2) == KEY_LONG_PRESS)){
        menu_run_2(); // 直接运行科目2
        key_clear_state(KEY_2);
    }
    if((key_get_state(KEY_3) == KEY_SHORT_PRESS) || (key_get_state(KEY_3) == KEY_LONG_PRESS)){
        menu_run_3(); // 直接运行科目3
        key_clear_state(KEY_3);
    }
    if((key_get_state(KEY_4) == KEY_SHORT_PRESS) || (key_get_state(KEY_4) == KEY_LONG_PRESS)){
        // 保留这个按键作为紧急停止按键，按下就停止电机运动
        motor_interface_power_flag = 0; // 关闭所有电机PWM输出
        key_clear_state(KEY_4);
    }
}

void menu_run_1(void){
    // ips200_show_string(0, 130, "RUN_1 function");
}

void menu_run_2(void){
    motor_interface_power_flag = 1; // 使能电机PWM输出
    motor_fun_soft_start(); // 负压风扇软启动
    system_delay_ms(500);
    motion_control_pit_run_flag = 1;
}

void menu_run_3(void){
    // ips200_show_string(0, 130, "RUN_3 function");
}

extern uint32 image_to_image_time;
extern uint32 image_process_time;

void network_print_info(void){
    static char info_buffer[120];
    sprintf(info_buffer, "%d,%d,%d,%d,%f,%f,%u,%u,%d,%ld,%u,%u,%d,%d\0",
        motor_left_speed,motor_right_speed,
        motor_left_pwm, motor_right_pwm,
        attitude.yaw, gyro_current_data.gyro_z,
        left_line_list[45],right_line_list[45],error_image,image_to_image_time,condition_T,T_index,
        imu660rc_gyro_z,battery_protection_adc_value
    );
    network_vofa_send_str(info_buffer);
}
#endif
