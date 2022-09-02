#pragma once
#include "RGB2YUV.h"
class TriangleTree;
class Font;

class Graphics { 
public:
    int xres;
    int yres;
    char **backbuffer;
    int cursorX, cursorY, cursorBaseX;
    unsigned int frontColor, backColor;
    Font *font;
    
    TriangleTree *triangleBuffer;
    TriangleTree *triangleRoot;
    int trinagleBufferSize;
    int triangleCount;

    Graphics(int w, int h, int initialTrinagleBufferSize = 0);
    void setTextColor(int front, int back = 0);
    void init();
    void setFont(Font &font);
    void setCursor(int x, int y);
    void print(const char *str);
    void print(int number, int base = 10, int minCharacters = 1);
    
    void clear(int clear = -1);
    void flush();

    inline void dotFast(int x, int y, unsigned int color) {
        int c = RGB2YUV[color & 0xfff];
        if(y & 1) {
            backbuffer[y & ~1][x] = (backbuffer[y & ~1][x] & 0xf) | (c & 0xf0);
            backbuffer[y][x] = c >> 8;
        }
        else {
            backbuffer[y][x] = c;
            backbuffer[y | 1][x] = (backbuffer[y | 1][x] & 0xf) | ((c >> 8) & 0xf0);        
        }
    }
    
    inline void dot(int x, int y, unsigned int color) {
        if((unsigned int)x < xres && (unsigned int)y < yres) {
            int c = RGB2YUV[color & 0xfff];
            if(y & 1) {
                backbuffer[y & ~1][x] = (backbuffer[y & ~1][x] & 0xf) | (c & 0xf0);
                backbuffer[y][x] = c >> 8;
            }
            else {
                backbuffer[y][x] = c;
                backbuffer[y | 1][x] = (backbuffer[y | 1][x] & 0xf) | ((c >> 8) & 0xf0);        
            }
        }
    }
    
    inline void dotAdd(int x, int y, unsigned int color) {
        if((unsigned int)x < xres && (unsigned int)y < yres) {
            backbuffer[y][x] = (color + backbuffer[y][x]) > 54 ? 54 : color + backbuffer[y][x];
        }
    }
    
    inline int get(int x, int y) {
        if((unsigned int)x < xres && (unsigned int)y < yres) {
            return backbuffer[y][x];
        }
        return 0;
    }
    inline void xLine(int x0, int x1, int y, unsigned int color) {
        if(x0 > x1) {
            int xb = x0;
            x0 = x1;
            x1 = xb;
        }
        if(x0 < 0) x0 = 0;
        if(x1 > xres) x1 = xres;
        for(int x = x0; x < x1; x++) {
            dotFast(x, y, color);
        }
    }

    ///converting from r8g8b8 to r4g4b4a4
    inline unsigned int rgb(int r, int g, int b) {
        return (r >> 4) + (g & 0xf0) + ((b & 0xf0) << 4) + 0xf000;
    }

    ///converting from r8g8b8a8 to r4g4b4a4
    inline unsigned int rgba(int r, int g, int b, int a) {
        return (r >> 4) + (g & 0xf0) + ((b & 0xf0) << 4) + ((b & 0xf0) << 12);
    }
    void enqueueTriangle(short *v0, short *v1, short *v2, unsigned int color);
    void triangle(short *v0, short *v1, short *v2, unsigned int color); 
    void line(int x1, int y1, int x2, int y2, unsigned int color);
    void fillRect(int x, int y, int w, int h, unsigned int color);
    void rect(int x, int y, int w, int h, unsigned int color);
};

