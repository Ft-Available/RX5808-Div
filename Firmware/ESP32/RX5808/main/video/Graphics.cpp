#include "Graphics.h"
#include "Font.h"
#include "TriangleTree.h"
#include <stdlib.h>

Graphics::Graphics(int w, int h, int initialTrinagleBufferSize)
  :xres(w), 
  yres(h) {
    font = 0;
    cursorX = cursorY = cursorBaseX = 0;
    trinagleBufferSize = initialTrinagleBufferSize;
    triangleCount = 0;
    frontColor = 0xffff;
    backColor = 0;
}

void Graphics::setTextColor(int front, int back)
{
    frontColor = front;
    backColor = back;
}

void Graphics::init() {
    backbuffer = (char**)malloc(yres * sizeof(char*));
    for(int y = 0; y < yres; y++) {
        backbuffer[y] = (char*)malloc(xres);
    }
    triangleBuffer = (TriangleTree*)malloc(sizeof(TriangleTree) * trinagleBufferSize);
}

void Graphics::setFont(Font &font) {
    this->font = &font;
}

void Graphics::setCursor(int x, int y) {
    cursorX = cursorBaseX = x;  
    cursorY = y;  
}

void Graphics::print(const char *str) {
    if(!font) return;
    while(*str) {
        if(*str >= 32 && *str < 128) {
            font->drawChar(*this, cursorX, cursorY, *str, frontColor, backColor);
        }
        cursorX += font->xres;
        if(cursorX + font->xres > xres || *str == '\n') {
            cursorX = cursorBaseX;
            cursorY += font->yres;
        }
        str++;
    }
}

void Graphics::print(int number, int base, int minCharacters) {
    bool sign = number < 0;
    if(sign) {
       number = -number;
    }
    const char baseChars[] = "0123456789ABCDEF";
    char temp[33];
    temp[32] = 0;
    int i = 31;
    do {
        temp[i--] = baseChars[number % base];
        number /= base;
    } while(number > 0);
    if(sign) {
        temp[i--] = '-';
    }
    for(;i > 31 - minCharacters; i--){
        temp[i] = ' ';
    }
    print(&temp[i + 1]);
}

void Graphics::clear(int clear) {
    if(clear > -1) {
        for(int y = 0; y < yres; y++) {
            for(int x = 0; x < xres; x++) {
                dotFast(x, y, clear);
            }
        }
    }
    triangleCount = 0;
    triangleRoot = 0;
}

void Graphics::flush() {
    if(triangleRoot) {
        triangleRoot->draw(*this);
    }
}


void Graphics::enqueueTriangle(short *v0, short *v1, short *v2, unsigned int color) {
    if(triangleCount >= trinagleBufferSize) return;
    TriangleTree &t = triangleBuffer[triangleCount++];
    t.set(v0, v1, v2, color);
    if(triangleRoot) {
        triangleRoot->add(&triangleRoot, t);
    }
    else {
        triangleRoot = &t;
    }
}

void Graphics::triangle(short *v0, short *v1, short *v2, unsigned int color) {
    short *v[3] = {v0, v1, v2};
    if(v[1][1] < v[0][1]) {
        short *vb = v[0]; v[0] = v[1]; v[1] = vb;
    }
    if(v[2][1] < v[1][1]) {
        short *vb = v[1]; v[1] = v[2]; v[2] = vb;
    }
    if(v[1][1] < v[0][1]) {
        short *vb = v[0]; v[0] = v[1]; v[1] = vb;
    }
    int y = v[0][1];
    int xac = v[0][0] << 16;
    int xab = v[0][0] << 16;
    int xbc = v[1][0] << 16;
    int xaci = 0;
    int xabi = 0;
    int xbci = 0;
  if(v[1][1] != v[0][1]) {
      xabi = ((v[1][0] - v[0][0]) << 16) / (v[1][1] - v[0][1]);
  }
  if(v[2][1] != v[0][1]) {
      xaci = ((v[2][0] - v[0][0]) << 16) / (v[2][1] - v[0][1]);
  }
  if(v[2][1] != v[1][1]) {
      xbci = ((v[2][0] - v[1][0]) << 16) / (v[2][1] - v[1][1]);
  }
  for(; y < v[1][1] && y < yres; y++) {
      if(y >= 0) {
          xLine(xab >> 16, xac >> 16, y, color);
      }
      xab += xabi;
      xac += xaci;
  }
  for(; y < v[2][1] && y < yres; y++) {
      if(y >= 0) {
          xLine(xbc >> 16, xac >> 16, y, color);
      }
      xbc += xbci;
      xac += xaci;
  }
}

void Graphics::line(int x1, int y1, int x2, int y2, unsigned int color) {
    int x, y, xe, ye;
    int dx = x2 - x1;
    int dy = y2 - y1;
    int dx1 = labs(dx);
    int dy1 = labs(dy);
    int px = 2 * dy1 - dx1;
    int py = 2 * dx1 - dy1;
    if(dy1 <= dx1) {
        if(dx >= 0) {
            x = x1;
            y = y1;
            xe = x2;
        } else {
            x = x2;
            y = y2;
            xe = x1;
        }
        dot(x, y, color);
        for(int i = 0; x < xe; i++) {
            x = x + 1;
            if(px < 0) {
                px = px + 2 * dy1;
            } else {
                if((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
                    y = y + 1;
                } else {
                    y = y - 1;
                }
                px = px + 2 *(dy1 - dx1);
            }
            dot(x, y, color);
        }
    } else {
        if(dy >= 0) {
            x = x1;
            y = y1;
            ye = y2;
        } else {
            x = x2;
            y = y2;
            ye = y1;
        }
        dot(x, y, color);
        for(int i = 0; y < ye; i++) {
            y = y + 1;
            if(py <= 0) {
                py = py + 2 * dx1;
            } else {
                if((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
                    x = x + 1;
                } else {
                    x = x - 1;
                }
                py = py + 2 * (dx1 - dy1);
            }
            dot(x, y, color);
        }
    }
}

void Graphics::fillRect(int x, int y, int w, int h, unsigned int color) {
    if(x < 0) {
        w += x;
        x = 0;
    }
    if(y < 0) {
        h += y;
        y = 0;
    }
    if(x + w > xres) {
        w = xres - x;
    }
    if(y + h > yres) {
        h = yres - y;
    }
    for(int j = y; j < y + h; j++) {
        for(int i = x; i < x + w; i++) {
            dotFast(i, j, color);
        }
    }
}

void Graphics::rect(int x, int y, int w, int h, unsigned int color) {
    fillRect(x, y, w, 1, color);
    fillRect(x, y, 1, h, color);
    fillRect(x, y + h - 1, w, 1, color);
    fillRect(x + w - 1, y, 1, h, color);
}


