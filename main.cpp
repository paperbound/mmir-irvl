#include <iostream>
#include <array>
#include <opencv2/opencv.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// extern "C"
// {
// #include "extra.c"
// }

using namespace std;

void myfill(uint8_t* img, int x, int y, int width, int height)
{
  if(x<0 || y<0 || x>=width || y>=height) {
    return;
  }
  int pos = y*width+x;
  if(img[pos] != 0x3) {
    return;
  }
  img[pos] = 0x0;
  myfill(img, x+1, y, width, height);
  myfill(img, x-1, y, width, height);
  myfill(img, x, y+1, width, height);
  myfill(img, x, y-1, width, height);
}

int sizefill(uint8_t* img, int x, int y, int width, int height)
{
  if(x<0 || y<0 || x>=width || y>=height) {
    return 0;
  }
  int pos = y*width+x;
  if(img[pos] != 0x3) {
    return 0;
  }
  img[pos] = 0x1c;
  return 1+sizefill(img, x+1, y, width, height)+sizefill(img, x-1, y, width, height)+sizefill(img, x, y+1, width, height)+sizefill(img, x, y-1, width, height);
}

void clearsizefill(uint8_t* img, int x, int y, int width, int height)
{
  if(x<0 || y<0 || x>=width || y>=height) {
    return;
  }
  int pos = y*width+x;
  if(img[pos] != 0x1c) {
    return;
  }
  img[pos] = 0x0;
  clearsizefill(img, x+1, y, width, height);
  clearsizefill(img, x-1, y, width, height);
  clearsizefill(img, x, y+1, width, height);
  clearsizefill(img, x, y-1, width, height);
}

typedef struct Point{
  int x;
  int y;
} Point_t;

void countorfill(uint8_t* img, int x, int y, int width, int height, vector<Point_t>& pixels)
{
  if(x<0 || y<0 || x>=width || y>=height) {
    return;
  }
  int pos = y*width+x;
  if(img[pos] != 0xe0) {
    return;
  }
  img[pos] = 0x03;
  pixels.push_back(Point_t{x, y});
  countorfill(img, x+1, y+1, width, height, pixels);
  countorfill(img, x+0, y+1, width, height, pixels);
  countorfill(img, x-1, y+1, width, height, pixels);
  countorfill(img, x+1, y+0, width, height, pixels);
  countorfill(img, x+0, y+0, width, height, pixels);
  countorfill(img, x-1, y+0, width, height, pixels);
  countorfill(img, x+1, y-1, width, height, pixels);
  countorfill(img, x+0, y-1, width, height, pixels);
  countorfill(img, x-1, y-1, width, height, pixels);
}

void countorfillred(uint8_t* img, int x, int y, int width, int height)
{
  if(x<0 || y<0 || x>=width || y>=height) {
    return;
  }
  int pos = y*width+x;
  if(img[pos] != 0x03) {
    return;
  }
  img[pos] = 0xe0;
  countorfillred(img, x+1, y+1, width, height);
  countorfillred(img, x+0, y+1, width, height);
  countorfillred(img, x-1, y+1, width, height);
  countorfillred(img, x+1, y+0, width, height);
  countorfillred(img, x+0, y+0, width, height);
  countorfillred(img, x-1, y+0, width, height);
  countorfillred(img, x+1, y-1, width, height);
  countorfillred(img, x+0, y-1, width, height);
  countorfillred(img, x-1, y-1, width, height);
}

int pdist2(Point_t a, Point_t b)
{
  int x = a.x-b.x;
  int y = a.y-b.y;
  return x*x+y*y;
}

typedef struct Line{
  int a;
  int b;
  int c;
} Line_t;

Line_t make_line(Point_t p1, Point_t p2)
{
  return Line_t{p1.y-p2.y, p2.x-p1.x, p1.x*p2.y-p2.x*p1.y};
}

int pldists(Line_t l, Point_t p)
{
  return l.a*p.x+l.b*p.y+l.c;
}

float pldist(Line_t l, Point_t p)
{
  return((float)abs(l.a*p.x+l.b*p.y+l.c))/sqrt((float)(l.a*l.a+l.b*l.b));
}

