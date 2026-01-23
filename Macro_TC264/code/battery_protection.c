#include "battery_protection.h"

uint16  battery_protection_adc_value = 0;    // 当前电池电压ADC值

void    battery_protection_init(void){
    adc_init(BATTERY_PROTECTION_ADC_CHANNEL, BATTERY_PROTECTION_ADC_RESOLUTION);
}

/*  检查电池电压是否过低
    返回值：
        0：电池电压正常
        1：电池电压过低
*/
uint8   battery_protection_check(void){
    battery_protection_adc_value = adc_mean_filter_convert(BATTERY_PROTECTION_ADC_CHANNEL, 5);

    return (battery_protection_adc_value <= BATTERY_PROTECTION_ADC_LOW_VALUE);
}
