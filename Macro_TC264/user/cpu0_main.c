#include "zf_common_headfile.h"
#include "image_process.h"
#include "network_interface.h"
#include "motor_control.h"
#include "battery_protection.h"
#include "gyroscope_interface.h"
#include "menu_interface.h"

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
    // gpio_init(P20_8, GPO, 0, GPO_PUSH_PULL);                    // 初始化GPIO
    // gpio_init(P20_9, GPO, 0, GPO_PUSH_PULL);                    // 初始化GPIO
    // 外设初始化
#ifdef SMARTCAR_DEBUG_IPS
    ips200_init(IPS200_TYPE_SPI);   // 初始化IPS200显示屏
    menu_interface_init();          // 菜单初始化
#endif
    menu_key_init();                // 按键初始化
    gyro_init();                    // 陀螺仪初始化
    
    battery_protection_init();      // 电池保护初始化
    motion_control_init();          // 运动控制初始化
    
    cpu_wait_event_ready();         // 等待所有核心初始化完毕
    
    motor_fun_pwm_duty  = 0;       // 负压风扇PWM初始占空比
    
    motor_interface_power_flag = 0;     // 使能所有电机PWM输出
    motion_control_run_flag = 1;        // 电机运动标志位，这个标志位为0时，电机速度环的目标速度为0
    
    motion_control_pit_run_flag = 0;    // 电机速度闭环算法运行标志位
#ifdef SMARTCAR_DEBUG_IPS
    menu_show_main();                   // 显示主菜单
#endif
    uint8 battery_check_flag = 0;
    while (TRUE)
    {
#ifdef SMARTCAR_DEBUG_IPS
        menu_key_event_handle();
        menu_event_handle(); // 菜单事件处理函数，根据按键状态切换菜单和修改变量值
#else
        no_screen_key_event_handle(); // 没有屏幕时的按键事件处理
#endif
        // 电池保护，运行1s后检测，防止电机启动电流过大导致电池电压瞬间下降误触发保护
        if(system_getval_ms() > 1000 && !battery_check_flag){
            // 电压检测
            if(battery_protection_check()){
                motor_interface_power_flag = 0; // 关闭电机PWM输出
                zf_assert(!battery_protection_check()); // 电池电压过低
                while(1);
            }
            battery_check_flag = 1;
        }
#ifdef SMARTCAR_DEBUG_NET_INFO
        // 每隔4ms发送一次网络调试信息
        if(system_getval_ms() % 4 == 0){
            menu_network_print_info();
        }
#endif
    }
}

#pragma section all restore
