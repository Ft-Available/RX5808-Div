#include "beep.h"

#define Passive_Buzzer  0


volatile bool beep_en=false;  //off

void Beep_Init()
{
	#if Passive_Buzzer == 0
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_8);	
	if(beep_en==true)
	{
	GPIO_SetBits(GPIOB,GPIO_Pin_8);
	SysTick_Delay_ms(100);
	GPIO_ResetBits(GPIOB,GPIO_Pin_8);	
	}
	#else	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE); 
	
	TIM_DeInit(TIM4);
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource8,GPIO_AF_TIM4);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;		
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;      
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_Init(GPIOB,&GPIO_InitStructure); 	
		
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);	
	TIM_TimeBaseStructure.TIM_Period = 99;			
	TIM_TimeBaseStructure.TIM_Prescaler = 249;		
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 					
	TIM_TimeBaseStructure.TIM_RepetitionCounter=0x0000;				
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);

  TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;				
	TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;	
	TIM_OCInitStructure.TIM_Pulse=50;							
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_Low;		
	//TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Set;	
	TIM_OC3Init(TIM4, &TIM_OCInitStructure);  	
	
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable); 
	TIM_ARRPreloadConfig(TIM4,ENABLE);
	 TIM_Cmd(TIM4, DISABLE); 
	 TIM_SetCompare3(TIM4,0);
	if(beep_en==true)
	{
	TIM_Cmd(TIM4, ENABLE); 
	TIM_SetCompare3(TIM4,50);
	SysTick_Delay_ms(100);
	TIM_SetCompare3(TIM4,0);
	TIM_Cmd(TIM4, DISABLE); 
	
	}
	#endif
  
}

void beep_set_enable_disable(uint8_t en)
{
	if(en)
   beep_en=true;
  else
	  beep_en=false;
}


uint8_t beep_get_status()
{
   return beep_en;
}

void beep_on_off(uint8_t on_off)
{
	//if(beep_en==false)
	//	return ;
  #if Passive_Buzzer == 0
	if(on_off&&beep_en)
  GPIO_SetBits(GPIOB,GPIO_Pin_8);
  else
	GPIO_ResetBits(GPIOB,GPIO_Pin_8);
	#else
	if(on_off&&beep_en) 
	{TIM_Cmd(TIM4, ENABLE); 
	 TIM_SetCompare3(TIM4,50);}	
  else
	{TIM_SetCompare3(TIM4,0);
		TIM_Cmd(TIM4, DISABLE); }
	#endif
}


void beep_set_tone(uint16_t tone)
{
	#if Passive_Buzzer == 1
	uint32_t psc=480000/tone;
  TIM4->PSC=(uint16_t)psc-1;
	#endif
}



