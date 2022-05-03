#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <graphics.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

void initwindow(int x, int y)
{
  if(y>=0x10000 || x>=0x10000) {
    int gd = DETECT, gm;
    initgraph(&gd, &gm, NULL);
    return;
  }
  int gd = USER;
  int gm = (x<<16)+y;
  initgraph(&gd, &gm, NULL);
}

void putrgbpixel(int x, int y, uint8_t* color)
{
  uint8_t old_color = _fgcolor;
  _fgcolor = (color[0] & 0xe0) | ((color[1] & 0xe0)>>3) | ((color[2] & 0xc0) >> 6);
  bar(x, y, x, y);
  _fgcolor = old_color;
}

void putpixelrgb(int x, int y, uint8_t color)
{
  uint8_t old_color = _fgcolor;
  _fgcolor = color;
  bar(x, y, x, y);
  _fgcolor = old_color;
}

void delayandclose(float d)
{
  delay(d);
  closegraph();
}
