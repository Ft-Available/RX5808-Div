#include "lvgl_init.h"
#include "page_start.h"


void lvgl_init()
{
	lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
	//lv_port_fs_init();
	page_start_create();
}

