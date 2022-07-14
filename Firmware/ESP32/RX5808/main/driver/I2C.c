#include "I2C.h"
#include <string.h>
#include "freertos/task.h"



void i2c_master_init(void)
{
    esp_err_t ret;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = io_i2c_gpio_sda,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = io_i2c_gpio_scl,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = i2c_fre,
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    ret = i2c_param_config(i2c_port, &conf);
    ESP_ERROR_CHECK(ret);

    ret = i2c_driver_install(i2c_port, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    ESP_ERROR_CHECK(ret);
}

// void i2c_slave_init()
//  {
//      int i2c_slave_port = I2C_SLAVE_NUM;
//      i2c_config_t conf_slave;
//      conf_slave.sda_io_num = i2c_gpio_sda;
//      conf_slave.sda_pullup_en = GPIO_PULLUP_ENABLE;
//      conf_slave.scl_io_num = i2c_gpio_scl;
//      conf_slave.scl_pullup_en = GPIO_PULLUP_ENABLE;
//      conf_slave.mode = I2C_MODE_SLAVE;
//      conf_slave.slave.addr_10bit_en = 0;
//      conf_slave.slave.slave_addr = ESP_SLAVE_ADDR;
//      i2c_param_config(i2c_slave_port, &conf_slave);
//      i2c_driver_install(i2c_slave_port, conf_slave.mode, I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);
//  }


esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t* data_wr, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( I2C_SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}


esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t* data_rd, size_t size)
{
    if (size == 0) {
        return ESP_OK;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( I2C_SLAVE_ADDR << 1 ) | READ_BIT, ACK_CHECK_EN);
    if (size > 1) {
        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}
