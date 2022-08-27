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

#pragma once

#include "sdkconfig.h"
#include "myconf.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


#define COMPOSITE_EVENT_FRAME_END_BIT (1<<0)
#define COMPOSITE_EVENT_FRAME_VISIBLE_END_BIT (1<<1)
#define COMPOSITE_EVENT_LINE_STARTS_BIT (1<<2)

/**
 * @brief Video modes.
 * 
 */
typedef enum _VIDEO_MODE
{
    VIDEO_MODE_PAL, ///< PAL, typically Europe 50 frames/second, max 625 scan lines. 14.75MHz or 7.375 MHz.
    VIDEO_MODE_PAL_BT601, ///< As \c VIDEO_MODE_PAL but using 13.5 or 6.75 MHz.
    // put mode PAL modes here

    VIDEO_MODE_NTSC, ///< NTSC, typically USA and Japan, 60 frames/second, max 525 scan lines. 12.273 or or 6.136 MHz.
    VIDEO_MODE_NTSC_BT601, ///< As \c VIDEO_MODE_NTSC but using 13.5 or 6.75 MHz.
    // put more NTSC modes here
} VIDEO_MODE;

/**
 * @brief Predefined video_graphics modes.
 * 
 * The list includes modes for PAL and NTSC.
 * 
 */
typedef enum _GRAPHICS_MODE
{
    PAL_384x288, ///< PAL Overscan low res
    PAL_320x256, ///< PAL Amiga low res non-interlaced
    PAL_320x200, ///< PAL Commodore 64 and others
    PAL_320x192, ///< PAL Atari 8 bit "Graphics 8"
    PAL_256x192, ///< PAL ZX Spectrum, MSX

    PAL_640x200, ///< PAL Hi res video_graphics, e.g. Amstrad CPC
    PAL_640x256, ///< PAL Amiga hi res non-interlaced
    PAL_512x192, ///< Timex, Sinclair QL, MSX2
    PAL_768x288, ///< PAL Overscan hi res; 

    PAL_360x288, ///< PAL Overscan DV/BT.601 (6.75 MHz)
    PAL_720x288, ///< PAL Overscan DV/BT.601 (13.5 MHz), non-interlaced

    NTSC_256x192, ///< NTSC ZX Spectrum, MSX
    NTSC_320x192, ///< NTSC Atari 8 bit "Graphics 8"
    NTSC_320x200, ///< NTSC Commodore 64 and others
    NTSC_640x200, ///< NTSC Hi res video_graphics, e.g. Amstrad CPC

    NTSC_320x240, ///< NTSC Overscan square pixels
    NTSC_640x240, ///< NTSC Overscan, non-interlaced

    NTSC_360x240, ///< NTSC Overscan DV/BT.601 (6.75 MHz)
    NTSC_720x240, ///< NTSC Overscan DV/BT.601 (13.5 MHz), non-interlaced

    NTSC_160x80, ///< NTSC

} GRAPHICS_MODE;

typedef enum _VSYNC_PULSE_LENGTH
{
    VSYNC_PULSE_SHORT,
    VSYNC_PULSE_LONG
} VSYNC_PULSE_LENGTH;

typedef enum _DAC_FREQUENCY
{
    DAC_FREQ_PAL_14_75MHz=14750004, // 14.75 MHz PAL square pixels 640 horizontal
    DAC_FREQ_PAL_7_357MHz=7375002, // 7.375 MHz PAL square pixels 320 horizontal
    DAC_FREQ_NTSC_12_273MHz=12272720, //12.273 MHz NTSC 640 pixels
    DAC_FREQ_NTSC_6_136MHz=6136360, // 6.136 MHz NTSC 320 pixels
    DAC_FREQ_PAL_NTSC_13_5MHz=13500001, // 13.5 MHz BT.601 640 pixels
    DAC_FREQ_PAL_NTSC_6_75MHz=6750000 // 6.75 MHz BT.601 320 pixels
} DAC_FREQUENCY;

/**
 * @brief Frame buffer format.
 * 
 * This directly affects the number of colors on the screen.
 * 
 */
typedef enum _FRAME_BUFFER_FORMAT
{
    FB_FORMAT_GREY_1BPP, ///< black and white only
    FB_FORMAT_GREY_4BPP, ///< 16 shadows
    FB_FORMAT_GREY_8BPP, ///< in theory 256 shadows, in practice 77-23=54 when no voltage divider used for DAC output, or 180 with voltage divider.
    FB_FORMAT_RGB_8BPP, ///< 3-3-2 color
    FB_FORMAT_RGB_16BPP, ///< 5-6-5 color
#if CONFIG_VIDEO_ENABLE_LVGL_SUPPORT
    FB_FORMAT_LVGL_1BPP, //< 1 bit color, pixel stored in one byte. LVGL video_graphics library compatible.
#endif
} FRAME_BUFFER_FORMAT;

typedef void (*p_pixel_render_func)(void);

typedef struct _VIDEO_SIGNAL_PARAMS
{
    VIDEO_MODE video_mode;
    uint16_t width_pixels;
    uint16_t height_pixels;
    uint16_t offset_x_samples;
    uint16_t offset_y_lines;
    uint16_t hsync_samples;
    uint16_t vsync_long_samples;
    uint16_t vsync_short_samples;
    uint16_t samples_per_line;
    uint16_t front_porch_samples;
    uint16_t back_porch_samples;
    uint16_t line_duration_us; ///< One scanline duration in microseconds 
    uint16_t number_of_lines; ///< number of scanlines; different for PAL and NTSC
    uint32_t dac_frequency; ///< DAC frequency in Hz

    uint8_t* frame_buffer;
    uint8_t bits_per_pixel;
    uint32_t frame_buffer_size_bytes;
    void (*pixel_render_func)(void);

} VIDEO_SIGNAL_PARAMS;

#if CONFIG_VIDEO_DIAG_DISPLAY_TEST_FUNC

typedef enum _TEST_VIDEO_TYPE
{
    VIDEO_TEST_WHITE,
    VIDEO_TEST_CHECKERS,
    VIDEO_TEST_PM5544
} VIDEO_TEST_TYPE;

void video_test_pal(VIDEO_TEST_TYPE test);
void video_test_ntsc(VIDEO_TEST_TYPE test);

#endif


extern volatile VIDEO_SIGNAL_PARAMS g_video_signal;

void video_init(uint16_t width, uint16_t height, FRAME_BUFFER_FORMAT fb_format, VIDEO_MODE mode, bool hires_pixel_width);
uint8_t* video_get_frame_buffer_address(void);
uint8_t* video_get_frame_buffer_size(void);
uint16_t video_get_width(void);
uint16_t video_get_height(void);
void video_graphics(GRAPHICS_MODE mode, FRAME_BUFFER_FORMAT fb_format);
void video_wait_frame(void);
void video_get_mode_description(char* buffer, size_t buffer_size);
void video_stop(void);

#if CONFIG_VIDEO_DIAG_ENABLE_INTERRUPT_STATS
void video_show_stats(void);
#endif