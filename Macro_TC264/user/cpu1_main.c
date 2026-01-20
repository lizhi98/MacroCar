#include "zf_common_headfile.h"
#include "image_process.h"
#include "network_interface.h"

#pragma section all "cpu1_dsram"
// 将本语句与#pragma section all restore语句之间的全局变量都放在CPU1的RAM中

/*
    CPU1 主要用于摄像头图像处理和图传
*/
// #define SMARTCAR_DEBUG

void core1_main(void)
{
    disable_Watchdog();                     // 关闭看门狗
    interrupt_global_enable(0);             // 打开全局中断

    // 外设初始化
    mt9v03x_init();
#ifdef SMARTCAR_DEBUG
    network_interface_init();
    network_interface_seekfree_host_config(&mt9v03x_copy_image[0][0]); // 配置逐飞助手摄像头信息
#endif
    cpu_wait_event_ready();                 // 等待所有核心初始化完毕


    while (TRUE)
    {
        if(mt9v03x_finish_flag)
        {
            network_interface_copy_image(mt9v03x_image[0], MT9V03X_W * MT9V03X_H); // 复制图像
            mt9v03x_finish_flag = 0;
#ifdef SMARTCAR_DEBUG
            if(!network_status)seekfree_assistant_camera_send(); // 网络状态正常则发送数据
#endif
        }
    }
}
#pragma section all restore
