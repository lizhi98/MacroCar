#include "network_interface.h"
#include "zf_driver_delay.h"

uint32 pack_id = 0;

NetworkPack * network_image_pack = NULL;

uint8 network_interface_init(void){
    // 初始化并连接wifi
    if(wifi_spi_init(WIFI_SSID, WIFI_PASSWORD)){
        return 1;
    }
    // 连接TCP服务器
    if(wifi_spi_socket_connect("TCP", HOST_IP, HOST_PORT, "6666")){
        return 2;
    }
    return 0;
}

void network_interface_test(void){
    wifi_spi_send_buffer((const uint8 *)"hello\n", 6);
    system_delay_ms(1000);
}

uint8 network_interface_copy_image(uint8 * image, size_t length){
    // image为空
    if(image == NULL){
        return 1;
    }
    // 若未分配network_image_pack，则创建
    if(network_image_pack == NULL){
        network_image_pack = (NetworkPack *)malloc(sizeof(uint32) + sizeof(uint16) + length);
        if(network_image_pack == NULL){
            return 2;
        }
    }
    if(length > network_image_pack->buffer_length){
        return 3;
    }
    memcpy(network_image_pack->buffer, image, length);
    return 0;
}

uint8 network_interface_send_pack(NetworkPackType pack_type, uint8 *buffer, uint16 length){
    switch(pack_type){
        case NETWORK_PACK_TYPE_PARAMETER:
            // 发送参数数据包
            {
                // 计算数据包大小
                size_t pack_size = sizeof(uint32) + sizeof(uint16) + length;
                // 分配内存
                NetworkPack *pack = (NetworkPack *)malloc(pack_size);
                if(pack == NULL){
                    return 1; // 内存分配失败
                }
                // 填充数据包
                pack->pack_id = pack_id++;
                pack->buffer_length = length;
                memcpy(pack->buffer, buffer, length);
                // 发送包
                wifi_spi_send_buffer((const uint8 *)pack, pack_size);
                // 释放内存
                free(pack);
            }
            break;
        case NETWORK_PACK_TYPE_IMAGE:
            // 发送图像数据包
            {

            }
    }
    return 0;
}

uint8 network_interface_receive_pack(){
    return 0;
}