bool square(vector<Point_t>& pixels, bool ir)
{
  int size = pixels.size();
  if(size < 64) {
    return false;
  }
  Point_t c1 = pixels[0];

  int max = pdist2(c1, pixels[1]);
  int maxi = 1;
  for(int i=2; i<size; i++) {
    int d = pdist2(c1, pixels[i]);
    if(d > max) {
      max = d;
      maxi = i;
    }
  }
  Point_t c3 = pixels[maxi];

  Line_t l = make_line(c1, c3);

  max = 0;
  maxi = 0;
  int min = 0;
  int mini = 0;
  for(int i=1; i<size; i++) {
    int d = pldists(l, pixels[i]);
    if(d > max) {
      max = d;
      maxi = i;
    }
    if(d < min) {
      min = d;
      mini = i;
    }
  }
  Point_t c2 = pixels[maxi];
  Point_t c4 = pixels[mini];

  Line_t l1 = make_line(c1, c2);
  Line_t l2 = make_line(c2, c3);
  Line_t l3 = make_line(c3, c4);
  Line_t l4 = make_line(c4, c1);

  for(int i=0; i<size; i++) {
    float d1 = pldist(l1, pixels[i]);
    float d2 = pldist(l2, pixels[i]);
    float d3 = pldist(l3, pixels[i]);
    float d4 = pldist(l4, pixels[i]);
    float d = std::min({d1, d2, d3, d4});
    if(d > (ir ? 5.0 : 10.0)) {
      return false;
    }
  }
  pixels.clear();
  pixels.push_back(c1);
  pixels.push_back(c2);
  pixels.push_back(c3);
  pixels.push_back(c4);
  return true;
}

void dot(uint8_t* img, int x, int y, int width, int height, uint8_t color, unsigned int size)
{
  x -= size/2;
  y -= size/2;
  for(int j=0; j<size; j++) {
    for(int i=0; i<size; i++) {
      int nx = x+i;
      int ny = y+j;
      if(nx<0 || ny<0 || nx>=width || ny>=height) {
        continue;
      }
      img[ny*width+nx] = color;
    }
  }
}

int area2(vector<Point_t> shape)
{
  int size = shape.size();
  int sum = 0;
  for(int i=1; i<=size; i++) {
    Point_t p1 = shape[i%size];
    Point_t p2 = shape[i-1];
    sum += (p2.x-p1.x)*(p2.y+p1.y);
  }
  return abs(sum);
}

Point_t intersect(Line_t l1, Line_t l2)
{
  int div = l1.b*l2.a-l1.a*l2.b;
  if(div == 0) {
    return Point_t{INT_MAX, INT_MAX};
  }
  return Point_t{(l1.c*l2.b-l1.b*l2.c)/div, (l1.a*l2.c-l1.c*l2.a)/div};
}

bool on_line(Point_t p, Line_t l)
{
  return pldist(l, p) < 1.0;
}

array<Point_t, 8> main_logic(bool ir);

cv::Point2f operator*(cv::Mat M, const cv::Point2f& p)
{
    cv::Mat_<double> src(3/*rows*/,1 /* cols */);

    src(0,0)=p.x;
    src(1,0)=p.y;
    src(2,0)=1.0;

    cv::Mat_<double> dst = M*src; //USE MATRIX ALGEBRA
    return cv::Point2f(dst(0,0)/dst(2,0),dst(1,0)/dst(2,0));
}

void array_rotate(array<Point_t, 4>& points, int t)
{
  while(t--) {
    Point_t a = points[0];
    points[0] = points[1];
    points[1] = points[2];
    points[2] = points[3];
    points[3] = a;
  }
}

void array_mirror(array<Point_t, 4>& points)
{
  Point_t a = points[1];
  points[1] = points[3];
  points[3] = a;
}

bool direction(Point_t a, Point_t b, Point_t c)
{
  return (a.x-c.x)*(b.y-c.y)-(a.y-c.y)*(b.x-c.x) < 0;
}

