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

/// @file

#include "sdkconfig.h"
#include "myconf.h"
#if CONFIG_VIDEO_ENABLE_LVGL_SUPPORT

#include "esp_log.h"
#include "lvgl_driver_video.h"
#include "video.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"

#define LV_TICK_PERIOD_MS 1

static const char* TAG="VIDEO";

static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static lv_color_t* g_lvgl_aux_buf=NULL;

void lv_video_third_set_g_lvgl_aux_buf(lv_color_t* g){
    g_lvgl_aux_buf = g;
}
void lv_video_third_set_draw_buf(lv_disp_draw_buf_t d){
    disp_buf = d;
}
lv_disp_drv_t* lv_video_disp_get_drv() {
    return &disp_drv;
}

/**
 * @brief Rounds to 32 bit boundary.
 * 
 * \a composite_buffer_flush_cb() is more effective operating on 32 bits.
 */
void composite_rounder_cb(lv_disp_drv_t * disp_drv, lv_area_t * area)
{
#if LV_COLOR_DEPTH==1
    if(NULL == g_lvgl_aux_buf)
    {
        // 4 pixels/32 bits
        area->x1 &= ~0b11;
        area->x2 |= 0b11;
    }
    else
    {
        // 32 pixels/32 bits
        area->x1 &= ~0b11111;
        area->x2 |= 0b11111;
    }
#elif LV_COLOR_DEPTH==8
    // 4 pixels/32 bits
    area->x1 &= ~0b11;
    area->x2 |= 0b11;
#elif LV_COLOR_DEPTH==16
    // 2 pixels/32 bits
    area->x1 &= ~1;
    area->x2 |= 1;
#else
#pragma GCC error "Not supported LVGL color depth"
#endif
}

static void composite_dummy_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    ESP_LOGD(TAG, "LVGL Updating area (%u,%u)-(%u,%u)", area->x1, area->y1, area->x2, area->y2 );

#if CONFIG_LVGL_VSYNC
    video_wait_frame();
#endif

	lv_disp_flush_ready(disp_drv);
}

static void composite_buffer_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    ESP_LOGD(TAG, "LVGL Updating area (%u,%u)-(%u,%u)", area->x1, area->y1, area->x2, area->y2 );
#if CONFIG_LVGL_VSYNC
    video_wait_frame();
#endif
    register uint32_t pixel_data;

    for(int y=area->y1; y<=area->y2; y++)
    {
#if LV_COLOR_DEPTH==1
    if(NULL==g_lvgl_aux_buf)
    {
            // Framebuffer is compatible with LVGL
            // 4 pixels/uint32_t
            uint32_t* dest = (uint32_t*)(video_get_frame_buffer_address()+y*video_get_width()+area->x1);
            for(int x = area->x1; x <= area->x2; x+=4)
            {
                pixel_data = *((uint8_t*)color_p);
                color_p++;

                pixel_data |= *((uint8_t*)color_p) << 8;
                color_p++;

                pixel_data |= *((uint8_t*)color_p) << 16;
                color_p++;

                pixel_data |= *((uint8_t*)color_p) << 24;
                color_p++;

                *dest = pixel_data;

                dest++;
            }
    }
    else
    {
            // framebuffer 1 pixel/bit
            uint32_t* dest = (uint32_t*)(video_get_frame_buffer_address()+y*video_get_width()/8+area->x1/8);
            for(int x = area->x1; x <= area->x2; x+=32)
            {
                pixel_data=0;
                for(int p=31;p>=0;p--)
                {
                    if( color_p->full )
                    {
                        pixel_data |= (1<<p);
                    }
                    color_p++;
                }
                *dest = pixel_data;

                dest++;
            }
    }
#elif LV_COLOR_DEPTH==8
        // 4 pixels/uint32_t
        uint32_t* dest = (uint32_t*)(video_get_frame_buffer_address()+y*video_get_width()+area->x1);
        for(int x = area->x1; x <= area->x2; x+=4)
        {
            pixel_data = *((uint8_t*)color_p);
            color_p++;

            pixel_data |= *((uint8_t*)color_p) << 8;
            color_p++;

            pixel_data |= *((uint8_t*)color_p) << 16;
            color_p++;

            pixel_data |= *((uint8_t*)color_p) << 24;
            color_p++;

            *dest = pixel_data;

            dest++;
        }
#elif LV_COLOR_DEPTH==16
        uint32_t* dest = (uint32_t*)(video_get_frame_buffer_address()+y*video_get_width()*2+area->x1*2);
        for(int x = area->x1; x <= area->x2; x+=2)
        {
            pixel_data = *((uint16_t*)color_p);
            color_p++;
            pixel_data |= *((uint16_t*)color_p) << 16;
            color_p++;

            *dest = pixel_data;

            dest++;
        }
#endif
    }
    
	lv_disp_flush_ready(disp_drv);
}


