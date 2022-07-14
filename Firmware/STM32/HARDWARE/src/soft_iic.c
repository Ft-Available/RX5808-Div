#include "soft_iic.h"



inline void soft_iic_delay(uint32_t time)
{
      SysTick_Delay_us(time);
}


void soft_iic_io_init()
{
	
#if RX5808_CONFIGT_FLASH_EEPROM==0

  
//	GPIO_InitTypeDef  GPIO_InitStructure;
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
		GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	iic_scl_out=1;
	iic_sda_out=1; 

#endif
}

inline void sda_io_set_in()
{
    GPIOA->MODER&=~(3<<(13*2));GPIOA->MODER|=0<<13*2;
}


inline void sda_io_set_out()
{
    GPIOA->MODER&=~(3<<(13*2));GPIOA->MODER|=1<<13*2;
}


void soft_iic_start(void)
{
	sda_io_set_out();    
	iic_sda_out=1;	  	  
	iic_scl_out=1;
	soft_iic_delay(5);
 	iic_sda_out=0; 
	soft_iic_delay(6);
	iic_scl_out=0;
}	

void soft_iic_stop(void)
{
	sda_io_set_out();
	iic_scl_out=0;
	iic_sda_out=0;
 	iic_scl_out=1; 
	soft_iic_delay(6); 
	iic_sda_out=1;
	soft_iic_delay(6);							   	
}


uint8_t soft_iic_wait_ack(void)
{
	uint8_t tempTime=0;
	sda_io_set_in();     
	iic_sda_out=1;
	soft_iic_delay(1);	   
	iic_scl_out=1;
	soft_iic_delay(1);	 
	while(iic_sda_in)
	{
		tempTime++;
		if(tempTime>250)
		{
			soft_iic_stop();
			return 1;
		}
	}
	iic_scl_out=0;
	return 0;  
} 

void soft_iic_send_ack(void)
{
	iic_scl_out=0;
	sda_io_set_out();
	iic_sda_out=0;
	soft_iic_delay(2);
	iic_scl_out=1;
	soft_iic_delay(5);
	iic_scl_out=0;
}
 
void soft_iic_send_no_ack(void)
{
	iic_scl_out=0;
	sda_io_set_out();
	iic_sda_out=1;
	soft_iic_delay(2);
	iic_scl_out=1;
	soft_iic_delay(5);
	iic_scl_out=0;
}	

void soft_iic_send_byte(uint8_t txd)
{                        
	uint8_t t;   
	sda_io_set_out(); 	    
	iic_scl_out=0;
	for(t=0;t<8;t++)
	{              
			if((txd&0x80)>0) 
		iic_sda_out=1;
	else
		iic_sda_out=0;
			txd<<=1; 	  
	soft_iic_delay(2);  
	iic_scl_out=1;
	soft_iic_delay(2); 
	iic_scl_out=0;	
	soft_iic_delay(2);
	}	 
} 

uint8_t soft_iic_read_byte(uint8_t ack)
{
	uint8_t i,receive=0;
	sda_io_set_in();
    for(i=0;i<8;i++ )
	{
        iic_scl_out=0; 
        soft_iic_delay(2);
		iic_scl_out=1;
        receive<<=1;
        if(iic_sda_in)receive++;   
		soft_iic_delay(1); 
    }					 
    if (!ack)
        soft_iic_send_no_ack();
    else
        soft_iic_send_ack(); 
    return receive;
}

