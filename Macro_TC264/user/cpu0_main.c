#include "zf_common_headfile.h"
#include "image_process.h"
#include "network_interface.h"
#include "motor_control.h"
#include "battery_protection.h"
#include "gyroscope_interface.h"
#include "menu_interface.h"

extern uint8 image_process_flag;

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
#ifdef SMARTCAR_DEBUG_IPS
    ips200_init(IPS200_TYPE_SPI);     // 初始化IPS200显示屏

#endif
    menu_interface_init();            // 菜单初始化
    gyro_init();                    // 陀螺仪初始化
    
    battery_protection_init();      // 电池保护初始化
    motion_control_init();          // 运动控制初始化
    
    cpu_wait_event_ready();         // 等待所有核心初始化完毕
    
    // motor_forward_speed = 700;      // 前进速度
    // motor_forward_speed = 700; // 科目2
    motor_forward_speed = 700; // 科目3
    motor_fun_pwm_duty  = 0;       // 负压风扇PWM初始占空比
    
    motor_interface_power_flag = 1; // 使能电机PWM输出
    motion_control_run_flag = 1;    // 电机运动标志位，这个标志位为0时，电机速度环的目标速度为0
    
    system_delay_ms(1000);          // 上电1s后开始运行，等待负压风扇稳定       
    
    motion_control_pit_run_flag = 0; // 电机速度闭环算法运行标志位
    while (TRUE)
    {
        menu_key_event_handle();
        // // 10ms以上运行一次
        // if(system_getval_ms() % 10 != 0){
        //     continue;
        // }
        // 电池保护，运行1.8s后开始检测，防止电机启动电流过大导致电池电压瞬间下降误触发保护
        if(system_getval_ms() > 3800 && battery_protection_check()){
            motor_interface_power_flag = 0; // 关闭电机PWM输出
            zf_assert(!battery_protection_check()); // 电池电压过低
        }
#ifdef SMARTCAR_DEBUG_IPS
        if(image_process_flag){ // 确保CPU1已经处理完图像了，CPU0可以访问图像数据了
            ips200_show_binary_image_with_line(0, 0, mt9v03x_copy_image[0], MT9V03X_W, MT9V03X_H); // 显示图像并带辅助线
            // ips200_displayimage03x(mt9v03x_copy_image[0], MT9V03X_W, MT9V03X_H);
        }
        menu_ips_print_info(MENU_INFO_EXTEND); // 显示扩展信息
#endif
#ifdef SMARTCAR_DEBUG_NET_INFO
        menu_network_print_info();
#endif
    }
}

#pragma section all restore
