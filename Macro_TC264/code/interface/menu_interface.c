#include "menu_interface.h"
extern uint32 image_to_image_time;
extern uint32 image_process_time;

MenuObjectIndex current_menu_index = MAIN; // 当前使用或显示的菜单项目索引

KeyCmd current_key_cmd = KEY_NO_CMD; // 当前按键命令

// 菜单项目列表
MenuObject menu_object_list[] = {
{
    MAIN, MAIN, MENU_OBJECT_PARENT, "MAIN",       
    {.parent_data = {.child_menu_max_index = 6, .selected_child_menu_index = 0, .child_menu_index_list = {INFO,RUN_1,RUN_2,RUN_3,Mod_S,Mod_T,Mod_PID} } }
},
{
    INFO, MAIN, MENU_OBJECT_FUNCTION, "INFO",     
    {.function_data = {FUNCTION_TYPE_RUN_CONTINUOUSLY, menu_ips_print_info} }
},
{
    RUN_1, MAIN, MENU_OBJECT_FUNCTION, "RUN_1",   
    {.function_data = {FUNCTION_TYPE_RUN_ONCE, menu_run_1} }
},
{
    RUN_2, MAIN, MENU_OBJECT_FUNCTION, "RUN_2",   
    {.function_data = {FUNCTION_TYPE_RUN_ONCE, menu_run_2} }
},
{
    RUN_3, MAIN, MENU_OBJECT_FUNCTION, "RUN_3",   
    {.function_data = {FUNCTION_TYPE_RUN_ONCE, menu_run_3} }
},
{
    Mod_S, MAIN, MENU_OBJECT_PARENT, "Mod_S",     
    {.parent_data = {.child_menu_max_index = 0, .selected_child_menu_index = 0, .child_menu_index_list = {MAIN} } }
},
{
    Mod_T, MAIN, MENU_OBJECT_PARENT, "Mod_T",     
    {.parent_data = {.child_menu_max_index = 0, .selected_child_menu_index = 0, .child_menu_index_list = {MAIN} } }
},
{
    Mod_PID, MAIN, MENU_OBJECT_PARENT,"Mod_PID",  
    {.parent_data = {.child_menu_max_index = 0, .selected_child_menu_index = 0, .child_menu_index_list = {MAIN} } }
}
};

// 功能菜单功能实现函数
void menu_run_1(void){
    // ips200_show_string(0, 130, "RUN_1 function");
}

void menu_run_2(void){
    motor_interface_power_flag = 1; // 使能电机PWM输出
    motor_fun_soft_start(); // 负压风扇软启动
    system_delay_ms(500);
    motion_control_pit_run_flag = 1;
}

void menu_run_3(void){
    // ips200_show_string(0, 130, "RUN_3 function");
}

extern volatile uint8 image_process_flag; // 图像处理标志位，0表示CPU1正在处理图像，1表示CPU1处理完图像了，CPU0可以访问图像数据了

void menu_ips_print_info(void){
    if(image_process_flag){ // 确保CPU1已经处理完图像了，CPU0可以访问图像数据了
        ips200_show_binary_image_with_line(0, 0, mt9v03x_copy_image[0], MT9V03X_W, MT9V03X_H); // 显示图像并带辅助线
        // ips200_displayimage03x(mt9v03x_copy_image[0], MT9V03X_W, MT9V03X_H);
    }
    static char info_buffer[64];
    sprintf(info_buffer, "L_S:%+4d   R_S:%+4d ", motor_left_speed, motor_right_speed);
    ips200_show_string(0, 130, info_buffer);
    sprintf(info_buffer, "L_P:%+5d  R_P:%+5d", motor_left_current_pwm_duty, motor_right_current_pwm_duty);
    ips200_show_string(0, 150, info_buffer);
    sprintf(info_buffer, "B_A:%+4d", battery_protection_adc_value);
    ips200_show_string(0, 170, info_buffer);
    sprintf(info_buffer, "I_E:%+5d", error_image);
    ips200_show_string(0, 190, info_buffer);
    sprintf(info_buffer, "GZ:%+6.2f YAW:%+6.2f", gyro_current_data.gyro_z, gyro_current_data.angle_z);
    ips200_show_string(0, 210, info_buffer);
    sprintf(info_buffer, "Index:%2u Con:%1u", T_index, condition_T);
    ips200_show_string(0, 230, info_buffer);
    sprintf(info_buffer, "L:%2u R:%2u,H %2u",result_feature.left, result_feature.right, result_feature.height);
    ips200_show_string(0, 250, info_buffer);
    sprintf(info_buffer, "TH:%3u I_T:%2lu L:%3u R:%3u", img_threshold, image_to_image_time, left_lost_times, right_lost_times);
    ips200_show_string(0, 270, info_buffer);
}

