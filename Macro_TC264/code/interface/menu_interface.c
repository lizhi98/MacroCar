#include "menu_interface.h"

void menu_interface_init(){

}

void menu_ips_print_info(MenuPrintInfoType info_type){
    static char info_buffer[64];
    switch (info_type)
    {
    case MENU_INFO_NORMAL:
        sprintf(info_buffer, "L_S:%+4d   R_S:%+4d ", motor_left_speed, motor_right_speed);
        ips200_show_string(0, 130, info_buffer);
        sprintf(info_buffer, "L_P:%+5d  R_P:%+5d", motor_left_current_pwm_duty, motor_right_current_pwm_duty);
        ips200_show_string(0, 150, info_buffer);
        sprintf(info_buffer, "B_A:%+4d", battery_protection_adc_value);
        ips200_show_string(0, 170, info_buffer);
        sprintf(info_buffer, "I_E:%+5d", error_image);
        ips200_show_string(0, 190, info_buffer);
        sprintf(info_buffer, "GZ:%+6.2f YAW:%+6.2f", gyro_current_data.gyro_z, gyro_current_data.angle_z);
        ips200_show_string(0, 210, info_buffer);
        sprintf(info_buffer, "T:%u index:%u condition:%u", corner_feature.T, T_index, condition);
        ips200_show_string(0, 230, info_buffer);
        break;
    case MENU_INFO_EXTEND:
        break;
    default:
        break;
    }
    
}

void menu_key_init(void){
    key_init(KEY_PIT_TIME);   // 按键初始化 5ms扫描一次 // 改这个需要改所处中断时间
    pit_ms_init(MENU_KEY_PIT_INDEX, KEY_PIT_TIME); // 按键扫描PIT初始化
}

void menu_key_event_handle(void){
    if((key_get_state(KEY_1) == KEY_SHORT_PRESS) || (key_get_state(KEY_1) == KEY_LONG_PRESS)){
            motor_interface_power_flag = !motor_interface_power_flag; // 切换电机PWM输出状态
            key_clear_state(KEY_1);
    }
    if((key_get_state(KEY_2) == KEY_SHORT_PRESS) || (key_get_state(KEY_2) == KEY_SHORT_PRESS)){
        motor_fun_pwm_duty = (motor_fun_pwm_duty == 0) ? MOTOR_FUN_NORMAL_PWM_DUTY : 0; // 切换负压风扇PWM占空比
        key_clear_state(KEY_2);
    }
}

extern uint32 image_to_image_time;

void menu_network_print_info(void){
    static char info_buffer[120];
    sprintf(info_buffer, "%d,%d,%d,%d,%f,%d,%ld,%u,%u,%u\0",
        motor_left_speed,motor_right_speed,
        motor_left_pwm, motor_right_pwm,
        attitude.yaw, error_image,image_to_image_time,corner_feature.T,corner_feature.left,corner_feature.right
    );
    network_vofa_send_str(info_buffer);
}
