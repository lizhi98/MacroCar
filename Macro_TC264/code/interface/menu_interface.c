#include "menu_interface.h"



extern uint32 image_to_image_time;
extern uint32 image_process_time;
//extern volatile uint8 image_process_finish_flag; // 图像处理标志位，0表示CPU1正在处理图像，1表示CPU1处理完图像了，CPU0可以访问图像数据了
//extern uint8 image_process_flag; // 为1时图像开始处理

KeyCmd current_key_cmd = KEY_NO_CMD; // 当前按键命令

#ifdef SMARTCAR_DEBUG_IPS

MenuObjectIndex current_menu_index = MAIN; // 当前使用或显示的菜单项目索引


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
#elif defined(SMARTCAR_DEBUG_IPS_PRO)

uint16 info_page_id; // 信息页面ID
uint16 para_page_id; // 参数页面ID
uint16 run_page_id;  // 启动页面ID
uint16 image_id; // 图像显示对象ID
uint16 info_table_id; // 信息显示表ID
uint16 para_table_id; // 参数调节表ID
uint16 run_table_id; // 启动功能表ID

#endif



void menu_ips_print_info(void){
#ifdef SMARTCAR_DEBUG_IPS
    if(image_process_finish_flag){ // 确保CPU1已经处理完图像了，CPU0可以访问图像数据了
        // ips200_show_binary_image_with_line(0, 0, mt9v03x_copy_image[0], MT9V03X_W, MT9V03X_H); // 显示图像并带辅助线
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
    sprintf(info_buffer, "Index:%2u Con:%1u", feature_T, condition_T);
    ips200_show_string(0, 230, info_buffer);
    sprintf(info_buffer, "L:%2u R:%2u,H %2u",result_feature.left, result_feature.right, result_feature.height);
    ips200_show_string(0, 250, info_buffer);
    sprintf(info_buffer, "TH:%3u I_T:%2lu L:%3u R:%3u", img_threshold, image_process_time, left_lost_times, right_lost_times);
    ips200_show_string(0, 270, info_buffer);
#endif
}

void menu_show_main(void){
#ifdef SMARTCAR_DEBUG_IPS
    current_menu_index = MAIN; // 切换到主菜单
    menu_refresh_screen(); // 刷新菜单显示
#endif
}

void menu_event_handle(void){
#ifdef SMARTCAR_DEBUG_IPS
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
#endif
}

void menu_refresh_screen(void){
#ifdef SMARTCAR_DEBUG_IPS
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
#endif
}



void menu_interface_init(){
#ifdef SMARTCAR_DEBUG_IPS
    ips200_init(IPS200_TYPE_SPI);   // 初始化IPS200显示屏
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
#elif defined(SMARTCAR_DEBUG_IPS_PRO)
    // 初始化屏幕
    info_page_id = ips200pro_init("信息", IPS200PRO_TITLE_BOTTOM, 30); // IPS200 Pro屏幕初始化
    para_page_id = ips200pro_page_create("参数");
    run_page_id = ips200pro_page_create("启动");
    ips200pro_page_switch(info_page_id, PAGE_ANIM_ON); // 切换到信息页面
    // 初始化信息页面
    // 创建容器
    uint16 info_container_id = ips200pro_container_create(0,0,240, 320); // 创建一个全屏的容器，后续信息显示对象都放在这个容器里
    ips200pro_set_parent(info_container_id, info_page_id); // 把容器设置为信息页面的子对象
    image_id = ips200pro_image_create(0, 0, MT9V03X_W, MT9V03X_H); // 创建图像显示对象，宽高和摄像头图像宽高一致
    ips200pro_set_parent(image_id, info_container_id); // 把图像显示对象设置为信息页面的子对象
    info_table_id = ips200pro_table_create(20, 20, 8, 2); 
    ips200pro_set_parent(info_table_id, info_container_id); // 把信息显示表设置为信息页面的子对象
#endif
}

// 功能菜单功能实现函数
void menu_run_1(void){
    // ips200_show_string(0, 130, "RUN_1 function");
}
uint32 car_start_time = 0;
void menu_run(void){
    run_control_protect_trigger_flag = 0;
    motor_fun_soft_start();         // 负压风扇软启动
    motor_traveling_power_flag = 1; // 使能行进电机PWM输出
    motor_traveling_pid_run_flag = 1; // 使能行进电机速度环运行
    run_control_protect_trigger_flag = 0;
}

void menu_run_3(void){
    // ips200_show_string(0, 130, "RUN_3 function");
}

void menu_key_init(void){
    key_init(KEY_PIT_TIME);   // 按键初始化 5ms扫描一次 // 改这个需要改所处中断时间
    pit_ms_init(MENU_KEY_PIT_INDEX, KEY_PIT_TIME); // 按键扫描PIT初始化
}

void network_print_info(void){
    static char info_buffer[128];
    // sprintf(info_buffer, "%d,%d,%d,%d,%d,%f,%f,%f,%d,%d,%d,%lu,%u,%d,%u,%d,%d,%u\0",
    //     motor_forward_speed,
    //     motor_left_speed,motor_right_speed,
    //     motor_left_pwm, motor_right_pwm,
    //     attitude.yaw, gyro_current_data.gyro_z,
    //     battery_voltage,
    //     motor_traveling_left_target_speed,motor_traveling_right_target_speed,
    //     error_image,image_process_time,condition_T,feature_T,img_threshold,motion_image_steering_speed,motor_steering_speed,detect_feature_row
    // );
    sprintf(info_buffer, "%d,%d,%d,%d,%d,%f,%f,%f,%d,%d,%d,%lu,%u,%d,%u,%d,%d,%u,%u,%d,%u\0",
        motor_forward_speed,
        motor_left_speed,motor_right_speed,
        motor_left_pwm, motor_right_pwm,
        attitude.yaw, gyro_current_data.gyro_z,
        battery_voltage,
        motor_traveling_left_target_speed,motor_traveling_right_target_speed,
        error_image,image_to_image_time,condition_T,feature_T,img_threshold,
        motion_image_steering_speed,motor_steering_speed,detect_feature_row,motor_fun_open_percent,back_condition_row,car_run_state
    );
    network_vofa_send_str(info_buffer);
}

void no_screen_key_event_handle(void){
    if(car_run_state != INIT_WAIT){
        return; // 只有在等待开始状态下才响应按键事件
    }
    // if((key_get_state(KEY_1) == KEY_SHORT_PRESS) || (key_get_state(KEY_1) == KEY_LONG_PRESS)){
    //     menu_run_1(); // 直接运行科目1
    //     key_clear_state(KEY_1);
    // }
    // if((key_get_state(KEY_2) == KEY_SHORT_PRESS) || (key_get_state(KEY_2) == KEY_LONG_PRESS)){
    //     // menu_run_2(); // 直接运行科目2
    //     key_clear_state(KEY_2);
    // }
    // if((key_get_state(KEY_3) == KEY_SHORT_PRESS) || (key_get_state(KEY_3) == KEY_LONG_PRESS)){
    //     menu_run_3(); // 直接运行科目3
    //     key_clear_state(KEY_3);
    // }
    if((key_get_state(KEY_4) == KEY_SHORT_PRESS) || (key_get_state(KEY_4) == KEY_LONG_PRESS)){
        car_run_state = WAIT_START; // 切换到启动状态
        key_clear_state(KEY_4);
    }
}

void menu_get_key_event(void){
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
