#ifndef __MENU_INTERFACE_H__
#define __MENU_INTERFACE_H__

#include "zf_device_ips200.h"
#include "zf_device_wifi_spi.h"
#include "motor_control.h"
#include "battery_protection.h"
#include "gyroscope_interface.h"
#include "zf_device_key.h"

typedef enum _MenuPrintInfoType{
    MENU_INFO_NORMAL = 0,
    MENU_INFO_EXTEND,
}MenuPrintInfoType;

void menu_interface_init(void);
void menu_ips_print_info(MenuPrintInfoType info_type);

void menu_key_event_handle(void);

#endif