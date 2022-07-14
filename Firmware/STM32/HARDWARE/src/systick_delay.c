#include "systick_delay.h"

#define SysTick_CLK_FAST 0

void SysTick_Handler(void)
{	
	  lv_tick_inc(1);

}
void SysTick_Delay_Init()
{
	#if SysTick_CLK_FAST
 	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	#else
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	#endif
;							
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;               
	#if SysTick_CLK_FAST
 		SysTick->LOAD=SystemCoreClock/1000;                       //1ms
	#else
		SysTick->LOAD=SystemCoreClock/8/1000;	           //1ms
	#endif
				
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk; 
}								    

 								   
void SysTick_Delay_us(uint32_t us)
{		
	uint32_t ticks;
	uint32_t told,tnow,tcnt=0;
	uint32_t reload=SysTick->LOAD;			   	 
	ticks=us*(reload/1000); 					
	told=SysTick->VAL;        			
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			
		}  
	};				
							    
}  

void SysTick_Delay_ms(uint32_t ms)
{	
	SysTick_Delay_us((uint32_t)(ms*1000));	
}


void delay_xms(uint32_t ms)
{
	uint32_t i;
	for(i=0;i<ms;i++) SysTick_Delay_us(1000);
	
}
















