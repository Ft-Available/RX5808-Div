/**
 * @file lv_port_disp_templ.c
 *
 */

 /*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include "../../lvgl.h"
#include "lcd.h"
#include "lvgl_driver_video.h"
/*********************
 *      DEFINES
 *********************/
#define DISP_BUF_SIZE        (MY_DISP_HOR_RES * MY_DISP_VER_RES)
lv_color_t lv_disp_buf1[DISP_BUF_SIZE];
lv_color_t lv_disp_buf2[DISP_BUF_SIZE];
//static lv_color_t lv_disp_buf3[240*140];
lv_disp_drv_t *disp_drv_spi;
lv_disp_t *default_disp = NULL;
bool g_dac_video_render = false;
uint8_t refresh_times = 0;
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
static void composite_rounder_cb(lv_disp_drv_t * disp_drv, lv_area_t * area);
void composite_monitor_cb(uint32_t time_ms, uint32_t px_num);
void composite_switch(bool flag) {
    g_dac_video_render = flag;
    if(g_dac_video_render) {
        // 注册A/V信号输出
        FRAME_BUFFER_FORMAT fb_format;
        fb_format = FB_FORMAT_RGB_16BPP;
        video_graphics(PAL_160x80, fb_format);
        refresh_times = 6;
        return;
    }
    video_stop();
}
void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /**
     * LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
     * The buffer has to be greater than 1 display row
     *
     * There are 3 buffering configurations:
     * 1. Create ONE buffer:
     *      LVGL will draw the display's content here and writes it to your display
     *
     * 2. Create TWO buffer:
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     *
     * 3. Double buffering
     *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
     *      This way LVGL will always provide the whole rendered screen in `flush_cb`
     *      and you only need to change the frame buffer's address.
     */

    /* Example for 1) */
    // static lv_disp_draw_buf_t draw_buf_dsc_1;
    // static lv_color_t buf_1[MY_DISP_HOR_RES * 10];                          /*A buffer for 10 rows*/
    // lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, MY_DISP_HOR_RES * 10);   /*Initialize the display buffer*/

    /* Example for 2) */
    static lv_disp_draw_buf_t draw_buf_dsc_2;
//    static lv_color_t buf_2_1[MY_DISP_HOR_RES * 10];                        /*A buffer for 10 rows*/
//    static lv_color_t buf_2_1[MY_DISP_HOR_RES * 10];                        /*An other buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_2, lv_disp_buf1, lv_disp_buf2, DISP_BUF_SIZE);   /*Initialize the display buffer*/
    // A/V信号绘图buffer
    lv_video_third_set_g_lvgl_aux_buf(lv_disp_buf1);
    // A/V信号显示buffer
    lv_video_third_set_draw_buf(draw_buf_dsc_2);
//    /* Example for 3) also set disp_drv.full_refresh = 1 below*/
//    static lv_disp_draw_buf_t draw_buf_dsc_3;
//    static lv_color_t buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES];            /*A screen sized buffer*/
//    static lv_color_t buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES];            /*An other screen sized buffer*/
//    lv_disp_draw_buf_init(&draw_buf_dsc_3, buf_3_1, buf_3_2, MY_DISP_VER_RES * LV_VER_RES_MAX);   /*Initialize the display buffer*/

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;
    disp_drv.rounder_cb = composite_rounder_cb;
    disp_drv.monitor_cb = composite_monitor_cb;
    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_2;

    /*Required for Example 3)*/
    //disp_drv.full_refresh = 1

    /* Fill a memory array with a color if you have GPU.
     * Note that, in lv_conf.h you can enable GPUs that has built-in support in LVGL.
     * But if you have a different GPU you can use with this callback.*/
    //disp_drv.gpu_fill_cb = gpu_fill;

    /*Finally register the driver*/
    disp_drv_spi = &disp_drv;
    default_disp = lv_disp_drv_register(disp_drv_spi);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /*You code here*/
}
#include "SPI.h"
/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
    //DAC flush
    // 加了以后画面更稳定, 但影响UI流畅度
    // video_wait_frame();
    if(g_dac_video_render) {
        lv_color_t *color_p_dac = color_p;

        register uint32_t pixel_data;
        for(int y=area->y1; y<=area->y2; ++y) {
            uint32_t* dest = (uint32_t*)(video_get_frame_buffer_address()+y*video_get_width()*2+area->x1*2);
            for(int x = area->x1; x <= area->x2; x+=2)
            {
                pixel_data = *((uint16_t*)color_p_dac);
                color_p_dac++;
                pixel_data |= *((uint16_t*)color_p_dac) << 16;
                color_p_dac++;

                *dest = pixel_data;
                dest++;
            }
        }
    }
    //LCD flush
    esp_err_t ret;
    spi_transaction_t t;
	uint16_t height,width;
	width=area->x2-area->x1+1; 			//??????????????
	height=area->y2-area->y1+1;			//????
    Address_Set(area->x1,area->y1,area->x2,area->y2);    
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=width*height*16;       //Command is 8 bits
    t.user=(void*)1;                //D/C needs to be set to 0
	t.tx_buffer=color_p;
    gpio_set_level(PIN_NUM_DC, 1);
 
    ret=spi_device_polling_transmit(my_spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
	lv_disp_flush_ready(disp_drv);
}

void composite_monitor_cb(uint32_t time_ms, uint32_t px_num) {
    // 每次切换OSD显示时, 都需要强制绘制屏幕
    if(refresh_times) {
        lv_obj_invalidate(lv_scr_act());
        --refresh_times;
    }

}
void composite_rounder_cb(lv_disp_drv_t * disp_drv, lv_area_t * area) {
    // RBG16bit的情况下，保证每次刷新的范围都32bit对齐
    area->x1 &= ~1;
    area->x2 |= 1;
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
