#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "system.h"
#include "lvgl_init.h"

void app_main(void)
{

    system_init();
    lvgl_init();
    while(1){
    lv_task_handler();
    vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}
