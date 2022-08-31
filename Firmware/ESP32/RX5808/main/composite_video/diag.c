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

#if CONFIG_VIDEO_DIAG_DISPLAY_TEST_FUNC
#include <esp_log.h>
#include "esp_system.h"
#include <assert.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "video.h"

static const char* TAG = "VIDEO";

#define HEADER_PIXEL(data,pixel) {\
pixel[0] = (((data[0] - 33) << 2) | ((data[1] - 33) >> 4)); \
pixel[1] = ((((data[1] - 33) & 0xF) << 4) | ((data[2] - 33) >> 2)); \
pixel[2] = ((((data[2] - 33) & 0x3) << 6) | ((data[3] - 33))); \
data += 4; \
}

#include "Philips_PM5544_384x288.h"
#include "Philips_PM5544_320x240.h"

static void display_pm554(bool pal_resolution)
{
    uint8_t pixel[3];
    uint8_t grey1,grey2;
    uint8_t mask;

    const char* p= pal_resolution ? pm5544_384x288_data : pm5544_320x240_data;
    const size_t ratio = 8/g_video_signal.bits_per_pixel;

    for(unsigned int y=0; y<g_video_signal.height_pixels; y++)
    {
        for(unsigned int x=0;x<g_video_signal.width_pixels/ratio; x++)
        {
            HEADER_PIXEL(p, pixel);
            grey1 = 0.30*pixel[0] + 0.59*pixel[1] + 0.11*pixel[2];
            
            if( g_video_signal.bits_per_pixel == 4 )
            {
                HEADER_PIXEL(p, pixel);
                grey2 = 0.30*pixel[0] + 0.59*pixel[1] + 0.11*pixel[2];

                g_video_signal.frame_buffer[y*g_video_signal.width_pixels/ratio+x] = (grey2/16) << 4 | (grey1/16);

            }
            else if(g_video_signal.bits_per_pixel == 8)
            {
                g_video_signal.frame_buffer[y*g_video_signal.width_pixels+x] = grey1;
            }
            else if(g_video_signal.bits_per_pixel == 1)
            {
                const uint8_t WHITE_LEVEL = 128;
                mask = 0;
                int i=8;
                while(i--)
                {
                    // 8 pixels
                    HEADER_PIXEL(p, pixel);
                    grey1 = 0.30*pixel[0] + 0.59*pixel[1] + 0.11*pixel[2];
                    if( grey1 >= WHITE_LEVEL )
                    {
                        mask |= 1U<<i;
                    }
                }

                g_video_signal.frame_buffer[y*g_video_signal.width_pixels/ratio+x] = mask;
            }
            else
            {
                assert(false);
            }


        }
    }
}

static void display_test_image_checkers(void)
{
    memset(g_video_signal.frame_buffer, 0, g_video_signal.frame_buffer_size_bytes);

    for (uint8_t* it = g_video_signal.frame_buffer; it < g_video_signal.frame_buffer + g_video_signal.frame_buffer_size_bytes; it += 20) 
    {
        memset(it, 0xFF, 20);
        if (((it - g_video_signal.frame_buffer) + 40) % (20 * 400) == 0)
        {
            it += 40;
        } 
        else if (((it - g_video_signal.frame_buffer) + 20) % (20 * 400) != 0)
        {
            it += 20;
        }
    }
}

static void display_test_image_white(void)
{
    memset(g_video_signal.frame_buffer, 0xFF, g_video_signal.frame_buffer_size_bytes);
}

#if CONFIG_VIDEO_DIAG_ENABLE_INTERRUPT_STATS
static void stat_loop(void)
{
	ESP_LOGI(TAG, "Stats Loop");

	int n = 5;
    while(n--)
	{
		vTaskDelay(5*1000/portTICK_PERIOD_MS); //5s
		video_show_stats();
	}
}
#endif

void video_test_pal(VIDEO_TEST_TYPE test)
{
    switch (test)
    {
    case VIDEO_TEST_CHECKERS:
        ESP_LOGI(TAG, "PAL Display test checkers");
        video_init(320, 200, FB_FORMAT_GREY_8BPP, VIDEO_MODE_PAL, false);
        display_test_image_checkers();
        break;

    case VIDEO_TEST_PM5544:
        ESP_LOGI(TAG, "PAL Display test PM5544");
        video_init(384, 288, FB_FORMAT_GREY_8BPP, VIDEO_MODE_PAL, false);
        display_pm554(true);
        break;

    default:
    case VIDEO_TEST_WHITE:
        ESP_LOGI(TAG, "PAL Display test white");
        video_init(384, 288, FB_FORMAT_GREY_8BPP, VIDEO_MODE_PAL, false);
        display_test_image_white();
        break;      
    }

#if CONFIG_VIDEO_DIAG_ENABLE_INTERRUPT_STATS
    stat_loop();
#endif
}

void video_test_ntsc(VIDEO_TEST_TYPE test)
{
    switch (test)
    {
    case VIDEO_TEST_CHECKERS:
        ESP_LOGI(TAG, "NTSC Display test checkers");
        video_init(320, 240, FB_FORMAT_GREY_8BPP, VIDEO_MODE_NTSC, false);
        display_test_image_checkers();
        break;

    case VIDEO_TEST_PM5544:
        ESP_LOGI(TAG, "NTSC Display test PM5544");
        video_init(320, 240, FB_FORMAT_GREY_8BPP, VIDEO_MODE_PAL, false);
        display_pm554(false);
        break;

    case VIDEO_TEST_WHITE:
        ESP_LOGI(TAG, "NTSC Display test white");
        video_init(320, 240, FB_FORMAT_GREY_8BPP, VIDEO_MODE_NTSC, false);
        display_test_image_white();
        break;
    }

#if CONFIG_VIDEO_DIAG_ENABLE_INTERRUPT_STATS
    stat_loop();
#endif
}

#endif
