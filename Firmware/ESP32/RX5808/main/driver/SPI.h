#ifndef SPI_H
#define SPI_H


#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "hardware/hwvers.h"



extern spi_device_handle_t my_spi;
void lcd_spi_pre_transfer_callback(spi_transaction_t *t);
void spi_init();
void spi_cmd(spi_device_handle_t spi, const uint8_t cmd);
void spi_data(spi_device_handle_t spi, const uint8_t *data, int len);






#endif