#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
    void graph_video_set_color(int x, int y, uint8_t r,uint8_t g,uint8_t b);
    void graph_video_set_color_8bit(int x_pos, int y_pos, uint8_t c);
    void graph_video_start(bool ntsc);
    void graph_video_stop();
    void graph_video_sync();
#ifdef __cplusplus
}
#endif