#include "zf_common_headfile.h"
#pragma section all "cpu1_dsram"
// 将本语句与#pragma section all restore语句之间的全局变量都放在CPU1的RAM中


void core1_main(void)
{
    disable_Watchdog();                     // 关闭看门狗
    interrupt_global_enable(0);             // 打开全局中断
    // 外设初始化


    cpu_wait_event_ready();                 // 等待所有核心初始化完毕
    while (TRUE)
    {



    }
}
#pragma section all restore
