#ifndef SPI_H
#define SPI_H


#include "driver/spi_master.h"
#include "driver/gpio.h"


#define SPI_HOST_USER    HSPI_HOST

#define SPI_NUM_MISO 12
#define SPI_NUM_MOSI 13
#define SPI_NUM_CLK  14
#define SPI_NUM_CS   15

#define PIN_NUM_DC   2
#define PIN_NUM_RST  27
#define PIN_NUM_BCKL 26

extern spi_device_handle_t my_spi;
void lcd_spi_pre_transfer_callback(spi_transaction_t *t);
void spi_init();
void spi_cmd(spi_device_handle_t spi, const uint8_t cmd);
void spi_data(spi_device_handle_t spi, const uint8_t *data, int len);






#endif