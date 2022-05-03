#include <iostream>
#include <array>
#include <opencv/cv.h>

extern "C"
{
#include "extra.c"
}

using namespace std;

int main()
{
  /*vector<cv::Point2f> a;
  vector<cv::Point2f> b;
  cv::findHomography(a, b);*/
  int width, height, bpp;
  bool ir = false;
  uint8_t* rgb_image = stbi_load(ir ? "ir.jpg" : "vis.jpg", &width, &height, &bpp, 3);
  initwindow(width, height);
  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      uint8_t* color = rgb_image+(j*width+i)*3;
      if((ir && color[2] > 96) || (!ir && (color[0] < 128 && color[1] < 128 && color[2] < 128))) {
        putpixelrgb(i, j, 0xff);
      } else {
        putpixelrgb(i, j, 0x00);
      }
    }
  }
  stbi_image_free(rgb_image);
  delayandclose(5000);
  //cout << "Hello World" << endl;
  return 0;
}
