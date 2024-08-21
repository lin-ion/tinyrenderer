#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
  for (int x = x0; x <= x1; x++) {
    float t = (x - x0) / (float)(x1 - x0);
    int y = y0 * (1. - t) + y1 * t;
    image.set(x, y, color);
  }
}

int main(int argc, char **argv) {
  TGAImage image(100, 100, TGAImage::RGB);
  image.set(52, 41, red);
  line(13, 20, 80, 40, image, white);
  line(20, 13, 40, 80, image, red); // not drawn
  line(80, 40, 13, 20, image, red); // some points not drawn

  image.flip_vertically();
  image.write_tga_file("output.tga");
  return 0;
}