void rotate_points(array<Point_t, 8>& points)
{
  array<Point_t, 4>   big_square = {points[0], points[1], points[2], points[3]};
  array<Point_t, 4> small_square = {points[4], points[5], points[6], points[7]};
  Point_t   big_center = Point_t{(  big_square[0].x+  big_square[1].x+  big_square[2].x+  big_square[3].x)/4, (  big_square[0].y+  big_square[1].y+  big_square[2].y+  big_square[3].y)/4};
  Point_t small_center = Point_t{(small_square[0].x+small_square[1].x+small_square[2].x+small_square[3].x)/4, (small_square[0].y+small_square[1].y+small_square[2].y+small_square[3].y)/4};
  int md = INT_MAX;
  int mc = -1;
  for(int i=0; i<4; i++) {
    int d = pdist2(big_square[i], small_center);
    if(d < md) {
      md = d;
      mc = i;
    }
  }
  array_rotate(big_square, mc);
  if(pdist2(big_square[3], small_center) < pdist2(big_square[1], small_center)) {
    array_mirror(big_square);
  }
  if(direction(big_square[0], big_square[1], big_center)) {
    array_mirror(big_square);
    array_rotate(big_square, 3);
  }
  md = INT_MAX;
  mc = -1;
  for(int i=0; i<4; i++) {
    int d = pdist2(small_square[i], big_square[0]);
    if(d < md) {
      md = d;
      mc = i;
    }
  }
  array_rotate(small_square, mc);
  if(pdist2(small_square[3], big_square[1]) < pdist2(small_square[1], big_square[1])) {
    array_mirror(small_square);
  }
  points = {big_square[0], big_square[1], big_square[2], big_square[3], small_square[0], small_square[1], small_square[2], small_square[3]};
}

int main()
{
  printf("a\n");
  array<Point_t, 8> ir  = main_logic(true);
  printf("a2\n");
  array<Point_t, 8> vis = main_logic(false);
  printf("b\n");
  rotate_points(ir);
  rotate_points(vis);
  printf("c\n");
  vector<cv::Point2f> a;
  vector<cv::Point2f> b;
  for(int i=0; i<4; i++) {
    a.push_back(cv::Point2f{(float)(ir[i].x), (float)(ir[i].y)});
    b.push_back(cv::Point2f{(float)(vis[i].x), (float)(vis[i].y)});
  }
  cv::Mat trans = cv::findHomography(b, a);//, CV_RANSAC);
  //printf("(%f; %f) = (%f; %f)\n", a[2].x, a[2].y, (trans*b[2]).x, (trans*b[2]).y);
  int width_ir, height_ir, bpp_ir;
  uint8_t* image_ir = stbi_load("ir.jpg", &width_ir, &height_ir, &bpp_ir, 3);
  int width_vis, height_vis, bpp_vis;
  uint8_t* image_vis = stbi_load("vis.jpg", &width_vis, &height_vis, &bpp_vis, 3);
  uint8_t* image8 = (uint8_t*)malloc(width_vis*height_vis);
  uint8_t* image8ir = (uint8_t*)malloc(width_ir*height_ir);
  uint8_t* image8vis = (uint8_t*)malloc(width_vis*height_vis);
  printf("e\n");
  for(int j=0; j<height_ir; j++) {
    for(int i=0; i<width_ir; i++) {
      uint8_t* ir_pixel_color = image_ir+(j*width_ir+i)*3;
      //      image8ir[j*width_ir+i] = rgbto8(ir_pixel_color);
    }
  }
  for(int j=0; j<height_vis; j++) {
    for(int i=0; i<width_vis; i++) {
      uint8_t* vis_pixel_color = image_vis+(j*width_vis+i)*3;
      //      image8[j*width_vis+i] = rgbto8(vis_pixel_color);
      //      image8vis[j*width_vis+i] = rgbto8(vis_pixel_color);

      //image8t[j*width_vis+i] = 0x00;
      cv::Point2f pixel_vis_cord = cv::Point2f{(float)i, (float)j};
      cv::Point2f pixel_ir_cord = trans*pixel_vis_cord;
      if(pixel_ir_cord.x < 0 || pixel_ir_cord.y < 0 || pixel_ir_cord.x > width_ir-1 || pixel_ir_cord.y > height_ir-1) {
        continue;
      }
      int x = (int)roundf(pixel_ir_cord.x);
      int y = (int)roundf(pixel_ir_cord.y); // TODO replace with someting beter like a liner combination
      uint8_t* ir_pixel_color = image_ir +(y*width_ir+x)*3;
      if(ir_pixel_color[0] > 96 && ir_pixel_color[1] > 96) {
	      //        image8[j*width_vis+i] = rgbto8(ir_pixel_color);
      }

    }
  }
  //printf("f\n");
  stbi_image_free(image_ir);
  //printf("f2\n");
  stbi_image_free(image_vis);
  //int scale = showscale8image(width_vis, height_vis, image8t, -1);
  /*
  dot(image8, vis[0].x, vis[0].y, width_vis, height_vis, 0x03, 21);
  dot(image8, vis[1].x, vis[1].y, width_vis, height_vis, 0x1c, 21);
  dot(image8, vis[2].x, vis[2].y, width_vis, height_vis, 0xe0, 21);
  dot(image8, vis[3].x, vis[3].y, width_vis, height_vis, 0xff, 21);
  *//*
  dot(image8, vis[4].x, vis[4].y, width_vis, height_vis, 0x03, 21);
  dot(image8, vis[5].x, vis[5].y, width_vis, height_vis, 0x1c, 21);
  dot(image8, vis[6].x, vis[6].y, width_vis, height_vis, 0xe0, 21);
  dot(image8, vis[7].x, vis[7].y, width_vis, height_vis, 0xff, 21);
    *//*
  dot(image8, ir[0].x, ir[0].y, width_ir, height_ir, 0x03, 7);
  dot(image8, ir[1].x, ir[1].y, width_ir, height_ir, 0x1c, 7);
  dot(image8, ir[2].x, ir[2].y, width_ir, height_ir, 0xe0, 7);
  dot(image8, ir[3].x, ir[3].y, width_ir, height_ir, 0xff, 7);
      *//*
  dot(image8, ir[4].x, ir[4].y, width_ir, height_ir, 0x03, 7);
  dot(image8, ir[5].x, ir[5].y, width_ir, height_ir, 0x1c, 7);
  dot(image8, ir[6].x, ir[6].y, width_ir, height_ir, 0xe0, 7);
  dot(image8, ir[7].x, ir[7].y, width_ir, height_ir, 0xff, 7);
        */
  //printf("g\n");
  // int scale = showscale8image(width_vis, height_vis, image8vis, -1);
  // int swidth = width_vis/scale;
  // int sheight = height_vis/scale;
  // if(swidth > width_ir && sheight > height_ir) {
  //   delay(10000);
  //   clear_screen(swidth, sheight);
  //   show8image(width_ir, height_ir, image8ir, -1);
  //   delay(10000);
  //   clear_screen(swidth, sheight);
  //   showscale8image(width_vis, height_vis, image8, -1);
  // }
  // delayandclose(10000);
  //printf("h\n");
  free(image8);
  free(image8vis);
  free(image8ir);
  //printf("i\n");
  /*setfontcolor(GREEN);
  for(int i=0; i<8; i++) {
    char str[256];
    sprintf(str, "%d", i);
    outtextxy(vis[i].x/scale, vis[i].y/scale, str);
  }
  delayandclose(20000);*/
  return 0;
}

