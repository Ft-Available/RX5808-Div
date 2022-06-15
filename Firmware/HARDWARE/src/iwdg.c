#include "iwdg.h"


void iwdg_init(uint32_t ms)
{
	IWDG->KR = IWDG_WriteAccess_Enable;
	
	IWDG->PR = IWDG_Prescaler_32;           //32*1000/32=1000hz
  
	IWDG->RLR = ms;

	IWDG->KR = KR_KEY_RELOAD;
	
	IWDG->KR = KR_KEY_ENABLE;
}

void iwdg_feed(void)
{
	IWDG->KR = KR_KEY_RELOAD;
}
