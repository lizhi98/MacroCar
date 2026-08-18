#ifndef __MENU_INTERFACE_H__
#define __MENU_INTERFACE_H__

#include "common_headfile.h"

#include "network_interface.h"
#include "motor_control.h"
#include "battery_protection.h"
#include "gyroscope_interface.h"
#include "zf_device_key.h"
#include "image_process.h"
#include "quaternion.h"
#include "zf_common_font.h"
#include "inertial_navigation.h"

#define KEY_PIT_TIME           5 // 按键扫描时间，单位ms
#define MENU_KEY_PIT_INDEX     CCU60_CH1

// 按键命令
typedef enum _KeyCmd{
    KEY_NO_CMD = 0,
    KEY_UP, KEY_DOWN, KEY_ENTER, KEY_BACK
}KeyCmd;
extern uint32 car_start_time;

#ifdef SMARTCAR_DEBUG_IPS
// =========================================SMARTCAR_DEBUG_IPS=========================================

#include "zf_device_ips200.h"

// // 菜单显示信息类型枚举，用于配置在屏幕上显示信息时选择显示的内容
// typedef enum _MenuPrintInfoType{
//     MENU_INFO_NORMAL = 0,
//     MENU_INFO_EXTEND,
// }MenuPrintInfoType;

// ==============================菜单项目数据类型定义==============================

// 菜单项目类型
typedef enum _MenuObjectType{
    MENU_OBJECT_PARENT,
    MENU_OBJECT_VARIABLE,
    MENU_OBJECT_FUNCTION,
}MenuObjectType;

// 菜单项目索引
typedef enum _MenuObjectIndex{
    MAIN,
    INFO, RUN_1, RUN_2, RUN_3, Mod_S, Mod_T, Mod_PID,
}MenuObjectIndex;

// 不同菜单菜单项目的数据，最后用联合体封装
typedef struct _ParentMenuObjectData{
    uint8 child_menu_max_index; // ！！这里的index是下面数组的下标，不是菜单项目索引！！
    uint8 selected_child_menu_index; // ！！这里的index是下面数组的下标，不是菜单项目索引！！
    MenuObjectIndex child_menu_index_list[10]; // 最多支持10个子菜单项目
}MenuObjectParentData;

typedef enum _VariableType{
    VARIABLE_TYPE_uint8,
    VARIABLE_TYPE_uint16,
    VARIABLE_TYPE_uint32,
    VARIABLE_TYPE_int8,
    VARIABLE_TYPE_int16,
    VARIABLE_TYPE_int32,
    VARIABLE_TYPE_float,
    VARIABLE_TYPE_double,
}VariableType;

typedef struct _VariableMenuObjectData{
    void* variable_addr;
    VariableType variable_type;
    int   increase_step_int;
    float increase_step_float;
}VariableMenuObjectData;

typedef enum _FunctionType{
    FUNCTION_TYPE_RUN_ONCE,
    FUNCTION_TYPE_RUN_CONTINUOUSLY,
}FunctionType;

typedef struct _FunctionMenuObjectData{
    FunctionType function_type;
    void (*function)(void);
}FunctionMenuObjectData;

typedef union _MenuObjectData{
    MenuObjectParentData parent_data;
    VariableMenuObjectData variable_data;
    FunctionMenuObjectData function_data;
}MenuObjectData;

// 菜单项目数据结构定义
typedef struct _MenuObject{
    MenuObjectIndex index;
    MenuObjectIndex parent_menu_index; // 父菜单项目索引，根菜单的父菜单索引为自己
    MenuObjectType type;
    char show_name[16];
    MenuObjectData data;
}MenuObject;

// ==============================菜单变量定义==============================

extern MenuObjectIndex  current_menu_index; // 当前使用或显示的菜单项目索引
extern MenuObject       menu_object_list[]; // 菜单项目列表

void menu_refresh_screen(void);

// =========================================SMARTCAR_DEBUG_IPS=========================================
#elif defined(SMARTCAR_DEBUG_IPS_PRO)
// =========================================SMARTCAR_DEBUG_IPS_PRO=========================================
#include "zf_device_ips200pro.h"



// =========================================SMARTCAR_DEBUG_IPS_PRO=========================================
#endif

void menu_interface_init(void);

void menu_ips_print_info(void);
void menu_show_main(void);
void menu_get_key_event(void);
void menu_event_handle(void);



// 菜单功能实现
void no_screen_key_event_handle(void);
void menu_key_init(void);
// 网络调试信息发送
void network_print_info(void);

// 分科目跑
void menu_run_1(void);
void menu_run(void);
void menu_run_3(void);


#endif
