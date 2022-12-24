#include "24cxx.h"
#include "MyI2C.h"


void eeprom_24cxx_init()
{
	i2c_master_init();
	//soft_iic_io_init();
}

void eeprom_24cxx_read_byte_len(uint16_t addr,uint8_t *databuf,uint16_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, EEPROM_24CXX_ADDR, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, addr>>8, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, addr%256, ACK_CHECK_EN);
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, EEPROM_24CXX_READ_ADDR, ACK_CHECK_EN);
	for(int i=0;i<len;i++)
	{	        	 
		if(i<len-1)		
		i2c_master_read_byte(cmd, &databuf[i], ACK_VAL);
		else
		 i2c_master_read_byte(cmd, &databuf[i], NACK_VAL);
		 
	}
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
    //ESP_ERROR_CHECK(ret);
    i2c_cmd_link_delete(cmd);	
} 
void eeprom_24cxx_write_byte_len(uint16_t addr,uint8_t *databuf,uint16_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, EEPROM_24CXX_ADDR, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, addr>>8, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, addr%256, ACK_CHECK_EN);
	for(int i=0;i<len;i++)
	{
		 i2c_master_write_byte(cmd,databuf[i], ACK_CHECK_EN);
	}
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
    //ESP_ERROR_CHECK(ret);
    i2c_cmd_link_delete(cmd);	
}



void eeprom_24cxx_write_half_word_len(uint16_t addr,uint16_t *databuf,uint16_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, EEPROM_24CXX_ADDR, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, addr>>8, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, addr%256, ACK_CHECK_EN);
	for(int i=0;i<len;i++)
	{
		 i2c_master_write_byte(cmd,databuf[i]>>8, ACK_CHECK_EN);
         i2c_master_write_byte(cmd,databuf[i]&0xff, ACK_CHECK_EN);
	}
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
    //ESP_ERROR_CHECK(ret);
    i2c_cmd_link_delete(cmd);	
}


void eeprom_24cxx_read_half_word_len(uint16_t addr,uint16_t *databuf,uint16_t len)
{
  	uint8_t *temp=malloc(len*2);																 
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, EEPROM_24CXX_ADDR, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, addr>>8, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, addr%256, ACK_CHECK_EN);
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, EEPROM_24CXX_READ_ADDR, ACK_CHECK_EN);
	for(int i=0;i<2*len;i++)
	{	
		if(i<2*len-1)		
		i2c_master_read_byte(cmd, &temp[i], ACK_VAL);
		else
		 i2c_master_read_byte(cmd, &temp[i], NACK_VAL);		 
	}
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
    //ESP_ERROR_CHECK(ret);
    i2c_cmd_link_delete(cmd);

	for(int i=0;i<len;i++)
	{	
		 databuf[i]=temp[2*i]<<8|temp[2*i+1];
	}
	free(temp);
}







