#ifndef _BATTERY_PROTECTION_H_
#define _BATTERY_PROTECTION_H_

#include "zf_driver_adc.h"
#include "beep_interface.h"

#define BATTERY_PROTECTION_ADC_CHANNEL      ADC0_CH11_A11
#define BATTERY_PROTECTION_ADC_RESOLUTION   ADC_12BIT
#define BATTERY_PROTECTION_ADC_LOW_VALUE    12.0     // 电池电压低于此值时报警

extern uint16  battery_protection_adc_value;    // 当前电池电压ADC值
extern float battery_voltage;                        // 当前电池电压值

void    battery_protection_init(void);
uint8   battery_protection_check(void);

#endif