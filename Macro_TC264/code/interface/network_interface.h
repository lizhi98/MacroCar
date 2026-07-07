#ifndef __NETWORK_INTERFACE_H__
#define __NETWORK_INTERFACE_H__

#include "zf_device_wifi_spi.h"
#include "seekfree_assistant.h"
#include "seekfree_assistant_interface.h"
#include "zf_device_mt9v03x_double.h"
#include "zf_driver_timer.h"
#include "motor_control.h"

#define WIFI_SSID           "XIAOMI_GoGoGo"
#define WIFI_PASSWORD       "9894653xxk"

#define HOST_IP             "192.168.31.98"
#define HOST_PORT           "9895"
#define HOST_CONNECT_MODE   "TCP"

typedef enum _NetworkPackType{
    NETWORK_PACK_TYPE_PARAMETER   = 0x00,
    NETWORK_PACK_TYPE_IMAGE       = 0x01,
}NetworkPackType;

typedef struct _NetworkPack{
    uint32 pack_id;
    uint16 buffer_length;
    uint8  buffer[];
}NetworkPack;

extern uint8 mt9v03x_copy_image[MT9V03X_H][MT9V03X_W];
extern uint8 network_status;

void  network_interface_init(void);

void  network_interface_copy_image(uint8 * image, size_t length);
// 逐飞助手
void network_interface_seekfree_host_config(uint8 *image_addr);
void network_vofa_send_str(char * str);
void network_print_info(void);

void network_vofa_cmd_process(void);

#endif
