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
    ips114_init();
    uint8 result = mt9v03x_init();
    cpu_wait_event_ready();         // 等待所有核心初始化完毕
    ips114_show_int(0, 0, result, 8); // 测试显示数据
    while (TRUE)
    {
        if(mt9v03x_finish_flag)
        {
            ips114_displayimage03x(mt9v03x_image[0], MT9V03X_W, MT9V03X_H); // 显示图像
            // network_interface_copy_image(mt9v03x_image[0], MT9V03X_W * MT9V03X_H); // 复制图像
            mt9v03x_finish_flag = 0;
            
            // 图像处理
            // image_process(mt9v03x_copy_image);
            // ips114_displayimage03x(mt9v03x_copy_image[0], MT9V03X_W, MT9V03X_H); // 显示图像
        }
    }
}

#pragma section all restore
