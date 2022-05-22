/*#include <iostream>
#include <array>
#include <opencv/cv.h>
#include <X11/Xlib.h> 

extern "C"
{
#include "extra.c"
}

using namespace std;*/

#include "vis.cpp"

vector<Point_t> contoure_ir(uint8_t* img, int width, int height, Point_t start) {
  vector<Point_t> ret;
  vector<Point_t> next;
  next.push_back(start);
  while(next.size()) {
    Point_t p = next.back();
    next.pop_back();
    if(outside(width, height, p)) {
      continue;
    }
    int pos = p.y*width+p.x;
    if(img[pos] != 0xe0) {
      continue;
    }
    img[pos] = 0x3;
    ret.push_back(p);
    next.push_back(Point_t{p.x-1, p.y-1});
    next.push_back(Point_t{p.x-1, p.y-0});
    next.push_back(Point_t{p.x-1, p.y+1});
    next.push_back(Point_t{p.x-0, p.y-1});
    //next.push_back(Point_t{p.x-0, p.y-0});
    next.push_back(Point_t{p.x-0, p.y+1});
    next.push_back(Point_t{p.x+1, p.y-1});
    next.push_back(Point_t{p.x+1, p.y-0});
    next.push_back(Point_t{p.x+1, p.y+1});
  }
  return ret;
}

int main(int argc, char** argv)
{
  if(argc != 1 && argc != 5) {
    printf("Wrong number of arguments\nRun with no arguments to use the default names of 'vis.jpg', 'ir.jpg', 'scale.png' and 'out.jpg' or suply all four names as arguments\n");
    return 2;
  }
  if(argc == 1) {
    name_vis = "vis.jpg";
    name_ir = "ir.jpg";
    name_scale = "scale.png";
    name_out = "out.jpg";
  } else {
    name_vis = argv[1];
    name_ir = argv[2];
    name_scale = argv[3];
    name_out = argv[4];
  }
  printf("Computing...\n");
  int width, height, bpp;
  uint8_t* imagergb = stbi_load(name_ir, &width, &height, &bpp, 3);
  uint8_t* image8 = (uint8_t*)malloc(width*height);
  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      uint8_t* colorrgb = imagergb+(j*width+i)*3;
      uint8_t* color8 = image8+j*width+i;
      if(colorrgb[0] < 128 && colorrgb[1] < 128 && colorrgb[2] < 128 && i < 272){//96
        *color8 = 0xff;
      } else {
        *color8 = 0x00;
      }
    }
  }
  stbi_image_free(imagergb);
  for(int j=1; j<height-1; j++) {
    for(int i=1; i<width-1; i++) {
      if(image8[j*width+i] == 0xff && (image8[j*width+(i-1)] == 0x00 || image8[j*width+(i+1)] == 0x00 || image8[(j-1)*width+i] == 0x00 || image8[(j+1)*width+i] == 0x00)) {
        image8[j*width+i] = 0xe0;
      }
    }
  }
  vector<array<Point_t, 4> > squares;
  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      if(image8[j*width+i] == 0xe0) {
        vector<Point_t> pixels = contoure_ir(image8, width, height, Point_t{i, j});
        sort(pixels.begin(), pixels.end(), [](Point_t a, Point_t b) {
            return a.x*a.x+a.y*a.y < b.x*b.x+b.y*b.y;
          });
        if(square(pixels, true)) {
          assert(pixels.size() == 4);
          array<Point_t, 4> cop;
          copy_n(pixels.begin(), 4, cop.begin());
          squares.push_back(cop);
        }
      } else if(image8[j*width+i] == 0xff) {
        image8[j*width+i] = 0x00;
      }
    }
  }
  free(image8);
  assert(squares.size() == 7);
  array<Point_t, 4> big_square = square_logic(squares);
  /*show8image(width, height, image8, -1);
  {
    int size = squares.size();
    for(int i=0; i<size; i++) {
      char text[256];
      sprintf(text, "%d", i);
      for(int k=0; k<4; k++) {
        outtextxy(squares[i][k].x, squares[i][k].y, text);
        //dot(image8, squares[i][k].x, squares[i][k].y, width, height, 0x1c+k, 5);
      }
    }
    for(int k=0; k<4; k++) {
      //dot(image8, big_square[k].x, big_square[k].y, width, height, 0xe0+k, 5);
    }
  }
  delayandclose(20000);*/
  //free(image8);
  //return 0;
  return main2(big_square, squares);
}

array<Point_t, 4> square_logic(vector<array<Point_t, 4> >& squares)
{
  {
    int size = squares.size();
    for(int i=0; i<size; i++) {
      if(!direction(squares[i][0], squares[i][2], squares[i][1])) {
        array_mirror(squares[i]);
      }
      assert(direction(squares[i][0], squares[i][2], squares[i][1]));
    }
  }
  array<Point_t, 4> big_square;
  {
    int size = squares.size();
    int min = INT_MAX;
    int mini = -1;
    int minci = -1;
    for(int i=0; i<size; i++) {
      int tmin = INT_MAX;
      int tminci = -1;
      for(int j=0; j<4; j++) {
        Point_t p = squares[i][j];
        int d = p.x*p.x+p.y*p.y;
        if(d < tmin) {
          tmin = d;
          tminci = j;
        }
      }
      if(tmin < min) {
        min = tmin;
        mini = i;
        minci = tminci;
      }
    }
    big_square = squares[mini];
    squares.erase(squares.begin()+mini);
    array_rotate(big_square, minci);
  }
  {
    int size = squares.size();
    for(int i=0; i<size; i++) {
      int min = INT_MAX;
      int mini = -1;
      for(int k=0; k<4; k++) {
        for(int l=0; l<4; l++) {
          int d = pdist2(squares[i][k], big_square[l]);
          if(d < min) {
            min = d;
            mini = k;
          }
        }
      }
      array_rotate(squares[i], mini);
    }
  }
  return big_square;
}
