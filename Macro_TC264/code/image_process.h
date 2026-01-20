#ifndef __IMAGE_PROCESS_H__
#define __IMAGE_PROCESS_H__

#ifdef _VSCODE

#define MT9V03X_W               (188)                                           // 图像宽度     范围 [1-752]
#define MT9V03X_H               (120)                                           // 图像高度     范围 [1-480]

#ifndef TRUE
#define TRUE (1u)
#endif
#ifndef FALSE
#define FALSE (0u)
#endif
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned long       uint32;
typedef unsigned long long  uint64;
typedef float               float32;
typedef double              float64;
typedef signed char         int8;
typedef signed short int    int16;
typedef signed int          int32;
typedef signed long long    int64;
#else
#include "zf_device_mt9v03x.h"
#endif

#define image_w  (MT9V03X_W)
#define image_h  (MT9V03X_H)
extern uint8 img_threshold;
extern int points_count;
extern int error_image;
extern uint8 (*image)[MT9V03X_W];
extern uint8 left_start_point;
extern uint8 right_start_point;
extern uint8 left_line_list[MT9V03X_H];
extern uint8 right_line_list[MT9V03X_H];
void image_process(uint8 (*source_image)[MT9V03X_W]);
#endif
