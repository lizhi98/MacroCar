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

    cpu_wait_event_ready();         // 等待所有核心初始化完毕
    while (TRUE)
    {
        ips200_show_gray_image(0, 0, image[0], MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);
        ips200_show_int(0, 140, left_start_point,3);
        ips200_show_int(0, 160, right_start_point,3);

        ips200_show_int(0, 180, points_count,3);
//        ips200_draw_point(left_start_point,117,RGB565_RED);
//        ips200_draw_point(left_start_point+1,117,RGB565_RED);
//        ips200_draw_point(left_start_point,118,RGB565_RED);
//        ips200_draw_point(left_start_point+1,118,RGB565_RED);
//        ips200_draw_point(right_start_point,117,RGB565_BLUE);
//        ips200_draw_point(right_start_point-1,117,RGB565_BLUE);
//        ips200_draw_point(right_start_point,118,RGB565_BLUE);
//        ips200_draw_point(right_start_point-1,118,RGB565_BLUE);
        for(int i=0;i<120;i++)
        {
            ips200_draw_point(left_line_list[i],i,RGB565_RED);
            ips200_draw_point(right_line_list[i],i,RGB565_BLUE);
        }
    }
}

#pragma section all restore
