/*
ESP32 Composite Video Library
Copyright (C) 2022 aquaticus

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "sdkconfig.h"
#include "video.h"
#include "esp_heap_caps.h"
#include "esp_attr.h"
#include "esp_intr_alloc.h"
#include "esp_err.h"
#include "soc/gpio_reg.h"
#include "soc/rtc.h"
#include "soc/soc.h"
#include "soc/i2s_struct.h"
#include "soc/i2s_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/io_mux_reg.h"
#include "esp32/rom/gpio.h"
#include "esp32/rom/lldesc.h"
#include "driver/periph_ctrl.h"
#include "driver/dac.h"
#include "driver/gpio.h"
#include "driver/i2s.h"
#include "math.h"
//#include <esp_log.h>
#include <string.h>
#include <freertos/event_groups.h>

static const char* TAG = "VIDEO";

#if CONFIG_VIDEO_DIAG_ENABLE_INTERRUPT_STATS
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define INTERRUPT_STOPWATCH_START() g_interrupt_stopwatch_start=esp_timer_get_time()
#define INTERRUPT_STOPWATCH_STOP() g_interrupt_stopwatch_delta=esp_timer_get_time()-g_interrupt_stopwatch_start; \
g_interrupt_min=MIN(g_interrupt_stopwatch_delta, g_interrupt_min); \
g_interrupt_max=MAX(g_interrupt_stopwatch_delta, g_interrupt_max);

static uint32_t g_interrupt_stopwatch_start;
static uint32_t g_interrupt_stopwatch_delta;
static uint32_t g_interrupt_min=UINT32_MAX;
static uint32_t g_interrupt_max=0;

#define PIXEL_STOPWATCH_START() g_pixel_stopwatch_start=esp_timer_get_time()
#define PIXEL_STOPWATCH_STOP() g_pixel_total_us+=esp_timer_get_time()-g_pixel_stopwatch_start; g_pixel_calls_count++

static uint32_t g_pixel_stopwatch_start;
static uint32_t g_pixel_total_us;
static uint32_t g_pixel_calls_count;

#else
#define INTERRUPT_STOPWATCH_START()
#define INTERRUPT_STOPWATCH_STOP()
#define PIXEL_STOPWATCH_START()
#define PIXEL_STOPWATCH_STOP()
#endif


#if CONFIG_VIDEO_ENABLE_DIAG_PIN

#define DIAG_PIN_HI() gpio_set_level(CONFIG_VIDEO_DIAG_PIN_NUMBER,1)
#define DIAG_PIN_LO() gpio_set_level(CONFIG_VIDEO_DIAG_PIN_NUMBER,0)

#endif

// PAL/NTSC
#define HSYNC_US 4.7
#define VSYNC_SHORT_US (HSYNC_US/2)

// No extra voltage divider on DAC output
#define DAC_LEVEL_SYNC 0 // sync pulse level 0V
#define DAC_LEVEL_BLACK 23 // black level 0.3V
#define DAC_LEVEL_WHITE 77 //white level 1V

// PAL
#define PAL_LINE_DURATION_US 64
#define PAL_FRONT_PORCH_US 1.65
#define PAL_BACK_PORCH_US 5.7
#define PAL_TOTAL_LINES_COUNT 312
#define PAL_CONST_OFFSET_Y CONFIG_VIDEO_PAL_OFFSET_Y
#define PAL_CONST_OFFSET_X 0

// NTSC
#define NTSC_LINE_DURATION_US 63.55
#define NTSC_FRONT_PORCH_US 1.5
#define NTSC_BACK_PORCH_US 4.5
#define NTSC_TOTAL_LINES_COUNT 262
#define NTSC_CONST_OFFSET_Y CONFIG_VIDEO_NTSC_OFFSET_Y
#define NTSC_CONST_OFFSET_X 0

#define US_TO_SAMPLES(time_us) (round(((double)g_video_signal.dac_frequency*time_us/1000000.0)))
#define SAMPLES_TO_US(samples) ((double)g_video_signal.line_duration_us/(double)g_video_signal.samples_per_line/(double)samples)

#define DMA_BUFFER_UINT16 ((uint16_t*)((lldesc_t*)I2S0.out_eof_des_addr)->buf)
#define DMA_BUFFER_UINT8 ((uint8_t*)((lldesc_t*)I2S0.out_eof_des_addr)->buf)
#define DMA_BUFFER_UINT32 ((uint32_t*)((lldesc_t*)I2S0.out_eof_des_addr)->buf)

static intr_handle_t i2s_interrupt_handle;
static lldesc_t DRAM_ATTR dma_buffers[2] = {0};
static int volatile g_current_scan_line = 0;
EventGroupHandle_t g_video_event_group=NULL;

DRAM_ATTR volatile VIDEO_SIGNAL_PARAMS g_video_signal;

static inline IRAM_ATTR void pal_render_scan_line(void) __attribute__((always_inline));
static inline IRAM_ATTR void signal_vertical_sync_line(VSYNC_PULSE_LENGTH first_pulse, VSYNC_PULSE_LENGTH second_pulse) __attribute__((always_inline));
static void IRAM_ATTR i2s_interrupt(void *dma_buffer_size_bytes);
static void setup_video_dac(void);

static void IRAM_ATTR render_pixels_grey_8bpp(void);
static void IRAM_ATTR render_pixels_grey_4bpp(void);
static void IRAM_ATTR render_pixels_grey_1bpp(void);
static void IRAM_ATTR render_pixels_color_8bpp(void);
static void IRAM_ATTR render_pixels_color_16bpp(void);
#if CONFIG_VIDEO_ENABLE_LVGL_SUPPORT
static void IRAM_ATTR render_pixels_lvgl_1bpp(void);
#endif

/// Set to true if video is generated and buffers allocated.
static bool g_video_initialized = false;

static void setup_video_signal(VIDEO_MODE mode, DAC_FREQUENCY dac_frequency, uint16_t width_pixels, uint16_t height_pixels, FRAME_BUFFER_FORMAT fb_format)
{
    g_video_signal.dac_frequency = (uint32_t)dac_frequency;

    if( mode == VIDEO_MODE_PAL || mode == VIDEO_MODE_PAL_BT601 )
    {
        g_video_signal.samples_per_line = US_TO_SAMPLES(PAL_LINE_DURATION_US);
        g_video_signal.front_porch_samples = US_TO_SAMPLES(PAL_FRONT_PORCH_US);
        g_video_signal.back_porch_samples = US_TO_SAMPLES(PAL_BACK_PORCH_US);
        g_video_signal.offset_x_samples = PAL_CONST_OFFSET_X;
        g_video_signal.offset_y_lines = PAL_CONST_OFFSET_Y + PAL_TOTAL_LINES_COUNT/2 - height_pixels/2;
        g_video_signal.number_of_lines = PAL_TOTAL_LINES_COUNT;
        g_video_signal.line_duration_us = PAL_LINE_DURATION_US;
    }
    else //NTSC
    {
        g_video_signal.samples_per_line = US_TO_SAMPLES(NTSC_LINE_DURATION_US);
        g_video_signal.front_porch_samples = US_TO_SAMPLES(NTSC_FRONT_PORCH_US);
        g_video_signal.back_porch_samples = US_TO_SAMPLES(NTSC_BACK_PORCH_US);
        g_video_signal.offset_x_samples = NTSC_CONST_OFFSET_X;
        g_video_signal.offset_y_lines = NTSC_CONST_OFFSET_Y + NTSC_TOTAL_LINES_COUNT/2 - height_pixels/2;
        g_video_signal.number_of_lines = NTSC_TOTAL_LINES_COUNT;
        g_video_signal.line_duration_us = NTSC_LINE_DURATION_US;
    }
    
    g_video_signal.samples_per_line &=~1; //must be even
    g_video_signal.hsync_samples = US_TO_SAMPLES(HSYNC_US);
    g_video_signal.vsync_short_samples = US_TO_SAMPLES(VSYNC_SHORT_US);
    g_video_signal.vsync_long_samples = g_video_signal.samples_per_line/2 - g_video_signal.hsync_samples;

    g_video_signal.width_pixels = width_pixels;
    g_video_signal.height_pixels = height_pixels;
    g_video_signal.offset_x_samples +=
            g_video_signal.back_porch_samples + 
            g_video_signal.hsync_samples +

           (g_video_signal.samples_per_line-
            g_video_signal.front_porch_samples-
            g_video_signal.back_porch_samples-
            g_video_signal.hsync_samples)
            /2 -
            (width_pixels/2);
    
    g_video_signal.video_mode = mode;

    switch (fb_format)
    {
        case FB_FORMAT_GREY_8BPP:
            g_video_signal.bits_per_pixel = 8;
            //ESP_LOGD(TAG, "FB format FB_FORMAT_GREY_8BPP");
            g_video_signal.pixel_render_func = render_pixels_grey_8bpp;
            break;

        case FB_FORMAT_GREY_4BPP:
            g_video_signal.bits_per_pixel = 4;
            //ESP_LOGD(TAG, "FB format FB_FORMAT_GREY_4BPP");
            g_video_signal.pixel_render_func = render_pixels_grey_4bpp;
            break;

        case FB_FORMAT_GREY_1BPP:
            g_video_signal.bits_per_pixel = 1;
            //ESP_LOGD(TAG, "FB format FB_FORMAT_GREY_1BPP");
            g_video_signal.pixel_render_func = render_pixels_grey_1bpp;
            break;

        case FB_FORMAT_RGB_8BPP:
            g_video_signal.bits_per_pixel = 8;
            //ESP_LOGD(TAG, "FB format FB_FORMAT_RGB_8BPP");
            g_video_signal.pixel_render_func = render_pixels_color_8bpp;
            break;

        case FB_FORMAT_RGB_16BPP:
            g_video_signal.bits_per_pixel = 16;
            //ESP_LOGD(TAG, "FB format FB_FORMAT_RGB_16BPP");
            g_video_signal.pixel_render_func = render_pixels_color_16bpp;
            break;

#if CONFIG_VIDEO_ENABLE_LVGL_SUPPORT
        case FB_FORMAT_LVGL_1BPP:
            g_video_signal.bits_per_pixel = 8;
            //ESP_LOGD(TAG, "FB format FB_FORMAT_LVGL_1BPP");
            g_video_signal.pixel_render_func = render_pixels_lvgl_1bpp;
            break;
#endif
        default:
            //ESP_LOGE(TAG, "Not supported frame buffer format");
            abort();
            break;
    }

    const uint8_t BITS_IN_BYTE=8;

    if( g_video_signal.bits_per_pixel <= BITS_IN_BYTE )
    {
        g_video_signal.frame_buffer_size_bytes = width_pixels*height_pixels / (BITS_IN_BYTE/g_video_signal.bits_per_pixel);
    }
    else
    {
        g_video_signal.frame_buffer_size_bytes = width_pixels*height_pixels*g_video_signal.bits_per_pixel/BITS_IN_BYTE;
    }
    //ESP_LOGD(TAG, "Bits per pixel: %u, %ux%u. FB size %u bytes ", g_video_signal.bits_per_pixel, g_video_signal.width_pixels, g_video_signal.height_pixels, g_video_signal.frame_buffer_size_bytes);

    assert(g_video_signal.frame_buffer_size_bytes%4==0); //for 32 bit access (read/write 4 bytes at once)

    const uint32_t caps = MALLOC_CAP_32BIT; //must be 8bit to allow LVGL direct framebuffer access (otherwise it can be 32bit)
    //ESP_LOGD(TAG, "Memory: total free: %u, largest block %u", heap_caps_get_free_size(caps), heap_caps_get_largest_free_block(caps));
    g_video_signal.frame_buffer = (uint8_t*)heap_caps_calloc(g_video_signal.frame_buffer_size_bytes, sizeof(uint8_t), caps);
    if(NULL == g_video_signal.frame_buffer)
    {
        //ESP_LOGE(TAG, "Failed to allocate %u bytes for frame buffer", g_video_signal.frame_buffer_size_bytes);
        heap_caps_print_heap_info(caps);
        assert(false);
    }
    //ESP_LOGI(TAG, "Allocated %u bytes for frame buffer", g_video_signal.frame_buffer_size_bytes);
}
static void __rtc_clk_apll_enable(bool enable, uint32_t sdm0, uint32_t sdm1,
        uint32_t sdm2, uint32_t o_div) 
{
    rtc_clk_apll_enable(enable);
    rtc_clk_apll_coeff_set(o_div, sdm0, sdm1, sdm2);
}
static void set_dac_frequency(void)
{
    switch(g_video_signal.dac_frequency)
    {
        case DAC_FREQ_PAL_14_75MHz:
            __rtc_clk_apll_enable(1, 0xCD, 0xCC, 0x07, 2); //= 14.750004 MHz
            //ESP_LOGI(TAG, "DAC clock configured to 14.75 MHz. PAL 640 pixels.");
            break;

        case DAC_FREQ_PAL_7_357MHz:
            __rtc_clk_apll_enable(1, 0xCD, 0xCC, 0x07, 6); //= 7.375002 MHz
            //ESP_LOGI(TAG, "DAC clock configured to 7.35 MHz. PAL 320 pixels.");
            break;

        case DAC_FREQ_NTSC_12_273MHz: //=12272720
            __rtc_clk_apll_enable(1, 209, 69, 8, 3);
            //ESP_LOGI(TAG, "DAC clock configured to 12.273 MHz. NTSC 640 pixels.");
            break;

        case DAC_FREQ_NTSC_6_136MHz: //=6.136360
            //ESP_LOGI(TAG, "DAC clock configured to 6.136 MHz. NTSC 320 pixels.");
            __rtc_clk_apll_enable(1, 209, 69, 8, 8);
            break;

        case DAC_FREQ_PAL_NTSC_13_5MHz: //=13500001
            //ESP_LOGI(TAG, "DAC clock configured to 13.5 MHz. BT.601 PAL/NTSC 640 pixels.");
            __rtc_clk_apll_enable(1, 205, 76, 20, 7);
            break;

        case DAC_FREQ_PAL_NTSC_6_75MHz: // =6.750000
            //ESP_LOGI(TAG, "DAC clock configured to 6.75 MHz. BT.601 PAL/NTSC 320 pixels.");
            __rtc_clk_apll_enable(1, 205, 76, 20, 16);
            break;

        default:
            //ESP_LOGE(TAG, "Not supported DAC frequency");
            assert(false);
            break;
    }
}

static void setup_video_dac(void)
{
	//ESP_LOGD(TAG, "DAC setup");

    const size_t dma_buffer_size_bytes = g_video_signal.samples_per_line*sizeof(uint16_t);
    //ESP_LOGD(TAG, "Computed DMA buffer size: %u", dma_buffer_size_bytes);

    periph_module_enable(PERIPH_I2S0_MODULE);
    ESP_ERROR_CHECK(esp_intr_alloc(ETS_I2S0_INTR_SOURCE, ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_IRAM, i2s_interrupt, (void*)dma_buffer_size_bytes, &i2s_interrupt_handle));
    //ESP_LOGD(TAG, "I²S interrupt configured");

    // reset conf
    I2S0.conf.val = 1;
    I2S0.conf.val = 0;
    I2S0.conf.tx_right_first = 1;
    I2S0.conf.tx_mono = 1;

    I2S0.conf2.lcd_en = 1;
    I2S0.fifo_conf.tx_fifo_mod_force_en = 1;
    I2S0.sample_rate_conf.tx_bits_mod = 16; //DAC uses MSB 8 bits of 16
    I2S0.conf_chan.tx_chan_mod = 1; //Mono mode

    I2S0.clkm_conf.clkm_div_num = 1;            // I2S clock divider's integral value.
    I2S0.clkm_conf.clkm_div_b = 0;              // Fractional clock divider's numerator value.
    I2S0.clkm_conf.clkm_div_a = 1;              // Fractional clock divider's denominator value
    I2S0.sample_rate_conf.tx_bck_div_num = 1;
    I2S0.clkm_conf.clka_en = 1;                 // use clk_apll clock
    I2S0.fifo_conf.tx_fifo_mod = 1; // 16-bit single channel data

	const size_t DMA_BUFFER_COUNT = sizeof(dma_buffers)/sizeof(lldesc_t);
    for (size_t n=0; n<DMA_BUFFER_COUNT; n++)
	{
        //ESP_LOGD(TAG, "Allocating DMA buffer: %u bytes", dma_buffer_size_bytes);
        dma_buffers[n].buf = (uint8_t*)heap_caps_calloc(dma_buffer_size_bytes, sizeof(uint8_t), MALLOC_CAP_DMA);
		assert(dma_buffers[n].buf != NULL);
        dma_buffers[n].owner = 1;
        dma_buffers[n].eof = 1;
        dma_buffers[n].length = dma_buffer_size_bytes;
        dma_buffers[n].size = dma_buffer_size_bytes;
        dma_buffers[n].empty = (uint32_t)(n==DMA_BUFFER_COUNT-1? &dma_buffers[0] : &dma_buffers[n+1]);
    }
    I2S0.out_link.addr = (uint32_t)&dma_buffers[0];
    //ESP_LOGI(TAG, "DMA buffers configured. Buffers: %u, Size: %u bytes each", DMA_BUFFER_COUNT, dma_buffer_size_bytes);

    set_dac_frequency();

    ESP_ERROR_CHECK(dac_output_enable(DAC_CHANNEL_1));
    //ESP_LOGI(TAG, "DAC output on GPIO25 (DAC_CHANNEL_1)");

    ESP_ERROR_CHECK(dac_i2s_enable());
    //ESP_LOGD(TAG, "DAC I²S enabled");

    // start transmission
    I2S0.conf.tx_start = 1;
    I2S0.int_clr.val = UINT32_MAX;
    I2S0.int_ena.out_eof = 1;
    I2S0.out_link.start = 1;

    ESP_ERROR_CHECK(esp_intr_enable(i2s_interrupt_handle));
    //ESP_LOGD(TAG, "I²S interrupt enabled");
}

/**
    @brief Stops video generation and frees resources
*/ 
void video_stop(void)
{
    //ESP_LOGD(TAG, "Composite stop");
    if( !g_video_initialized )
    {
        //ESP_LOGW(TAG, "Video is not generated. Ignored.");
        return;
    }

    // disable interrupt
    //ESP_LOGD(TAG, "Disable I²S interrupt");
    ESP_ERROR_CHECK(esp_intr_disable(i2s_interrupt_handle));
    ESP_ERROR_CHECK(esp_intr_free(i2s_interrupt_handle));

    // stop DAC
    I2S0.out_link.start = 0;

    //disable i2s DAC
    //ESP_LOGD(TAG, "Disable DAC");
    dac_i2s_disable();
    dac_output_disable(DAC_CHANNEL_1);
    
    // free DMA buffers 
    const size_t DMA_BUFFER_COUNT = sizeof(dma_buffers)/sizeof(lldesc_t);
    for(size_t i=0;i<DMA_BUFFER_COUNT;i++)
    {
        if( dma_buffers[i].buf )
        {
            //ESP_LOGD(TAG, "Free DMA buffers");
            heap_caps_free((uint8_t*)dma_buffers[i].buf);
            dma_buffers[i].buf=NULL;
        }
    }

    // disable I2S
    //ESP_LOGD(TAG, "Disable I²S module");
    periph_module_disable(PERIPH_I2S0_MODULE);

    // free frame buffer
    if( g_video_signal.frame_buffer )
    {
        //ESP_LOGD(TAG, "Free framebuffer memory");
        heap_caps_free(g_video_signal.frame_buffer);
        g_video_signal.frame_buffer=NULL;
    }

    if( g_video_event_group )
    {
        //ESP_LOGD(TAG, "Delete event group");
        vEventGroupDelete(g_video_event_group);
        g_video_event_group = NULL;
    }

    //ESP_LOGI(TAG, "Video generation stopped.");

    g_video_initialized = false;
}

