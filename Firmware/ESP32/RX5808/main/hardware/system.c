#include <stdint.h>
#include "driver/gpio.h"
#include "24cxx.h"
#include "beep.h"
#include "lcd.h"
#include "rx5808.h"
#include "ws2812.h"
#include "fan.h"
#include "rx5808_config.h"
#include "system.h"
#include "esp_timer.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "hwvers.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void create_cpu_stack_monitor_task();
void cpu_stack_monitor_task(void *param);

static void led_flash_task(void *param)
{

	while(1)
	{
         gpio_set_level(LED0_PIN, 0);
		 vTaskDelay(300/portTICK_PERIOD_MS);
		 gpio_set_level(LED0_PIN, 1);
		 vTaskDelay(300/portTICK_PERIOD_MS);
	}
}

void LED_Init()
{ 
    gpio_set_direction(LED0_PIN, GPIO_MODE_OUTPUT);		
	gpio_set_level(LED0_PIN, 0);

	xTaskCreatePinnedToCore((TaskFunction_t)led_flash_task,
	                          "led_task", 
							  512, 
							  NULL,
							   0,
							    NULL, 
								0 );
	
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
	LCD_Init();
	//printf("lcd init success!\n");
	fan_Init();	
	//printf("fan init success!\n");
 	eeprom_24cxx_init();	
	//printf("24cxx init success!\n");
 	rx5808_div_setup_load();
	//printf("setup load success!\n");
 	LED_Init();
	//printf("led init success!\n"); 	
 	Beep_Init();
	//printf("beep init success!\n");
    timer_init();  
	//printf("timer init success!\n");	
    RX5808_Init();
	//printf("RX5808 init success!\n");
	//ws2812_init();
	//printf("ws2812 init success!\n");
	//while(1);

    //create_cpu_stack_monitor_task();
	
}

/*
make menuconfig -> Component config -> FreeRTOS -> Enable FreeRTOS trace facility
make menuconfig -> Component config -> FreeRTOS -> Enable FreeRTOS trace facility -> Enable FreeRTOS stats formatting functions
make menuconfig -> Component config -> FreeRTOS -> Enable FreeRTOS to collect run time stats*/
void create_cpu_stack_monitor_task()
{
    xTaskCreate((TaskFunction_t )cpu_stack_monitor_task, /* 任务入口函数 */
                  (const char* )"CPU_STACK",
                  (uint16_t )3072, 
                  (void* )NULL, 
                  (UBaseType_t )1, 
                  NULL);
}


void cpu_stack_monitor_task(void *param)
{
	uint8_t CPU_STACK_RunInfo[400]; 

	while (1) {
		// memset(CPU_STACK_RunInfo,0,400); 

		// vTaskList((char *)&CPU_STACK_RunInfo);

		// printf("-----------------heap_monitor-----------------\r\n");
		// printf("name        status   priority  stack  label\r\n");
		// printf("%s", CPU_STACK_RunInfo);
		// printf("---------------------------------------------\r\n");

		// memset(CPU_STACK_RunInfo,0,400);

		// vTaskGetRunTimeStats((char *)&CPU_STACK_RunInfo);
        
		// printf("-----------------running_time_monitor----------------\r\n");
		// printf("name             count              precent\r\n");printf("%s", CPU_STACK_RunInfo);
		// printf("---------------------------------------------\r\n\n");
		vTaskDelay(3000/portTICK_PERIOD_MS);
	}

}

/*
-----------------heap_monitor-----------------
name        status   priority  stack  label
CPU_STACK       X       1       936     16
main            R       1       256     5
IDLE            R       0       1044    6
led_task        R       0       108     13
IDLE            R       0       1048    7
rx5808_task     B       5       372     15
Tmr Svc         B       1       1660    8
ipc0            B       24      1092    1
upload_task     B       2       288     12
esp_timer       S       22      3672    3
beep_task       B       1       336     14
ipc1            B       24      1132    2
---------------------------------------------
-----------------running_time_monitor----------------
name             count              precent
CPU_STACK       499214          2%
led_task        1268            <1%
IDLE            24446855                99%
IDLE            13928661                56%
main            6032396         24%
rx5808_task     3895619         15%
ipc0            13420           <1%
upload_task     7729            <1%
beep_task       1033            <1%
esp_timer       520712          2%
ipc1            18465           <1%
Tmr Svc         12              <1%
---------------------------------------------
*/


