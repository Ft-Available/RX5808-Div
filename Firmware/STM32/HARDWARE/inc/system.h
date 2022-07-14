#ifndef __MODULE_H
#define __MODULE_H

#include "stm32f4xx.h" 


#define LED0 PBout(9)


void LED_Init(void);
void KEY_Init(void);
void USART_init(u32 bound);
void system_init(void);

#endif