/**
 * @brief Starts video generation and allocates all required resources.
 * 
 * To stop generating video call \a video_stop(). To change video mode just call
 * the function with new parameters.
 * 
 * @param width Image width in pixels.
 * @param height Image height in pixels.
 * @param fb_format Frame buffer format. This directly sets number of colors on the screen.
 * @param mode PAL or NTSC mode
 * @param hires_pixel Double horizontal density of pixels. Use for hires 640 px modes.
 * 
 * @see video_stop()
 */
void video_init(uint16_t width, uint16_t height, FRAME_BUFFER_FORMAT fb_format, VIDEO_MODE mode, bool hires_pixel)
{
    if( g_video_initialized )
    {
        video_stop();
    }

    if( NULL == g_video_event_group )
    {
        g_video_event_group = xEventGroupCreate();


        if( NULL == g_video_event_group )
        {   
            //ESP_LOGE(TAG,"Failed to create event group");

            assert(false);
            return;
        }
    }

    DAC_FREQUENCY freq;
    switch(mode)
    {
        case VIDEO_MODE_PAL:
            freq = hires_pixel ? DAC_FREQ_PAL_14_75MHz : DAC_FREQ_PAL_7_357MHz;
            break;

        case VIDEO_MODE_PAL_BT601:
            freq = hires_pixel ? DAC_FREQ_PAL_NTSC_13_5MHz : DAC_FREQ_PAL_NTSC_6_75MHz;
            break;

        case VIDEO_MODE_NTSC:
            freq = hires_pixel ? DAC_FREQ_NTSC_12_273MHz : DAC_FREQ_NTSC_6_136MHz;
            break;

        case VIDEO_MODE_NTSC_BT601:
            freq = hires_pixel ? DAC_FREQ_PAL_NTSC_13_5MHz : DAC_FREQ_PAL_NTSC_6_75MHz;
            break;

        default:
            assert(false);
            break;
    }

    setup_video_signal(mode, freq, width, height, fb_format);

    //ESP_LOGD(TAG, "Scan line duration: %d DAC samples (%.2fµs) (PAL:64µs, NTSC:63.5µs)", g_video_signal.samples_per_line, SAMPLES_TO_US(g_video_signal.samples_per_line));
    //ESP_LOGD(TAG, "HSYNC: %u samples (%.2fµs)", g_video_signal.hsync_samples,SAMPLES_TO_US(g_video_signal.hsync_samples));
    //ESP_LOGD(TAG, "VSYNC LONG: %u samples (%.2fµs)", g_video_signal.vsync_long_samples, SAMPLES_TO_US(g_video_signal.vsync_long_samples));
    //ESP_LOGD(TAG, "VSYNC SHORT: %d samples (%.2fµs)", g_video_signal.vsync_short_samples, SAMPLES_TO_US(g_video_signal.vsync_short_samples));

    //ESP_LOGD(TAG, "Offset X %u samples (%.2fµs)", g_video_signal.offset_x_samples, SAMPLES_TO_US(g_video_signal.offset_x_samples));
    //ESP_LOGD(TAG, "Offset Y %u lines", g_video_signal.offset_y_lines);
    //ESP_LOGD(TAG, "Front porch %u samples (%.2fµs)", g_video_signal.front_porch_samples, SAMPLES_TO_US(g_video_signal.front_porch_samples));
    //ESP_LOGD(TAG, "Back porch %u samples (%.2fµs)", g_video_signal.back_porch_samples, SAMPLES_TO_US(g_video_signal.back_porch_samples));

	setup_video_dac();

    //ESP_LOGD(TAG,"rtc_clk_xtal_freq_get() = %d", (int)rtc_clk_xtal_freq_get());
    //ESP_LOGI(TAG,"DAC frequency: %u Hz", (uint32_t)g_video_signal.dac_frequency);
    //ESP_LOGD(TAG,"DAC SYNC  level: %u", DAC_LEVEL_SYNC);
    //ESP_LOGD(TAG,"DAC BLACK level: %u", DAC_LEVEL_BLACK);
    //ESP_LOGD(TAG,"DAC WHITE level: %u", DAC_LEVEL_WHITE);

#if CONFIG_VIDEO_ENABLE_DIAG_PIN
    gpio_config_t io_conf;
    io_conf.intr_type=GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = 1ULL<<CONFIG_VIDEO_DIAG_PIN_NUMBER;
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	gpio_config(&io_conf);

    //ESP_LOGI(TAG,"GPIO0%d configured for diagnostic", CONFIG_VIDEO_DIAG_PIN_NUMBER);
#endif

    g_video_initialized = true;
}

