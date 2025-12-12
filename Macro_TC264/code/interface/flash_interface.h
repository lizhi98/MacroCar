#ifndef _FLASH_INTERFACE_H_
#define _FLASH_INTERFACE_H_

#include "zf_common_typedef.h"
#include "zf_driver_flash.h"

typedef struct _FlashDataSetInfo
{
    uint8  data_valid;
    uint32 data_size;
}FlashDataSetInfo;

void flash_interface_init(void);
void flash_get_data_info(FlashDataSetInfo* info);
void flash_read_data(uint8* dest, uint32 size);
void flash_write_data(const uint8* src, uint32 size);

#endif