#include "zf_common_headfile.h"
#include "image_process.h"
#include "network_interface.h"
#include "motor_control.h"
#include "battery_protection.h"
#include "gyroscope_interface.h"
#include "menu_interface.h"

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
    key_init(MOTOR_INTERFACE_PIT_TIME);   // 按键初始化 5ms扫描一次 // 改这个需要改所处中断时间
    system_start();                 // 启动系统计时器

    cpu_wait_event_ready();         // 等待所有核心初始化完毕

    motor_interface_power_flag = 1; // 使能电机PWM输出
    motion_control_run_flag = 1;    // 发车标志位

    motor_forward_speed = 920;      // 前进速度
    motor_fun_pwm_duty  = MOTOR_FUN_NORMAL_PWM_DUTY;      // 负压风扇PWM占空比
    
    while (TRUE)
    {
        menu_key_event_handle();
        // 30ms以上运行一次
        if(system_getval_ms() % 30 != 0){
            continue;
        }
        // 电池保护，运行1.8s后开始检测，防止电机启动电流过大导致电池电压瞬间下降误触发保护
        if(system_getval_ms() > 1800 && battery_protection_check()){
            motor_interface_power_flag = 0; // 关闭电机PWM输出
            zf_assert(!battery_protection_check()); // 电池电压过低
        }
#ifdef SMARTCAR_DEBUG_IPS
        // 多核访问控制
        if(!core_busy_flag){
            core_busy_flag = 1;
            menu_ips_print_info(MENU_INFO_NORMAL);
            core_busy_flag = 0;
        }
#endif
    }
}

#pragma section all restore
