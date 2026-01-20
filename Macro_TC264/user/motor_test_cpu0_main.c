// #include "zf_common_headfile.h"
// #include "image_process.h"
// #include "network_interface.h"

// #pragma section all "cpu0_dsram"
// // 将本语句与#pragma section all restore语句之间的全局变量都放在CPU0的RAM中

// int core0_main(void)
// {
//     clock_init();                   // 获取时钟频率<务必保留>
//     debug_init();                   // 初始化默认调试串口
//     // 外设初始化
//     ips200_init(IPS200_TYPE_PARALLEL8);
//     cpu_wait_event_ready();         // 等待所有核心初始化完毕

//     pwm_init(ATOM0_CH4_P02_4, 17000, 3000); // 初始化PWM  频率17KHz  占空比30%
//     gpio_init(P02_5, GPO, 1, GPO_PUSH_PULL); // 初始化P02_5为推挽输出 高电平
//     while (TRUE)
//     {
//         system_delay_ms(1000);      // 延时1s
//         gpio_toggle_level(P02_5);   // 翻转P02_5电平
//     }
// }

// #pragma section all restore
