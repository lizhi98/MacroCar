#include "quaternion.h"

#define SAMPLE_TIME_MS  1.0f
#define DEG_TO_RAD      0.017453292519943f

Quaternion attitude = {1.0f, 0.0f, 0.0f, 0.0f};
static uint8 initialized = 0;

float angle_z = 0.0f;

int32 angle_z_count = 0;
static float yaw_last = 0.0f;

void quaternion_init(void)
{
    attitude.q0 = 1.0f;
    attitude.q1 = 0.0f;
    attitude.q2 = 0.0f;
    attitude.q3 = 0.0f;
    initialized = 1;
    
}

void quaternion_update(void)
{
    if(!initialized) return;

    yaw_last = attitude.yaw;

    // 获取原始陀螺仪数据并转换为弧度/秒
    float gx = imu660rc_gyro_transition(imu660rc_gyro_x/10*10) * DEG_TO_RAD;
    float gy = imu660rc_gyro_transition(imu660rc_gyro_y/10*10) * DEG_TO_RAD;
    float gz = imu660rc_gyro_transition(imu660rc_gyro_z/10*10) * DEG_TO_RAD;

    // 一阶龙格-库塔法积分
    float dt = SAMPLE_TIME_MS / 1000.0f;
    float q0 = attitude.q0;
    float q1 = attitude.q1;
    float q2 = attitude.q2;
    float q3 = attitude.q3;

    // 四元数微分方程
    attitude.q0 += (-q1*gx - q2*gy - q3*gz) * 0.5f * dt;
    attitude.q1 += (q0*gx - q3*gy + q2*gz) * 0.5f * dt;
    attitude.q2 += (q3*gx + q0*gy - q1*gz) * 0.5f * dt;
    attitude.q3 += (-q2*gx + q1*gy + q0*gz) * 0.5f * dt;

    // 四元数归一化
    float norm = sqrtf(attitude.q0 * attitude.q0 + attitude.q1 * attitude.q1 +
                       attitude.q2 * attitude.q2 + attitude.q3 * attitude.q3);
    if(norm < 1e-6f) return;
    norm = 1.0f / norm;
    attitude.q0 *= norm;
    attitude.q1 *= norm;
    attitude.q2 *= norm;
    attitude.q3 *= norm;

    // 转换为欧拉角
    attitude.yaw = atan2f(2.0f * (attitude.q0 * attitude.q3 + attitude.q1 * attitude.q2),
                          1.0f - 2.0f * (attitude.q2 * attitude.q2 + attitude.q3 * attitude.q3))
                   * (1.0f / DEG_TO_RAD);
    
    // 计算角度Z
    if(yaw_last < -90.0f && attitude.yaw > 90.0f){
        angle_z_count--;
    } else if(yaw_last > 90.0f && attitude.yaw < -90.0f){
        angle_z_count++;
    }
    angle_z = attitude.yaw + angle_z_count * 360.0f;
}
float get_angle_err(float angle)
{
    float diff = fmodf(fabsf(attitude.yaw - angle), 360.0f);
    return (diff > 180.0f) ? (360.0f - diff) : diff;
}


