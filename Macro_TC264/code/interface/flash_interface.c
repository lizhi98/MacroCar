#include "flash_interface.h"

// typedef struct _FlashTestData{
//     int32  int32_data;
//     float32 float32_data;
// } FlashTestData;

// static FlashTestData flash_test_data[512];

// void  flash_interface_init(void){

// }

// // uint8 flash_write_data_info(FlashDataInfo* info){
// //     return 0;
// // }

// void flash_read_data(uint8 data_set_index, uint8 *dest, uint32 size){

// }
// void flash_write_data(uint8 data_set_index, uint8 *src, uint32 size){

// }

// void flash_test(void){
//     // uint32 test_pointer = 0;
//     // while(test_pointer < 512){
//     //     flash_test_data[test_pointer].int32_data = (int32)test_pointer;
//     //     flash_test_data[test_pointer].float32_data = test_pointer * 1.000f;
//     //     test_pointer ++;
//     // }
//     flash_read_page(0, 0, (uint32*)&flash_test_data, 500);
//     char buffer[128];
//     for(uint32 i = 0; i < 512; i++){
//         sprintf(buffer, "%lu,%d,%f\n", i, flash_test_data[i].int32_data, flash_test_data[i].float32_data);
//         // debug_print(buffer);
//         network_vofa_send_str(buffer);
//     }
//     // flash_write_page(0, 0, (const uint32*)&flash_test_data, 100);
//     while(1);
// }
