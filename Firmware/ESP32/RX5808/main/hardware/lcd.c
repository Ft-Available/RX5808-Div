#include "lcd.h"
#include "SPI.h"
#include "freertos/task.h"
#include <string.h>
#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

#define st7735_lcd_backlight_min 10
#define st7735_lcd_backlight_max 100

volatile uint8_t st7735_lcd_backlight=100;


inline void LCD_Writ_Bus(uint8_t dat) 
{	

    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&dat;               //The data is the cmd itself
    //t.user=(void*)1;                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(my_spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.

}



void LCD_WR_DATA8(uint8_t da) 
{ 
   LCD_Writ_Bus(da);
}  
void LCD_WR_DATA(uint16_t dat)
{
    LCD_Writ_Bus(dat>>8);
	LCD_Writ_Bus(dat);

}   

void LCD_WR_REG(uint8_t da)   
{ 
	gpio_set_level(PIN_NUM_DC, 0);
	LCD_Writ_Bus(da);
	gpio_set_level(PIN_NUM_DC, 1);
}


void Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{ 

if(USE_HORIZONTAL==0)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1+26);
		LCD_WR_DATA(x2+26);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1+1);
		LCD_WR_DATA(y2+1);
		LCD_WR_REG(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==1)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1+26);
		LCD_WR_DATA(x2+26);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1+1);
		LCD_WR_DATA(y2+1);
		LCD_WR_REG(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==2)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1+1);
		LCD_WR_DATA(x2+1);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1+26);
		LCD_WR_DATA(y2+26);
		LCD_WR_REG(0x2c);//储存器写
	}
	else
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1+1);
		LCD_WR_DATA(x2+1);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1+26);
		LCD_WR_DATA(y2+26);
		LCD_WR_REG(0x2c);//储存器写
	}
	
}


//#define GPIO_PWM0A_OUT 23   //Set GPIO 19 as PWM0A
#define GPIO_PWM0B_OUT 26   //Set GPIO 18 as PWM0B

void pwm_init()
{
	mcpwm_pin_config_t pin_config = {
		.mcpwm0a_out_num = -1,
		.mcpwm0b_out_num = GPIO_PWM0B_OUT
	};
	mcpwm_set_pin(MCPWM_UNIT_0, &pin_config);
	mcpwm_config_t pwm_config;
	pwm_config.frequency = 10000;    //frequency = 1000Hz
	pwm_config.cmpr_a = 0.0;       //duty cycle of PWMxA = 60.0%
	pwm_config.cmpr_b = 0;       //duty cycle of PWMxb = 50.0%
	pwm_config.counter_mode = MCPWM_UP_COUNTER;
	pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
	mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);   //Configure PWM0A & PWM0B with above settings

  //mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, 0, 100);
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, 1, 0);

}
   

void LCD_Init(void)
{
  
    spi_init();
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    //gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);
	gpio_set_level(PIN_NUM_DC, 1);
	gpio_set_level(PIN_NUM_RST, 1);
	//gpio_set_level(PIN_NUM_BCKL, 0);
    pwm_init();
	
	//gpio_set_direction(SPI_NUM_CS, GPIO_MODE_OUTPUT);
    //Reset the display
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);


    LCD_WR_REG(0x11);     //Sleep out
	vTaskDelay(120 / portTICK_RATE_MS);           //Delay 120ms
	LCD_WR_REG(0xB1);     //Normal mode
	LCD_WR_DATA8(0x05);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_REG(0xB2);     //Idle mode
	LCD_WR_DATA8(0x05);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_REG(0xB3);     //Partial mode
	LCD_WR_DATA8(0x05);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_DATA8(0x05);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_REG(0xB4);     //Dot inversion
	LCD_WR_DATA8(0x03);   
	LCD_WR_REG(0xC0);     //AVDD GVDD
	LCD_WR_DATA8(0xAB);   
	LCD_WR_DATA8(0x0B);   
	LCD_WR_DATA8(0x04);   
	LCD_WR_REG(0xC1);     //VGH VGL
	LCD_WR_DATA8(0xC5);   //C0
	LCD_WR_REG(0xC2);     //Normal Mode
	LCD_WR_DATA8(0x0D);   
	LCD_WR_DATA8(0x00);   
	LCD_WR_REG(0xC3);     //Idle
	LCD_WR_DATA8(0x8D);   
	LCD_WR_DATA8(0x6A);   
	LCD_WR_REG(0xC4);     //Partial+Full
	LCD_WR_DATA8(0x8D);   
	LCD_WR_DATA8(0xEE);   
	LCD_WR_REG(0xC5);     //VCOM
	LCD_WR_DATA8(0x0F);   
	LCD_WR_REG(0xE0);     //positive gamma
	LCD_WR_DATA8(0x07);   
	LCD_WR_DATA8(0x0E);   
	LCD_WR_DATA8(0x08);   
	LCD_WR_DATA8(0x07);   
	LCD_WR_DATA8(0x10);   
	LCD_WR_DATA8(0x07);   
	LCD_WR_DATA8(0x02);   
	LCD_WR_DATA8(0x07);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x0F);   
	LCD_WR_DATA8(0x25);   
	LCD_WR_DATA8(0x36);   
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0x08);   
	LCD_WR_DATA8(0x04);   
	LCD_WR_DATA8(0x10);   
	LCD_WR_REG(0xE1);     //negative gamma
	LCD_WR_DATA8(0x0A);   
	LCD_WR_DATA8(0x0D);   
	LCD_WR_DATA8(0x08);   
	LCD_WR_DATA8(0x07);   
	LCD_WR_DATA8(0x0F);   
	LCD_WR_DATA8(0x07);   
	LCD_WR_DATA8(0x02);   
	LCD_WR_DATA8(0x07);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x0F);   
	LCD_WR_DATA8(0x25);   
	LCD_WR_DATA8(0x35);   
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x04);   
	LCD_WR_DATA8(0x10);
		 
	LCD_WR_REG(0xFC);    
	LCD_WR_DATA8(0x80);  
		
	LCD_WR_REG(0x3A);     
	LCD_WR_DATA8(0x05);   
	LCD_WR_REG(0x36);
	if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x08);
	else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC8);
	else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x78);
	else LCD_WR_DATA8(0xA8);   
	LCD_WR_REG(0x21);     //Display inversion
	LCD_WR_REG(0x29);     //Display on
	LCD_WR_REG(0x2A);     //Set Column Address
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0x1A);  //26  
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0x69);   //105 
	LCD_WR_REG(0x2B);     //Set Page Address
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0x01);    //1
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0xA0);    //160
	LCD_WR_REG(0x2C); 
	
    LCD_Fill(0,0,160,80,BLACK);
	vTaskDelay(50 / portTICK_RATE_MS); 
	//gpio_set_level(PIN_NUM_BCKL, 1);
	mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, 1, st7735_lcd_backlight);

} 

void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{          
	uint16_t i,j; 
	Address_Set(xsta,ysta,xend-1,yend-1);//设置显示范围
	for(i=ysta;i<yend;i++)
	{													   	 	
		for(j=xsta;j<xend;j++)
		{
			LCD_WR_DATA(color);
		}
	} 					  	    
}


void LCD_SET_BLK(int8_t light)
{
	if(light<=st7735_lcd_backlight_max&&light>=st7735_lcd_backlight_min)
	{
		st7735_lcd_backlight=light;
		mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, 1, st7735_lcd_backlight);
	}

}

uint8_t LCD_GET_BLK(void)
{
	return st7735_lcd_backlight;
}
