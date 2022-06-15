 #include "rx5808_config.h"
#include "lcd.h"
#include "beep.h"
#include "rx5808.h"
#include "page_start.h"

#if RX5808_CONFIGT_FLASH_EEPROM==1
uint32_t  rx5808_div_setup[rx5808_div_config_setup_count];
#else
uint16_t  rx5808_div_setup[rx5808_div_config_setup_count];
#endif

void rx5808_div_setup_load()
{ 
	#if RX5808_CONFIGT_FLASH_EEPROM==1
	STMF4_FLASH_Read_Word(rx5808_div_setup,rx5808_div_config_setup_count);
	#else
	eeprom_24cxx_read_half_word_len(0,rx5808_div_setup,rx5808_div_config_setup_count);
	#endif
	if(rx5808_div_setup[rx5808_div_config_setup_id]!=SETUP_ID_DEFAULT)
	{		
	  rx5808_div_setup[rx5808_div_config_start_animation]=START_ANIMATION_DEFAULT;
		rx5808_div_setup[rx5808_div_config_beep]=BEEP_DEFAULT;
		rx5808_div_setup[rx5808_div_config_backlight]=BACKLIGHT_DEFAULT;
		rx5808_div_setup[rx5808_div_config_channel]=CHANNEL_DEFAULT;
		rx5808_div_setup[rx5808_div_config_rssi_adc_value_min0]=RSSI0_MIN_DEFAULT;
		rx5808_div_setup[rx5808_div_config_rssi_adc_value_max0]=RSSI0_MAX_DEFAULT;
		rx5808_div_setup[rx5808_div_config_rssi_adc_value_min1]=RSSI1_MIN_DEFAULT;
		rx5808_div_setup[rx5808_div_config_rssi_adc_value_max1]=RSSI1_MAX_DEFAULT;
		rx5808_div_setup[rx5808_div_config_setup_id]=SETUP_ID_DEFAULT;
		#if RX5808_CONFIGT_FLASH_EEPROM==1
		STMF4_FLASH_Write_Word(rx5808_div_setup,rx5808_div_config_setup_count);
	  #else
	  eeprom_24cxx_write_half_word_len(0,rx5808_div_setup,rx5808_div_config_setup_count);
	  #endif
	}
	
	  page_set_animation_en(rx5808_div_setup[rx5808_div_config_start_animation]);
		beep_set_enable_disable(rx5808_div_setup[rx5808_div_config_beep]);
	  LCD_SET_BLK(rx5808_div_setup[rx5808_div_config_backlight]);
		Rx5808_Set_Channel(rx5808_div_setup[rx5808_div_config_channel]);
		RX5808_Set_RSSI_Ad_Min0(rx5808_div_setup[rx5808_div_config_rssi_adc_value_min0]);
		RX5808_Set_RSSI_Ad_Max0(rx5808_div_setup[rx5808_div_config_rssi_adc_value_max0]);
		RX5808_Set_RSSI_Ad_Min1(rx5808_div_setup[rx5808_div_config_rssi_adc_value_min1]);
		RX5808_Set_RSSI_Ad_Max1(rx5808_div_setup[rx5808_div_config_rssi_adc_value_max1]);

}

void rx5808_div_setup_upload()
{
    rx5808_div_setup[rx5808_div_config_start_animation]=page_get_animation_en();
		rx5808_div_setup[rx5808_div_config_beep]=beep_get_status();;
		rx5808_div_setup[rx5808_div_config_backlight]=LCD_GET_BLK();
		rx5808_div_setup[rx5808_div_config_channel]=Rx5808_Get_Channel();
		rx5808_div_setup[rx5808_div_config_rssi_adc_value_min0]=RX5808_Get_RSSI_Ad_Min0();
		rx5808_div_setup[rx5808_div_config_rssi_adc_value_max0]=RX5808_Get_RSSI_Ad_Max0();
		rx5808_div_setup[rx5808_div_config_rssi_adc_value_min1]=RX5808_Get_RSSI_Ad_Min1();
		rx5808_div_setup[rx5808_div_config_rssi_adc_value_max1]=RX5808_Get_RSSI_Ad_Max1();
		rx5808_div_setup[rx5808_div_config_setup_id]=SETUP_ID_DEFAULT;
	
	#if RX5808_CONFIGT_FLASH_EEPROM==1
		STMF4_FLASH_Write_Word(rx5808_div_setup,rx5808_div_config_setup_count);
	#else
	  eeprom_24cxx_write_half_word_len(0,rx5808_div_setup,rx5808_div_config_setup_count);
	#endif
	
}

