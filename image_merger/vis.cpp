#include <iostream>
#include <array>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <opencv/cv.h>
//#include <X11/Xlib.h> 

/*extern "C"
{
#include "extra.c"
}*/

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

const char* name_vis;
const char* name_ir;
const char* name_scale;
const char* name_out;

using namespace std;

typedef struct Point{
  int x;
  int y;
} Point_t;

array<Point_t, 4> square_logic(vector<array<Point_t, 4> >& squares);

#include "con.cpp"

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
  int acc = 0;
  vector<Point_t> v;
  v.push_back(Point_t{x, y});
  while(v.size()) {
    Point_t p = v.back();
    v.pop_back();
    if(outside(width, height, p)) {
      assert(false);
      continue;
    }
    int pos = p.y*width+p.x;
    if(img[pos] != 0x03) {
      continue;
    }
    img[pos] = 0x1c;
    acc++;
    v.push_back(Point_t{p.x+1, p.y});
    v.push_back(Point_t{p.x-1, p.y});
    v.push_back(Point_t{p.x, p.y+1});
    v.push_back(Point_t{p.x, p.y-1});
  }
  return acc;
}

int sizefill_old(uint8_t* img, int x, int y, int width, int height)
{
  if(x<0 || y<0 || x>=width || y>=height) {
    return 0;
  }
  int pos = y*width+x;
  if(img[pos] != 0x3) {
    return 0;
  }
  img[pos] = 0x1c;
  int t1 = sizefill(img, x+1, y, width, height);
  if(t1 > 1024) {
    return INT_MAX;
  }
  int t2 = sizefill(img, x-1, y, width, height);
  if(t2 > 1024) {
    return INT_MAX;
  }
  int t3 = sizefill(img, x, y+1, width, height);
  if(t3 > 1024) {
    return INT_MAX;
  }
  int t4 = sizefill(img, x, y-1, width, height);
  if(t4 > 1024) {
    return INT_MAX;
  }
  return 1 + t1 + t2 + t3 + t4;
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
  assert(img[pos] != 0x03);
  img[pos] = 0x0;
  clearsizefill(img, x+1, y, width, height);
  clearsizefill(img, x-1, y, width, height);
  clearsizefill(img, x, y+1, width, height);
  clearsizefill(img, x, y-1, width, height);
}

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
  if(size < (ir ? 8 : 64)) {
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

void numberfill(uint8_t* img, int width, int heigth, uint8_t num, Point_t start) {
  vector<Point_t> next;
  uint8_t col = img[start.y*width+start.x];
  next.push_back(start);
  while(next.size()) {
    Point_t p = next.back();
    next.pop_back();
    //assert(!(outside(width, heigth, p) && col != 0x00));
    if(outside(width, heigth, p)) {
      continue;
    }
    int pos = p.y*width+p.x;
    if(img[pos] == 0xff || img[pos] == num) {
      continue;
    }
    assert(img[pos] == col);
    /*if(img[pos] != col) {
      assert(false);
      continue;
    }*/
    img[pos] = num;
    next.push_back(Point_t{p.x+1, p.y});
    next.push_back(Point_t{p.x-1, p.y});
    next.push_back(Point_t{p.x, p.y+1});
    next.push_back(Point_t{p.x, p.y-1});
  }
}

bool exist(vector<uint8_t> v, uint8_t e) {
  int s = v.size();
  for(int i=0; i<s; i++) {
    if(v[i] == e) {
      return true;
    }
  }
  return false;
}

bool exist_p(vector<Point_t> v, Point_t e) {
  int s = v.size();
  for(int i=0; i<s; i++) {
    if(v[i] == e) {
      return true;
    }
  }
  return false;
}

int exist_num(vector<uint8_t> v, uint8_t e) {
  int s = v.size();
  for(int i=0; i<s; i++) {
    if(v[i] == e) {
      return i;
    }
  }
  return -1;
}

bool ecual(vector<uint8_t> a, vector<uint8_t> b) {
  int s1 = a.size();
  int num = exist_num(b, 0xff);
  if(num != -1) {
    b.erase(b.begin()+num);
  }
  int s2 = b.size();
  if(s1 != s2) {
    return false;
  }
  std::sort(b.begin(), b.end());
  for(int i=0; i<s1; i++) {
    if(a[i] != b[i]) {
      return false;
    }
  }
  return true;
}


tuple<vector<Point_t>, int> clear_contoure(uint8_t* img, int width, int height, Point_t start) {
  vector<Point_t> ret;
  int size;
  vector<Point_t> next;
  next.push_back(start);
  uint8_t col = img[start.y*width+start.x];
  while(next.size()) {
    Point_t p = next.back();
    next.pop_back();
    /*if(outside(width, height, p)) {
      printf("(%d; %d) ~ (%d; %d)\n", p.x, p.y, width, height);
      printf("(%d; %d)\n", start.x, start.y);
      printf("%d %d %d %d\n", ret.size(), size, next.size(), col);
    }*/
    assert(!outside(width, height, p));
    int pos = p.y*width+p.x;
    if(img[pos] == 0x00) {
      continue;
    }
    if(img[pos] == 0xff) {
      ret.push_back(p);
      continue;
    }
    assert(img[pos] == col);
    size++;
    img[pos] = 0x00;
    next.push_back(Point_t{p.x+1, p.y});
    next.push_back(Point_t{p.x-1, p.y});
    next.push_back(Point_t{p.x, p.y+1});
    next.push_back(Point_t{p.x, p.y-1});
  }
  return std::make_tuple(ret, size);
}

bool alowed(vector<tuple<array<Point_t, 4>, int> > v) {
  int s = v.size();
  vector<int> test;
  for(int i=0; i<s; i++) {
    test.push_back(std::get<1>(v[i]));
  }
  assert(test.size() == s);
  sort(test.begin(), test.end());
  auto last = unique(test.begin(), test.end());
  test.erase(last, test.end());
  return test.size() == s;
}

bool next_assignment(vector<tuple<array<Point_t, 4>, int> >& s, int max) {
  int size = s.size()-1;
  int p = size;
  do{
    tuple<array<Point_t, 4>, int> t = s[p];
    int v = std::get<1>(t) + 1;
    if(v >= max) {
      std::get<1>(t) = 0;
      s[p] = t;
      p--;
      continue;
    }
    std::get<1>(t) = v;
    s[p] = t;
    if(alowed(s)) {
      return false;
    }
    p = size;
  } while(p >= 0);
  return true;
}

void print_asignment(vector<tuple<array<Point_t, 4>, int> > s) {
  int size = s.size();
  for(int i=0; i<size; i++) {
    printf("%d", get<1>(s[i]));
  }
  printf("\n");
}

/*void next_elements(vector<int>& ret, vector<Point_t> in, Point_t pos) {
  int s = in.size();
  int diffx[4] = {1, -1, 0, 0};
  int diffy[4] = {0, 0, 1, -1};
  for(int j=0; j<4; j++) {
    for(int i=0; i<s; i++) {
      if(in[i].x == pos.x + diffx[j] && in[i].y == pos.y + diffy[j]) {
        ret.push_back(i);
        break;
      }
    }
  }
}
*/

vector<uint8_t> neightbors(uint8_t* img, int width, int height, Point_t pos) {
  vector<uint8_t> pm;
  int i = pos.x;
  int j = pos.y;
  if(!outside(width, height, Point_t{i-1, j})) {
    pm.push_back(img[j*width+(i-1)]);
  }
  if(!outside(width, height, Point_t{i+1, j})) {
    pm.push_back(img[j*width+(i+1)]);
  }
  if(!outside(width, height, Point_t{i, j-1})) {
    pm.push_back(img[(j-1)*width+i]);
  }
  if(!outside(width, height, Point_t{i, j+1})) {
    pm.push_back(img[(j+1)*width+i]);
  }
  sort(pm.begin(), pm.end());
  auto last = unique(pm.begin(), pm.end());
  pm.erase(last, pm.end());
  return pm;
}

vector<Point_t> new_contor(uint8_t* img, int width, int height, Point_t start, vector<Point_t> v, vector<uint8_t> col) {
  vector<Point_t> ret;
  vector<Point_t> list;
  list.push_back(Point_t{start.x-1, start.y-1});
  list.push_back(Point_t{start.x-1, start.y-0});
  list.push_back(Point_t{start.x-1, start.y+1});
  list.push_back(Point_t{start.x-0, start.y-1});
  //list.push_back(Point_t{start.x-0, start.y-0});
  list.push_back(Point_t{start.x-0, start.y+1});
  list.push_back(Point_t{start.x+1, start.y-1});
  list.push_back(Point_t{start.x+1, start.y-0});
  list.push_back(Point_t{start.x+1, start.y+1});
  ret.push_back(start);
  while(list.size()) {
    Point_t p = list.back();
    list.pop_back();
    if(outside(width, height, p)) {
      continue;
    }
    if(img[p.y*width+p.x] != 0xff) {
      continue;
    }
    /*if(!ecual(col, neightbors(img, width, height, p))) {
      continue;
    }*///?????????????????????????????????????????????????????????????????????????????????????????????????????
    if(exist_p(ret, p)) {
      continue;
    }
    if(!exist_p(v, p)) {
      continue;
    }
    ret.push_back(p);
    list.push_back(Point_t{p.x-1, p.y-1});
    list.push_back(Point_t{p.x-1, p.y-0});
    list.push_back(Point_t{p.x-1, p.y+1});
    list.push_back(Point_t{p.x-0, p.y-1});
    //list.push_back(Point_t{p.x-0, p.y-0});
    list.push_back(Point_t{p.x-0, p.y+1});
    list.push_back(Point_t{p.x+1, p.y-1});
    list.push_back(Point_t{p.x+1, p.y-0});
    list.push_back(Point_t{p.x+1, p.y+1});
  }
  return ret;
}

bool angle(Point_t a, Point_t b) {
  float x1 = (float)(a.x);
  float x2 = (float)(b.x);
  float y1 = (float)(a.y);
  float y2 = (float)(b.y);
  float v = (x1*x2+y1*y2)/(sqrt(x1*x1+y1*y1)*sqrt(x2*x2+y2*y2));
  return -0.5 < v && v < 0.5;
  //return true;
}

vector<array<Point_t, 4> > squares_in_square(vector<array<Point_t, 4> > squares, array<Point_t, 4> square) {
  vector<array<Point_t, 4> > ret;
  int minx = INT_MAX;
  int maxx = INT_MIN;
  int miny = INT_MAX;
  int maxy = INT_MIN;
  for(int i=0; i<4; i++) {
    if(square[i].x < minx) {
      minx = square[i].x;
    }
    if(square[i].x > maxx) {
      maxx = square[i].x;
    }
    if(square[i].y < miny) {
      miny = square[i].y;
    }
    if(square[i].y > maxy) {
      maxy = square[i].y;
    }
  }
  int size = squares.size();
  for(int i=0; i<size; i++) {
    int x = 0;
    int y = 0;
    for(int j=0; j<4; j++) {
      x += squares[i][j].x;
      y += squares[i][j].y;
    }
    x = x >> 2;
    y = y >> 2;
    if(minx < x && x < maxx && miny < y && y < maxy) {
      ret.push_back(squares[i]);
    }
  }
  return ret;
}

void merge(array<Point_t, 4> vis, array<Point_t, 4> ir) {
  vector<cv::Point2f> a;
  vector<cv::Point2f> b;
  for(int i=0; i<4; i++) {
    a.push_back(cv::Point2f{(float)(ir[i].x), (float)(ir[i].y)});
    b.push_back(cv::Point2f{(float)(vis[i].x), (float)(vis[i].y)});
  }
  cv::Mat trans = cv::findHomography(b, a, CV_RANSAC);
  printf("(%f; %f) = (%f; %f)\n", a[2].x, a[2].y, (trans*b[2]).x, (trans*b[2]).y);
  int width_ir, height_ir, bpp_ir;
  uint8_t* image_ir = stbi_load(name_ir, &width_ir, &height_ir, &bpp_ir, 3);
  int width_vis, height_vis, bpp_vis;
  uint8_t* image_vis = stbi_load(name_vis, &width_vis, &height_vis, &bpp_vis, 3);
  int width_scl, height_scl, bpp_scl;
  uint8_t* image_scl = stbi_load(name_scale, &width_scl, &height_scl, &bpp_scl, 3);
  for(int j=0; j<height_vis; j++) {
    for(int i=0; i<width_vis; i++) {
      uint8_t* vis_pixel_color = image_vis+(j*width_vis+i)*3;
      cv::Point2f pixel_vis_cord = cv::Point2f{(float)i, (float)j};
      cv::Point2f pixel_ir_cord = trans*pixel_vis_cord;
      if(pixel_ir_cord.x < 0 || pixel_ir_cord.y < 0 || pixel_ir_cord.x > width_ir-1 || pixel_ir_cord.y > height_ir-1) {
        continue;
      }
      int x = (int)roundf(pixel_ir_cord.x);
      int y = (int)roundf(pixel_ir_cord.y); // TODO replace with someting beter like a liner combination
      uint8_t* ir_pixel_color = image_ir+(y*width_ir+x)*3;
      if(ir_pixel_color[0] < 96 && ir_pixel_color[1] < 96 && ir_pixel_color[2] < 96) {
        int sum = ir_pixel_color[0] + ir_pixel_color[1] + ir_pixel_color[2];
        int scl_y = (int)roundf((((float)sum)/765.0)*((float)height_scl));
        if(scl_y < 0) {
          scl_y = 0;
        }
        if(scl_y >= height_scl) {
          scl_y = height_scl-1;
        }
        uint8_t* scl_pixel_color = image_scl+(scl_y*width_scl+(width_scl >> 1))*3;
        vis_pixel_color[0] = scl_pixel_color[0];
        vis_pixel_color[1] = scl_pixel_color[1];
        vis_pixel_color[2] = scl_pixel_color[2];
      }
    }
  }
  printf("q\n");
  stbi_write_png("out.png", width_vis, height_vis, 3, image_vis, width_vis*3);
  printf("s\n");
  stbi_image_free(image_scl);
  stbi_image_free(image_vis);
  stbi_image_free(image_ir);
  printf("Done\n");
  return;
}

float corner(Point_t p, array<Point_t, 4> square) {
  int min = INT_MAX;
  for(int i=0; i<4; i++) {
    int d = pdist2(p, square[i]);
    if(d < min) {
      min = d;
    }
  }
  return sqrt((float)min);
}

float length(array<Point_t, 4> square) {
  float acc = 0.0;
  for(int i=0; i<4; i++) {
    acc += sqrt((float)pdist2(square[i], square[(i+1)%4]));
  }
  return acc / 4.0;
}

float area(array<Point_t, 4> square) {
  int a = area2(std::vector<Point_t>(square.begin(), square.end()));
  return ((float)a)/2.0;
}

float length_c(array<Point_t, 4> s, array<Point_t, 4> b) {
  int min = INT_MAX;
  for(int i=0; i<4; i++) {
    Point_t p = s[i];
    for(int j=0; j<4; j++) {
      int d = pdist2(p, b[j]);
      if(d < min) {
        min = d;
      }
    }
  }
  return sqrt((float)min);
}

int length_c2(array<Point_t, 4> s, array<Point_t, 4> b) {
  int min = INT_MAX;
  int min2 = INT_MAX;
  int minc = -1;
  for(int i=0; i<4; i++) {
    Point_t p = s[i];
    for(int j=0; j<4; j++) {
      int d = pdist2(p, b[j]);
      if(d < min) {
        minc = j;
        min2 = min;
        min = d;
      } else if(d < min2) {
        min2 = d;
      }
    }
  }
  float af = sqrt((float)min);
  float bf = sqrt((float)min2);
  return af < bf*0.99 ? minc : -1;
}

int length_c3(array<Point_t, 4> s, array<Point_t, 4> b) {
  int min = INT_MAX;
  int minc = -1;
  for(int i=0; i<4; i++) {
    Point_t p = s[i];
    for(int j=0; j<4; j++) {
      int d = pdist2(p, b[j]);
      if(d < min) {
        minc = j;
        min = d;
      }
    }
  }
  return minc;
}

int my_mod(int a, int b) {
  int t = a%b;
  return t < 0 ? t+b : t;
}

bool tup_eq(tuple<int, int> a, tuple<int, int> b) {
  return my_mod(get<0>(a)-get<1>(a), 4) == my_mod(get<0>(b)-get<1>(b), 4);
}

float error(vector<tuple<array<Point_t, 4>, int> > squares, array<Point_t, 4> big_square, vector<array<Point_t, 4> > ir_squares, array<Point_t, 4> ir_big_square) {
  int size = squares.size();
  float acc = 0.0;
  float ir_length = length(ir_big_square);
  float ir_area = area(ir_big_square);
  float vis_length = length(big_square);
  float vis_area = area(big_square);
  for(int i=0; i<size; i++) {
    tuple<array<Point_t, 4>, int> tup = squares[i];
    array<Point_t, 4> square = get<0>(tup);
    int to_ir = get<1>(tup);
    array<Point_t, 4> ir_square = ir_squares[to_ir];
    float a = area(square)/vis_area - area(ir_square)/ir_area;
    a = a*a;
    float l = length_c(square, big_square)/vis_length - length_c(ir_square, ir_big_square)/ir_length;
    l = l*l;
    acc += a + l;
    // OPENCV DATABASE tout138.jpg
    // imgcat
  }
  return acc;
}

int find_min_error(vector<tuple<array<Point_t, 4>, int> > squares, array<Point_t, 4> big_square, vector<array<Point_t, 4> > ir_squares, array<Point_t, 4> ir_big_square, vector<int> test_squares) {
  int size = test_squares.size();
  float min_error = FLT_MAX;
  int ret = -1;
  float ir_length = length(ir_big_square);
  float ir_area = area(ir_big_square);
  float vis_length = length(big_square);
  float vis_area = area(big_square);
  for(int i=0; i<size; i++) {
    int s = test_squares[i];
    tuple<array<Point_t, 4>, int> tup = squares[s];
    array<Point_t, 4> square = get<0>(tup);
    int to_ir = get<1>(tup);
    array<Point_t, 4> ir_square = ir_squares[to_ir];
    float a = area(square)/vis_area - area(ir_square)/ir_area;
    a = a*a;
    float l = length_c(square, big_square)/vis_length - length_c(ir_square, ir_big_square)/ir_length;
    l = l*l;
    float error = a+l;
    if(error < min_error) {
      min_error = error;
      ret = i;
    }
  }
  return ret;
}

//array<Point_t, 8> main_logic(bool ir)
int main2(array<Point_t, 4> ir_big_square, vector<array<Point_t, 4> >ir_squares)
{
  printf("a\n");
  bool ir = false;
  int width, height, bpp;
  //uint8_t* imagergb = stbi_load(ir ? "ir.jpg" : "vis.jpg", &width, &height, &bpp, 3);
  uint8_t* imagergb = stbi_load(name_vis, &width, &height, &bpp, 3);
  stbi_image_free(imagergb);
  uint8_t* image8 = vis_con();
  printf("b\n");
  /*uint8_t* image8 = (uint8_t*)malloc(width*height);
  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      uint8_t* colorrgb = imagergb+(j*width+i)*3;
      uint8_t* color8 = image8+j*width+i;
      if(
         (ir && !(colorrgb[2] > 64 && colorrgb[0] < 224 && colorrgb[1] < 224)) ||
         (!ir && colorrgb[0] < 128 && colorrgb[1] < 128 && colorrgb[2] < 128)
         ){//96
        *color8 = 0xff;
      } else {
        *color8 = 0x00;
      }
    }
  }*/
  /*showrgbimage(width, height, imagergb, 10000);
  stbi_image_free(imagergb);
  show8image(width, height, image8, 10000);
  free(image8);
  return 0;
}
array<Point_t, 8> t2(uint8_t* image8, int width, int height, bool ir)
{*/
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
  printf("c\n");
  for(int i=0; i<width; i++) {
    if(image8[i]==0x3) {
      image8[i]=0x0;
    }
    if(image8[i+(height-1)*width]==0x3) {
      image8[i+(height-1)*width]=0x0;
    }
  }
  printf("d\n");
  for(int i=0; i<height; i++) {
    if(image8[i*width]==0x3) {
      image8[i*width]=0x0;
    }
    if(image8[i*width+(width-1)]==0x3) {
      image8[i*width+(width-1)]=0x0;
    }
  }
  printf("e\n");
  for(int j=1; j<height-1; j++) {
    for(int i=1; i<width-1; i++) {
      if(image8[j*width+i] == 0x3 && !(image8[j*width+i+1] && image8[j*width+i-1] && image8[(j+1)*width+i] && image8[(j-1)*width+i])) {
        image8[j*width+i] = 0x0;
      }
    }
  }
  printf("f\n");
  for(int j=height-2; j>0; j--) {
    for(int i=width-2; i>0; i--) {
      if(image8[j*width+i] == 0x3 && !(image8[j*width+i+1] && image8[j*width+i-1] && image8[(j+1)*width+i] && image8[(j-1)*width+i])) {
        image8[j*width+i] = 0x0;
      }
    }
  }
  printf("g\n");
  for(int j=1; j<height-1; j++) {
    for(int i=1; i<width-1; i++) {
      if(image8[j*width+i] == 0x3 && !(image8[j*width+i+1] && image8[j*width+i-1] && image8[(j+1)*width+i] && image8[(j-1)*width+i])) {
        myfill(image8, i, j, width, height);
      }
    }
  }
  printf("h\n");
  //show8image(width, height, image8, 20000);
  //free(image8);
  //return 0;
  for(int j=1; j<height-1; j++) {
    for(int i=1; i<width-1; i++) {
      if(image8[j*width+i] == 0x3) {
        int size = sizefill(image8, i, j, width, height);
        if(size < 128) {
          clearsizefill(image8, i, j, width, height);
        }
      }
    }
  }
  printf("i\n");
  //show8image(width, height, image8, 20000);
  //free(image8);
  //return 0;
  for(int j=1; j<height-1; j++) {
    for(int i=1; i<width-1; i++) {
      if(image8[j*width+i] == 0xe0 && image8[j*width+i+1] != 0x1c && image8[j*width+i-1] != 0x1c && image8[(j+1)*width+i] != 0x1c && image8[(j-1)*width+i] != 0x1c) {
        image8[j*width+i] = 0x0;
      }
    }
  }
  printf("j\n");
  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      if(image8[j*width+i] == 0xe0) {//0x1c
        image8[j*width+i] = 0xff;//0x00
      } else {
        image8[j*width+i] = 0x00;
      }
    }
  }
  printf("k\n");
  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      if(image8[j*width+i] == 0x00) {
        numberfill(image8, width, height, 0xfe, Point_t{i, j});
        vector<uint8_t> ne;
        for(int l=0; l<height; l++) {
          for(int k=0; k<width; k++) {
            if(image8[l*width+k] == 0xff) {
              vector<uint8_t> pm;
              if(!outside(width, height, Point_t{k-1, l})) {
                pm.push_back(image8[l*width+(k-1)]);
              }
              if(!outside(width, height, Point_t{k+1, l})) {
                pm.push_back(image8[l*width+(k+1)]);
              }
              if(!outside(width, height, Point_t{k, l-1})) {
                pm.push_back(image8[(l-1)*width+k]);
              }
              if(!outside(width, height, Point_t{k, l+1})) {
                pm.push_back(image8[(l+1)*width+k]);
              }
              if(exist(pm, 0xfe)) {
                int s = pm.size();
                for(int m=0; m<s; m++) {
                  if(pm[m] != 0xfe) {
                    if(!exist(ne, pm[m])) {
                      ne.push_back(pm[m]);
                    }
                  }
                }
              }
            }
          }
        }
        uint8_t acc = 1;
        while(exist(ne, acc)) {
          acc++;
          assert(acc != 0xfe);
        }
        numberfill(image8, width, height, acc, Point_t{i, j});
      }
    }
  }
  printf("l\n");
  for(int i=0; i<width; i++) {
    int p1 = i;
    int p2 = (height-1)*width+i;
    if(image8[p1] != 0x00 && image8[p1] != 0xff) {
      numberfill(image8, width, height, 0x00, Point_t{i, 0});
    }
    if(image8[p2] != 0x00 && image8[p2] != 0xff) {
      numberfill(image8, width, height, 0x00, Point_t{i, height-1});
    }
  }
  for(int i=0; i<height; i++) {
    int p1 = i*width;
    int p2 = i*width+(width-1);
    if(image8[p1] != 0x00 && image8[p1] != 0xff) {
      numberfill(image8, width, height, 0x00, Point_t{0, i});
    }
    if(image8[p2] != 0x00 && image8[p2] != 0xff) {
      numberfill(image8, width, height, 0x00, Point_t{width-1, i});
    }
  }
  //printf("%d\n", image8[3]);
  printf("m\n");
  /*for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      vector<uint8_t> pm;
      if(!outside(width, height, Point_t{i-1, j})) {
        pm.push_back(image8[j*width+(i-1)]);
      }
      if(!outside(width, height, Point_t{i+1, j})) {
        pm.push_back(image8[j*width+(i+1)]);
      }
      if(!outside(width, height, Point_t{i, j-1})) {
        pm.push_back(image8[(j-1)*width+i]);
      }
      if(!outside(width, height, Point_t{i, j+1})) {
        pm.push_back(image8[(j+1)*width+i]);
      }
      unique(pm.begin(), pm.end());
      if(pm.size() - ((int)exist(pm, 0xff)) <= 1) {
        image8[j*width+i] = 0x00;
      }
    }
  }
  printf("n\n");*/
  vector<array<Point_t, 4> > squares;
  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      if(image8[j*width+i] != 0xff && image8[j*width+i] != 0x00/* && image8[j*width+i] != 0xfe*/) {
        tuple<vector<Point_t>, int> a = clear_contoure(image8, width, height, Point_t{i, j});
        vector<Point_t> v = std::get<0>(a);
        int s = std::get<1>(a);
        if(s < 128 || v.size() < 64) {
          continue;
        }
        std::sort(v.begin(), v.end(), [](Point_t a, Point_t b) {
            return (a.y == b.y) ? (a.x < b.x) : (a.y < b.y);
          });
        //unique(v.begin(), v.end());
        Point_t start = v[0];
        vector<uint8_t> col = neightbors(image8, width, height, start);
        int num = exist_num(col, 0xff);
        if(num != -1) {
          col.erase(col.begin()+num);
        }
        std::sort(col.begin(), col.end());
        vector<Point_t> pixels = new_contor(image8, width, height, start, v, col);
        if(square(pixels, ir)) {
          assert(pixels.size() == 4);
          if(
             angle(pixels[1]-pixels[0], pixels[3]-pixels[0]) &&
             angle(pixels[2]-pixels[1], pixels[0]-pixels[1]) &&
             angle(pixels[3]-pixels[2], pixels[1]-pixels[2]) &&
             angle(pixels[0]-pixels[3], pixels[2]-pixels[3]))
            {
              array<Point_t, 4> cop;
              copy_n(pixels.begin(), 4, cop.begin());
              squares.push_back(cop);
            }
        }
      }
    }
  }
  free(image8);
  printf("n (%lu)\n", squares.size());
  {
    vector<array<Point_t, 4> > max;
    int size = squares.size();
    for(int i=0; i<size; i++) {
      vector<array<Point_t, 4> > t = squares_in_square(squares, squares[i]);
      if(t.size() > max.size()) {
        max = t;
      }
    }
    squares = max;
  }
  printf("o (%lu)\n", squares.size());
  if(squares.size() < 3) {
    printf("Failed to detect the marker\n");
    return 1;
  }
  array<Point_t, 4> big_square = square_logic(squares);
  /*show8image(width, height, image8, -1);
  {
    int size = squares.size();
    for(int i=0; i<size; i++) {
      char text[256];
      sprintf(text, "%d", i);
      for(int k=0; k<4; k++) {
        outtextxy(squares[i][k].x, squares[i][k].y, text);
      }
    }
  }
  delayandclose(20000);
  free(image8);*/
  vector<tuple<array<Point_t, 4>, int> > asignment;
  {
    int size = squares.size();
    int ir_size = ir_squares.size();
    for(int i=0; i<size; i++) {
      asignment.push_back(std::make_tuple(squares[i], i));
    }
    vector<tuple<array<Point_t, 4>, int> > min;
    float minf = FLT_MAX;
    do{
      //print_asignment(asignment);
      float test = error(asignment, big_square, ir_squares, ir_big_square);
      if(test < minf) {
        minf = test;
        min = asignment;
      }
    } while(!next_assignment(asignment, ir_size));
    /*for(int i=0; i<size; i++) {
      printf("%d -> %d\n", i, get<1>(min[i]));
      }*/
    vector<tuple<int, tuple<int, int> > > count;
    for(int i=0; i<size; i++) {
      int p = i;
      int d = length_c3(get<0>(min[p]), big_square);
      int q = get<1>(min[p]);
      int b = length_c3(ir_squares[q], ir_big_square);
      tuple<int, int> find = std::make_tuple(d, b);
      int cs = count.size();
      bool is_found = true;
      for(int j=0; j<cs; j++) {
        tuple<int, tuple<int, int> > e = count[j];
        int ei = get<0>(e);
        tuple<int, int> et = get<1>(e);
        if(tup_eq(et, find)) {
          ei++;
          get<0>(e) = ei;
          count[j] = e;
          is_found = false;
          break;
        }
      }
      if(is_found) {
        count.push_back(std::make_tuple(1, find));
      }
    }
    int max_in_count = INT_MIN;
    int max_count_in_count = 0;
    int max_i_in_count = -1;
    int cs = count.size();
    for(int i=0; i<cs; i++) {
      if(get<0>(count[i]) > max_in_count) {
        max_in_count = get<0>(count[i]);
        max_count_in_count = 1;
        max_i_in_count = i;
      } else if(get<0>(count[i]) == max_in_count) {
        max_count_in_count++;
      }
    }
    if(max_count_in_count == 1) {
      array_rotate(ir_big_square, get<1>(get<1>(count[max_i_in_count])));
      array_rotate(big_square, get<0>(get<1>(count[max_i_in_count])));
    } else {
      //printf("TODO, is not implemented\n");
      printf("Detected the marker but failed to detect the rotation of it\n A more robust implementation may fix this in the future\n");
      return 1;
    }
    /*
    vector<int> test_squares;
    for(int i=0; i<size; i++) {
      test_squares.push_back(i);
    }
    bool bre = true;
    while(test_squares.size()) {
      int p = find_min_error(min, big_square, ir_squares, ir_big_square, test_squares);
      test_squares.erase(test_squares.begin()+p);
      int d = length_c2(squares[p], big_square);
      if(d >= 0) {
        // p-d pair p=square d=big_square
        int q = get<1>(min[p]);
        int b = length_c3(ir_squares[q], ir_big_square);
        // q-b pair q=ir_square b=ir_big_square
        array_rotate(ir_big_square, b);
        array_rotate(big_square, d);
        bre = false;
        break;
      }
    }
    if(bre) {
      return;
      }*/
  }
  printf("p\n");
  //return;
  merge(big_square, ir_big_square);
  /*{
    for(int k=0; k<4; k++) {
      dot(image8, big_square[k].x, big_square[k].y, width, height, 0xe0+k, 5);
    }
    int size = squares.size();
    for(int i=0; i<size; i++) {
      for(int k=0; k<4; k++) {
        dot(image8, squares[i][k].x, squares[i][k].y, width, height, 0x1c+k, 5);
      }
    }
  }
  show8image(width, height, image8, 20000);*/
  return 0;
}