#if CONFIG_LVGL_STATS
void composite_monitor_cb(lv_disp_drv_t * disp_drv, uint32_t time, uint32_t px)
{
    static uint32_t min=0xFFFFFFFF, max, count, avg;

    if( time < min ) min = time;
    if( time > max ) max = time;
    avg += time;
    count++;

    const int64_t period = 1000*1000; //us
    static int64_t prev = 0;
    int64_t now = esp_timer_get_time();
    if( now - prev > period )
    {
        prev = now;
        ESP_LOGD(TAG, "LVGL AVG: %u, MIN: %u, MAX: %u [ms]", avg/count, min, max);
        count = avg = max = 0;
        min = 0xFFFFFFFF;
    }
}
#endif

static void lv_tick_task(void* arg)
{
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

/**
 * @brief Initializes LVGL to work directly with framebuffer.
 * This mode does not use an extra memory for buffer but may degrade animations.
 * Tearing effect may be visible.
 * 
 * @param mode graphics mode, resolution and TV system PAL or NTSC.
 * 
 * @note This function calls \a lv_video_disp_init_buf()
 * 
 * @sa lv_video_disp_init_buf()
 */
void lv_video_disp_init(GRAPHICS_MODE mode)
{
    lv_video_disp_init_buf(mode, NULL, 0, false);
}

/**
 * @brief Initialize LVGL to use auxilary buffer.
 * Using additional buffer improves quality of animations.
 * If \a pixel_buffer is NULL LVGL will access framebuffer directly.
 * 
 * @param mode graphics mode, resolution and TV system PAL or NTSC.
 * @param pixel_buffer pointer to auxilary buffer. LVGL uses it to refresh a screen. If set to NULL LVGL access directly framebuffer.
 * @param buffer_pixel_count size of \a pixel_buffer for in pixels.
 * 
 * @sa lv_video_disp_init()
 */
void lv_video_disp_init_buf(GRAPHICS_MODE mode, lv_color_t* pixel_buffer, uint32_t buffer_pixel_count, bool just_drv)
{
    ESP_LOGI(TAG, "LVGL Init. Color depth %d", LV_COLOR_DEPTH);
    FRAME_BUFFER_FORMAT fb_format;

#if LV_COLOR_DEPTH==1
    if( NULL == g_lvgl_aux_buf )
    {
        // Framebuffer compatible with LVGL 1 pixel/byte
        fb_format = FB_FORMAT_LVGL_1BPP;
    }
    else
    {
        // More memory efficient format 8 pixels/byte but needs buffers
        fb_format = FB_FORMAT_GREY_1BPP;
    }
#elif LV_COLOR_DEPTH==8
    fb_format = FB_FORMAT_RGB_8BPP;
#elif LV_COLOR_DEPTH==16
    fb_format = FB_FORMAT_RGB_16BPP;
#else
#pragma GCC error "Not supported LVGL color depth"
#endif

    video_graphics(mode, fb_format);
    if (!just_drv) {
	    lv_init();
    }
	lv_disp_drv_init(&disp_drv);

    if( NULL == pixel_buffer)
    {
        ESP_LOGD(TAG, "LVGL updates framebuffer directly");
        lv_disp_draw_buf_init(&disp_buf, video_get_frame_buffer_address(), NULL, video_get_width()*video_get_height());
        disp_drv.direct_mode = 1; // Draw directly to framebuffer
        disp_drv.flush_cb = composite_dummy_flush_cb;
        g_lvgl_aux_buf = NULL;
    }
    else
    {
        ESP_LOGD(TAG, "LVGL updates using buffer");
        lv_disp_draw_buf_init(&disp_buf, pixel_buffer, NULL, buffer_pixel_count);
        disp_drv.flush_cb = composite_buffer_flush_cb;
        disp_drv.rounder_cb = composite_rounder_cb;
        g_lvgl_aux_buf = pixel_buffer;
    }

    disp_drv.draw_buf = &disp_buf;
    assert(disp_drv.draw_buf);

	disp_drv.draw_buf = &disp_buf;
	disp_drv.hor_res = video_get_width();
	disp_drv.ver_res = video_get_height();

#if CONFIG_LVGL_STATS
    disp_drv.monitor_cb = composite_monitor_cb;
#endif
    if (!just_drv) {
        lv_disp_drv_register(&disp_drv);
    }
    const esp_timer_create_args_t periodic_timer_args = 
	{
        .callback = &lv_tick_task,
        .name = "lvgl_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));    
}

#endif