array<Point_t, 8> main_logic(bool ir)
{
  int width, height, bpp;
  uint8_t* imagergb = stbi_load(ir ? "ir.jpg" : "vis.jpg", &width, &height, &bpp, 3);
  uint8_t* image8 = (uint8_t*)malloc(width*height);
  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      uint8_t* colorrgb = imagergb+(j*width+i)*3;
      uint8_t* color8 = image8+j*width+i;
      if((ir && !(colorrgb[2] > 64 && colorrgb[0] < 224 && colorrgb[1] < 224)) || (!ir && colorrgb[0] < 128 && colorrgb[1] < 128 && colorrgb[2] < 128)) {//96
        *color8 = 0xff;
      } else {
        *color8 = 0x00;
      }
    }
  }
  //showrgbimage(width, height, imagergb, 2000);
  stbi_image_free(imagergb);
  /*show8image(width, height, image8, 10000);
  free(image8);
  return 0;*/
  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      int pos = j*width+i;
      if(image8[pos] == 0xff) {
        if(i<1 || j<1 || i>=(width-1) || j>=(height-1)) {
          image8[pos] = 0x3;
        } else {
          image8[pos] = (image8[j*width+i+1] && image8[j*width+i-1] && image8[(j+1)*width+i] && image8[(j-1)*width+i]) ? 0x3 : 0xe0;
        }
      }
    }
  }
  for(int i=0; i<width; i++) {
    if(image8[i]==0x3) {
      image8[i]=0x0;
    }
    if(image8[i+(height-1)*width]==0x3) {
      image8[i+(height-1)*width]=0x0;
    }
  }
  for(int i=0; i<height; i++) {
    if(image8[i*width]==0x3) {
      image8[i*width]=0x0;
    }
    if(image8[i*width+(width-1)]==0x3) {
      image8[i*width+(width-1)]=0x0;
    }
  }
  for(int j=1; j<height-1; j++) {
    for(int i=1; i<width-1; i++) {
      if(image8[j*width+i] == 0x3 && !(image8[j*width+i+1] && image8[j*width+i-1] && image8[(j+1)*width+i] && image8[(j-1)*width+i])) {
        image8[j*width+i] = 0x0;
      }
    }
  }
  for(int j=height-2; j>0; j--) {
    for(int i=width-2; i>0; i--) {
      if(image8[j*width+i] == 0x3 && !(image8[j*width+i+1] && image8[j*width+i-1] && image8[(j+1)*width+i] && image8[(j-1)*width+i])) {
        image8[j*width+i] = 0x0;
      }
    }
  }
  for(int j=1; j<height-1; j++) {
    for(int i=1; i<width-1; i++) {
      if(image8[j*width+i] == 0x3 && !(image8[j*width+i+1] && image8[j*width+i-1] && image8[(j+1)*width+i] && image8[(j-1)*width+i])) {
        myfill(image8, i, j, width, height);
      }
    }
  }
  for(int j=1; j<height-1; j++) {
    for(int i=1; i<width-1; i++) {
      if(image8[j*width+i] == 0x3) {
        if(sizefill(image8, i, j, width, height) < 128) {
          clearsizefill(image8, i, j, width, height);
        }
      }
    }
  }
  for(int j=1; j<height-1; j++) {
    for(int i=1; i<width-1; i++) {
      if(image8[j*width+i] == 0xe0 && image8[j*width+i+1] != 0x1c && image8[j*width+i-1] != 0x1c && image8[(j+1)*width+i] != 0x1c && image8[(j-1)*width+i] != 0x1c) {
        image8[j*width+i] = 0x0;
      }
    }
  }
  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      if(image8[j*width+i] == 0x1c) {
        image8[j*width+i] = 0x0;
      }
    }
  }
  printf("a1\n");
  vector<vector<Point_t> > squares;
  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      if(image8[j*width+i] == 0xe0) {
        vector<Point_t> pixels;
        countorfill(image8, i, j, width, height, pixels);
        if(square(pixels, ir)) {
          squares.push_back(pixels);
          /*for(int k=0; k<4; k++) {
            dot(image8, pixels[k].x-3, pixels[k].y-3, width, height, 0x1c);
          }*/
        }
        /*vector<cv::Point2f> pixels;
        countorfill(image8, i, j, width, height, pixels);
        vector<cv::Point2f> new_pixels;
        cv::approxPolyDP(pixels, new_pixels, 3, true);
        int size = new_pixels.size();
        for(int k=0; k<size; k++) {
          dot(image8, new_pixels[k].x-3, new_pixels[k].y-3, width, height);
          }*/
        //vector<vector<cv::Point> > contours;
        //vector<cv::Vec4i> hierarchy;
        //findContours(src, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
      }
    }
  }
  /*show8image(width, height, image8, 10000);
  free(image8);
  return 0;
  {
    int size = squares.size();
    for(int i=0; i<size; i++) {
      for(int k=0; k<4; k++) {
        dot(image8, squares[i][k].x-3, squares[i][k].y-3, width, height, 0xe0);
      }
    }
  }
  show8image(width, height, image8, 10000);
  free(image8);
  return 0;*/

  //FAKE QR CODE -----------------------------------------------------------------------------------------------------------------------------
    printf("a1.5\n");

  vector<Point_t> big_square;
  vector<Point_t> small_square;
  array<Point_t, 8> ret;
  {
    int size = squares.size();
    vector<array<int, 2> > areas;
    for(int i=0; i<size; i++) {
      areas.push_back({area2(squares[i]), i});
    }
    printf("a1.75\n");

    sort(areas.begin(), areas.end());
    int last = size-1;
    int lasti = areas[last][1];
    Point_t last_center = Point_t{(squares[lasti][0].x + squares[lasti][lasti].x + squares[lasti][2].x + squares[lasti][3].x)/4, (squares[lasti][0].y + squares[lasti][1].y + squares[lasti][2].y + squares[lasti][3].y)/4};
    int closesti = -1;
    int closestd = INT_MAX;
    for(int i=0; i<size; i++) {
      if(i == lasti) {
        continue;
      }
      int x = squares[i][0].x + squares[i][1].x + squares[i][2].x + squares[i][3].x;
      int y = squares[i][0].y + squares[i][1].y + squares[i][2].y + squares[i][3].y;
      int d = pdist2(Point_t{x/4, y/4}, last_center);
      if(d < closestd) {
        closestd = d;
        closesti = i;
      }
    }
    printf("a1.95, lasti is %d and closesti %d and squares is %d big\n", lasti, closesti, squares.size());
    big_square = squares[lasti];
    small_square = squares[closesti];
    printf("a1.98\n");
    ret = {big_square[0], big_square[1], big_square[2], big_square[3], small_square[0], small_square[1], small_square[2], small_square[3]};
  }
  return ret;
  /*{
    for(int k=0; k<4; k++) {
      dot(image8, big_square[k].x-3, big_square[k].y-3, width, height, 0x1c);
      dot(image8, small_square[k].x-3, small_square[k].y-3, width, height, 0x1c);
    }
  }
  show8image(width, height, image8, 10000);
  free(image8);
  return 0;

  //KEYBORD CODE -----------------------------------------------------------------------------------------------------------------------------

  {
    int size = squares.size();
    vector<int> areas;
    for(int i=0; i<size; i++) {
      areas.push_back(area2(squares[i]));
    }
    sort(areas.begin(), areas.end());
    int reference = areas[size/2];
    int difference = reference/2;
    int max = reference+difference*2;
    int min = reference-difference;
    bool did_brake = true;
    while(did_brake) {//Yes this can be optimized, but that is for later
      did_brake = false;
      size = squares.size();
      for(int i=0; i<size; i++) {
        int area = area2(squares[i]);
        if(area > max || area < min) {
          did_brake = true;
          squares.erase(squares.begin()+i);
          break;
        }
      }
    }
  }
  int size = squares.size();
  for(int i=0; i<size; i++) {
    countorfillred(image8, squares[i][0].x, squares[i][0].y, width, height);
  }
  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      if(image8[j*width+i] == 0x03) {
        image8[j*width+i] = 0x00;
      }
    }
  }
  vector<Point_t> square_center;
  for(int i=0; i<size; i++) {
    int x = squares[i][0].x + squares[i][1].x + squares[i][2].x + squares[i][3].x;
    int y = squares[i][0].y + squares[i][1].y + squares[i][2].y + squares[i][3].y;
    square_center.push_back(Point_t{x/4, y/4});
  }
  vector<Line_t> lines;
  {
    float dist = sqrt((float)pdist2(square_center[0], squares[0][0]));
    int maxcount = 0;
    Line_t mainl;
    for(int j=0; j<size-1; j++) {
      for(int i=j+1; i<size; i++) {
        Line_t l = make_line(square_center[i], square_center[j]);
        int count;
        for(int k=0; k<size; k++) {
          float d = pldist(l, square_center[k]);
          if(d < dist) {
            count++;
          }
        }
        if(count > maxcount) {
          maxcount = count;
          mainl = l;
        }
      }
    }
    lines.push_back(mainl);
    for(int i=size-1; i>=0; i--) {
      float d = pldist(mainl, square_center[i]);
      if(d < dist) {
        square_center.erase(square_center.begin()+i);
      }
    }
    int size = square_center.size();
    maxcount = 0;
    Line_t maxl;
    while(size > 1) {
      maxcount = 0;
      for(int j=0; j<size-1; j++) {
        for(int i=j+1; i<size; i++) {
          Line_t l = make_line(square_center[i], square_center[j]);
          Point_t intsect = intersect(l, mainl);
          if(intsect.x > 0 && intsect.y > 0 && intsect.x < width && intsect.y < height) {
            continue;
          }
          int count;
          for(int k=0; k<size; k++) {
            float d = pldist(l, square_center[k]);
            if(d < dist) {
              count++;
            }
          }
          if(count > maxcount) {
            maxcount = count;
            maxl = l;
          }
        }
      }
      if(maxcount == 0) {
        printf("ERROR ((%d))\n", size);
        break;
      }
      lines.push_back(maxl);
      for(int i=size-1; i>=0; i--) {
        float d = pldist(maxl, square_center[i]);
        if(d < dist) {
          square_center.erase(square_center.begin()+i);
        }
      }
      size = square_center.size();
    }
  }
  {
    int size = lines.size();
    for(int j=0; j<height; j++) {
      for(int i=0; i<width; i++) {
        for(int k=0; k<size; k++) {
          if(on_line(Point_t{i, j}, lines[k])) {
            image8[j*width+i] = 0x03;
          }
        }
      }
    }
    size = squares.size();
    for(int i=0; i<size; i++) {
      for(int k=0; k<4; k++) {
          dot(image8, squares[i][k].x-3, squares[i][k].y-3, width, height, 0x1c);
      }
    }
  }
  //showscale8image(width, height, image8, 500);
  show8image(width, height, image8, 10000);
  free(image8);
  return 0;*/
}