void menu_key_init(void){
    key_init(KEY_PIT_TIME);   // 按键初始化 5ms扫描一次 // 改这个需要改所处中断时间
    pit_ms_init(MENU_KEY_PIT_INDEX, KEY_PIT_TIME); // 按键扫描PIT初始化
}

void menu_key_event_handle(void){
    if((key_get_state(KEY_1) == KEY_SHORT_PRESS) || (key_get_state(KEY_1) == KEY_LONG_PRESS)){
        current_key_cmd = KEY_BACK;
        key_clear_state(KEY_1);
    }
    if((key_get_state(KEY_2) == KEY_SHORT_PRESS) || (key_get_state(KEY_2) == KEY_LONG_PRESS)){
        current_key_cmd = KEY_UP;
        key_clear_state(KEY_2);
    }
    if((key_get_state(KEY_3) == KEY_SHORT_PRESS) || (key_get_state(KEY_3) == KEY_LONG_PRESS)){
        current_key_cmd = KEY_DOWN;
        key_clear_state(KEY_3);
    }
    if((key_get_state(KEY_4) == KEY_SHORT_PRESS) || (key_get_state(KEY_4) == KEY_LONG_PRESS)){
        current_key_cmd = KEY_ENTER;
        key_clear_state(KEY_4);
    }
}
void no_screen_key_event_handle(void){
    if((key_get_state(KEY_1) == KEY_SHORT_PRESS) || (key_get_state(KEY_1) == KEY_LONG_PRESS)){
        menu_run_1(); // 直接运行科目1
        key_clear_state(KEY_1);
    }
    if((key_get_state(KEY_2) == KEY_SHORT_PRESS) || (key_get_state(KEY_2) == KEY_LONG_PRESS)){
        menu_run_2(); // 直接运行科目2
        key_clear_state(KEY_2);
    }
    if((key_get_state(KEY_3) == KEY_SHORT_PRESS) || (key_get_state(KEY_3) == KEY_LONG_PRESS)){
        menu_run_3(); // 直接运行科目3
        key_clear_state(KEY_3);
    }
    if((key_get_state(KEY_4) == KEY_SHORT_PRESS) || (key_get_state(KEY_4) == KEY_LONG_PRESS)){
        // 保留这个按键作为紧急停止按键，按下就停止电机运动
        motor_interface_power_flag = 0; // 关闭所有电机PWM输出
        key_clear_state(KEY_4);
    }
}


void menu_network_print_info(void){
    static char info_buffer[120];
    sprintf(info_buffer, "%d,%d,%d,%d,%f,%f,%u,%u,%d,%ld,%u,%u\0",
        motor_left_speed,motor_right_speed,
        motor_left_pwm, motor_right_pwm,
        attitude.yaw, gyro_current_data.gyro_z,
        left_line_list[45],right_line_list[45],error_image,image_to_image_time,condition_T,T_index
    );
    network_vofa_send_str(info_buffer);
}


