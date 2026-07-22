#include "network_interface.h"
#include "zf_driver_delay.h"

uint8 network_status = 1; // 网络状态，0表示正常 1表示未初始化 2表示初始化失败

uint8 mt9v03x_copy_image[MT9V03X_H][MT9V03X_W];

#define HOST_CMD_BUFFER_SIZE 32
static uint8 host_cmd_buffer[HOST_CMD_BUFFER_SIZE]; // 上位机命令接收缓冲区
static uint8 host_cmd_received_flag = 0; // 上位机命令接收完成标志
static uint8 host_cmd_buffer_index = 0; // 上位机命令缓冲区索引

// vofa+
uint32  vofa_pack_id = 0;
static  char    network_vofa_send_buffer[128];

void network_interface_init(void){
    // 初始化并连接wifi
    if(wifi_spi_init(WIFI_SSID, WIFI_PASSWORD)){
        network_status = 2;
        return;
    }
    // 连接TCP服务器
    if(wifi_spi_socket_connect("TCP", HOST_IP, HOST_PORT, "6666")){
        network_status = 2;
        return;
    }
    network_status = 0;
}

void network_interface_copy_image(uint8 * image, size_t length){
    // image为空
    if(image == NULL){
        zf_assert(0);
    }
    memcpy(mt9v03x_copy_image, image, length);
}

// 逐飞助手
void network_interface_seekfree_host_config(uint8 *image_addr){
    seekfree_assistant_interface_init(SEEKFREE_ASSISTANT_WIFI_SPI);
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, image_addr, MT9V03X_W, MT9V03X_H);
}

// VOFA+
void network_vofa_send_str(char * str){
    if(network_status != 0){
        return; // 网络未初始化或初始化失败，不发送数据
    }
    // sprintf(network_vofa_send_buffer, "%lu,%s\n" , vofa_pack_id++, str);
    sprintf(network_vofa_send_buffer, "%lu,%s\n" , system_getval_ms(), str);
    wifi_spi_send_buffer((const uint8 *)network_vofa_send_buffer, strlen(network_vofa_send_buffer));
}

uint8 data_parse(uint8 *target_buffer, uint8 *origin_buffer, char start_char, char end_char)
{
    uint8 return_state = 0;
    char *location1 = NULL;
    char *location2 = NULL;
    location1 = strchr((char *)origin_buffer, start_char);
    if(location1)
    {
        location1 ++;
        location2 = strchr(location1, end_char);
        if(location2)
        {
            memcpy(target_buffer, location1, location2-location1);
            *(target_buffer + (location2-location1)) = '\0'; // 确保字符串以null结尾
        }
        else
        {
            return_state = 1;
        }
    }
    else
    {
        return_state = 1;
    }
    return return_state;
}

void network_vofa_cmd_process(){
    // 上位机命令存储
    if(!host_cmd_received_flag){
        // 从WiFi模块的UART接收缓冲区读取数据，存储到host_cmd_buffer中，并设置host_cmd_received_flag标志
        char temp = '\0';
        if(wifi_spi_read_buffer((uint8*)&temp, 1) == 1){ // 从WiFi模块的UART接收缓冲区读取一个字节
            if(temp == '\r'){ // 以\r作为命令结束标志
                if(host_cmd_buffer_index > HOST_CMD_BUFFER_SIZE - 1){
                    host_cmd_buffer_index = HOST_CMD_BUFFER_SIZE - 1;
                }
                host_cmd_buffer[host_cmd_buffer_index] = '\0'; // 确保字符串以null结尾
                host_cmd_received_flag = 1; // 设置命令接收完成标志
            } else {
                // 将接收到的字节存储到host_cmd_buffer中，注意防止缓冲区溢出
                if(host_cmd_buffer_index < HOST_CMD_BUFFER_SIZE - 1){ // 确保有空间存储新数据
                    host_cmd_buffer[host_cmd_buffer_index] = temp; // 存储接收到的字节并更新索引
                    host_cmd_buffer_index++;
                } else {
                    host_cmd_buffer_index = 0; // 如果缓冲区满了，重置索引，丢弃之前的数据
                }
            }
        }
    }
    // 将host_cmd_buffer中的命令解析出来，根据命令内容调整工作模式、风扇转速或报警状态等
    if(host_cmd_received_flag){
        // 解析数据
        char cmd[HOST_CMD_BUFFER_SIZE];
        if(!data_parse((uint8*)cmd, host_cmd_buffer, '|', '|')){ // 从host_cmd_buffer中解析出命令
            // 解析成功
            // wifi_spi_send_buffer((uint8*)cmd, strlen(cmd), WIFI_SPI_SEND_WAIT); // 通过WiFi模块的SPI发送数据
            // 解析工作模式
            do{
                int32 speed_forward_linear;
                if(sscanf(cmd, "S_L:%d", &speed_forward_linear) == 1){ // 从命令中提取工作模式字符串
                    motor_forward_linear_speed = speed_forward_linear;
                    break;
                }
                int32 speed_forward_curve;
                if(sscanf(cmd, "S_C:%d", &speed_forward_curve) == 1){ // 从命令中提取转弯速度
                    motor_forward_curve_speed = speed_forward_curve;
                    break;
                }
                float image_kp;
                if(sscanf(cmd, "IMP:%f", &image_kp) == 1){ // 从命令中提取图像处理比例系数
                    motion_image_steering_pid.kp = image_kp;
                    break;
                }
                float image_kd;
                if(sscanf(cmd, "IMD:%f", &image_kd) == 1){ // 从命令中提取图像处理微分系数
                    motion_image_steering_pid.kd = image_kd;
                    break;
                }
                float gz_kp;
                if(sscanf(cmd, "GZP:%f", &gz_kp) == 1){ // 从命令中提取陀螺仪比例系数
                    motor_steering_pid.kp = gz_kp;
                    break;
                }
                float gz_kd;
                if(sscanf(cmd, "GZD:%f", &gz_kd) == 1){ // 从命令中提取陀螺仪微分系数
                    motor_steering_pid.kd = gz_kd;
                    break;
                }
                int32 exp_time;
                if(sscanf(cmd, "EXP:%d", &exp_time) == 1){ // 从命令中提取曝光时间
                    if(exp_time < 0) exp_time = 0;
                    if(exp_time > 512) exp_time = 512;
                    mt9v03x_set_exposure_time_sccb_1(exp_time);
                    // char buffer[32];
                    // sprintf(buffer, "EXP:%d\r\n", exp_time);
                    // wifi_spi_send_buffer(buffer, strlen(buffer)); // 通过WiFi模块的SPI发送数据
                    break;
                }
            }while(0);
        }
        host_cmd_received_flag = 0; // 处理完成后重置命令接收标志
        host_cmd_buffer_index = 0; // 重置命令缓冲区索引，准备接收下一条命令
    }
}