static inline IRAM_ATTR void signal_vertical_sync_line(const VSYNC_PULSE_LENGTH first_pulse, const VSYNC_PULSE_LENGTH second_pulse)
{
    const int half_bytes = g_video_signal.samples_per_line; //number of bytes = samples x2, so half = samples
    int first_pulse_width_bytes =   first_pulse == VSYNC_PULSE_LONG ? g_video_signal.vsync_long_samples*2 : g_video_signal.vsync_short_samples*2;
    int second_pulse_width_bytes = second_pulse == VSYNC_PULSE_LONG ? g_video_signal.vsync_long_samples*2 : g_video_signal.vsync_short_samples*2;

    memset(DMA_BUFFER_UINT8, DAC_LEVEL_SYNC, first_pulse_width_bytes);
    memset(DMA_BUFFER_UINT8+first_pulse_width_bytes, DAC_LEVEL_BLACK, half_bytes - first_pulse_width_bytes);

    memset(DMA_BUFFER_UINT8+half_bytes, DAC_LEVEL_SYNC, second_pulse_width_bytes);
    memset(DMA_BUFFER_UINT8+half_bytes+second_pulse_width_bytes, DAC_LEVEL_BLACK, half_bytes - second_pulse_width_bytes);
}

static IRAM_ATTR inline void signal_blank_line(void)
{
    const size_t hsync = g_video_signal.hsync_samples*sizeof(uint16_t);
    memset(DMA_BUFFER_UINT8, DAC_LEVEL_SYNC, hsync);
    memset(DMA_BUFFER_UINT8+hsync, DAC_LEVEL_BLACK, (g_video_signal.samples_per_line-g_video_signal.hsync_samples)*sizeof(uint16_t));
}

