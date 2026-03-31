#ifndef __MENU_INTERFACE_H__
#define __MENU_INTERFACE_H__

#include "zf_device_ips200.h"
#include "network_interface.h"
#include "motor_control.h"
#include "battery_protection.h"
#include "gyroscope_interface.h"
#include "zf_device_key.h"
#include "image_process.h"
#include "quaternion.h"
#include "zf_common_font.h"

#define KEY_PIT_TIME           5 // 按键扫描时间，单位ms
#define MENU_KEY_PIT_INDEX     CCU60_CH1

typedef enum _MenuPrintInfoType{
    MENU_INFO_NORMAL = 0,
    MENU_INFO_EXTEND,
}MenuPrintInfoType;

// typedef enum _MenuObjectType{
//     MENU_OBJECT_PARENT,
//     MENU_OBJECT_VARIABLE,
//     MENU_OBJECT_FUNCTION,
// }MenuObjectType;

// typedef enum _MenuObjectIndex{
    
// }MenuObjectIndex;

// typedef struct _ParentMenuObjectData{
//     MenuObjectIndex child_menu_index_list[9]; // 最多10个子菜单
// }MenuObjectParentData;


// typedef union _MenuObjectData{

// }

// typedef struct _MenuObject{
//     MenuObjectType type;
//     char show_name[16];

// }

extern int32 motor_left_pwm;
extern int32 motor_right_pwm;

void menu_interface_init(void);

void menu_ips_print_info(MenuPrintInfoType info_type);
void menu_network_print_info(void);

void menu_key_init(void);
void menu_key_event_handle(void);

void image_show_line(void);

void menu_main(void);


#endif