#include "capi_video.h"
#include "SimplePALOutput.h"

#include <soc/rtc.h>
#include "Graphics.h"
#include "Font.h"
#include "math.h"

//lincude graphics and sounds
namespace font88 {
    #include "gfx/font.h"
}
Font font(8, 8, font88::pixels);

///////////////////////////
//Video configuration
//PAL MAX, half: 324x268 full: 648x536
//NTSC MAX, half: 324x224 full: 648x448
const int XRES = 320;
//const int YRES = 144;
const int YRES = 240;
Graphics graphics(XRES, YRES);

void graph_video_start() {
    //initialize composite output and graphics
    graphics.init();
    graphics.setFont(font);
    graphics.clear(0);
    SimplePALOutput::GetInstance()->start(&graphics);
}
void graph_video_set_color(int x, int y, uint8_t r,uint8_t g,uint8_t b) {
    graphics.dotFast(x, y, graphics.rgb(r, g, b));
}
void graph_video_stop() {
    SimplePALOutput::GetInstance()->stop();
}