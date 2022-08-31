#include "SPI.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv_port_disp.h"
#include "../../lvgl.h"

#define SPI_BAUDRATE_84MHZ  84*1000*1000
#define SPI_BAUDRATE_80MHZ  80*1000*1000
#define SPI_BAUDRATE_42MHZ  42*1000*1000
#define SPI_BAUDRATE_21MHZ  21*1000*1000
#define SPI_BAUDRATE_12MHZ  12*1000*1000
#define SPI_BAUDRATE_6MHZ   6*1000*1000
#define SPI_BAUDRATE_3MHZ   3*1000*1000
#define SPI_BAUDRATE_2MHZ   2*1000*1000


void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}
extern lv_disp_drv_t *disp_drv_spi;
extern LV_ATTRIBUTE_FLUSH_READY void lv_disp_flush_ready(lv_disp_drv_t * disp_drv); 
void lcd_spi_transfer_completed_callback(spi_transaction_t *trans)
{
    int info=(int)trans->user;
    if(info==1)
        lv_disp_flush_ready(disp_drv_spi);
}

spi_device_handle_t my_spi;

void spi_init()
{

    esp_err_t ret;

    spi_bus_config_t buscfg={
        .miso_io_num=SPI_NUM_MISO,
        .mosi_io_num=SPI_NUM_MOSI,
        .sclk_io_num=SPI_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=160*80*2
    };
    spi_device_interface_config_t devcfg={
        .clock_speed_hz=SPI_BAUDRATE_80MHZ,     //Clock out at 10 MHz
        .mode=0,                                //SPI mode 0
        .spics_io_num=SPI_NUM_CS,               //CS pin
        .queue_size=7,                          //We want to be able to queue 7 transactions at a time
        .pre_cb=NULL,  //Specify pre-transfer callback to handle D/C line
        .post_cb=lcd_spi_transfer_completed_callback,
        //.post_cb=NULL,
        .input_delay_ns=0,
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(SPI_HOST_USER, &buscfg, SPI_DMA_CH2);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(SPI_HOST_USER, &devcfg, &my_spi);
    ESP_ERROR_CHECK(ret);
   
}

void spi_cmd(spi_device_handle_t spi, const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void spi_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}