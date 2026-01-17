#include "zf_common_headfile.h"
#include "image_process.h"
#include "network_interface.h"

#pragma section all "cpu1_dsram"
// 将本语句与#pragma section all restore语句之间的全局变量都放在CPU1的RAM中


void core1_main(void)
{
    disable_Watchdog();                     // 关闭看门狗
    interrupt_global_enable(0);             // 打开全局中断

    // 外设初始化
    mt9v03x_init();
    network_interface_init();

    cpu_wait_event_ready();                 // 等待所有核心初始化完毕

    network_interface_seekfree_host_config(&mt9v03x_copy_image[0][0]); // 配置逐飞助手摄像头信息

    while (TRUE)
    {
        if(mt9v03x_finish_flag)    // 判断一场图像是否采集完成
        {
            network_interface_copy_image(mt9v03x_image[0], MT9V03X_W * MT9V03X_H); // 复制图像数据到网络接口缓存
            mt9v03x_finish_flag = 0;
            seekfree_assistant_camera_send(); // 发送摄像头图像到逐飞助手
        }
    }
}
#pragma section all restore
