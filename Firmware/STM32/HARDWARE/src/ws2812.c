#include "ws2812.h"


#define WS2812_BYTE_ONE  80	// 高电平时间计数值
#define WS2812_BYTE_ZERO 40 // 低电平时间计数值

uint16_t LED_BYTE_Buffer[24];

void WS2812_init()
{

  GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);  
	
	TIM_DeInit(TIM1);
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource8,GPIO_AF_TIM1);

	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;		
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;      
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_Init(GPIOA,&GPIO_InitStructure); 	
	
	TIM_TimeBaseStructure.TIM_Period = (WS2812_BYTE_ONE+WS2812_BYTE_ZERO-1);	
	TIM_TimeBaseStructure.TIM_Prescaler = 0;		
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		
	TIM_TimeBaseStructure.TIM_ClockDivision=0; 				
	TIM_TimeBaseStructure.TIM_RepetitionCounter=0;			
	
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);			
	
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;			
	TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;	
	TIM_OCInitStructure.TIM_Pulse=0;						
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;		
	TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Set;	
	
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);  

	
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  

 
	TIM_CtrlPWMOutputs(TIM1, ENABLE); 
	TIM_Cmd(TIM1, ENABLE);
	
	DMA_DeInit(DMA2_Stream1);
	DMA_InitStructure.DMA_Channel = DMA_Channel_6;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&TIM1->CCR1);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)LED_BYTE_Buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;	
	DMA_InitStructure.DMA_BufferSize = 24;	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = 	DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;  	
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream1, &DMA_InitStructure);
	
	DMA_ITConfig(DMA2_Stream1, DMA_IT_TC, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
  
  SysTick_Delay_ms(10);
	//WS2812_color_config(30,0,0);

}

void WS2812_color_config(uint8_t r,uint8_t g,uint8_t b)
{
   uint8_t i=0;
	 for(i=0;i<8;i++)
	{
		LED_BYTE_Buffer[i]=g&0x80?WS2812_BYTE_ONE : WS2812_BYTE_ZERO;
	  g=g<<1;
	}
	 for(i=0;i<8;i++)
	{
		LED_BYTE_Buffer[8+i]=r&0x80?WS2812_BYTE_ONE : WS2812_BYTE_ZERO;
	  r=r<<1;
	}
	 for(i=0;i<8;i++)
	{
		LED_BYTE_Buffer[16+i]=b&0x80?WS2812_BYTE_ONE : WS2812_BYTE_ZERO;
	  b=b<<1;
	}
  WS2812_setcolor();
}

void WS2812_setcolor()
{
	
	DMA_Cmd(DMA2_Stream1, ENABLE);   
	TIM_DMACmd(TIM1,TIM_DMA_CC1,ENABLE);
	TIM_Cmd(TIM1, ENABLE);                       // enable Timer 1

	                      			// enable DMA channel 6 
//	while(!DMA_GetFlagStatus(DMA2_Stream1,DMA_FLAG_TCIF1)) ;    // wait until transfer complete
//	TIM_Cmd(TIM1, DISABLE);         														// disable Timer1
//	DMA_Cmd(DMA2_Stream1, DISABLE);                         		// disable DMA channel 6
//	DMA_ClearFlag(DMA2_Stream1,DMA_FLAG_TCIF1);                 // clear DMA2 Chann
}

void DMA2_Stream1_IRQHandler(void)
{

	if (DMA_GetITStatus(DMA2_Stream1, DMA_IT_TCIF1))
	{
		DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);
		TIM_Cmd(TIM1, DISABLE);         														// disable Timer1
		DMA_Cmd(DMA2_Stream1, DISABLE);                         		// disable DMA channel 6
		DMA_ClearFlag(DMA2_Stream1,DMA_FLAG_TCIF1);                 // clear DMA2 Chann    
	}

}


