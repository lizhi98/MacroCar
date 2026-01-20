#include "zf_common_headfile.h"
#include "image_process.h"
#include "network_interface.h"

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
    
    cpu_wait_event_ready();         // 等待所有核心初始化完毕

    while (TRUE)
    {

    }
}

#pragma section all restore
