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
#include "myconf.h"
#if CONFIG_VIDEO_ENABLE_LVGL_SUPPORT

#pragma once

#include "video.h"

#include CONFIG_VIDEO_LVGL_INCLUDE_PATH

void lv_video_disp_init(GRAPHICS_MODE mode);
void lv_video_disp_init_buf(GRAPHICS_MODE mode, lv_color_t* pixel_buffer, uint32_t buffer_pixel_count, bool just_drv);
lv_disp_drv_t* lv_video_disp_get_drv();
#endif