void menu_show_main(void){
    current_menu_index = MAIN; // 切换到主菜单
    menu_refresh_screen(); // 刷新菜单显示
}

void menu_event_handle(void){
    uint8 menu_need_refresh_flag = 0; // 菜单是否需要刷新显示的标志
    // 首先判断当前菜单项目类型，根据类型执行不同的操作
    switch (menu_object_list[current_menu_index].type)
    {
    case MENU_OBJECT_PARENT:
        // 当前菜单项目是一个父菜单
        // 根据按键判断执行操作
        switch (current_key_cmd)
        {
        case KEY_BACK:
            current_menu_index = menu_object_list[current_menu_index].parent_menu_index; // 切换到父菜单
            menu_need_refresh_flag = 1; // 刷新菜单显示
            break;
        case KEY_UP:
            if(menu_object_list[current_menu_index].data.parent_data.selected_child_menu_index > 0){
                menu_object_list[current_menu_index].data.parent_data.selected_child_menu_index--; // 选择上一个子菜单
                menu_need_refresh_flag = 1; // 刷新菜单显示
            }else if(menu_object_list[current_menu_index].data.parent_data.selected_child_menu_index == 0){
                menu_object_list[current_menu_index].data.parent_data.selected_child_menu_index = menu_object_list[current_menu_index].data.parent_data.child_menu_max_index; // 循环选择到最后一个子菜单
                menu_need_refresh_flag = 1; // 刷新菜单显示
            }
            break;
        case KEY_DOWN:
            if(menu_object_list[current_menu_index].data.parent_data.selected_child_menu_index < menu_object_list[current_menu_index].data.parent_data.child_menu_max_index){
                menu_object_list[current_menu_index].data.parent_data.selected_child_menu_index++; // 选择下一个子菜单
                menu_need_refresh_flag = 1; // 刷新菜单显示
            }else if(menu_object_list[current_menu_index].data.parent_data.selected_child_menu_index == menu_object_list[current_menu_index].data.parent_data.child_menu_max_index){
                menu_object_list[current_menu_index].data.parent_data.selected_child_menu_index = 0; // 循环选择到第一个子菜单
                menu_need_refresh_flag = 1; // 刷新菜单显示
            }
            break;
        case KEY_ENTER:
            current_menu_index = menu_object_list[current_menu_index].data.parent_data.child_menu_index_list[menu_object_list[current_menu_index].data.parent_data.selected_child_menu_index]; // 切换到选中的子菜单
            menu_need_refresh_flag = 1; // 刷新菜单显示
            break;
        case KEY_NO_CMD:
            // 不执行操作
            break;
        default:
            zf_assert(0); // 不应该出现其他按键命令
            break;
        }
        break;
    case MENU_OBJECT_VARIABLE:
        // 当前菜单是一个变量修改菜单
        // 根据按键判断执行操作
        switch (current_key_cmd)
        {
        case KEY_BACK:
            current_menu_index = menu_object_list[current_menu_index].parent_menu_index; // 切换到父菜单
            menu_need_refresh_flag = 1; // 刷新菜单显示
            break;
        case KEY_UP:
        case KEY_DOWN:
            {
                // 增加变量值
                // 先判断是增加还是减少变量值
                int     variable_increase_step_int      = menu_object_list[current_menu_index].data.variable_data.increase_step_int;
                float   variable_increase_step_float    = menu_object_list[current_menu_index].data.variable_data.increase_step_float;
                if(current_key_cmd == KEY_DOWN){
                    variable_increase_step_int      = -variable_increase_step_int; // 减少变量值，把增加步长取负数
                    variable_increase_step_float    = -variable_increase_step_float; // 减少变量值，把增加步长取负数
                }
                // 把变量地址转换为对应类型的指针，然后根据变量类型增加变量值
                VariableType current_variable_type = menu_object_list[current_menu_index].data.variable_data.variable_type;
                switch (current_variable_type)
                {
                    case VARIABLE_TYPE_uint8:
                        *(uint8*)(menu_object_list[current_menu_index].data.variable_data.variable_addr) += (uint8)(variable_increase_step_int);
                        break;
                    case VARIABLE_TYPE_uint16:
                        *(uint16*)(menu_object_list[current_menu_index].data.variable_data.variable_addr) += (uint16)(variable_increase_step_int);
                        break;
                    case VARIABLE_TYPE_uint32:
                        *(uint32*)(menu_object_list[current_menu_index].data.variable_data.variable_addr) += (uint32)(variable_increase_step_int);
                        break;
                    case VARIABLE_TYPE_int8:
                        *(int8*)(menu_object_list[current_menu_index].data.variable_data.variable_addr) += (int8)(variable_increase_step_int);
                        break;
                    case VARIABLE_TYPE_int16:
                        *(int16*)(menu_object_list[current_menu_index].data.variable_data.variable_addr) += (int16)(variable_increase_step_int);
                        break;
                    case VARIABLE_TYPE_int32:
                        *(int32*)(menu_object_list[current_menu_index].data.variable_data.variable_addr) += (int32)(variable_increase_step_int);
                        break;
                    case VARIABLE_TYPE_float:
                        *(float*)(menu_object_list[current_menu_index].data.variable_data.variable_addr) += (float)(variable_increase_step_float);
                        break;
                    case VARIABLE_TYPE_double:
                        *(double*)(menu_object_list[current_menu_index].data.variable_data.variable_addr) += (double)(variable_increase_step_float);
                        break;
                }
                menu_need_refresh_flag = 1; // 刷新菜单显示
            }
            break;
        case KEY_ENTER:
        case KEY_NO_CMD:
            // 不执行操作
            break;
        default:
            zf_assert(0); // 不应该出现其他按键命令
            break;
        }
        break;
    case MENU_OBJECT_FUNCTION:
        // 当前菜单是一个功能执行菜单
        // 根据按键判断执行操作
        // 只需要判断是否返回
        if(current_key_cmd == KEY_ENTER){
            current_menu_index = menu_object_list[current_menu_index].parent_menu_index; // 切换到父菜单
            menu_need_refresh_flag = 1; // 刷新菜单显示
        }else{
            // 执行功能函数
            void (*function)(void) = menu_object_list[current_menu_index].data.function_data.function;
            FunctionType function_type = menu_object_list[current_menu_index].data.function_data.function_type;
            // 首先运行函数一次
            function();
            // 如果是只运行一次的函数，直接返回上一级菜单
            if(function_type == FUNCTION_TYPE_RUN_ONCE){
                current_menu_index = menu_object_list[current_menu_index].parent_menu_index; // 切换到父菜单
                menu_need_refresh_flag = 1; // 刷新菜单显示
            }else{
                // 如果是持续运行的函数，不用处理，等下一次进入这个菜单时再运行一次函数
                menu_need_refresh_flag = 0; // 不需要刷新菜单显示，因为菜单没有切换
            }
        }
        break;
    default:
        zf_assert(0); // 不应该出现其他菜单类型
        break;
    }
    // 如果需要刷新菜单显示，调用显示函数
    if(menu_need_refresh_flag){
        menu_refresh_screen(); // 刷新菜单显示
    }
    current_key_cmd = KEY_NO_CMD; // 处理完按键命令后，重置按键命令为无命令
}

