#ifndef __rx5808_config_H
#define __rx5808_config_H

#include "include.h"


//STM32F4 最小FlASH扇区16K 写入需擦除，时间较长，不建议使用FLASH
#define RX5808_CONFIGT_FLASH_EEPROM  0     //0 EEPROM  | 1 FLASH 

#define  STATUS_ON   1
#define  STATUS_OFF  0


#define  START_ANIMATION_DEFAULT  STATUS_ON
#define  BEEP_DEFAULT             STATUS_ON
#define  BACKLIGHT_DEFAULT        100
#define  CHANNEL_DEFAULT          0
#define  RSSI0_MIN_DEFAULT        0
#define  RSSI0_MAX_DEFAULT        4095
#define  RSSI1_MIN_DEFAULT        0
#define  RSSI1_MAX_DEFAULT        4095
#define  SETUP_ID_DEFAULT         0x1234

	
typedef enum
{
	  rx5808_div_config_start_animation=0,
		rx5808_div_config_beep,
		rx5808_div_config_backlight,
	  rx5808_div_config_channel,
		rx5808_div_config_rssi_adc_value_min0,
		rx5808_div_config_rssi_adc_value_max0,
		rx5808_div_config_rssi_adc_value_min1, 
		rx5808_div_config_rssi_adc_value_max1,
	  rx5808_div_config_setup_id,
	  rx5808_div_config_setup_count,
}rx5808_div_setup_enum;

void rx5808_div_setup_load(void);
void rx5808_div_setup_upload(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif




#endif
