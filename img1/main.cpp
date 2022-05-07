#include <iostream>
#include <array>
#include <opencv/cv.h>

extern "C"
{
#include "extra.c"
}

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

bool square(vector<Point_t>& pixels)
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
    if(d > 5.0) {
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

void dot(uint8_t* img, int x, int y, int width, int height, uint8_t color)
{
  for(int j=0; j<7; j++) {
    for(int i=0; i<7; i++) {
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

int main()
{
  int width, height, bpp;
  bool ir = true;
  uint8_t* imagergb = stbi_load(ir ? "ir.jpg" : "vis.jpg", &width, &height, &bpp, 3);
  uint8_t* image8 = (uint8_t*)malloc(width*height);
  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      uint8_t* colorrgb = imagergb+(j*width+i)*3;
      uint8_t* color8 = image8+j*width+i;
      if((ir && colorrgb[2] > 96) || (!ir && colorrgb[0] < 128 && colorrgb[1] < 128 && colorrgb[2] < 128)) {//96
        *color8 = 0xff;
      } else {
        *color8 = 0x00;
      }
    }
  }
  showrgbimage(width, height, imagergb, 10000);
  stbi_image_free(imagergb);
  show8image(width, height, image8, 10000);
  free(image8);
  return 0;
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
  vector<vector<Point_t> > squares;
  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      if(image8[j*width+i] == 0xe0) {
        vector<Point_t> pixels;
        countorfill(image8, i, j, width, height, pixels);
        if(square(pixels)) {
          squares.push_back(pixels);
          /*for(int k=0; k<4; k++) {
            dot(image8, pixels[k].x-3, pixels[k].y-3, width, height);
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
  /*{
    int size = squares.size();
    for(int i=0; i<size; i++) {
      for(int k=0; k<4; k++) {
        dot(image8, squares[i][k].x-3, squares[i][k].y-3, width, height, 0xe0);
      }
    }
  }*/
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
  return 0;
}
