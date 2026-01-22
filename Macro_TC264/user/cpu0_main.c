#include "zf_common_headfile.h"
#include "image_process.h"
#include "network_interface.h"
#include "motor_control.h"

#pragma section all "cpu0_dsram"
// 将本语句与#pragma section all restore语句之间的全局变量都放在CPU0的RAM中

/*
    CPU0 主要用于运动控制、菜单
*/

int core0_main(void)
{
    clock_init();                   // 获取时钟频率<务必保留>
    debug_init();                   // 初始化默认调试串口
    
    // 外设初始化
    motion_control_init();          // 运动控制初始化
    cpu_wait_event_ready();         // 等待所有核心初始化完毕
    int32 motor_left_speed = 0;
    int32 motor_right_speed = 0;
    char info_buffer[32];
    motor_interface_power_flag = 1; // 使能电机PWM输出
    motion_control_run_flag = 1;   // 使能运动控制
    while (TRUE)
    {
        motor_get_speed(&motor_left_speed, &motor_right_speed);
        sprintf(info_buffer, "L_S:%4d   R_S:%4d ", motor_left_speed, motor_right_speed);
        ips200_show_string(0, 0, info_buffer);
        sprintf(info_buffer, "L_P:%5d  R_P:%5d", motor_left_current_pwm_duty, motor_right_current_pwm_duty);
        ips200_show_string(0, 20, info_buffer);
    }
}

#pragma section all restore