static void IRAM_ATTR render_pixels_grey_8bpp(void)
{
    const uint32_t factor_x1000 = ((DAC_LEVEL_WHITE-DAC_LEVEL_BLACK)*1000)/255;
    const int fb_y = g_current_scan_line-g_video_signal.offset_y_lines;

    // use 32 bit access (4 times faster)
    //4 pixels per 32 bits
    uint32_t* p = DMA_BUFFER_UINT32+g_video_signal.offset_x_samples/2;
    uint32_t* s = (uint32_t*)g_video_signal.frame_buffer + fb_y*g_video_signal.width_pixels/4;
    size_t len = g_video_signal.width_pixels/4;

    uint32_t p4;
    while(len--)
    {
        p4 = *s;
        s++;

        uint8_t pixel1 = DAC_LEVEL_BLACK + (((p4 & 0xFF000000) >> 24) * factor_x1000)/1000;
        uint8_t pixel2 = DAC_LEVEL_BLACK + (((p4 & 0x00FF0000) >> 16) * factor_x1000)/1000;
        uint8_t pixel3 = DAC_LEVEL_BLACK + (((p4 & 0x0000FF00) >> 8 ) * factor_x1000)/1000;
        uint8_t pixel4 = DAC_LEVEL_BLACK + (((p4 & 0x000000FF) >> 0 ) * factor_x1000)/1000;

        // DAC uses MSB byte of uint16_t
        *p = pixel4 << 24 | pixel3 << 8;
        p++;

       *p = pixel2 << 24 | pixel1 << 8; 
        p++;
    }
}

