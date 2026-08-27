#ifndef CODE_INERTIAL_NAVIGATION_H_
#define CODE_INERTIAL_NAVIGATION_H_

#include "zf_common_typedef.h"
#include "quaternion.h"
#include "zf_driver_encoder.h"
#include "motor_interface.h"
#include "zf_driver_flash.h"
#include "interface/network_interface.h"

// #define PI 3.1415926f
// #define radius 15

#define INS_DATA_POINT_NUM_MAX 500
#define INS_DATA_SHEET_NUM_MAX 7

// 0 164   1 157
#define INS_DATA_VALID_INDEX_MAX {160,157,134,152,144,152} // 有效数据的最大索引值，超过该值的数据将被视为无效数据

#define INS_DATA_RECORD_DISTANCE 500 // 记录数据的距离间隔，单位为编码器计数

#define INS_PROSPECT_INDEX 3    // 惯导前瞻索引

typedef struct _InertialNavigationData
{
    float32 angle_z; // 角度
} InertialNavigationData;

typedef enum _InertialNavigationMode
{
    INERTIAL_NAVIGATION_MODE_IDLE = 0,
    INERTIAL_NAVIGATION_MODE_RECORD,
    INERTIAL_NAVIGATION_MODE_NAVIGATION,
} InertialNavigationMode;

extern uint8 ins_data_valid_index_max[INS_DATA_SHEET_NUM_MAX]; // 有效数据的最大索引值，超过该值的数据将被视为无效数据

extern InertialNavigationMode inertial_navigation_mode;
extern uint8 ins_current_data_sheet_index;

extern uint32 ins_data_record_index;

extern int32 ins_start_motor_average_distance_count_sum;
extern float ins_start_angle_z;

extern InertialNavigationData ins_data[INS_DATA_SHEET_NUM_MAX][INS_DATA_POINT_NUM_MAX];
extern float ins_output_angle;

extern volatile uint8 ins_finish_flag; // 惯性导航完成标志位，0表示未完成，1表示已完成

void inertial_navigation_init(void);
void inertial_navigation_update(void);

void inertial_navigation_save_data(void);
void inertial_navigation_load_data(void);

#endif