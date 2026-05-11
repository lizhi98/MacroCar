#ifndef __IMAGE_PROCESS_H__
#define __IMAGE_PROCESS_H__

#ifdef _VSCODE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

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
#include "zf_device_mt9v03x_double.h"

#endif
#include "quaternion.h"
#define image_w  (MT9V03X_W-1)
#define image_h  (MT9V03X_H)



typedef struct
{
    int left_feature_flag1;
    int left_feature_flag2;
    int right_feature_flag1;
    int right_feature_flag2;
    int height_feature_flag1;
    int height_feature_flag2;
}FeatureDetectResult;
typedef struct
{
    uint8 left;
    uint8 right;
    uint8 height;
}feature_result;

typedef struct
{
    uint8 left;
    uint8 right;
    uint8 T;
}corner_result;

extern uint8 left_lost_times;
extern uint8 right_lost_times;
extern uint8 feature_raw_l;
extern uint8 feature_raw_r;
extern uint8 feature_label;
extern uint8 detect_feature_row;
extern uint8 condition_corner;
extern int feature_corner_left;
extern int feature_corner_right;
extern int T_corner;
extern int up_feature_row;
extern FeatureDetectResult image_feature;
// extern int condition;

// extern int feature_corner_l;
// extern int feature_corner_r;
// extern float parameterB;
// extern uint8 T_index;
// extern FeatureDetectResult image_feature;
// extern FeatureDetectResult image_plan_feature;
// //enum TurnDirection {LEFT=-1, STRAIGHT=0, RIGHT=1};
// extern int stop_line;
extern uint8 img_threshold;
// extern int points_count;
// extern int error_image;
extern int error_image_last;
// extern uint8 (*image)[MT9V03X_W];
extern uint8 left_line_list[MT9V03X_H];
extern uint8 right_line_list[MT9V03X_H];
extern uint8 mid_line_list[MT9V03X_H];

extern uint8 speed_select_label;

extern feature_result result_feature;

extern uint8 deceleration_label;
// T控制相关
// extern uint8 index_sum_num;
// extern int * T_index_list;

#define white_point 255
#define black_point 0

extern int error_image;
extern int feature_T;
extern uint8 condition_T;
void image_process(uint8 (*source_image)[MT9V03X_W]);

#endif
