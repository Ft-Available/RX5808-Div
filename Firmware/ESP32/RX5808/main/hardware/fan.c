#include "fan.h"
#include "driver/ledc.h"
#include "hwvers.h"
volatile uint8_t fan_speed=100;

void fan_Init(void)
{
   ledc_timer_config_t ledc_timer_ls = {
			.duty_resolution = LEDC_TIMER_13_BIT, // 设置分辨率,最大为2^13-1
			.freq_hz = 8000,                      // PWM信号频率
			.speed_mode = LEDC_LOW_SPEED_MODE,    // 定时器模式（“高速”或“低速”）
			.timer_num = LEDC_TIMER_1,            // 设置定时器源（0-3）
			.clk_cfg = LEDC_AUTO_CLK,             // 配置LEDC时钟源（这里是自动选择）
		};
		// 初始化ledc的定时器配置
		ledc_timer_config(&ledc_timer_ls);
		/*
		* 通过选择为 LEDC 控制器的每个通道准备单独的配置：
		* - 控制器的通道号
		* - 输出占空比，初始设置为 0
		* - LEDC 连接到的 GPIO 编号
		* - 速度模式，高或低
		* - 为LEDC通道指定定时器
		*   注意: 如果不同通道使用一个定时器，那么这些通道的频率和占空比分辨率将相同
		*/
	    gpio_reset_pin(FAN_IO_NUM);	
		ledc_channel_config_t ledc_channel = {
			.channel    = LEDC_CHANNEL_0,
			.duty       = 0,
			.gpio_num   = FAN_IO_NUM,
			.speed_mode = LEDC_LOW_SPEED_MODE,
			.hpoint     = 0,
			.timer_sel  = LEDC_TIMER_1
		};
		// 初始化ledc的通道
		ledc_channel_config(&ledc_channel);        
}

void fan_set_speed(uint8_t duty)
{
        if(duty>100)
        duty=100;

		fan_speed=duty;

        int fan_duty=(int)(duty*81.91f);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, fan_duty);
		ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
		//ledc_timer_resume(LEDC_LOW_SPEED_MODE, LEDC_TIMER_1);
}

uint16_t fan_get_speed()
{
        return (uint16_t)fan_speed;
}