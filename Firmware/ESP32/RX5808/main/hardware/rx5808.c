#include "rx5808.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include "sys/unistd.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define RX5808_TOGGLE_DEAD_ZONE 1

#define Synthesizer_Register_A 				              0x00  
#define Synthesizer_Register_B 				              0x01  
#define Synthesizer_Register_C 				              0x02
#define Synthesizer_Register_D 				              0x03
#define VCO_Switch_Cap_Control_Register 		        0x04
#define DFC_Control_Register 				                0x05
#define _6M_Audio_Demodulator_Control_Register 			0x06
#define _6M5_Audio_Demodulator_Control_Register 	  0x07
#define Receiver_control_Register_1 				        0x08
#define Receiver_control_Register_2 				        0x09
#define Power_Down_Control_Register                 0x0A 
#define State_Register                              0x0F  


#define RX5808_RSSI0_CHAN  ADC1_CHANNEL_0
#define RX5808_RSSI1_CHAN  ADC1_CHANNEL_3
#define VBAT_ADC_CHAN      ADC1_CHANNEL_1
#define KEY_ADC_CHAN       ADC1_CHANNEL_2


adc1_channel_t adc_dma_chan[]={RX5808_RSSI0_CHAN,RX5808_RSSI1_CHAN,VBAT_ADC_CHAN,KEY_ADC_CHAN};


#define RX5808_SCLK       18
#define RX5808_MOSI       23
#define RX5808_CS         5
#define RX5808_SWITCH0    4
#define RX5808_SWITCH1    12

uint16_t adc_convert_temp0[32][3];
uint16_t adc_convert_temp1[32][3];
uint16_t adc_converted_value[3]={1024,1024,1024};

volatile int8_t channel_count = 0;
volatile int8_t Chx_count = 0;
volatile uint8_t Rx5808_channel;
volatile uint16_t Rx5808_RSSI_Ad_Min0=0;
volatile uint16_t Rx5808_RSSI_Ad_Max0=4095;
volatile uint16_t Rx5808_RSSI_Ad_Min1=0;
volatile uint16_t Rx5808_RSSI_Ad_Max1=4095;
uint16_t Rx5808_Language=1;
uint16_t Rx5808_Signal_Source=0;


const uint16_t Rx5808_Freq[6][8]=
{
	{5865,5845,5825,5805,5785,5765,5745,5725},	  //A	CH1-8
    {5733,5752,5771,5790,5809,5828,5847,5866},		//B	CH1-8
    {5705,5685,5665,5645,5885,5905,5925,5945},		//C	CH1-8
    {5740,5760,5780,5800,5820,5840,5860,5880},		//D	CH1-8
    {5658,5695,5732,5769,5806,5843,5880,5917},		//E	CH1-8
    {5362,5399,5436,5473,5510,5547,5584,5621}		  //F	CH1-8
};

static esp_adc_cal_characteristics_t adc1_chars;

#define ADC_RESULT_BYTE     2
#define ADC_CONV_LIMIT_EN   1                       //For ESP32, this should always be set to 1
#define ADC_CONV_MODE       ADC_CONV_SINGLE_UNIT_1  //ESP32 only supports ADC1 DMA mode
#define ADC_OUTPUT_TYPE     ADC_DIGI_OUTPUT_FORMAT_TYPE1



void RX5808_RSSI_ADC_Init()
{
     esp_err_t ret;
    ret = esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF);
    if (ret == ESP_OK) {
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
    } else {
        printf("adc calib failed!\n");
    }
    adc_set_clk_div(1);
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(RX5808_RSSI0_CHAN, ADC_ATTEN_DB_11);
	adc1_config_channel_atten(RX5808_RSSI1_CHAN, ADC_ATTEN_DB_11);
	adc1_config_channel_atten(VBAT_ADC_CHAN, ADC_ATTEN_DB_11);
	adc1_config_channel_atten(KEY_ADC_CHAN, ADC_ATTEN_DB_11);


    // adc_digi_init_config_t adc_dma_config = {
    //     .max_store_buf_size = 1024,
    //     .conv_num_each_intr = 32,
    //     .adc1_chan_mask = BIT(7),
    //     .adc2_chan_mask = 0,
    // };
    // ESP_ERROR_CHECK(adc_digi_initialize(&adc_dma_config));

    // adc_digi_configuration_t dig_cfg = {
    //     .conv_limit_en = ADC_CONV_LIMIT_EN,
    //     .conv_limit_num = 32,
    //     .sample_freq_hz = 40 * 1000,
    //     .conv_mode = ADC_CONV_MODE,
    //     .format = ADC_OUTPUT_TYPE,
    // };

    // adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    // dig_cfg.pattern_num = 4;
    // for (int i = 0; i < 4; i++) {
    //     uint8_t unit = GET_UNIT(adc_dma_chan[i]);
    //     uint8_t ch = adc_dma_chan[i] & 0x7;
    //     adc_pattern[i].atten = ADC_ATTEN_DB_11;
    //     adc_pattern[i].channel = ch;
    //     adc_pattern[i].unit = unit;
    //     adc_pattern[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

    //     //ESP_LOGI(TAG, "adc_pattern[%d].atten is :%x", i, adc_pattern[i].atten);
    //    // ESP_LOGI(TAG, "adc_pattern[%d].channel is :%x", i, adc_pattern[i].channel);
    //     //ESP_LOGI(TAG, "adc_pattern[%d].unit is :%x", i, adc_pattern[i].unit);
    // }
    // dig_cfg.adc_pattern = adc_pattern;
    // ESP_ERROR_CHECK(adc_digi_controller_configure(&dig_cfg));
	// adc_digi_start();

}


