#include <stdint.h>
#include "driver/gpio.h"
#include "24cxx.h"
#include "beep.h"
#include "lcd.h"
#include "rx5808.h"
#include "rx5808_config.h"
#include "system.h"
#include "esp_timer.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"

#define LED0_PIN 21


void LED_Init()
{
 
    gpio_set_direction(LED0_PIN, GPIO_MODE_OUTPUT);		
	gpio_set_level(LED0_PIN, 0);
	
}

void USART_init(uint32_t bound)
{
//   GPIO_InitTypeDef GPIO_InitStructure;
// 	USART_InitTypeDef USART_InitStructure;
// 	NVIC_InitTypeDef NVIC_InitStructure;
// 	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 
// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);

	
// 	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);
// 	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);

	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; 
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
// 	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
// 	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
// 	GPIO_Init(GPIOA,&GPIO_InitStructure); 


// 	USART_InitStructure.USART_BaudRate = bound;
// 	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
// 	USART_InitStructure.USART_StopBits = USART_StopBits_2;
// 	USART_InitStructure.USART_Parity = USART_Parity_Even;
// 	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
// 	USART_InitStructure.USART_Mode = USART_Mode_Rx;	
// 	USART_Init(USART1, &USART_InitStructure); 
// 	USART_Cmd(USART1, ENABLE); 
// 	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

// 	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
// 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
// 	NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;		
// 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
// 	NVIC_Init(&NVIC_InitStructure);	

}

esp_timer_handle_t esp_timer_tick = 0;
void timer_periodic_cb(void *arg) {
   lv_tick_inc(1);
}

esp_timer_create_args_t periodic_arg = { .callback =
		&timer_periodic_cb, 
		.arg = NULL, 
		.name = "LVGL_TICK_TIMER" 
};


void timer_init()
{
      esp_err_t err;
      err = esp_timer_create(&periodic_arg, &esp_timer_tick);
	  ESP_ERROR_CHECK(err);
	  err = esp_timer_start_periodic(esp_timer_tick, 1 * 1000);
      ESP_ERROR_CHECK(err);
      //printf("Timer_Init! OK\n");

}

void system_init(void)
{

// 	SysTick_Delay_Init();
 	eeprom_24cxx_init();	
 	rx5808_div_setup_load();
 	LED_Init();
 	LCD_Init();
 	Beep_Init();
// 	//USART_init(115200);
    timer_init();  
    RX5808_Init();	
// 	KEY_Init();
// 	WS2812_init();
// 	iwdg_init(500);
	//while(1);
	
}


