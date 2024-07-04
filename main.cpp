#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

void line_parametric(const int x0, const int y0, const int x1, const int y1,
                     TGAImage &image, const TGAColor &color) {
  for (float t = 0.; t < 1.; t += .01) {
    const int x = x0 + t * (x1 - x0);
    const int y = y0 + t * (y1 - y0);
    image.set(x, y, color);
  }
}

int main(int argc, char **argv) {
  TGAImage image(100, 100, TGAImage::RGB);
  image.set(52, 41, red);
  line_parametric(13, 20, 80, 40, image, white);

  image.flip_vertically(); // i want to have the origin at the left bottom
                           // corner of the image
  image.write_tga_file("output.tga");
  return 0;
}