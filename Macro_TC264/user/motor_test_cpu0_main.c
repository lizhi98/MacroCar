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
//     ips200_init(IPS200_TYPE_SPI);
//     cpu_wait_event_ready();         // 等待所有核心初始化完毕
//     // g-pio_init(P20_9, GPO, 1, GPO_PUSH_PULL); // 初始化P20_9为推挽输出，初始电平为高
//     ips200_show_string(0, 0, "Hello TC264!"); // 在屏幕上显示字符串
//     while (TRUE)
//     {
        
//     }
// }

// #pragma section all restore
