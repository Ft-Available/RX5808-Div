#ifndef __BEEP_H
#define __BEEP_H

#include <stdint.h>
#include "hardware/hwvers.h"
//#define BEEP_5V PBout(8)

void PWM_Enable(void);
void PWM_Disable(void);
void Beep_Init(void);
void beep_on_off(uint8_t on_off);
void beep_set_enable_disable(uint8_t en);
uint8_t beep_get_status(void);
void beep_set_tone(uint16_t tone);

#endif

