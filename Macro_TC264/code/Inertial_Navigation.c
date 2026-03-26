#include "Inertial_Navigation.h"
void Inertial_Navigation_Init(void)
{
    //imu660ra_init();
    quaternion_init();
}

int mode = 0; //0表示空 ,1表示采集数据，2表示读取数据

// 获取姿态数据（将其放入10ms的中断中），记录100s的数据
float yaw_data[data_num];
uint32 index = 0;
int judge_distance(void)
{
    int detected_distance=0;
    float left_count=(float)encoder_get_count(MOTOR_LEFT_ENCODER_INDEX);
    float right_count=(float)encoder_get_count(MOTOR_RIGHT_ENCODER_INDEX);
    float distance=(left_count+right_count)/2/1024*2*PI*radius; //单位mm
    if(distance>=10) //10mm记录一个数据
    {
        detected_distance=1;
        encoder_clear_count(MOTOR_LEFT_ENCODER_INDEX);
        encoder_clear_count(MOTOR_RIGHT_ENCODER_INDEX);
    }
    return detected_distance ;
}

void Inertial_Navigation_Getdate(void)
{
    if( mode==1 && judge_distance() )
    {
        yaw_data[index++] =attitude.yaw;
    }
}

float get_yaw_error(float target_yaw)
{
    float yaw_error;
    if( target_yaw - attitude.yaw > 180.0f )
    {
        yaw_error = target_yaw - attitude.yaw - 360.0f;
    }
    else if( target_yaw - attitude.yaw < -180.0f )
    {
        yaw_error = target_yaw - attitude.yaw + 360.0f;
    }
    else
    {
        yaw_error = target_yaw - attitude.yaw;
    }

    return yaw_error;
}


float get_target_yaw(void)
{
    float target_yaw=0.0f;
    float yaw_error=0.0f;
    int idx=0;
    if( mode==2 && judge_distance() &&idx<index )
    {
        target_yaw=yaw_data[idx++];
    }
    yaw_error=get_yaw_error(target_yaw);
    return yaw_error;
}