#if CONFIG_VIDEO_ENABLE_LVGL_SUPPORT
/**
 * @brief Renders pixel for LVGL 1 bit per pixel compatible framebuffer.
 * 
 * LVGL actually uses one bye per pixel. The value of the byte may be
 * 1 or 0.
 * 
 */
static void IRAM_ATTR render_pixels_lvgl_1bpp(void)
{
    const int fb_y = g_current_scan_line-g_video_signal.offset_y_lines;
 
    uint32_t* p = DMA_BUFFER_UINT32+g_video_signal.offset_x_samples/2;
    uint32_t* s = (uint32_t*)g_video_signal.frame_buffer + fb_y*g_video_signal.width_pixels/4;
    size_t len = g_video_signal.width_pixels/4;

    uint32_t p4;
    while(len--)
    {
        p4 = *s;
        s++;

        uint8_t pixel1 = ((p4 & 0xFF000000) >> 24) ? DAC_LEVEL_WHITE : DAC_LEVEL_BLACK;
        uint8_t pixel2 = ((p4 & 0x00FF0000) >> 16) ? DAC_LEVEL_WHITE : DAC_LEVEL_BLACK;
        uint8_t pixel3 = ((p4 & 0x0000FF00) >> 8 ) ? DAC_LEVEL_WHITE : DAC_LEVEL_BLACK;
        uint8_t pixel4 = ((p4 & 0x000000FF) >> 0 ) ? DAC_LEVEL_WHITE : DAC_LEVEL_BLACK;

        // DAC uses MSB byte of uint16_t
        *p = pixel4 << 24 | pixel3 << 8;
        p++;

       *p = pixel2 << 24 | pixel1 << 8; 
        p++;
    }
}
#endif

/**
 * @brief Converts RGB332 color format to individual R, G, B values.
 * Values are 3 bit.
 */
#define RGB332_SPLIT(raw_byte) \
        r = raw_byte >> 5; \
        g = (raw_byte & 0b00011100 ) >> 2; \
        b = (raw_byte & 0b00000011 ) < 1; //normalize to 3 bits

/// Convers R,G,B to luma value
//#define RGB_TO_LUMA() ((r+r+b+g+g+g)/6)
#define RGB_TO_LUMA()  ((2126 * r + 7152 * g + 722 * b)/10000)

/// Converts 3 bit luma value to DAC level
#define LUMA_TO_DAC(luma) (DAC_LEVEL_BLACK + (luma*factor_x1024)/1024)

