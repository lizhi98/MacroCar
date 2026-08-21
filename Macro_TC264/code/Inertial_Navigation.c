#include "inertial_navigation.h"

InertialNavigationMode inertial_navigation_mode = INERTIAL_NAVIGATION_MODE_IDLE;

uint8 ins_data_valid_index_max[INS_DATA_SHEET_NUM_MAX] = INS_DATA_VALID_INDEX_MAX; // 有效数据的最大索引值，超过该值的数据将被视为无效数据

uint8 ins_current_data_sheet_index = 0; // 惯性导航数据集索引

uint32 ins_data_record_index = 0; // 当前记录数据点索引

static int32 motor_distance_last_point = 0;

InertialNavigationData ins_data[INS_DATA_SHEET_NUM_MAX][INS_DATA_POINT_NUM_MAX];

float ins_output_angle = 0.0f; // 惯性导航误差角度，单位为度

volatile uint8 ins_finish_flag = 0; // 惯性导航完成标志位，0表示未完成，1表示已完成

int32 ins_start_motor_average_distance_count_sum = 0; // 惯性导航开始时的电机平均编码器计数值
float ins_start_angle_z = 0.0f; // 惯性导航开始时偏移的角度，单位为度

void inertial_navigation_init(void){
    inertial_navigation_load_data();
}

void inertial_navigation_update(void){
    if(inertial_navigation_mode == INERTIAL_NAVIGATION_MODE_IDLE){
        return;
    }

    if(inertial_navigation_mode == INERTIAL_NAVIGATION_MODE_RECORD){
        // 记录数据
        if(ins_data_record_index < INS_DATA_POINT_NUM_MAX){
            if(motor_average_distance_count_sum - motor_distance_last_point >= INS_DATA_RECORD_DISTANCE){
                ins_data[ins_current_data_sheet_index][ins_data_record_index].angle_z = angle_z;
                ins_data_record_index++;
                motor_distance_last_point = motor_average_distance_count_sum;
            }
        }
    }else if(inertial_navigation_mode == INERTIAL_NAVIGATION_MODE_NAVIGATION){
        // 导航数据
        // 计算当前为第几个index
        int32 current_index = (motor_average_distance_count_sum - ins_start_motor_average_distance_count_sum) / INS_DATA_RECORD_DISTANCE + INS_PROSPECT_INDEX;
        if(current_index < 0){
            current_index = 0;
        }
        if(current_index < ins_data_valid_index_max[ins_current_data_sheet_index]){
            ins_output_angle = ins_data[ins_current_data_sheet_index][current_index].angle_z + ins_start_angle_z;
        }else{
            // ins_output_angle = ins_data[INS_DATA_VALID_INDEX_MAX - 1].angle_z;
            ins_finish_flag = 1; // 惯性导航完成
        }
    }
}

void inertial_navigation_save_data(void){
    // flash_read_page(0, 0, (uint32 *)ins_data, sizeof(InertialNavigationData) / sizeof(uint32) * INS_DATA_VALID_INDEX_MAX);
    char buffer[64];
    for(uint32 i = 0; i < ins_data_record_index; i++){
        sprintf(buffer, "%lu,%f\n", i, ins_data[ins_current_data_sheet_index][i].angle_z);
        network_vofa_send_str(buffer);
    }
    flash_write_page(0, ins_current_data_sheet_index, (const uint32 *)&ins_data[ins_current_data_sheet_index][0], sizeof(InertialNavigationData) / sizeof(uint32) * (ins_data_record_index));
    
}

void inertial_navigation_load_data(void){
    for(uint32 i = 0; i < INS_DATA_SHEET_NUM_MAX; i++){
        flash_read_page(0, i, (uint32 *)&ins_data[i][0], sizeof(InertialNavigationData) / sizeof(uint32) * ins_data_valid_index_max[i]);
    }
    // flash_read_page(0, 0, (uint32 *)ins_data, sizeof(InertialNavigationData) / sizeof(uint32) * INS_DATA_VALID_INDEX_MAX);
    // char buffer[100];
    // for(uint32 i = 0; i < INS_DATA_VALID_INDEX_MAX; i++){
    //     sprintf(buffer, "%lu,%f\n", i, ins_data[i].angle_z);
    //     network_vofa_send_str(buffer);
    // }
}