void menu_refresh_screen(void){
    // 刷新显示
    // 首先清屏
    ips200_clear();
    // 首先判断当前菜单项目类型，根据类型执行不同的显示操作
    switch (menu_object_list[current_menu_index].type)
    {
    case MENU_OBJECT_PARENT:
        // 当前菜单项目是一个父菜单
        // 把子菜单列表显示出来，并且把选中的子菜单高亮显示
        // 首先显示菜单标题
        ips200_show_string(30, 0, menu_object_list[current_menu_index].show_name);
        // 显示子菜单列表
        for(int i=0; i<=menu_object_list[current_menu_index].data.parent_data.child_menu_max_index; i++){
            uint16 y = 30 + i * 20; // 子菜单显示的Y坐标，间隔20像素
            char * child_menu_show_name = menu_object_list[menu_object_list[current_menu_index].data.parent_data.child_menu_index_list[i]].show_name;
            // 显示子菜单名字
            ips200_show_string(20, y, child_menu_show_name);
            if(i == menu_object_list[current_menu_index].data.parent_data.selected_child_menu_index){
                // 选中的子菜单高亮显示
                ips200_show_string(0, y, ">>"); // 在选中的子菜单前显示">>"符号表示高亮
            }
        }
        break;
    case MENU_OBJECT_VARIABLE:
        // 当前菜单是一个变量修改菜单
        // 把变量名称和当前变量值显示出来
        // 首先显示变量名称，即菜单标题
        ips200_show_string(0, 0, menu_object_list[current_menu_index].show_name);
        // 显示当前变量值
        char variable_value_buffer[16];
        VariableType current_variable_type = menu_object_list[current_menu_index].data.variable_data.variable_type;
        switch (current_variable_type)
        {
            case VARIABLE_TYPE_uint8:
                sprintf(variable_value_buffer, "%+4u", *(uint8*)(menu_object_list[current_menu_index].data.variable_data.variable_addr));
                break;
            case VARIABLE_TYPE_uint16:
                sprintf(variable_value_buffer, "%+6u", *(uint16*)(menu_object_list[current_menu_index].data.variable_data.variable_addr));
                break;
            case VARIABLE_TYPE_uint32:
                sprintf(variable_value_buffer, "%+10lu", *(uint32*)(menu_object_list[current_menu_index].data.variable_data.variable_addr));
                break;
            case VARIABLE_TYPE_int8:
                sprintf(variable_value_buffer, "%+4d", *(int8*)(menu_object_list[current_menu_index].data.variable_data.variable_addr));
                break;
            case VARIABLE_TYPE_int16:
                sprintf(variable_value_buffer, "%+6d", *(int16*)(menu_object_list[current_menu_index].data.variable_data.variable_addr));
                break;
            case VARIABLE_TYPE_int32:
                sprintf(variable_value_buffer, "%+10d", *(int32*)(menu_object_list[current_menu_index].data.variable_data.variable_addr));
                break;
            case VARIABLE_TYPE_float:
                sprintf(variable_value_buffer, "%+10.2f", *(float*)(menu_object_list[current_menu_index].data.variable_data.variable_addr));
                break;
            case VARIABLE_TYPE_double:
                sprintf(variable_value_buffer, "%+10.2f", *(double*)(menu_object_list[current_menu_index].data.variable_data.variable_addr));
                break;
        }
        ips200_show_string(0, 20, variable_value_buffer);
        break;
    case MENU_OBJECT_FUNCTION:
        // 当前菜单是一个功能执行菜单
        // 把功能名称显示出来
        // ips200_show_string(0, 0, menu_object_list[current_menu_index].show_name);
        break;
    default:
        zf_assert(0); // 不应该出现其他菜单类型
        break;
    }
}



void menu_interface_init(){
    // 对菜单项目进行排序，按照菜单项目索引从小到大排序，确保菜单项目索引的顺序和菜单项目在列表中的顺序一致，这样方便通过菜单项目索引访问菜单项目数据
    for(int i=0; i < sizeof(menu_object_list) / sizeof(MenuObject); i++){
        for(int j=0; j < sizeof(menu_object_list) / sizeof(MenuObject)-1-i; j++){
            if(menu_object_list[j].index > menu_object_list[j+1].index){
                // 交换菜单项目
                MenuObject temp = menu_object_list[j];
                menu_object_list[j] = menu_object_list[j+1];
                menu_object_list[j+1] = temp;
            }
        }
    }
}
