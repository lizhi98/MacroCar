#include "zf_common_headfile.h"
#include "image_process.h"
#include "network_interface.h"
#include "gyroscope_interface.h"
#include "menu_interface.h"

volatile uint8 image_process_finish_flag = 0; // 图像处理标志位，0表示CPU1正在处理图像，1表示CPU1处理完图像了，CPU0可以访问图像数据了

uint8 image_process_flag = 0; // 为1时图像开始处理
uint32 image_to_image_time = 0;
uint32 image_process_time = 0;

#pragma section all "cpu1_dsram"
// 将本语句与#pragma section all restore语句之间的全局变量都放在CPU1的RAM中

/*
    CPU1 主要用于摄像头图像处理和图传
*/

void core1_main(void)
{
    disable_Watchdog();                     // 关闭看门狗
    interrupt_global_enable(0);             // 打开全局中断

    // 外设初始化
    mt9v03x_init();                         // 初始化总钻风摄像头

    // 初始化蜂鸣器
    // gpio_init(P33_10, GPO, 1, GPO_PUSH_PULL);
    cpu_wait_event_ready();                 // 等待所有核心初始化完毕

    uint32 image_time = 0;
    while (TRUE)
    {
        
        if(mt9v03x_finish_flag)
        {
            image_process_finish_flag = 0;
            network_interface_copy_image(mt9v03x_image[0], MT9V03X_W * MT9V03X_H); // 复制图像
            mt9v03x_finish_flag = 0; // 让摄像头继续采集下一帧图像并通过DMA传输到RAM中
#ifdef SMARTCAR_DEBUG_NET_IMG
            if(!network_status)seekfree_assistant_camera_send(); // 网络状态正常则发送数据
#endif
            image_to_image_time = (system_getval_ms() - image_time);
            image_time = system_getval_ms();
            image_process(mt9v03x_copy_image);
            image_process_finish_flag = 1;
            image_process_time = system_getval_ms() - image_time; // 记录图像处理时间
        }
    }
}
#pragma section all restore