// this version converts color RBG332 to greyscale
static void IRAM_ATTR render_pixels_color_8bpp(void)
{
    const uint32_t factor_x1024 = ((DAC_LEVEL_WHITE-DAC_LEVEL_BLACK)*1024)/0b00000111;
    const int fb_y = g_current_scan_line-g_video_signal.offset_y_lines;

    // use 32 bit access (4 times faster)
    // 4 pixels per 32 bits 
    uint32_t* p = DMA_BUFFER_UINT32+g_video_signal.offset_x_samples/2;
    uint32_t* s = (uint32_t*)g_video_signal.frame_buffer + fb_y*g_video_signal.width_pixels/4;
    size_t len = g_video_signal.width_pixels/4;
    uint8_t r, g, b, luma;
    uint8_t p1, p2, p3, p4;
    uint8_t pixel1, pixel2, pixel3, pixel4;

    while(len--)
    {
        uint32_t four_pixels = *s;
        s++;

        p1 = (four_pixels & 0xFF000000) >> 24;
        p2 = (four_pixels & 0x00FF0000) >> 16;
        p3 = (four_pixels & 0x0000FF00) >> 8 ;
        p4 = (four_pixels & 0x000000FF) >> 0 ;

        RGB332_SPLIT(p1);
        luma = RGB_TO_LUMA();
        pixel1 = LUMA_TO_DAC(luma);

        RGB332_SPLIT(p2);
        luma = RGB_TO_LUMA();
        pixel2 = LUMA_TO_DAC(luma);

        RGB332_SPLIT(p3);
        luma = RGB_TO_LUMA();
        pixel3 = LUMA_TO_DAC(luma);

        RGB332_SPLIT(p4);
        luma = RGB_TO_LUMA();
        pixel4 = LUMA_TO_DAC(luma);


        // DAC uses MSB byte of uint16_t
        *p = pixel4 << 24 | pixel3 << 8;
        p++;

       *p = pixel2 << 24 | pixel1 << 8; 
        p++;
    }
}


#define RGB565_SPLIT(raw_byte) \
        r = (uint8_t)((raw_byte & 0b1111100000000000) >> 10); \
        g = (uint8_t)((raw_byte & 0b0000011111100000) >> 5); \
        b = (uint8_t)((raw_byte & 0b0000000000011111) << 1); //normalize to 6 bits

/**
 * @brief Renders pixels for RGB565 framebuffer color
 * Current version converts to greyscale.
 */
static void IRAM_ATTR render_pixels_color_16bpp(void)
{
    const uint32_t factor_x1024 = ((DAC_LEVEL_WHITE-DAC_LEVEL_BLACK)*1024)/0b00111111; //6 bit/color
    const int fb_y = g_current_scan_line-g_video_signal.offset_y_lines;

    // use 32 bit access (4 times faster)
    // 2 pixels per 32 bits
    uint32_t* p = DMA_BUFFER_UINT32+g_video_signal.offset_x_samples/2;
    uint32_t* s = (uint32_t*)g_video_signal.frame_buffer + fb_y*g_video_signal.width_pixels/2;
    size_t len = g_video_signal.width_pixels/2;
    uint8_t r, g, b, luma;
    uint16_t p1, p2;
    uint16_t pixel1, pixel2;

    while(len--)
    {
        uint32_t two_pixels = *s;
        s++;

        p1 = two_pixels >> 16;
        p2 = two_pixels & 0x0000FFFF;

        RGB565_SPLIT(p1);
        luma = RGB_TO_LUMA();
        pixel1 = LUMA_TO_DAC(luma);

        RGB565_SPLIT(p2);
        luma = RGB_TO_LUMA();
        pixel2 = LUMA_TO_DAC(luma);

        // DAC uses MSB byte of uint16_t
       *p = pixel2 << 24 | pixel1 << 8;
        p++;
    }
}


static void IRAM_ATTR render_pixels_grey_4bpp(void)
{
    const uint32_t factor_x1000 = ((DAC_LEVEL_WHITE-DAC_LEVEL_BLACK)*1000)/15;
    const int fb_y = g_current_scan_line-g_video_signal.offset_y_lines;

    // use 32 bit access (4 times faster)
    //4 pixels per 32 bits 
    uint32_t* p = DMA_BUFFER_UINT32+g_video_signal.offset_x_samples*2/4;
    uint32_t* s = (uint32_t*)g_video_signal.frame_buffer + fb_y*g_video_signal.width_pixels/8;
    size_t len = g_video_signal.width_pixels/8;

    uint32_t p4;
    while(len--)
    {
        p4 = *s;
        s++;

        uint8_t pixel1 = DAC_LEVEL_BLACK + (((p4 & 0xF0000000) >> 28) * factor_x1000)/1000;
        uint8_t pixel2 = DAC_LEVEL_BLACK + (((p4 & 0x0F000000) >> 24) * factor_x1000)/1000;
        uint8_t pixel3 = DAC_LEVEL_BLACK + (((p4 & 0x00F00000) >> 20) * factor_x1000)/1000;
        uint8_t pixel4 = DAC_LEVEL_BLACK + (((p4 & 0x000F0000) >> 16) * factor_x1000)/1000;
        uint8_t pixel5 = DAC_LEVEL_BLACK + (((p4 & 0x0000F000) >> 12) * factor_x1000)/1000;
        uint8_t pixel6 = DAC_LEVEL_BLACK + (((p4 & 0x00000F00) >> 8 ) * factor_x1000)/1000;
        uint8_t pixel7 = DAC_LEVEL_BLACK + (((p4 & 0x000000F0) >> 4 ) * factor_x1000)/1000;
        uint8_t pixel8 = DAC_LEVEL_BLACK + (((p4 & 0x0000000F) >> 0 ) * factor_x1000)/1000;

        *p = pixel8 << 24 | pixel7 << 8;
        p++;

        *p = pixel6 << 24 | pixel5 << 8; 
        p++;

        *p = pixel4 << 24 | pixel3 << 8;
        p++;

       *p = pixel2 << 24 | pixel1 << 8; 
        p++;
    }
}

