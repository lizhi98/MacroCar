#include "zf_common_headfile.h"
#include "image_process.h"
#pragma section all "cpu0_dsram"
// 将本语句与#pragma section all restore语句之间的全局变量都放在CPU0的RAM中

int core0_main(void)
{
    clock_init();                   // 获取时钟频率<务必保留>
    debug_init();                   // 初始化默认调试串口
    // 外设初始化
    mt9v03x_init();
    ips200_init(IPS200_TYPE_PARALLEL8);
    if(wifi_spi_init("AP-lizhi","9894653xxk") == 1){
        ips200_show_string(0,0, "init failed      ");
        while(1);
    }
    wifi_spi_socket_connect("TCP", "10.206.51.252", "8086", "9999");
    ips200_show_string(0,0, "connected     ");
    seekfree_assistant_interface_init(SEEKFREE_ASSISTANT_WIFI_SPI); // 使用高速WIFI SPI模块进行数据收发
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, mt9v03x_image, MT9V03X_W, MT9V03X_H); // 配置摄像头图像信息
    cpu_wait_event_ready();         // 等待所有核心初始化完毕
    while (TRUE)
    {
        if(mt9v03x_finish_flag)    // 判断一场图像是否采集完成
        {
            seekfree_assistant_camera_send(); // 发送摄像头图像到逐飞助手
            mt9v03x_finish_flag = 0;
        }
    }
}

#pragma section all restore
