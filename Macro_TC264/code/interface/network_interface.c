#include "network_interface.h"
#include "zf_driver_delay.h"

uint8 network_status = 1; // 网络状态，0表示正常 1表示未初始化 2表示连接失败

uint32 pack_id = 0;

NetworkPack * network_image_pack = NULL;
uint8 mt9v03x_copy_image[MT9V03X_H][MT9V03X_W];

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
    // sprintf(network_vofa_send_buffer, "%lu,%s\n" , vofa_pack_id++, str);
    sprintf(network_vofa_send_buffer, "%lu,%s\n" , system_getval_ms(), str);
    wifi_spi_send_buffer((const uint8 *)network_vofa_send_buffer, strlen(network_vofa_send_buffer));
}