void RX5808_Init()
{

    gpio_set_direction(RX5808_SCLK, GPIO_MODE_OUTPUT);	
	gpio_set_direction(RX5808_MOSI, GPIO_MODE_OUTPUT);	
	gpio_set_direction(RX5808_CS, GPIO_MODE_OUTPUT);	
	gpio_set_direction(RX5808_SWITCH0, GPIO_MODE_OUTPUT);
	gpio_reset_pin(RX5808_SWITCH1);	
	gpio_set_direction(RX5808_SWITCH1, GPIO_MODE_OUTPUT);	

	gpio_set_level(RX5808_SWITCH0, 1);
	gpio_set_level(RX5808_SWITCH1, 0);
	
	Send_Register_Data(Synthesizer_Register_A,0x00008);

	Send_Register_Data(Power_Down_Control_Register,0x10DF3);

	RX5808_Set_Freq(Rx5808_Freq[Chx_count][channel_count]);

	RX5808_RSSI_ADC_Init();	

    xTaskCreate((TaskFunction_t)DMA2_Stream0_IRQHandler, // 任务函数
			"rx5808_task",//任务名称，没什么用
			1024,//任务堆栈大小
			NULL,//传递给 任务函数的参数
			5,//任务优先级
			NULL//任务句柄
			);
      
}


void Soft_SPI_Send_One_Bit(uint8_t bit)
{
	gpio_set_level(RX5808_SCLK, 0);
	usleep(20);
    gpio_set_level(RX5808_MOSI, ((bit&0x01)==1));
	usleep(30);
	gpio_set_level(RX5808_SCLK, 1);
	usleep(20);
}

void Send_Register_Data(uint8_t addr,uint32_t data)   
{
  gpio_set_level(RX5808_CS, 0);
  uint8_t read_write=1;   //1 write     0 read
 
	for(uint8_t i=0;i<4;i++)
	  Soft_SPI_Send_One_Bit(((addr>>i)&0x01));
      Soft_SPI_Send_One_Bit(read_write&0x01);
	for(uint8_t i=0;i<20;i++)
	  Soft_SPI_Send_One_Bit(((data>>i)&0x01));
	
	gpio_set_level(RX5808_CS, 1);
	gpio_set_level(RX5808_SCLK, 0);
	gpio_set_level(RX5808_MOSI, 0);
}

void RX5808_Set_Freq(uint16_t Fre)   
{
	uint16_t F_LO=(Fre-479)>>1;
	uint16_t N;
	uint16_t A;
	
	N=F_LO/32;    
	A=F_LO%32;    

	Send_Register_Data(Synthesizer_Register_B,N<<7|A);
}

void Rx5808_Set_Channel(uint8_t ch)
{
	if(ch>47)
		return ;
    Rx5808_channel=ch;
	Chx_count=Rx5808_channel/8;
	channel_count=Rx5808_channel%8;
}

void RX5808_Set_RSSI_Ad_Min0(uint16_t value)
{
    Rx5808_RSSI_Ad_Min0=value;
}

void RX5808_Set_RSSI_Ad_Max0(uint16_t value)
{
    Rx5808_RSSI_Ad_Max0=value;
}
void RX5808_Set_RSSI_Ad_Min1(uint16_t value)
{
    Rx5808_RSSI_Ad_Min1=value;
}
void RX5808_Set_RSSI_Ad_Max1(uint16_t value)
{
    Rx5808_RSSI_Ad_Max1=value;
}

