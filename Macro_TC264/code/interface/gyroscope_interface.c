#include "gyroscope_interface.h"

GyroData gyro_current_data = {0};

void gyro_pit_init(void){
    pit_ms_init(GYRO_PIT_INDEX, GYRO_PIT_TIME);
}

void gyro_pit_callback(void){
    imu660ra_get_gyro();
    gyro_current_data.gyro_z = imu660ra_gyro_transition(imu660ra_gyro_z);
    // gyro_current_data.gyro_x = imu660ra_gyro_transition(imu660ra_gyro_x);
    // gyro_current_data.gyro_y = imu660ra_gyro_transition(imu660ra_gyro_y);
    // gyro_current_data.angle_x += gyro_current_data.gyro_x * GYRO_PIT_TIME / 1000.0f;
    // gyro_current_data.angle_y += gyro_current_data.gyro_y * GYRO_PIT_TIME / 1000.0f;
    gyro_current_data.angle_z += gyro_current_data.gyro_z * GYRO_PIT_TIME / 1000.0f;
    // quaternion_update();
}

// void gyro_get_data();

void gyro_init(void){
    if(!imu660ra_init()){
        // quaternion_init();
        gyro_pit_init();
    }
}
