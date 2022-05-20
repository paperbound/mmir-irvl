
// You the user of this interface is responsible to create and free instances of the image_t struct
typedef struct image{
  int width;
  int height;
  uint8_t* image; // should point to 'width'*'height'*3 bytes of memory
} image_t;

// The interface is responsible to create and free the data_t struct
typedef struct data{
  //stuff
} data_t;

// Will read from the image_t (assuming it is a ir image of the marker) and create a data_t for future use.
// Will read from img but not free it. (You are alowed to free the image_t struct if you want)
data_t evaluate_ir(image_t* img);

// Will update the (visible) image in image_t with data from the ir image.
// Will read and write to img, but not free it.
// Will read from data, but not free it. (You can reuse the data_t struct if you want)
void update_vis(image_t* img, data_t* data);

// Will free the data_t struct, please do not use it or any copy from it again.
void free_data(data_t data);