void RX5808_Set_Language(uint16_t value)
{
    Rx5808_Language = value;
}

void RX5808_Set_Signal_Source(uint16_t value)
{
    Rx5808_Signal_Source = value;
}

uint8_t Rx5808_Get_Channel()
{
	return Rx5808_channel;
}

uint16_t RX5808_Get_RSSI_Ad_Min0()
{
    return  Rx5808_RSSI_Ad_Min0;
}

uint16_t RX5808_Get_RSSI_Ad_Max0()
{
    return Rx5808_RSSI_Ad_Max0;
}
uint16_t RX5808_Get_RSSI_Ad_Min1()
{
    return Rx5808_RSSI_Ad_Min1;
}
uint16_t RX5808_Get_RSSI_Ad_Max1()
{
    return Rx5808_RSSI_Ad_Max1;
}

uint16_t RX5808_Get_Language()
{
    return Rx5808_Language;
}
uint16_t RX5808_Get_Signal_Source()
{
    return Rx5808_Signal_Source;
}

float Rx5808_Calculate_RSSI_Precentage(uint16_t value, uint16_t min, uint16_t max)
{
  float precent=((float)((value - min)*100)) / (float)(max - min);	  
	if(precent>99.0f)
		precent=99.0f;
	if(precent<0)
		precent=0;
   return precent;   
}

float Rx5808_Get_Precentage0()
{
    return Rx5808_Calculate_RSSI_Precentage(adc_converted_value[0],Rx5808_RSSI_Ad_Min0,Rx5808_RSSI_Ad_Max0);
}

float Rx5808_Get_Precentage1()
{
    return Rx5808_Calculate_RSSI_Precentage(adc_converted_value[1], Rx5808_RSSI_Ad_Min1,Rx5808_RSSI_Ad_Max1);
}


float Get_Battery_Voltage()
{
    return (float)adc_converted_value[2]/4095*6.8;
}



void DMA2_Stream0_IRQHandler(void)     
{
	static uint16_t count=0;
	static uint8_t rx5808_cur_receiver_best=rx5808_receiver_count;
	static uint8_t rx5808_pre_receiver_best=rx5808_receiver_count;

	while(1)
	{
    uint32_t sum0=0,sum1=0;
    
	for(int i=0;i<16;i++)
	{
		sum0+=adc1_get_raw(RX5808_RSSI0_CHAN);
		sum1+=adc1_get_raw(RX5808_RSSI1_CHAN);
	}
	adc_converted_value[0]=sum0>>4;		
	adc_converted_value[1]=sum1>>4;	
	adc_converted_value[2]=adc1_get_raw(VBAT_ADC_CHAN);		
		
	
	if(Rx5808_Signal_Source==1)
	{
		gpio_set_level(RX5808_SWITCH1, 1);
		gpio_set_level(RX5808_SWITCH0, 0);

	}
	else if(Rx5808_Signal_Source==2)
	{
		gpio_set_level(RX5808_SWITCH0, 1);
		gpio_set_level(RX5808_SWITCH1, 0);
	}
	else
	{
		float rssi0=Rx5808_Get_Precentage0();
		float rssi1=Rx5808_Get_Precentage1();
		float rssi_diff=0;
		if(rssi0>rssi1){
			rssi_diff=rssi0-rssi1;
		}
		else{
			rssi_diff=rssi1-rssi0;
		}
		if(rssi_diff>=RX5808_TOGGLE_DEAD_ZONE)
		{
			if(rssi0>rssi1){
			rx5808_cur_receiver_best=rx5808_receiver0;
			}
			else{
			rx5808_cur_receiver_best=rx5808_receiver1;
			}
			if(rx5808_cur_receiver_best==rx5808_pre_receiver_best)
				{
				if(rx5808_cur_receiver_best==rx5808_receiver0){
				gpio_set_level(RX5808_SWITCH0, 1);
				gpio_set_level(RX5808_SWITCH1, 0);
				}
				else if(rx5808_cur_receiver_best==rx5808_receiver1){
				gpio_set_level(RX5808_SWITCH1, 1);
				gpio_set_level(RX5808_SWITCH0, 0);
				}
		}		
		rx5808_pre_receiver_best=rx5808_cur_receiver_best;		
		}

		}
		(count==100)?(count=0):(++count);
	vTaskDelay(10 / portTICK_PERIOD_MS);
	}
		
}

