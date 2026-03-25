#include "zf_common_headfile.h"
#include "image_process.h"
#include "network_interface.h"
#include "gyroscope_interface.h"

extern uint8 core_busy_flag;

uint32 image_to_image_time = 0;

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
#ifdef SMARTCAR_DEBUG_IPS
    ips200_init(IPS200_TYPE_PARALLEL8);     // 初始化IPS200显示屏
#endif
    mt9v03x_init();                         // 初始化总钻风摄像头
#if defined(SMARTCAR_DEBUG_NET_IMG) || defined(SMARTCAR_DEBUG_NET_INFO)
    network_interface_init();
    network_interface_seekfree_host_config(&mt9v03x_copy_image[0][0]); // 配置逐飞助手摄像头信息
#endif
    // 初始化蜂鸣器
    // gpio_init(P33_10, GPO, 1, GPO_PUSH_PULL);
    cpu_wait_event_ready();                 // 等待所有核心初始化完毕

    uint32 image_time = 0;
    while (TRUE)
    {
        if(mt9v03x_finish_flag)
        {
            network_interface_copy_image(mt9v03x_image[0], MT9V03X_W * MT9V03X_H); // 复制图像
            mt9v03x_finish_flag = 0; // 让摄像头继续采集下一帧图像并通过DMA传输到RAM中
#ifdef SMARTCAR_DEBUG_NET_IMG
            if(!network_status)seekfree_assistant_camera_send(); // 网络状态正常则发送数据
#endif
            // 图像处理
            // image_process(mt9v03x_copy_image, gyro_current_data.angle_z);
            image_process(mt9v03x_copy_image, attitude.yaw);
            image_to_image_time = system_getval_ms() - image_time;
            image_time = system_getval_ms();
#ifdef SMARTCAR_DEBUG_IPS
            // 多核访问控制
            if(!core_busy_flag){
                core_busy_flag = 1;
                ips200_displayimage03x(mt9v03x_copy_image[0], MT9V03X_W, MT9V03X_H); // 显示图像
                core_busy_flag = 0;
            }
#endif

        }
    }
}
#pragma section all restore
