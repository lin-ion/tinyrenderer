#include "geometry.h"
#include "tgaimage.h"
#include <algorithm>
#include <array>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
  bool steep = false;
  if (std::abs(x1 - x0) < std::abs(y1 - y0)) { // transpose
    std::swap(x0, y0);
    std::swap(x1, y1);
    steep = true;
  }

  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  int dx = x1 - x0;
  int dy = y1 - y0;
  float derror2 = std::abs(dy) * 2;
  float error2 = 0;
  int y = y0;
  for (int x = x0; x <= x1; x++) {
    if (steep) { // de-transpose
      image.set(y, x, color);
    } else {
      image.set(x, y, color);
    }

    error2 += derror2;
    if (error2 > dx) {
      y += (y1 > y0 ? 1 : -1);
      error2 -= dx * 2;
    } // else y is not change
  }
}

void line(Vec2i t0, Vec2i t1, TGAImage &image, TGAColor color) {
  line(t0.x, t0.y, t1.x, t1.y, image, color);
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
  std::array<Vec2i, 3> vertices = {t0, t1, t2};
  for (int i = 0; i < 3; i++) {
    line(vertices[i], vertices[(i + 1) % 3], image, color);
  }

  std::sort(vertices.begin(), vertices.end(),
            [](Vec2i a, Vec2i b) { return a.y < b.y; });

  Vec2i &p0 = vertices[0]; // bottom
  Vec2i &p1 = vertices[1]; // middle
  Vec2i &p2 = vertices[2]; // top

  for (int y = p0.y; y < p1.y; y++) {
    float a = (y - p0.y) / (float)(p1.y - p0.y);
    int x0 = p0.x + a * (p1.x - p0.x);

    float b = (y - p0.y) / (float)(p2.y - p0.y);
    int x1 = p0.x + b * (p2.x - p0.x);

    line(x0, y, x1, y, image, color);
  }

  for (int y = p1.y; y <= p2.y; y++) {
    float a = (y - p1.y) / (float)(p2.y - p1.y);
    int x0 = p1.x + a * (p2.x - p1.x);

    float b = (y - p0.y) / (float)(p2.y - p0.y);
    int x1 = p0.x + b * (p2.x - p0.x);

    line(x0, y, x1, y, image, color);
  }
}

int main(int argc, char **argv) {
  TGAImage image(200, 200, TGAImage::RGB);

  Vec2i t0[3] = {Vec2i(10, 70), Vec2i(50, 160), Vec2i(70, 80)};
  Vec2i t1[3] = {Vec2i(180, 50), Vec2i(150, 1), Vec2i(70, 180)};
  Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
  triangle(t0[0], t0[1], t0[2], image, red);
  triangle(t1[0], t1[1], t1[2], image, white);
  triangle(t2[0], t2[1], t2[2], image, green);

  image.flip_vertically();
  image.write_tga_file("output.tga");
  return 0;
}