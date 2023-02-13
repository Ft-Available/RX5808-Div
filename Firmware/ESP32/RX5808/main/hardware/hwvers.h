#ifndef __RX5808_DIV_HW_VERS_H
#define __RX5808_DIV_HW_VERS_H


#define RX5808_VERSION_MAJOR  1
#define RX5808_VERSION_MINOR  4
#define RX5808_VERSION_PATCH  0

//#define ST7735S
#define D0WDQ6_VER
#define SPI_LOW_SPEED


#define SPI_HOST_USER    HSPI_HOST

#define SPI_NUM_MISO 12
#define SPI_NUM_MOSI 13
#define SPI_NUM_CLK  14
#define SPI_NUM_CS   15

#define PIN_NUM_DC   2
#define PIN_NUM_RST  27
#define PIN_NUM_BCKL 26


#define io_i2c_gpio_sda  32
#define io_i2c_gpio_scl  33
#define i2c_fre  400*1000
#define i2c_port  I2C_NUM_0



#define RX5808_RSSI0_CHAN  ADC1_CHANNEL_0
#define RX5808_RSSI1_CHAN  ADC1_CHANNEL_3
#ifndef D0WDQ6_VER
#define VBAT_ADC_CHAN      ADC1_CHANNEL_1
#define KEY_ADC_CHAN       ADC1_CHANNEL_2
#else
#define VBAT_ADC_CHAN      ADC1_CHANNEL_7
#define KEY_ADC_CHAN       ADC1_CHANNEL_6
#endif

#define RX5808_SCLK       18
#define RX5808_MOSI       23
#define RX5808_CS         5

#define RX5808_SWITCH0    4
#define RX5808_SWITCH1    12

#define RX5808_TOGGLE_DEAD_ZONE 1
#define RX5808_CALIB_RSSI_ADC_VALUE_THRESHOULD 100


#define Beep_Is_Src 1
#define RMT_TX_GPIO 19
#define LED0_PIN 21
#define FAN_IO_NUM 0
#define Beep_Pin_Num  22



#endif