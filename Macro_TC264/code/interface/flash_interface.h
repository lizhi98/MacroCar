#ifndef _FLASH_INTERFACE_H_
#define _FLASH_INTERFACE_H_

#include "zf_common_typedef.h"
#include "zf_driver_flash.h"

typedef struct _FlashDataSetInfo
{
    uint8  data_valid;
    uint8  page_from_num;
    uint8  page_to_num;
    uint32 data_size;
}FlashDataSetInfo;

typedef struct _FlashDataInfo
{
    uint8            set_num;
    FlashDataSetInfo data_set_info[];
}FlashDataInfo;

void  flash_interface_init(void);
uint8 flash_get_data_info(FlashDataInfo* info);
uint8 flash_write_data_info(FlashDataInfo* info);

void flash_read_data(uint8 data_set_index, uint8 *dest, uint32 size);
void flash_write_data(uint8 data_set_index, uint8 *src, uint32 size);

#endif