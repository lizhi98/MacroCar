#include "zf_common_headfile.h"
#include "image_process.h"
#include "network_interface.h"
#include "motor_control.h"
#include "battery_protection.h"
#include "gyroscope_interface.h"
#include "menu_interface.h"
#include "seekfree_assistant.h"
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
    system_start();                 // 启动系统计时器
    // 外设初始化
#if defined(SMARTCAR_DEBUG_IPS) || defined(SMARTCAR_DEBUG_IPS_PRO)
    menu_interface_init();          // 菜单初始化
#endif
#if defined(SMARTCAR_DEBUG_NET_IMG_1) || defined(SMARTCAR_DEBUG_NET_IMG_2) || defined(SMARTCAR_DEBUG_NET_INFO)
    network_interface_init();
    network_interface_seekfree_host_config(&mt9v03x_image[0][0]); // 配置逐飞助手摄像头信息
    seekfree_assistant_camera_boundary_config(X_BOUNDARY, 120, left_line_list, right_line_list, mid_line_list, NULL, NULL, NULL); // 配置逐飞助手摄像头信息
#endif

    menu_key_init();                // 按键初始化
    gyro_init();                    // 陀螺仪初始化
    
    battery_protection_init();      // 电池保护初始化

    motion_control_init();          // 运动控制初始化

    cpu_wait_event_ready();         // 等待所有核心初始化完毕

    motor_traveling_power_flag = 0;     // 行进电机PWM输出标志位，0:PWM输出为0  1:PWM输出正常
    motion_control_run_flag = 1;        // 行进电机运动标志位，0:电机速度环的目标速度为0，负压不变  1:电机速度和负压电压受到算法控制
    forward_speed_decision_enable = 1;  // 速度决策标志位，0:不执行速度决策  1:执行速度决策
#ifdef SMARTCAR_DEBUG_IPS
    menu_show_main();                   // 显示主菜单
#endif
    uint8 battery_checked_flag = 0;
    // 延迟一段时间
    system_delay_ms(100);
    
    car_run_state = INIT_WAIT; // 初始化完成等待

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
                // zf_assert(0); // 电池电压过低
                // while(1);
            }
        }
        battery_checked_flag = 1;

        if(car_run_state == STARTING){
            run_control_protect_trigger_flag = 0;
            motor_fun_soft_start();         // 负压风扇软启动
            run_control_protect_enable = 1; // 使能运行保护
            motor_traveling_power_flag = 1; // 使能行进电机PWM输出
            motor_traveling_pid_run_flag = 1; // 使能行进电机速度环运行
            run_control_protect_trigger_flag = 0;
            car_run_state = RUNNING; // 切换到运行状态，电机速度环和负压风扇开度受到算法控制
        }
#ifdef SMARTCAR_DEBUG_NET_INFO
        // 每隔4ms发送一次网络调试信息
        network_print_info();
#endif
#ifdef SMARTCAR_DEBUG_NET_CMD
        network_vofa_cmd_process(); // 处理上位机发送的命令
#endif
#ifdef SMARTCAR_DEBUG_NET_IMG_1
            if(!network_status)seekfree_assistant_camera_send(); // 网络状态正常则发送数据
#endif
    }
}

#pragma section all restore
