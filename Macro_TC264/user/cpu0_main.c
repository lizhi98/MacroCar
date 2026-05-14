#include "zf_common_headfile.h"
#include "image_process.h"
#include "network_interface.h"
#include "motor_control.h"
#include "battery_protection.h"
#include "gyroscope_interface.h"
#include "menu_interface.h"
// #include "menu_pro.h"

#pragma section all "cpu0_dsram"
// 将本语句与#pragma section all restore语句之间的全局变量都放在CPU0的RAM中

/*
    CPU0 主要用于运动控制、菜单
*/

int core0_main(void)
{
    clock_init();                   // 获取时钟频率<务必保留>
    debug_init();                   // 初始化默认调试串口
    system_start();                 // 启动系统计时器，计算车上电时间
    // 外设初始化
#if defined(SMARTCAR_DEBUG_IPS) || defined(SMARTCAR_DEBUG_IPS_PRO)
    menu_interface_init();          // 菜单初始化
#endif
    menu_key_init();                // 按键初始化
    gyro_init();                    // 陀螺仪初始化
    
    battery_protection_init();      // 电池保护初始化
    motion_control_init();          // 运动控制初始化
    
    battery_protection_init();      // 电池保护初始化
    motion_control_init();          // 运动控制初始化

    cpu_wait_event_ready();         // 等待所有核心初始化完毕
    
    motor_traveling_power_flag = 0;     // 全部电机PWM输出标志位，0:PWM输出为0  1:PWM输出正常
    motion_control_run_flag = 1;        // 行进电机运动标志位，0:电机速度环的目标速度为0，负压不变  1:电机速度盒负压电压受到算法控制
    forward_speed_decision_enable = 1;  // 速度决策标志位，0:不执行速度决策  1:执行速度决策
#ifdef SMARTCAR_DEBUG_IPS
    menu_show_main();                   // 显示主菜单
#endif
    uint8 battery_checked_flag = 0;

    // 延迟一段时间，等待负压自检
    system_delay_ms(500);
    while (TRUE)
    {
        // if(T_index == 7){
        //     motor_traveling_power_flag = 0;
        // }
#ifdef SMARTCAR_DEBUG_IPS
        menu_get_key_event();
        menu_event_handle(); // 菜单事件处理函数，根据按键状态切换菜单和修改变量值
#else
        no_screen_key_event_handle(); // 没有屏幕时的按键事件处理
#endif
        if(battery_protection_check()){
            if(!battery_checked_flag)
            {
                motor_traveling_power_flag = 0; // 关闭电机PWM输出
                zf_assert(0); // 电池电压过低
                while(1);
            }
        }
        battery_checked_flag = 1;
#ifdef SMARTCAR_DEBUG_NET_INFO
        // 每隔4ms发送一次网络调试信息
        if(system_getval_ms() % 4 == 0){
            network_print_info();
        }
#endif
    }
}

#pragma section all restore
