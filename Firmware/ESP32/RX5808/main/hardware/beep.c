#include "lcd.h"
#include "beep.h"
#include "SPI.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#define Beep_Pin_Num  22

uint8_t beep_en=1;  //off

void Beep_Init()
{
    gpio_set_direction(Beep_Pin_Num, GPIO_MODE_OUTPUT);

	if(beep_en==1)
	{
    gpio_set_level(Beep_Pin_Num, 1);
	vTaskDelay(100 / portTICK_PERIOD_MS);
 	gpio_set_level(Beep_Pin_Num, 0);
 	}
}

void beep_set_enable_disable(uint8_t en)
{
	if(en)
   beep_en=1;
  else
	  beep_en=0;
}


uint8_t beep_get_status()
{
   return beep_en;
}

void beep_on_off(uint8_t on_off)
{
	//if(beep_en==0)
	//	return ;
	if(on_off&&beep_en)
   gpio_set_level(Beep_Pin_Num, 1);
  else
	gpio_set_level(Beep_Pin_Num, 0);
}


void beep_set_tone(uint16_t tone)
{
// 	#if Passive_Buzzer == 1
// 	uint32_t psc=480000/tone;
//   TIM4->PSC=(uint16_t)psc-1;
// 	#endif
}