static void IRAM_ATTR render_pixels_grey_1bpp(void)
{
    const int fb_y = g_current_scan_line-g_video_signal.offset_y_lines;

    uint32_t* p = DMA_BUFFER_UINT32+g_video_signal.offset_x_samples/2;
    uint32_t* s = (uint32_t*)g_video_signal.frame_buffer + fb_y*g_video_signal.width_pixels/32;
    size_t len = g_video_signal.width_pixels/32;
    size_t pixel_count;
    uint32_t p4;
    uint8_t b[2];
    int b_count;

    while(len--)
    {
        pixel_count = 32;
        p4 = *s;
        b_count = 0;
        
        while(pixel_count--)
        {
            if( p4 & (1U<<pixel_count) )
            {
                b[b_count] = DAC_LEVEL_WHITE;
            }
            else
            {
                b[b_count] = DAC_LEVEL_BLACK;
            }

            if( b_count >= 1 )
            {
                b_count=0;
                *p = b[0] << 24 | b[1] << 8;
                p++;
            }
            else
            {
                b_count++;
            }
        }
        s++;
    }
}

static inline IRAM_ATTR void pal_render_scan_line(void)
{
    static bool even_frame = true;

    if( 0 == g_current_scan_line )
    {
        even_frame = !even_frame;
#if CONFIG_VIDEO_TRIGGER_MODE_FIELD
        DIAG_PIN_HI();
#endif
        xEventGroupClearBits( g_video_event_group,
            COMPOSITE_EVENT_FRAME_END_BIT |
            COMPOSITE_EVENT_FRAME_VISIBLE_END_BIT
        );
    }

    g_current_scan_line++;


#if CONFIG_VIDEO_TRIGGER_MODE_LINE
        DIAG_PIN_HI();
#endif

    if( g_current_scan_line <= 2) // lines 1,2
    {
        signal_vertical_sync_line(VSYNC_PULSE_LONG,VSYNC_PULSE_LONG);
    }
    else if( g_current_scan_line == 3) //line 3
    {
        signal_vertical_sync_line(VSYNC_PULSE_LONG,VSYNC_PULSE_SHORT);
    }
    else if( g_current_scan_line <= 5) // lines 4,5
    {
        signal_vertical_sync_line(VSYNC_PULSE_SHORT,VSYNC_PULSE_SHORT);
    }
    else if( g_current_scan_line < g_video_signal.offset_y_lines )
    {
        signal_blank_line();
    }
    else if (g_current_scan_line < g_video_signal.offset_y_lines+g_video_signal.height_pixels)
    {
        PIXEL_STOPWATCH_START();
        signal_blank_line(); //TODO optimize this
        g_video_signal.pixel_render_func();
        PIXEL_STOPWATCH_STOP();
    }
    else if( g_current_scan_line < g_video_signal.number_of_lines - 2 ) // PAL 310 / NTSC 260
    {
        if( g_current_scan_line == g_video_signal.offset_y_lines+g_video_signal.height_pixels && even_frame )
        {
            // All visible lines passed
            xEventGroupSetBits(g_video_event_group, COMPOSITE_EVENT_FRAME_VISIBLE_END_BIT);
        }

        signal_blank_line();
    }
    else if (g_current_scan_line <= g_video_signal.number_of_lines) // PAL lines 310-312 / NTSC 260-262
    {
        signal_vertical_sync_line(VSYNC_PULSE_SHORT, VSYNC_PULSE_SHORT);
    }
    
#if CONFIG_VIDEO_TRIGGER_MODE_LINE
    DIAG_PIN_LO();
#endif

    if( g_current_scan_line >= PAL_TOTAL_LINES_COUNT ) // line 312
    {
#if CONFIG_VIDEO_TRIGGER_MODE_FIELD
        DIAG_PIN_LO();
#endif
        g_current_scan_line=0;
        if( even_frame )
        {
            xEventGroupSetBits(g_video_event_group, COMPOSITE_EVENT_FRAME_END_BIT);
        }
    }
}

static inline IRAM_ATTR void ntsc_render_scan_line(void)
{
    static bool first_field = true;

    if( 0 == g_current_scan_line )
    {
        first_field = !first_field;
#if CONFIG_VIDEO_TRIGGER_MODE_FIELD
        DIAG_PIN_HI();
#endif
        xEventGroupClearBits( g_video_event_group,
                              COMPOSITE_EVENT_FRAME_END_BIT |
                                  COMPOSITE_EVENT_FRAME_VISIBLE_END_BIT
        );
    }

    g_current_scan_line++;

#if CONFIG_VIDEO_TRIGGER_MODE_LINE
    DIAG_PIN_HI();
#endif

    if( g_current_scan_line <= 3) // lines 1,2,3
    {
        signal_vertical_sync_line(VSYNC_PULSE_SHORT,VSYNC_PULSE_SHORT);
    }
    else if( g_current_scan_line <= 6 ) //line 4,5,6
    {
        signal_vertical_sync_line(VSYNC_PULSE_LONG,VSYNC_PULSE_LONG);
    }
    else if( g_current_scan_line <= 9) // lines 7,8,9
    {
        signal_vertical_sync_line(VSYNC_PULSE_SHORT,VSYNC_PULSE_SHORT);
    }
    else if( g_current_scan_line < g_video_signal.offset_y_lines )
    {
        signal_blank_line();
    }
    else if (g_current_scan_line < g_video_signal.offset_y_lines+g_video_signal.height_pixels)
    {
        PIXEL_STOPWATCH_START();
        signal_blank_line();
        g_video_signal.pixel_render_func();
        PIXEL_STOPWATCH_STOP();
    }
    else if( g_current_scan_line < g_video_signal.number_of_lines  )
    {
        if( g_current_scan_line == g_video_signal.offset_y_lines+g_video_signal.height_pixels && first_field )
        {
            // All visible lines passed
            xEventGroupSetBits(g_video_event_group, COMPOSITE_EVENT_FRAME_VISIBLE_END_BIT);
        }

        signal_blank_line();
    }

#if CONFIG_VIDEO_TRIGGER_MODE_LINE
    DIAG_PIN_LO();
#endif

    if( g_current_scan_line >= g_video_signal.number_of_lines )
    {
#if CONFIG_VIDEO_TRIGGER_MODE_FIELD
        DIAG_PIN_LO();
#endif
        g_current_scan_line=0;
        if( !first_field )
        {
            xEventGroupSetBits(g_video_event_group, COMPOSITE_EVENT_FRAME_END_BIT);
        }
    }
}

