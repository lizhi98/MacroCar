#ifndef __BEEP_INTERFACE_H__
#define __BEEP_INTERFACE_H__

#include "zf_driver_pwm.h"

#define BEEP_PWM_PIN      ATOM3_CH0_P33_10
#define BEEP_FREQUENCY    740     // 蜂鸣器频率 740Hz
#define BEEP_DUTY         5000    // 蜂鸣器占空比

void beep_init(void);
void beep_on(void);
void beep_off(void);

#endif