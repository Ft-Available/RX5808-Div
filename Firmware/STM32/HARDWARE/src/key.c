#include "KEY.h"


void KEY_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);																
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;                                          
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;																			
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;																				
  GPIO_Init(GPIOB, &GPIO_InitStructure);																							
}




