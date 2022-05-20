
// You the user of this interface is responsible to create and free instances of the image_t struct
typedef struct image{
  int width;
  int height;
  uint8_t* image; // should point to 'width'*'height'*3 bytes of memory
} image_t;

// Assumes that image_a is the visible image, and that image_b is the ir image.
// Assumes that image_b has our marker in the image.
// image_a will be updated width the overlay derived from image_b.
// image_a don't have to have our marker in the image.
// This function will not free image_b.
void merge_images(image_t* image_a, image_t* image_b);
