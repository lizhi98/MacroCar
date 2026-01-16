#include "flash_interface.h"

void  flash_interface_init(void);
// uint8 flash_get_data_info(FlashDataInfo* info){
//     flash_read_page
// }
uint8 flash_write_data_info(FlashDataInfo* info);

void flash_read_data(uint8 data_set_index, uint8 *dest, uint32 size);
void flash_write_data(uint8 data_set_index, uint8 *src, uint32 size);
