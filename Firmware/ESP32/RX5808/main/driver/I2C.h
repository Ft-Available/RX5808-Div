#ifndef I2C_H
#define I2C_H


#include "driver/i2c.h"
#include "driver/gpio.h"


#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define WRITE_BIT I2C_MASTER_WRITE  /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ    /*!< I2C master read */
#define ACK_CHECK_EN 0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0           /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                 /*!< I2C ack value */
#define NACK_VAL 0x1                /*!< I2C nack value */
#define I2C_SLAVE_ADDR 0x28

#define io_i2c_gpio_sda  32
#define io_i2c_gpio_scl  33
#define i2c_fre  200*1000
#define i2c_port  I2C_NUM_0


void i2c_master_init(void);
// void i2c_slave_init();
esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t* data_wr, size_t size);
esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t* data_rd, size_t size);

#endif