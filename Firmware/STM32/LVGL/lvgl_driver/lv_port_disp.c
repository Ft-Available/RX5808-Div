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
/*********************
 *      DEFINES
 *********************/
#define DISPLAY_BUF_SIZE        (MY_DISP_HOR_RES * MY_DISP_VER_RES )
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


static lv_disp_drv_t *lv_disp_drv;                         /*Descriptor of a display driver*/
static lv_color_t buf_2_1[DISPLAY_BUF_SIZE];                        /*A buffer for 10 rows*/
static lv_color_t buf_2_2[DISPLAY_BUF_SIZE];                        /*An other buffer for 10 rows*/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void stm32_spi_dma_init()
{
	  DMA_InitTypeDef  DMA_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); 
    DMA_DeInit(DMA2_Stream3);
    while(DMA_GetCmdStatus(DMA2_Stream3) != DISABLE) {} 

    /* 配置 DMA Stream */
    DMA_InitStructure.DMA_Channel = DMA_Channel_3;  
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (SPI1->DR); 
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)buf_2_1;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = sizeof(buf_2_1);
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
			
    //DMA_DoubleBufferModeCmd(DMA2_Stream3, ENABLE);
	  //DMA_DoubleBufferModeConfig(DMA2_Stream3, (u32)buf_2_2, DMA_Memory_0);
    DMA_Init(DMA2_Stream3, &DMA_InitStructure);
        
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE); 
    
    NVIC_EnableIRQ(DMA2_Stream3_IRQn);
    DMA_ITConfig(DMA2_Stream3, DMA_IT_TC, ENABLE);
		
		DMA_Cmd(DMA2_Stream3, DISABLE);
		

}

void stm32_spi_dma_start(const lv_area_t * area,lv_color_t *buf)
{
	uint16_t height,width;
	width=area->x2 - area->x1+1; 			//得到填充的宽度
	height=area->y2 - area->y1+1;			//高度
	LCD_Address_Set(area->x1,area->y1,area->x2,area->y2);//设置显示范围
	PAout(4)=0;    //CS
	PAout(3)=1;    //DC	
	DMA_Cmd(DMA2_Stream3, DISABLE);
	DMA2_Stream3->M0AR = (uint32_t)buf;
	DMA2_Stream3->NDTR = width*height*sizeof(lv_color_t);
	DMA_Cmd(DMA2_Stream3, ENABLE);
 
}

void DMA2_Stream3_IRQHandler(void)
{
    /*DMA发送完成中断*/
    if(DMA_GetITStatus(DMA2_Stream3, DMA_IT_TCIF3) != RESET)
    {
			  //LCD_CS_Set();	
			  PAout(4)=1;    //CS
        lv_disp_flush_ready(lv_disp_drv);
        DMA_ClearITPendingBit(DMA2_Stream3, DMA_IT_TCIF3);
    }
}



void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();
    stm32_spi_dma_init();
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
//    static lv_disp_draw_buf_t draw_buf_dsc_1;
//    static lv_color_t buf_1[MY_DISP_HOR_RES * 10];                          /*A buffer for 10 rows*/
//    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, MY_DISP_HOR_RES * 10);   /*Initialize the display buffer*/

    /* Example for 2) */
    static lv_disp_draw_buf_t draw_buf_dsc_2;
//    static lv_color_t buf_2_1[MY_DISP_HOR_RES * MY_DISP_VER_RES/2];                        /*A buffer for 10 rows*/
//    static lv_color_t buf_2_2[MY_DISP_HOR_RES * MY_DISP_VER_RES/2];                        /*An other buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2_1, buf_2_2, DISPLAY_BUF_SIZE);   /*Initialize the display buffer*/

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

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_2;

    /*Required for Example 3)*/
    //disp_drv.full_refresh = 1

    /* Fill a memory array with a color if you have GPU.
     * Note that, in lv_conf.h you can enable GPUs that has built-in support in LVGL.
     * But if you have a different GPU you can use with this callback.*/
    //disp_drv.gpu_fill_cb = gpu_fill;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /*You code here*/
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t * display_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
    lv_disp_drv=display_drv;
//    int32_t x;
//    int32_t y;
//    for(y = area->y1; y <= area->y2; y++) {
//        for(x = area->x1; x <= area->x2; x++) {
//            /*Put a pixel to the display. For example:*/
//            /*put_px(x, y, *color_p)*/
//            color_p++;
//        }
//    }
	 
//LCD_Color_Fill(area->x1,area->y1,area->x2,area->y2,(uint16_t*)color_p);
	  stm32_spi_dma_start(area,color_p);
    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    //lv_disp_flush_ready(display_drv); 
}

/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color*/
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf+=dest_width;    /*Go to the next line*/
//    }
//}


#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
