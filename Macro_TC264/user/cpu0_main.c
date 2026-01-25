#include "zf_common_headfile.h"
#include "image_process.h"
#include "network_interface.h"
#include "motor_control.h"
#include "battery_protection.h"
#include "gyroscope_interface.h"

uint8 core_busy_flag = 0; // 核心忙标志

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
    battery_protection_init();      // 电池保护初始化
    gyro_init();                    // 陀螺仪初始化
    key_init(5);                   // 按键初始化 5ms扫描一次 // 改这个需要改所处中断时间
    cpu_wait_event_ready();         // 等待所有核心初始化完毕

    motor_interface_power_flag = 1; // 使能电机PWM输出
    motion_control_run_flag = 1;    // 使能运动控制
    motor_forward_speed = 380;       // 前进速度
    motor_fun_pwm_duty = 2000;       // 负压风扇PWM占空比
    // 计时器
    system_start();                 // 启动系统定时器
    char info_buffer[32];
    while (TRUE)
    {
        if(key_get_state(KEY_1) == KEY_SHORT_PRESS){
            motor_interface_power_flag = !motor_interface_power_flag; // 切换电机PWM输出状态
            key_clear_state(KEY_1);
        }
        if(key_get_state(KEY_2) == KEY_SHORT_PRESS){
            motor_fun_pwm_duty = (motor_fun_pwm_duty == 0) ? 2000 : 0; // 切换负压风扇PWM占空比
            key_clear_state(KEY_2);
        }
        // 30ms以上运行一次
        if(system_getval_ms() % 30 != 0){
            continue;
        }
        if(battery_protection_check()){
            motor_interface_power_flag = 0; // 关闭电机PWM输出
            motor_fun_pwm_duty = 0;          // 关闭负压风扇PWM输出
            zf_assert(!battery_protection_check()); // 电池电压过低
        }
#ifdef SMARTCAR_DEBUG_IPS
        // 多核访问控制
        if(!core_busy_flag){
            core_busy_flag = 1;
            sprintf(info_buffer, "L_S:%+4d   R_S:%+4d ", motor_left_speed, motor_right_speed);
            ips200_show_string(0, 130, info_buffer);
            sprintf(info_buffer, "L_P:%+5d  R_P:%+5d", motor_left_current_pwm_duty, motor_right_current_pwm_duty);
            ips200_show_string(0, 150, info_buffer);
            sprintf(info_buffer, "B_A:%+4d", battery_protection_adc_value);
            ips200_show_string(0, 170, info_buffer);
            sprintf(info_buffer, "I_E:%+5d", error_image);
            ips200_show_string(0, 190, info_buffer);
            sprintf(info_buffer, "GZ:%+6.2f  ", gyro_current_data.gyro_z);
            ips200_show_string(0, 210, info_buffer);
            core_busy_flag = 0;
        }

#endif

        
    }
}

#pragma section all restore
