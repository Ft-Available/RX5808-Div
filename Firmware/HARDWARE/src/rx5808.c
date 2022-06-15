#include "rx5808.h"

#define RX5808_TOGGLE_DEAD_ZONE 2

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

uint16_t adc_convert_temp0[32][3];
uint16_t adc_convert_temp1[32][3];
uint16_t adc_converted_value[3];

volatile int8_t channel_count = 0;
volatile int8_t Chx_count = 0;
volatile uint8_t Rx5808_channel;
volatile uint16_t Rx5808_RSSI_Ad_Min0;
volatile uint16_t Rx5808_RSSI_Ad_Max0;
volatile uint16_t Rx5808_RSSI_Ad_Min1;
volatile uint16_t Rx5808_RSSI_Ad_Max1;


const uint16_t Rx5808_Freq[6][8]=
{
	  {5865,5845,5825,5805,5785,5765,5745,5725},	  //A	CH1-8
    {5733,5752,5771,5790,5809,5828,5847,5866},		//B	CH1-8
    {5705,5685,5665,5645,5885,5905,5925,5945},		//C	CH1-8
    {5740,5760,5780,5800,5820,5840,5860,5880},		//D	CH1-8
    {5658,5695,5732,5769,5806,5843,5880,5917},		//E	CH1-8
    {5362,5399,5436,5473,5510,5547,5584,5621}		  //F	CH1-8
};

void RX5808_RSSI_ADC_Init()
{
  GPIO_InitTypeDef  GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	DMA_DeInit(DMA2_Stream0);
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&adc_convert_temp0;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = 3*32;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc =  DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);
	DMA_DoubleBufferModeCmd(DMA2_Stream0, ENABLE);//Ê¹ÄÜË«»º³å
	DMA_DoubleBufferModeConfig(DMA2_Stream0, (u32)adc_convert_temp1, DMA_Memory_0);
		
	DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TC); 
  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);                                      
  DMA_Cmd(DMA2_Stream0, ENABLE);
	
  NVIC_EnableIRQ(DMA2_Stream0_IRQn);

	ADC_CommonStructInit(&ADC_CommonInitStructure);
	ADC_CommonInitStructure.ADC_Mode             = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler        = ADC_Prescaler_Div8;
	ADC_CommonInitStructure.ADC_DMAAccessMode    = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_ContinuousConvMode       = ENABLE;
	ADC_InitStructure.ADC_Resolution               = ADC_Resolution_12b;
	//ADC_InitStructure.ADC_ExternalTrigConv         = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_ExternalTrigConvEdge     = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign                = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion          = 3;
	ADC_InitStructure.ADC_ScanConvMode             = ENABLE;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_112Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_112Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 3, ADC_SampleTime_112Cycles);

	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);

	ADC_SoftwareStartConv(ADC1);
}


void RX5808_Init()
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_10|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	RX5808_VIDEO_SWITCH0=1;
	RX5808_VIDEO_SWITCH1=0;
  
	Send_Register_Data(Synthesizer_Register_A,0x00008);
	
	Send_Register_Data(Power_Down_Control_Register,0x10DF3);
	
  RX5808_Set_Freq(Rx5808_Freq[Chx_count][channel_count]);
	
  RX5808_RSSI_ADC_Init();	
}


static void Soft_SPI_Send_One_Bit(uint8_t bit)
{
  RX5808_SCLK =0;
	SysTick_Delay_us(20);
	RX5808_MOSI=((bit&0x01)==1);
	SysTick_Delay_us(30);
	RX5808_SCLK =1;
	SysTick_Delay_us(20);
}


static void Send_Register_Data(uint8_t addr,uint32_t data)   
{
  RX5808_CS=0;   
  uint8_t read_write=1;   //1 write     0 read
 
	for(uint8_t i=0;i<4;i++)
	  Soft_SPI_Send_One_Bit(((addr>>i)&0x01));
  Soft_SPI_Send_One_Bit(read_write&0x01);
	for(uint8_t i=0;i<20;i++)
	  Soft_SPI_Send_One_Bit(((data>>i)&0x01));
	
	RX5808_CS=1;
	RX5808_SCLK=0;
	RX5808_MOSI=0;
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

static float Rx5808_Calculate_RSSI_Precentage(uint16_t value, uint16_t min, uint16_t max)
{
  float precent=((float)((value - min)*100)) / (float)(max - min);	  
	if(precent>99)
		precent=99;
	if(precent<0)
		precent=0;
   return (uint8_t)precent;   
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
	static uint8_t count=0;
	static uint8_t rx5808_cur_receiver_best=rx5808_receiver0;
	static uint8_t rx5808_pre_receiver_best=rx5808_receiver0;
	++count;
	if (DMA_GetFlagStatus(DMA2_Stream0, DMA_IT_TCIF0) == SET)  
	{
		DMA_ClearFlag(DMA2_Stream0, DMA_IT_TCIF0); 
		uint32_t sum0=0,sum1=0;
			
		if(DMA_GetCurrentMemoryTarget(DMA2_Stream0) == DMA_Memory_0)
		{
		  	for(int i=0;i<32;i++)
				{
					 sum0+= *(*(adc_convert_temp1+i)+0);
					 sum1+= *(*(adc_convert_temp1+i)+1);
				}
				adc_converted_value[0]=sum0>>5;
				adc_converted_value[1]=sum1>>5;
				adc_converted_value[2]=adc_convert_temp1[0][2];
		}
		else
		{
		    for(int i=0;i<32;i++)
				{
					 sum0+= *(*(adc_convert_temp0+i)+0);
					 sum1+= *(*(adc_convert_temp0+i)+1);
				}
				adc_converted_value[0]=sum0>>5;
				adc_converted_value[1]=sum1>>5;
				adc_converted_value[2]=adc_convert_temp0[0][2];
		}
		
	
	}
	if(count%10==0)
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
		if(rssi_diff>RX5808_TOGGLE_DEAD_ZONE)
		{
			if(rssi0>rssi1){
		    rx5808_cur_receiver_best=rx5808_receiver0;
		  }
		  else{
		    rx5808_cur_receiver_best=rx5808_receiver1;
		  }
			if(rx5808_cur_receiver_best==rx5808_pre_receiver_best)
			{
				if(rx5808_cur_receiver_best==rx5808_receiver0)
				{RX5808_VIDEO_SWITCH0=1;RX5808_VIDEO_SWITCH1=0;}
				else if(rx5808_cur_receiver_best==rx5808_receiver1)
				{RX5808_VIDEO_SWITCH1=1;RX5808_VIDEO_SWITCH0=0;}
			}		
			rx5808_pre_receiver_best=rx5808_cur_receiver_best	;		
		
		}
		
	 (count==100)?(count=0,LED0=~LED0):(count=count);
	}
}

