#include "beep_interface.h"

void beep_init(void){
    pwm_init(BEEP_PWM_PIN, BEEP_FREQUENCY, BEEP_DUTY);
}
void beep_on(void){
    pwm_set_duty(BEEP_PWM_PIN, 0);
}
void beep_off(void){
    pwm_set_duty(BEEP_PWM_PIN, BEEP_DUTY);
}
