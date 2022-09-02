#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
    void graph_video_set_color(int x, int y, uint8_t r,uint8_t g,uint8_t b);
    void graph_video_start();
    void graph_video_stop();
#ifdef __cplusplus
}
#endif