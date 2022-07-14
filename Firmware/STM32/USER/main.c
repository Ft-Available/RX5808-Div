#include "stm32f4xx.h"
#include "include.h"

int main(void)
{
  system_init();
  lvgl_init();
  while(1)
	{
    lv_task_handler();
		SysTick_Delay_ms(1);
		iwdg_feed();
	}
}


