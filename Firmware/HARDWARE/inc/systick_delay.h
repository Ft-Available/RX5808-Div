#ifndef __SYSTICK_DELAY_H
#define __SYSTICK_DELAY_H 	

#include <stdint.h>
#include "include.h"					//FreeRTOSÊ¹Ó
#include "lvgl.h"
#include "lv_port_disp.h"

void SysTick_Delay_Init(void);
void SysTick_Delay_us(uint32_t nus);
void SysTick_Delay_ms(uint32_t nms);
void SysTick_Delay_xms(uint32_t nms);
#endif





























