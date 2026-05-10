#include "gyroscope_interface.h"

GyroData gyro_current_data = {0};

void gyro_pit_init(void){
    pit_ms_init(GYRO_PIT_INDEX, GYRO_PIT_TIME);
}
float gyro_z_sum = 0.0f;
int gyro_z_count = 0;
void gyro_pit_callback(void){
    imu660rc_get_gyro();
    gyro_z_sum += imu660rc_gyro_transition(imu660rc_gyro_z);
    gyro_z_count++;
    if(gyro_z_count >= 5){ // 每5次取平均值，减少噪声影响
        float gyro_z_avg = gyro_z_sum / gyro_z_count;
        gyro_current_data.gyro_z = gyro_z_avg;
        gyro_z_sum = 0.0f;
        gyro_z_count = 0;
    }
    // gyro_current_data.gyro_x = imu660rc_gyro_transition(imu660rc_gyro_x);
    // gyro_current_data.gyro_y = imu660rc_gyro_transition(imu660rc_gyro_y);
    // gyro_current_data.angle_x += gyro_current_data.gyro_x * GYRO_PIT_TIME / 1000.0f;
    // gyro_current_data.angle_y += gyro_current_data.gyro_y * GYRO_PIT_TIME / 1000.0f;
    // gyro_current_data.angle_z += gyro_current_data.gyro_z * GYRO_PIT_TIME / 1000.0f;
    quaternion_update();
}

// void gyro_get_data();

void gyro_init(void){
    if(!imu660rc_init(IMU660RC_QUARTERNION_DISABLE)){
        quaternion_init();
        gyro_pit_init();
    }
}
