#ifndef __IMAGE_PROCESS_H__
#define __IMAGE_PROCESS_H__

#include "zf_device_mt9v03x.h"

extern uint8 img_threshold;
extern int points_count;
extern int error_image;
extern uint8  image[MT9V03X_H][MT9V03X_W];
extern uint8 left_start_point;
extern uint8 right_start_point;
extern uint8 left_line_list[MT9V03X_H];
extern uint8 right_line_list[MT9V03X_H];
void image_process();
#endif
