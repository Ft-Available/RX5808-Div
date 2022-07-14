#ifndef __include_H
#define __include_H

#include <stdint.h>
#include "system.h"
#include "systick_delay.h"
#include "stmf4flash.h"
#include "lcd.h"
#include "ws2812.h"
#include "rx5808.h"
#include "beep.h"
#include "key.h"
#include "iwdg.h"
#include "rx5808_config.h"
#include "24cxx.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lvgl_init.h"

#include "stm32f4xx.h"
#include "stdbool.h"


#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
#define GPIOA_ODR_Addr    (GPIOA_BASE+20) //0x40020014
#define GPIOB_ODR_Addr    (GPIOB_BASE+20) //0x40020414 
#define GPIOC_ODR_Addr    (GPIOC_BASE+20) //0x40020814 
#define GPIOD_ODR_Addr    (GPIOD_BASE+20) //0x40020C14 
 
#define GPIOA_IDR_Addr    (GPIOA_BASE+16) //0x40020010 
#define GPIOB_IDR_Addr    (GPIOB_BASE+16) //0x40020410 
#define GPIOC_IDR_Addr    (GPIOC_BASE+16) //0x40020810 
#define GPIOD_IDR_Addr    (GPIOD_BASE+16) //0x40020C10 

#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)
#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n) 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n) 
#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n) 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  
#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  


/*********************************端口功能************************************
//-----------------------------------  UART  -----------------------------------

//      模块通道        端口      可选范围                建议                                     
#define USART1_RX_PIN    A10        
#define USART1_TX_PIN    A9        
                                       
#define USART2_RX_PIN    A3        //D6
#define USART2_TX_PIN    A2        //D5

#define USART3_RX_PIN    B11        //C11
#define USART3_TX_PIN    B10        //C10

#define UART4_RX_PIN    C11        
#define UART4_TX_PIN    C10        

#define UART4_RX_PIN    C11        
#define UART4_TX_PIN    D2        
//-----------------------------------  FTM  -----------------------------------
//      模块通道        端口      可选范围                建议
#define TIM1_CH1_PIN    A8        //E9
#define TIM1_CH2_PIN    A9        //E11
#define TIM1_CH3_PIN    A10        /E13
#define TIM1_CH4_PIN    A11        //E14

#define TIM2_CH1_PIN    A0        //
#define TIM2_CH2_PIN    A1        //B3
#define TIM2_CH3_PIN    A2        //B11
#define TIM2_CH4_PIN    A3        //B12

#define TIM3_CH1_PIN    A6        //C6 B4
#define TIM3_CH2_PIN    A7        //C7 B5
#define TIM3_CH3_PIN    B0        //C8
#define TIM3_CH4_PIN    B1        //C9

#define TIM4_CH1_PIN    B6        //D12
#define TIM4_CH2_PIN    B7        //D13
#define TIM4_CH3_PIN    B8        //D14
#define TIM4_CH4_PIN    B9        //D15

#define TIM5_CH1_PIN    A0        
#define TIM5_CH2_PIN    A1       
#define TIM5_CH3_PIN    A2        
#define TIM5_CH4_PIN    A3        
//-----------------------------------  I2C  -----------------------------------
//在切换引脚时，务必成对切换。编号相邻得两个引脚为一对
//      模块通道        端口      可选范围                建议
#define I2C1_SCL_PIN    B6        //B8
#define I2C1_SDA_PIN    B7        //B9
                                       
#define I2C2_SCL_PIN    B10        //
#define I2C2_SDA_PIN    B11        //


//-----------------------------------  SPI  -----------------------------------
//      模块通道        端口      可选范围              建议
//在切换引脚时，务必成对切换。编号相邻得四个引脚为一对
#define SPI1_SCK_PIN    A5       //B3
#define SPI1_SOUT_PIN   A6       //B4
#define SPI1_SIN_PIN    A7       //B5                        
#define SPI1_PCS0_PIN   A4       //A15
                                      
                                      
#define SPI2_SCK_PIN    B13
#define SPI2_SOUT_PIN   B14
#define SPI2_SIN_PIN    B15
#define SPI2_PCS0_PIN   B12


//-----------------------------------  CAN  -----------------------------------
//在切换引脚时，务必成对切换。编号相邻得两个引脚为一对   H2与E7为一对
#define CAN0_TX_PIN     A12    //D1  B9
#define CAN0_RX_PIN     A11    //D0  B8
 

*****************************************************************************/
#endif
