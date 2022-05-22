/*#include <iostream>
#include <array>
#include <opencv/cv.h>
#include <X11/Xlib.h> 

extern "C"
{
#include "extra.c"
}

using namespace std;

typedef struct Point{
  int x;
  int y;
  } Point_t;*/

Point_t operator+(Point_t a, Point_t b) {
  return Point_t{a.x+b.x, a.y+b.y};
}

Point_t operator-(Point_t a, Point_t b) {
  return Point_t{a.x-b.x, a.y-b.y};
}

bool operator==(Point_t a, Point_t b) {
  return a.x == b.x && a.y == b.y;
}

Point_t operator+(Point_t a, int b) {
  return Point_t{a.x+b, a.y+b};
}

Point_t operator-(Point_t a, int b) {
  return Point_t{a.x-b, a.y-b};
}

bool outside(int width, int height, Point_t pos) {
  return pos.x < 0 || pos.y < 0 || pos.x >= width || pos.y >= height;
}

bool wak(uint8_t* img, int width, int height, Point_t pos, Point_t dir) {
  for(int i=0; i<8; i++) {
    pos = pos+dir;
    if(outside(width, height, pos)) {
      return false;
    }
    if(img[pos.y*width+pos.x]) {
      return true;
    }
  }
  return false;
}

Point_t closest(uint8_t* img, int width, int height, Point_t pos) {
  int d = 1;
  while(d < 18) {
    Point_t c = pos-d;
    if(outside(width, height, c) || outside(width, height, pos+d)) {
      return Point_t{INT_MIN, INT_MIN};
    }
    int s = d*2+1;
    for(int j=0; j<4; j++) {
      for(int i=0; i<s; i++) {
        int addx = j&1;
        int addy = 1-addx;
        int conx = j == 2;
        int cony = j == 3;
        Point_t p = Point_t{c.x+addx*i+conx*(s-1), c.y+addy*i+cony*(s-1)};
        if(img[p.y*width+p.x]) {
          return p;
        }
      }
    }
    d++;
  }
  return Point_t{INT_MIN, INT_MIN};
}

uint8_t* vis_con() {
  int width, height, bpp;
  uint8_t* imagergb = stbi_load(name_vis, &width, &height, &bpp, 3);
  //float* imagergbf = (float*)malloc(width*height*3);
  uint8_t* image8 = (uint8_t*)malloc(width*height*3);
  {
    int a = 5;
    int b = a >> 1;
    a = a*a;
    for(int j=b; j<height-b; j++) {
      for(int i=b; i<width-b; i++) {
        uint8_t* col = imagergb+(j*width+i)*3;
        //float* colf = imagergbf+(j*width+i)*3;
        uint8_t* col8 = image8+(j*width+i);
        int r3 = 0;
        uint8_t* pix[a];
        for(int r1 = -b; r1 <= b; r1++) {
          for(int r2 = -b; r2 <= b; r2++) {
            pix[r3] = imagergb+((j+r1)*width+(i+r2))*3;
            r3++;
          }
        }
        assert(r3 == a);
        float accr = 0.0;
        float accg = 0.0;
        float accb = 0.0;
        for(int k=0; k<a; k++) {
          accr += (float)(pix[k][0]);
          accg += (float)(pix[k][1]);
          accb += (float)(pix[k][2]);
        }
        accr = accr/((float)a);
        accg = accg/((float)a);
        accb = accb/((float)a);
        //colf[0] = ((float)(col[0])) + (((float)(col[0])) - accr);
        //colf[1] = ((float)(col[1])) + (((float)(col[1])) - accg);
        //colf[2] = ((float)(col[2])) + (((float)(col[2])) - accb);
        accr = ((float)(col[0])) + 4*(((float)(col[0])) - accr);
        accg = ((float)(col[1])) + 4*(((float)(col[1])) - accg);
        accb = ((float)(col[2])) + 4*(((float)(col[2])) - accb);
        float x = 32.0;
        if(accr < x && accg < x && accb < x) {
          *col8 = 0xff;
        } else {
          *col8 = 0x00;
        }
      }
    }
  }
  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      if(!image8[j*width+i]) {
        Point_t p = Point_t{i, j};
        if(wak(image8, width, height, p, Point_t{1,0}) && wak(image8, width, height, p, Point_t{-1,0}) && wak(image8, width, height, p, Point_t{0,1}) && wak(image8, width, height, p, Point_t{0,-1})) {
          image8[j*width+i] = 0xff;
        }
      }
    }
  }
  uint8_t* image8t = (uint8_t*)malloc(width*height*3);
  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      image8t[j*width+i] = image8[j*width+i];
      if(!image8[j*width+i]) {
        Point_t p = Point_t{i, j};
        Point_t c = closest(image8, width, height, p);
        if(outside(width, height, c)) {
          continue;
        }
        uint8_t* col1 = imagergb+(p.y*width+p.x)*3;
        uint8_t* col2 = imagergb+(c.y*width+c.x)*3;
        int diffr = abs(((int)(col1[0]))-((int)(col2[0])));
        int diffg = abs(((int)(col1[1]))-((int)(col2[1])));
        int diffb = abs(((int)(col1[2]))-((int)(col2[2])));
        if(diffr < 8 && diffg < 8 && diffb < 8) {
          image8t[j*width+i] = 0xff;
        }
      }
    }
  }
  stbi_image_free(imagergb);
  //show8image(width, height, image8t, 20000);
  free(image8);
  //free(image8t);
  return image8t;
}
