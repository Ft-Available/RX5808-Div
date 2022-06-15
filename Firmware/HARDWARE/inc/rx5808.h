#ifndef __RX5808_H
#define __RX5808_H

#include "include.h"

#define RX5808_SCLK   PBout(13)
#define RX5808_MOSI   PBout(15)
#define RX5808_CS     PBout(12)

#define RX5808_VIDEO_SWITCH0     PBout(10)
#define RX5808_VIDEO_SWITCH1     PBout(2)

typedef enum
{
   rx5808_receiver0=0,
	 rx5808_receiver1,
}rx5808_receive;

extern const uint16_t Rx5808_Freq[6][8];
extern volatile int8_t channel_count;
extern volatile int8_t Chx_count;
extern uint16_t adc_converted_value[3];

void RX5808_RSSI_ADC_Init(void);
void RX5808_Init(void);
void Soft_SPI_Send_One_Bit(uint8_t bit);
void Send_Register_Data(uint8_t addr,uint32_t data);
void RX5808_Set_Freq(uint16_t Fre);
void Rx5808_Set_Channel(uint8_t ch);
void RX5808_Set_RSSI_Ad_Min0(uint16_t value);
void RX5808_Set_RSSI_Ad_Max0(uint16_t value);
void RX5808_Set_RSSI_Ad_Min1(uint16_t value);
void RX5808_Set_RSSI_Ad_Max1(uint16_t value);
uint8_t Rx5808_Get_Channel(void);
uint16_t RX5808_Get_RSSI_Ad_Min0(void);
uint16_t RX5808_Get_RSSI_Ad_Max0(void);
uint16_t RX5808_Get_RSSI_Ad_Min1(void);
uint16_t RX5808_Get_RSSI_Ad_Max1(void);
float Rx5808_Calculate_RSSI_Precentage(uint16_t value,uint16_t min,uint16_t max);
float Rx5808_Get_Precentage0(void);
float Rx5808_Get_Precentage1(void);
float Get_Battery_Voltage(void);

#endif