static void IRAM_ATTR i2s_interrupt(void *dma_buffer_size_bytes)
{
	if (I2S0.int_st.out_eof)
	{
#if CONFIG_VIDEO_TRIGGER_MODE_ISR
        DIAG_PIN_HI();
#endif
        INTERRUPT_STOPWATCH_START();

        if( g_video_signal.video_mode >= VIDEO_MODE_NTSC )
            ntsc_render_scan_line();
        else
            pal_render_scan_line();

        INTERRUPT_STOPWATCH_STOP();
#if CONFIG_VIDEO_TRIGGER_MODE_ISR
        DIAG_PIN_LO();
#endif
	}

	// reset the interrupt
    I2S0.int_clr.val = I2S0.int_st.val;
}


#if CONFIG_VIDEO_DIAG_ENABLE_INTERRUPT_STATS
void video_show_stats(void)
{
    uint32_t pixel_avg_us = g_pixel_total_us/g_pixel_calls_count;

    //ESP_LOGI(TAG, "Interrupt MAX: %u µs, MIN: %u µs. Pixel AVG: %u µs", g_interrupt_max, g_interrupt_min, pixel_avg_us );

    g_pixel_total_us = g_pixel_calls_count = 0;
    g_interrupt_min = UINT32_MAX;
    g_interrupt_max = 0;
}
#endif

void video_graphics(GRAPHICS_MODE mode, FRAME_BUFFER_FORMAT fb_format)
{
    switch( mode )
    {
        case PAL_384x288:
            video_init(384, 288, fb_format, VIDEO_MODE_PAL, false);
            break;

        case PAL_768x288:
            video_init(768, 288, fb_format, VIDEO_MODE_PAL, true);
            break;

        case PAL_360x288:
            video_init(360, 288, fb_format, VIDEO_MODE_PAL_BT601, false);
            break;

        case PAL_720x288:
            video_init(720, 288, fb_format, VIDEO_MODE_PAL_BT601, true);
            break;

        case PAL_320x200:
            video_init(320, 200, fb_format, VIDEO_MODE_PAL, false);
            break;
        
        case PAL_320x192:
            video_init(320, 192, fb_format, VIDEO_MODE_PAL, false);
            break;

        case PAL_256x192:
            video_init(256, 192, fb_format, VIDEO_MODE_PAL, false);
            break;

        case PAL_512x192:
            video_init(512, 192, fb_format, VIDEO_MODE_PAL, true);
            break;

        case PAL_320x256:
            video_init(320, 256, fb_format, VIDEO_MODE_PAL, false);
            break;

        case PAL_640x256:
            video_init(640, 192, fb_format, VIDEO_MODE_PAL, true);
            break;

        case PAL_640x200:
            video_init(256, 192, fb_format, VIDEO_MODE_PAL, true);
            break;
        case PAL_160x80:
            video_init(160, 80, fb_format, VIDEO_MODE_PAL, false);
            break;
            
        case NTSC_256x192:
            video_init(256, 192, fb_format, VIDEO_MODE_NTSC, false);
            break;

        case NTSC_320x192:
            video_init(320, 192, fb_format, VIDEO_MODE_NTSC, false);
            break;

        case NTSC_320x200:
            video_init(320, 200, fb_format, VIDEO_MODE_NTSC, false);
            break;

        case NTSC_640x200:
            video_init(640, 200, fb_format, VIDEO_MODE_NTSC, true);
            break;

        case NTSC_320x240:
            video_init(320, 240, fb_format, VIDEO_MODE_NTSC, false);
            break;

        case NTSC_640x240:
            video_init(640, 240, fb_format, VIDEO_MODE_NTSC, true);
            break;

        case NTSC_360x240:
            video_init(360, 240, fb_format, VIDEO_MODE_NTSC, false);
            break;

        case NTSC_720x240:
            video_init(720, 240, fb_format, VIDEO_MODE_NTSC, true);
            break;
        case NTSC_160x80:
            video_init(160, 80, fb_format, VIDEO_MODE_NTSC, true);
            break;
        default:
            //ESP_LOGE(TAG, "Not supported video_graphics mode");
            assert(false);
    }

}

uint8_t* video_get_frame_buffer_address(void)
{
    return g_video_signal.frame_buffer;
}

uint8_t* video_get_frame_buffer_size(void)
{
    return (uint8_t*)g_video_signal.frame_buffer_size_bytes;
}

uint16_t video_get_width(void)
{
    return g_video_signal.width_pixels;
}

uint16_t video_get_height(void)
{
    return g_video_signal.height_pixels;
}

void video_wait_frame(void)
{
    const TickType_t xTicksToWait = 1000 / portTICK_PERIOD_MS;

    xEventGroupWaitBits(
            g_video_event_group,
            COMPOSITE_EVENT_FRAME_VISIBLE_END_BIT,
            pdTRUE,         // clear bits (true) or not
            pdFALSE,
            xTicksToWait ); //max 100ms

}

/**
 * @brief Get the mode description, e.g. "NTSC 320x200"
 * 
 * @param buffer pointer to buffer where to store description
 * @param buffer_size size of \a buffer. It should be minimum \b 14 bytes.
 */
void video_get_mode_description(char* buffer, size_t buffer_size)
{
    const char* mode_name;

    switch(g_video_signal.video_mode)
    {
        case VIDEO_MODE_PAL:
            mode_name = "PAL";
            break;

        case VIDEO_MODE_NTSC:
            mode_name = "NTSC";
            break;

        case VIDEO_MODE_PAL_BT601:
            mode_name = "PAL'";
            break;       

        case VIDEO_MODE_NTSC_BT601:
            mode_name = "NTSC'";
            break; 

        default:
            mode_name = "";
            break;
    }

    snprintf(buffer, buffer_size, "%s %ux%u", mode_name, g_video_signal.width_pixels, g_video_signal.height_pixels);
}
